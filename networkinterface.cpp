#include "networkinterface.h"

NetworkInterface::NetworkInterface(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
    qDebug() << udpSocket->bind(udpPortServer);
    connect(udpSocket, &QUdpSocket::readyRead, this, &NetworkInterface::receiveUdpPackage);

    tcpServer = new QTcpServer(this);
    // Connect signals/slots for handling new connections
    connect(tcpServer, &QTcpServer::newConnection, this, &NetworkInterface::incomingConnection);
    // Start listening on a specific port
    if (!tcpServer->listen(QHostAddress::Any, tcpPort)) {
        qDebug() << "Unable to start the TCP server:" << tcpServer->errorString();
    }


}
NetworkInterface::~NetworkInterface()
{
    qDebug() << "closing network interface";
    // Cleanup or resource release code goes here

    // Close and delete the UDP socket
    if (udpSocket) {
        udpSocket->close();
        delete udpSocket;
        udpSocket = nullptr;
    }

    for (const clientPi& client : clientsVector) {
        if (client.tcpSocket && client.tcpSocket->isOpen()) {
            client.tcpSocket->close();
            client.tcpSocket->deleteLater();
        }
    }


    if (tcpServer && tcpServer->isListening()) {
        tcpServer->close();
        tcpServer->deleteLater();
    }

}

void NetworkInterface::transmitUdpData(const QString &data, const QHostAddress &destinationAddress, quint16 destinationPort)
{
    QByteArray message;
    QByteArray startDelimiters;
    QByteArray lengthBytes;

    startDelimiters.append('\xEB');
    startDelimiters.append('\x90');

    // Convert the length of the data to two bytes
    quint16 dataLength = static_cast<quint16>(data.size());
    lengthBytes.append((dataLength >> 8) & 0xFF);
    lengthBytes.append(dataLength & 0xFF);

    // Append start delimiters and length bytes to the message
    message.append(startDelimiters);
    message.append(lengthBytes);

    // Append the actual data
    message.append(data.toUtf8());

    qint64 bytesSent = udpSocket->writeDatagram(message, destinationAddress, destinationPort);

    if (bytesSent == -1) {
        qDebug() << "Error sending message:" << udpSocket->errorString();
    } else {
        qDebug() << "Message sent successfully!";
    }
}

void NetworkInterface::receiveUdpPackage()
{
    static QMap<QString, QByteArray> incompletePackets;

    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress senderHostAddress;
        QString senderAddress;
        quint16 senderPort;

        // Receive the datagram
        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderHostAddress, &senderPort);
        senderAddress = senderHostAddress.toString();

        // Process the received data
        qDebug() << "Received datagram from" << senderAddress << "on port" << senderPort;

        // Check for the start delimiters
        if (datagram.startsWith('\xEB') && datagram.at(1) == '\x90') {
            // New packet started, clear incomplete packet and put in the new packet
            incompletePackets[senderAddress] = datagram;
        } else {
            // If incomplete packet is empty, discard incoming data
            if (!incompletePackets.contains(senderAddress)) {
                qDebug() << "Discarding incoming data as it doesn't start with EB 90 and no incomplete packet for IP:" << senderAddress;
            } else {
                // Append the data to the existing incomplete packet for this IP
                incompletePackets[senderAddress] += datagram;
            }
        }

        // Check if the packet is complete based on the length
        while (incompletePackets.contains(senderAddress) && !incompletePackets[senderAddress].isEmpty() &&
               incompletePackets[senderAddress].size() >= 4) {
            quint16 packetLength = (static_cast<quint16>(incompletePackets[senderAddress].at(2)) << 8) |
                                   static_cast<quint16>(incompletePackets[senderAddress].at(3));
            if (incompletePackets[senderAddress].size() >= (4 + packetLength)) {
                // Complete packet received, call parseUDP
                QByteArray completePacket = incompletePackets[senderAddress].mid(4, packetLength);
                parseUDP(completePacket, senderHostAddress);
                incompletePackets[senderAddress].remove(0, 4 + packetLength);  // Remove processed packet from the buffer
            } else {
                // Incomplete packet, break the loop
                break;
            }
        }
    }
}


