#ifndef DATATHREAD_H
#define DATATHREAD_H

#include <QThread>
#include <QtNetwork>

class DataThread : public QThread
{
public:
    DataThread();
    ~DataThread();

    DataThread(QTcpSocket * inSocket);

private:
    QDataStream str;
};

#endif // DATATHREAD_H
