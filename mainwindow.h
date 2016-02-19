#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>



#include "def.h"
#include "datareader.h"
#include "classifier.h"
#include "biglib.h"

#define DATA_READER 1

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();



signals:
public slots:

    void socketErrorSlot(QAbstractSocket::SocketError);
    void serialPortErrorSlot(QSerialPort::SerialPortError);

    void socketConnectedSlot();
    void socketDisconnectedSlot();

    void sendOne();
    void sendTwo();

    // ui slots
    void connectSlot();
    void disconnectSlot();
    void serverAddressSlot(int a);
    void startSlot();
    void endSlot(); /// is needed?
    void comPortSlot();

    void startStopSlot(int var); /// from dataReader

private:
    Ui::MainWindow * ui;
    QTcpSocket * socket = nullptr;
    QSerialPort * comPort = nullptr;
    QDataStream comPortDataStream{};

    QThread * myDataThread = nullptr;
    DataReaderHandler * myDataReaderHandler = nullptr;

    QThread * myNetThread = nullptr;
    NetHandler * myNetHandler = nullptr;



    /// moved to DataReader
#if !DATA_READER
private:

    QDataStream socketDataStream;

    bool inProcess = false;
    bool fullDataFlag = false;

    double bitWeight{};
    double samplingRate{};
    int numOfChannels{};
    std::list<std::vector<short>> eegData{};

public slots:
    void receiveDataSlot();

public:
    void readStartInfo();
    void startStopTransmisson();
    void dataSliceCame();
#endif
};

#endif // MAINWINDOW_H
