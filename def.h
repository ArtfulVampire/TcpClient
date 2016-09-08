#ifndef DEF_H
#define DEF_H

#include <set>
#include <vector>
#include <valarray>
#include <string>
#include <list>
#include <utility>
#include <bitset>

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





/// if one changes one of these - certain death
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

#define CPP_11 1
#define USE_DATA_STREAM 1
#define SOCKET_IN_MAIN 0
#define COM_IN_MAIN 0
#define DATA_READER 1

/// but one can change these
#define MY_LINDROWS 0
#define OFFLINE_SUCCESSIVE 0
#define VERBOSE_OUTPUT 1





//typedef quint8 markerType; /// online
typedef quint32 markerType; /// offline

enum class errorNetType {SME, maxDist};

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

struct Pack
{
    Int packSize = 0;
    DWORD packId = 0;
    QByteArray packData;
};

}


namespace def
{
/// non-consts
extern eegDataType eegData; /// make ring-style container
extern int currentType;
extern int slicesCame;
extern QString currentName;
extern markerType currentMarker;
extern QHostAddress hostAddress;
extern int hostPort;
extern bool fullDataFlag;
extern QString comPortName;
extern int numOfReal;
extern int numOfWind;

/// consts
constexpr int eegNs = 19;
constexpr int ns = 24;
constexpr int markerChannel = 22;
constexpr int eog1 = 22;
constexpr int eog2 = 23;
const QSet<int> dropChannels{};
//const QSet<int> dropChannels{7, 11};
const errorNetType errType = errorNetType::SME; /// how to calculate
constexpr double errorThreshold = 0.9;

const std::vector<qint16> markers{241, 247, 254};
const QStringList fileMarkers{"_241", "_247", "_254"}; /// needed?

constexpr double freq = 250.;
constexpr int fftLength = 1024;
constexpr int timeShift = 125;
constexpr int windowLength = 1024;

constexpr double leftFreq = 5.;
constexpr double rightFreq = 20.;

/// make GUI for these variables
const QString ExpName = "AAU_test";
#if MY_LINDROWS
const QString workPath = "/media/Files/Data/RealTime/";
#elif 0
const QString workPath = "D:/MichaelAtanov/workData/";
#else
const QString workPath = "C:/RealTime/workData/";
#endif
/// to read
const QString spectraPath = workPath + "SpectraSmooth/windows";
const QString eyesFilePath = workPath + "eyes.txt";
/// to write
const QString netLogPath = workPath + "log.txt";
const QString netResPath = workPath + "results.txt";
const QString netBadPath = workPath + "badFiles.txt";


/// funcs (ORDER is important, or extern+cpp)
constexpr int fftLimit(const double & inFreq)
{
    return ceil(inFreq / def::freq * def::fftLength - 0.5);
}
inline int numOfClasses() {return def::fileMarkers.length();}
constexpr double spStep() {return def::freq / def::fftLength;}
constexpr int left()  {return fftLimit(def::leftFreq);}
constexpr int right() {return fftLimit(def::rightFreq) + 1;}
constexpr int spLength() {return (def::right() - def::left());}
}


#endif // DEF_H
