#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    networkInterfaceObj = new NetworkInterface(this);
    connect(this, MainWindow::sendDataSignal, networkInterfaceObj, NetworkInterface::sendDataSlot);
    connect(networkInterfaceObj, &NetworkInterface::receiveDataSignal, this, &MainWindow::receiveDataSlot);
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

void MainWindow::receiveDataSlot(QByteArray &data){
    ui->devices->append(QString("Received Packet: \n"));
    ui->devices->append(data);
}