void NetworkInterface::parseUDP(const QByteArray &packet, const QHostAddress &senderAddress)
{
    // Log the received message for now
    qDebug() << "Received complete packet:" << packet;
    qDebug() << "from: " << senderAddress;

    // Check if the packet has at least two bytes
    if (packet.size() >= 2) {
        // Check if the first byte is 0x01
        if (static_cast<quint8>(packet.at(0)) == 0x01) {
            // Extract the client ID from the second byte
            int clientId = static_cast<quint8>(packet.at(1));

            // Check if the client ID is within the valid range (1 to 127)
            if (clientId >= 1 && clientId <= 127) {
                // Check if the IP already exists with a different ID
                for (int i = 0; i < clientsVector.size(); ++i) {
                    const clientPi &client = clientsVector.at(i);
                    if (client.ipAddress == senderAddress.toString() && client.id != clientId) {
                        // Remove the original IP and ID combination
                        qDebug() << "Removing original client with ID:" << client.id << "and IP:" << client.ipAddress;
                        clientsVector.remove(i);
                        break;
                    }
                }

                // Check if the client ID is not already in the list
                bool clientExists = false;
                for (const clientPi &client : clientsVector) {
                    if (client.id == clientId) {
                        clientExists = true;
                        break;
                    }
                }

                // If the client ID is not in the list, add it
                if (!clientExists) {
                    clientPi newClient;
                    newClient.id = clientId;
                    newClient.ipAddress = senderAddress.toString();

                    clientsVector.append(newClient);

                    qDebug() << "Added new client with ID:" << clientId << "and IP:" << newClient.ipAddress;
                    QByteArray ackMessage;
                    ackMessage.append('\x02');
                    QHostAddress destinationAddress(newClient.ipAddress);

                    transmitUdpData(ackMessage,destinationAddress,udpPortClient);
                }
            } else {
                qDebug() << "Invalid client ID:" << clientId << ". Client ID must be between 1 and 127.";
            }
        }
    }

}
void NetworkInterface::startDiscoverySlot(){
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    qDebug() << "Network Interfaces (excluding loopback):";


    foreach (const QNetworkInterface& interface, interfaces) {
        // Check if the interface is not a loopback interface and is active
        if (!(interface.flags() & QNetworkInterface::IsLoopBack) &&
            (interface.flags() & QNetworkInterface::IsUp) &&
            (interface.flags() & QNetworkInterface::IsRunning)) {
            qDebug() << "Name:" << interface.name();
            qDebug() << "Hardware Address (MAC):" << interface.hardwareAddress();
            qDebug() << "IPv4 Addresses:";

            // Collect only the first three parts (network identifier) of IPv4 addresses
            foreach (const QNetworkAddressEntry& entry, interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    QString ipv4Address = entry.ip().toString();
                    QStringList parts = ipv4Address.split('.');
                    if (parts.size() >= 3) {
                        QString networkIdentifier = parts[0] + "." + parts[1] + "." + parts[2];
                        qDebug() << "  " << ipv4Address << " (Network Identifier: " << networkIdentifier << ")";
                        QString broadcastAddress = networkIdentifier + ".255";
                        QHostAddress destinationAddress(broadcastAddress);
                        QByteArray discoveryMessage;
                        discoveryMessage.append('\x01');
                        discoveryMessage.append('\x00');
                        transmitUdpData(discoveryMessage, destinationAddress, udpPortClient);


                    }
                }
            }
            QMessageBox::information(nullptr, "Scan Complete", "Finished device discovery");

            qDebug() << "-----------------------------";
        }
    }


}


/*******************TCP SETUP*****************************************/
void NetworkInterface::incomingConnection() {
    // Create a new QTcpSocket for the incoming connection
    QTcpSocket* clientSocket = tcpServer->nextPendingConnection();
    QString clientIpAddress = clientSocket->peerAddress().toString();  // Assuming ipAddress is stored as QString
    auto it = std::find_if(clientsVector.begin(), clientsVector.end(), [clientIpAddress](const clientPi& client) {
        return client.ipAddress == clientIpAddress;
    });
    if (it != clientsVector.end()) {
        it->tcpSocket = clientSocket;

        // Do additional setup or handle the connection as needed
        connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkInterface::onTcpReadyRead);
        connect(clientSocket, &QTcpSocket::disconnected, this, &NetworkInterface::onTcpDisconnected);

        qDebug() << "Client connected. ID: " << it->id << ", IP: " << it->ipAddress;
        emit deviceConnectedSignal(it->id);
    } else {
        // If the client is not found, clean up the socket
        qDebug() << "Client not recognized. Closing connection.";
        clientSocket->disconnectFromHost();
        clientSocket->deleteLater();
    }

}
void NetworkInterface::onTcpDisconnected() {
    // Handle disconnection and clean up resources
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        qDebug() << "Client disconnected. IP: " << socket->peerAddress().toString();

        // Find the client in the vector based on the disconnected socket
        auto it = std::find_if(clientsVector.begin(), clientsVector.end(), [socket](const clientPi& client) {
            return client.tcpSocket == socket;
        });

        // If the client is found, perform cleanup
        if (it != clientsVector.end()) {
            // Additional cleanup or handling logic can be added here

            // Remove the association between the socket and the client
            it->tcpSocket = nullptr;

            // Clean up the socket
            socket->deleteLater();
            emit deviceDisconnectedSignal(it->id);
        } else {
            // This might happen if the client was not found in the vector, handle accordingly
            qDebug() << "Warning: Client not found in the vector.";
        }
    }
}
void NetworkInterface::onTcpReadyRead() {
    // Handle data received from the sockets
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        QByteArray data = socket->readAll();

        // Find the client in the vector based on the connected socket
        auto it = std::find_if(clientsVector.begin(), clientsVector.end(), [socket](const clientPi& client) {
            return client.tcpSocket == socket;
        });

        // If the client is found, handle the received data
        if (it != clientsVector.end()) {
            // Additional data handling or processing logic can be added here
            qDebug() << "Data received from client ID " << it->id << ", IP " << socket->peerAddress().toString() << ": " << data;
            emit receiveTcpPacketSignal(it->id, data);
        } else {
            // This might happen if the client was not found in the vector, handle accordingly
            qDebug() << "Warning: Client not found in the vector.";
        }
    }
}

void NetworkInterface::SendTcpPacketSlot(int id, const QByteArray &packet) {
    auto it = std::find_if(clientsVector.begin(), clientsVector.end(), [id](const clientPi& client) {
        return client.id == id;
    });

    if (it != clientsVector.end()) {
        // Found the client with the specified id
        clientPi& client = *it;

        // Check if the client has a valid TCP socket
        if (client.tcpSocket && client.tcpSocket->isOpen()) {
            // Send the packet on the TCP socket
            client.tcpSocket->write(packet);
            client.tcpSocket->flush();
        } else {
            // Handle the case where the TCP socket is not valid or not open
            qDebug() << "Error: Invalid or closed TCP socket for client with id" << id;
            // You may want to perform additional error handling or logging here
        }
    } else {
        // Handle the case where the client with the specified id is not found
        qDebug() << "Error: Client with id" << id << "not found in clientsVector";
        // You may want to perform additional error handling or logging here
    }
}





