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

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sendButton_clicked();
public slots:
    void receiveDataSlot(QByteArray &data);

private:
    Ui::MainWindow *ui;
    NetworkInterface *networkInterfaceObj;

};
#endif // MAINWINDOW_H
