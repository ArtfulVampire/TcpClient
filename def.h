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

template <typename T> class eegContType : public std::list<T>{}; /// Type Of Container
typedef std::vector<qint16> eegSliceType;
typedef eegContType<eegSliceType> eegDataType;

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

/// non-consts
extern eegDataType eegData; /// make ring-style container
extern int currentType;
extern QString currentName;
extern int currentMarker;

/// consts
const int ns = 19;
const int timeShift = 125;
const int windowLength = 1000;
const QString spectraPath = "/media/Files/Data/RealTime/SpectraSmooth/windows";
const QString netLogPath = "/media/Files/Data/RealTime/log.txt";
const QString netResPath = "/media/Files/Data/RealTime/results.txt";
const QString netBadPath = "/media/Files/Data/RealTime/badFiles.txt";

}

#endif // DEF_H
