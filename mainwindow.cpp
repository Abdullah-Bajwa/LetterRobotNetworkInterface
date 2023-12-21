#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    networkInterfaceObj = new NetworkInterface(this);
    connect(this, MainWindow::sendDataSignal, networkInterfaceObj, NetworkInterface::sendDataSlot);
    connect(networkInterfaceObj, &NetworkInterface::receiveDataSignal, this, &MainWindow::receiveDataSlot);
    connect(this, &MainWindow::startDiscoverySignal, networkInterfaceObj, &NetworkInterface::startDiscoverySlot);
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_sendButton_clicked()
{
    emit sendDataSignal("hello");
}

void MainWindow::receiveDataSlot(const QByteArray &data){
    ui->devices->append(QString("Received Packet: \n"));
    ui->devices->append(data);
}


void MainWindow::on_discoveryButton_clicked()
{
    qDebug() << "discover";
    emit startDiscoverySignal();
}


void MainWindow::on_devicesButton_clicked()
{
    ui->devices->clear();
    foreach (const NetworkInterface::clientPi &client, networkInterfaceObj->clientsVector) {
        QString clientInfo = QString("ID: %1, IP: %2").arg(client.id).arg(client.ipAddress);
        ui->devices->append(clientInfo);
    }
}

