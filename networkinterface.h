#ifndef NETWORKINTERFACE_H
#define NETWORKINTERFACE_H
#include <QUdpSocket>
#include <QHostAddress>


#include <QObject>

class NetworkInterface : public QObject
{
    Q_OBJECT
public:
    NetworkInterface(QObject *parent = nullptr);
    QUdpSocket *udpSocket;
signals:
    void receiveDataSignal(QByteArray &data);

public slots:
    void sendDataSlot(const QString &data);
    void receivePackage();

};

#endif // NETWORKINTERFACE_H
