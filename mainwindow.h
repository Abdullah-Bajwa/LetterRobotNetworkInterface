#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QVector>
#include <QMessageBox>
#include <QString>
#include "networkinterface.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void startDiscoverySignal();
    void sendTcpPacketSignal(int id, const QByteArray &packet);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_discoveryButton_clicked();

    void on_sendBtn_clicked();

public slots:
    void receiveDataSlot(int id, const QByteArray &data);
    void deviceConnectedSlot(int id);
    void deviceDisconnectedSlot(int id);

private:
    Ui::MainWindow *ui;
    NetworkInterface *networkInterfaceObj;
    QVector<int> connectedDevices;
    void updateDevicesText();

};
#endif // MAINWINDOW_H
