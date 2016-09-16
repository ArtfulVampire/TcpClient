#ifndef DATAREADER_H
#define DATAREADER_H

#include <QObject>
#include "def.h"
#include "biglib.h"

class DataReader : public QObject
{
    Q_OBJECT

public:
    DataReader(QObject * inParent = nullptr);
    ~DataReader();

   void readStartInfo();
   void startStopTransmisson();
   void dataSliceCame();
   void markerCame();
   void markerCameAVS();
   void sendStartRequest();


signals:
   void sliceReady(eegSliceType slice);
   void retranslateMessage(QString);

public slots:
   void receiveData();
   void socketConnectedSlot();
   void socketDisconnectedSlot();
   void socketErrorSlot();

private:

   bool fullDataFlag = true;

   double bitWeight{};
   double samplingRate{};
   int numOfChannels{};



   eegSliceType oneSlice;
   qint32 sliceNumberPrevious = 0;

   QBuffer * myBuffer;
   bool inProcess = false;

   QTcpSocket * socket = nullptr;
#if USE_DATA_STREAM
   QDataStream socketDataStream;
#endif
   char * tmpData = new char [10000]; /// for unknown incoming data
};



class DataReaderHandler : public QObject
{
    Q_OBJECT
public:
    DataReaderHandler();

    ~DataReaderHandler();

    void dealWithMarkers(const eegSliceType & slic, int & slicesCam);

protected:
   void timerEvent(QTimerEvent *event);

public slots:
    void startReadData();
    void stopReadData();

    void receiveSlice(eegSliceType slice);
    void retranslateMessageSlot(QString);

signals:
    void finishReadData(); /// optional?private:
    void dataSend(eegDataType::iterator, eegDataType::iterator);
    void retranslateMessage(QString);

private:
    DataReader * myReader;

};

#endif // DATAREADER_H
