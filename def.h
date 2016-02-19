#ifndef DEF_H
#define DEF_H

#include <set>
#include <vector>
#include <valarray>
#include <string>
#include <list>
#include <utility>

#include <chrono>
#include <random>
#include <thread>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <ctime>

#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <ios>
#include <fstream>

#define CPP_11 1
#define MY_QT 1

#if MY_QT
#include <QtCore>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QtNetwork>
#include <QSerialPort>
#include <QSerialPortInfo>
#endif

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

}
namespace def
{
/// non-consts
extern eegDataType eegData; /// make ring-style container
extern int currentType;
extern QString currentName;
extern int currentMarker;

extern QStringList fileMarkers;

/// consts
const int eegNs = 19;
const int ns = 24;
const int eog1 = 22;
const int eog2 = 23;

const double freq = 250.;
const int fftLength = 1024;
const int timeShift = 125;
const int windowLength = 1024;

const double leftFreq = 5.;
const double rightFreq = 20.;

const QString workPath = "/media/Files/Data/RealTime/";
const QString spectraPath = workPath + "SpectraSmooth/windows";
const QString netLogPath = workPath + "log.txt";
const QString netResPath = workPath + "results.txt";
const QString netBadPath = workPath + "badFiles.txt";
const QString eyesFilePath = workPath + "eyes.txt";


//const bool withMarkersFlag = true; /// should check everywhere if changed to false

/// funcs
///
extern int right();
extern int left();
inline int numOfClasses() {return def::fileMarkers.length();}
inline int spLength() {return def::right() - def::left();}
inline double spStep() {return def::freq / def::fftLength;}
}

inline int fftLimit(const double & inFreq,
                    const double & sampleFreq = def::freq,
                    const int & fftL = def::fftLength)
{
    return ceil(inFreq / sampleFreq * fftL - 0.5);
}


#endif // DEF_H
