#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    networkInterfaceObj = new NetworkInterface(this);
    connect(this, &MainWindow::startDiscoverySignal, networkInterfaceObj, &NetworkInterface::startDiscoverySlot);
    connect(networkInterfaceObj, &NetworkInterface::deviceConnectedSignal, this, &MainWindow::deviceConnectedSlot);
    connect(networkInterfaceObj, &NetworkInterface::deviceDisconnectedSignal, this, &MainWindow::deviceDisconnectedSlot);
    connect(this, &MainWindow::sendTcpPacketSignal, networkInterfaceObj, &NetworkInterface::SendTcpPacketSlot);
    connect(networkInterfaceObj, &NetworkInterface::receiveTcpPacketSignal, this, &MainWindow::receiveDataSlot);


    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::receiveDataSlot(int id, const QByteArray &data){
    ui->rxLog->append(QString("%1: %2").arg(id).arg(QString(data)));
}


void MainWindow::on_discoveryButton_clicked()
{
    qDebug() << "discover";
    emit startDiscoverySignal();
}



void MainWindow::deviceConnectedSlot(int id){
    if (!connectedDevices.contains(id)) {
        // Add the ID to the vector
        connectedDevices.append(id);
    }else{
        QMessageBox::warning(nullptr, "Warning", "New connection ID already exisits");
    }
    updateDevicesText();
}
void MainWindow::deviceDisconnectedSlot(int id){
    int index = connectedDevices.indexOf(id);

    // Check if the ID is found in the vector
    if (index != -1) {
        // Remove the ID from the vector
        connectedDevices.remove(index);
    }else{
        QMessageBox::warning(nullptr, "Warning", "Removed ID did not exist");
    }
    updateDevicesText();
}
void MainWindow::updateDevicesText() {
    // Clear the existing content
    ui->devices->clear();

    // Populate the QTextEdit with device information
    for (int id : connectedDevices) {
        ui->devices->append(QString("device: %1").arg(id));
    }
}



void MainWindow::on_sendBtn_clicked()
{
    QString text = ui->textToSend->text();
    QByteArray packet = text.toUtf8();
    int id = ui->targetIdSpinBox->value();
    emit sendTcpPacketSignal(id, packet);
}

