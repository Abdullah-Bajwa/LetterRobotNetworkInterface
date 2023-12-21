#ifndef NETWORKINTERFACE_H
#define NETWORKINTERFACE_H
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTimer>
#include <QMap>
#include <QString>
#include <QVector>
#include <QList>
#include <QMessageBox>


#include <QObject>

class NetworkInterface : public QObject
{
    Q_OBJECT
public:
    NetworkInterface(QObject *parent = nullptr);
    ~NetworkInterface();

private:
    const int udpPortServer = 5454;
    const int udpPortClient = 5454;
    const int tcpPort = 5464;
    struct clientPi {
        int id = 0;
        QString ipAddress = "";
        QTcpSocket* tcpSocket = nullptr;
    };
    QVector<clientPi> clientsVector;
    QUdpSocket *udpSocket;
    QTcpServer* tcpServer;
    void transmitUdpData(const QString &data, const QHostAddress &destinationAddress, quint16 destinationPort);
    void parseUDP(const QByteArray &packet, const QHostAddress &senderAddress);

private slots:
    void receiveUdpPackage();
    void incomingConnection();
    void onTcpDisconnected();
    void onTcpReadyRead();


public slots:
    void startDiscoverySlot();
    void SendTcpPacketSlot(int id, const QByteArray &packet);

signals:
    void receiveTcpPacketSignal(int id,const QByteArray &data);
    void deviceConnectedSignal(int id);
    void deviceDisconnectedSignal(int id);

};

#endif // NETWORKINTERFACE_H
