#ifndef DATAREADER_H
#define DATAREADER_H

#include <QObject>
#include "def.h"

class DataReader : public QObject
{
    Q_OBJECT

public:
    DataReader(QObject * inParent = nullptr,
               QTcpSocket * inSocket = nullptr,
               bool inFullDataFlag = true);
    ~DataReader();

   void readStartInfo();
   void startStopTransmisson();
   void dataSliceCame();
   void sendStartRequest();

signals:
   void sliceReady(std::vector<short> slice);
   void startStopSignal(int var);

public slots:
   void receiveData();

private:

   bool fullDataFlag = true;

   double bitWeight{};
   double samplingRate{};
   int numOfChannels{};

   QDataStream socketDataStream;    bool inProcess = true;

   QTcpSocket * socket = nullptr;
};

class DataReaderHandler : public QObject
{
    Q_OBJECT
public:
    DataReaderHandler(QObject * inParent = nullptr,
                      QTcpSocket * inSocket = nullptr,
                      bool inFullDataFlag = true);
    ~DataReaderHandler();
private:
    DataReader * myReader;

public slots:
    void startReadData();
    void stopReadData();

    void receiveSlice(const std::vector<short> & slice);
//    void startStopSlot(int var);

signals:
    void finishReadData(); /// optional?private:

private:
    bool fullDataFlag = true;
    QTcpSocket * socket = nullptr;
    std::list<std::vector<short>> eegData; /// make ring-style container


};

#endif // DATAREADER_H
