#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <ios>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <valarray>
#include <set>
#include <list>
#include <algorithm>
#include <chrono>
#include <random>
#include <thread>
#include <utility>


#include "def.h"
#include "datathread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void readStartInfo();
    void startStopTransmisson();
    void dataSliceCame();

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
    void dataReadThread();

private:
    Ui::MainWindow *ui;

    bool inProcess = false;
    bool fullDataFlag = false;
    QTcpSocket * socket = nullptr;
    QDataStream socketDataStream;
    quint16 blockSize = 0;

    std::thread dataThread{};
    QSerialPort * comPort = nullptr;

    //
    double bitWeight{};
    double samplingRate{};
    int numOfChannels{};
    std::list<std::vector<short>> eegData{};
};

#endif // MAINWINDOW_H
