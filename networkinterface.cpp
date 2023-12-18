#include "networkinterface.h"

NetworkInterface::NetworkInterface(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);

    qDebug() << udpSocket->bind(5453);

    connect(udpSocket, &QUdpSocket::readyRead, this, &NetworkInterface::receivePackage);


}

void NetworkInterface::sendDataSlot(const QString &data){
    QUdpSocket udpSocket;
    QHostAddress destinationAddress("192.168.100.25");
    quint16 destinationPort = 5454;

    QByteArray message;
    message.append(data);
    qint64 bytesSent = udpSocket.writeDatagram(message, destinationAddress, destinationPort);
    if (bytesSent == -1) {
        qDebug() << (QString("Error sending message: %1").arg(udpSocket.errorString()));
    } else {
        qDebug() << ("Message sent successfully!");
    }
}

void NetworkInterface::receivePackage(){
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress senderAddress;
        quint16 senderPort;

        // Receive the datagram
        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);
        qDebug() << datagram;
        qDebug() << "*_*";

        // Process the received data
        //ui->devices->append(QString("Received message from %1 on PORT %2: \n").arg(senderAddress.toString()).arg(senderPort));
        //ui.devices.append(datagram);
        emit receiveDataSignal(datagram);
    }



}



