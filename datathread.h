#ifndef DATATHREAD_H
#define DATATHREAD_H

#include <QThread>
#include <QtNetwork>
#include <chrono>
#include <thread>
#include <ios>
#include <iostream>
#include <fstream>

#include "def.h"


class DataThread : public QThread
{
    Q_OBJECT
public:
    DataThread();
    ~DataThread();

    DataThread(QTcpSocket * inSocket, bool fullData);
    void run();

private:
    QDataStream str;
    bool fullDataFlag;
    std::list<std::vector<short>> eegData;
};

#endif // DATATHREAD_H
