#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>



#include "def.h"
#include "datareader.h"
#include "classifier.h"
#include "biglib.h"


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

#if COM_IN_MAIN
    void serialPortErrorSlot(QSerialPort::SerialPortError);
    void sendOne();
    void sendTwo();
    void comPortSlot();
#endif
#if SOCKET_IN_MAIN
    void socketErrorSlot(QAbstractSocket::SocketError);

    void socketConnectedSlot();
    void socketDisconnectedSlot();
    void connectSlot();
    void disconnectSlot();

#endif


    // ui slots
    void serverAddressSlot(int a);
    void startSlot();
    void endSlot(); /// is needed?

    void sendOne();
    void sendTwo();

    void retranslateMessageSlot(QString); /// from dataReader


private:
    Ui::MainWindow * ui;
#if SOCKET_IN_MAIN
    QTcpSocket * socket = nullptr;
#endif

#if COM_IN_MAIN
    QSerialPort * comPort = nullptr;
    QDataStream comPortDataStream{};
#endif

    QThread * myDataThread = nullptr;
    DataReaderHandler * myDataReaderHandler = nullptr;

    QThread * myNetThread = nullptr;
    NetHandler * myNetHandler = nullptr;


    QDataStream comPortDataStream;


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
