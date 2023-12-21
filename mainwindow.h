#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include "networkinterface.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void sendDataSignal(const QString &data);
    void startDiscoverySignal();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sendButton_clicked();
    void on_discoveryButton_clicked();

    void on_devicesButton_clicked();

public slots:
    void receiveDataSlot(const QByteArray &data);

private:
    Ui::MainWindow *ui;
    NetworkInterface *networkInterfaceObj;

};
#endif // MAINWINDOW_H
