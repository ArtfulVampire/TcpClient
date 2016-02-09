#include "datathread.h"

DataThread::DataThread()
{
}

DataThread::~DataThread()
{
}

DataThread::DataThread(QTcpSocket * inSocket)
{
    str.setDevice(inSocket);
    str.setByteOrder(QDataStream::LittleEndian); // least significant bytes first
}
