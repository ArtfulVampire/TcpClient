#ifndef DATAREADER_H
#define DATAREADER_H

#include <QObject>
#include "def.h"
#include "biglib.h"

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
   void markerCame();
   void sendStartRequest();

protected:
   void timerEvent(QTimerEvent *event);

signals:
   void sliceReady(eegSliceType slice);
   void startStopSignal(int var);

public slots:
   void receiveData();

private:

   bool fullDataFlag = true;

   double bitWeight{};
   double samplingRate{};
   int numOfChannels{};

#if USE_DATA_STREAM
   QDataStream socketDataStream;
#endif

   eegSliceType oneSlice;
   qint32 sliceNumberPrevious = 0;

   QBuffer * myBuffer;
   bool inProcess = false;

   QTcpSocket * socket = nullptr;
};

class DataReaderHandler : public QObject
{
    Q_OBJECT
public:
    DataReaderHandler(QTcpSocket * inSocket = nullptr,
                      bool inFullDataFlag = true);
    ~DataReaderHandler();

    void dealWithMarkers(const eegSliceType & slic, int & slicesCam);

protected:
   void timerEvent(QTimerEvent *event);

public slots:
    void startReadData();
    void stopReadData();

    void receiveSlice(eegSliceType slice);
    void startStopSlot(int var);

signals:
    void finishReadData(); /// optional?private:
    void startStopSignal(int var);
    void dataSend(eegDataType::iterator, eegDataType::iterator);

private:
    bool fullDataFlag = true;
    QTcpSocket * socket = nullptr;

    DataReader * myReader;

};

#endif // DATAREADER_H
