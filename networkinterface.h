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


#include <QObject>

class NetworkInterface : public QObject
{
    Q_OBJECT
public:
    NetworkInterface(QObject *parent = nullptr);
    const int udpPort = 5453;
    const int udpPortAlt = 5454;
    const int tcpPort = 5464;
    struct clientPi {
        int id = 0;
        QString ipAddress = "";
        QTcpSocket* tcpSocket = nullptr;
    };
    QVector<clientPi> clientsVector;

    QUdpSocket *udpSocket;
    void parseUDP(const QByteArray &packet, const QHostAddress &senderAddress);

signals:
    void receiveDataSignal(const QByteArray &data);
    void networkDiscoveryCompleteSignal();

public slots:
    void sendDataSlot(const QString &data);
    void transmitUdpData(const QString &data, const QHostAddress &destinationAddress, quint16 destinationPort);
    void receiveUdpPackage();
    void startDiscoverySlot();
    void onDisconnected();
    void onReadyRead();
protected:
    void incomingConnection(qintptr socketDescriptor) override;

};

#endif // NETWORKINTERFACE_H
