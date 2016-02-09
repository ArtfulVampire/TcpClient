#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <iostream>
#include <utility>
#include <fstream>
#include <ios>

#include <valarray>
#include <list>
#include <chrono>
#include <thread>

#include "def.h"
using namespace enc;
using namespace std;
using namespace std::chrono;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void readStartInfo(QDataStream & str);
    void startStopTransmisson(QDataStream & str);
    void dataSliceCame(QDataStream & str);

signals:
public slots:
    void react();
    void askServer();

    void changeFullDataFlag(int);

    void socketErrorSlot(QAbstractSocket::SocketError);
    void serialPortErrorSlot(QSerialPort::SerialPortError);
    void connectSlot();
    void disconnectSlot();
    void socketConnectedSlot();
    void socketDisconnectedSlot();

    void serverAddressSlot(int a);
    void comPortSlot();

    void startSlot();
    void receiveDataSlot();
    void endSlot();

private:
    Ui::MainWindow *ui;

    bool inProcess = false;
    bool fullDataFlag = false;
    QTcpSocket * socket = nullptr;
    quint16 blockSize = 0;

    QSerialPort * comPort;

    //
    double bitWeight;
    double samplingRate;
    int numOfChannels;
    std::list<std::vector<short>> eegData{};
};

#endif // MAINWINDOW_H
