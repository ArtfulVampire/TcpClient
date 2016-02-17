#ifndef DEF_H
#define DEF_H

#include <QByteArray>

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

namespace enc
{
typedef quint32 DWORD;
typedef qint32 Int;
typedef qint16 Short;
typedef quint8 BYTE;
typedef double Double;

struct String
{
    int numChars;
    char * str;
};


std::string readString(QDataStream & in);


struct Pack
{
    int packSize = 0;
    DWORD packId = 0;
    QByteArray packData;
};

}

#endif // DEF_H
