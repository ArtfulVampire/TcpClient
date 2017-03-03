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

#include <QtCore>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QtNetwork>
#include <QSerialPort>
#include <QSerialPortInfo>

#define OFFLINE_SUCCESSIVE 0
/// 0 1 2 3
#define VERBOSE_OUTPUT 1





//typedef quint8 markerType; /// online
typedef quint32 markerType; /// offline


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

} /// end of namespace enc



namespace def
{
enum class errorNetType {SME, maxDist};
enum class solveType {notYet, right, wrong};

/// non-consts
extern eegDataType eegData;			/// should make ring-style container
extern int currentType;
extern int slicesCame;				/// counter for slices in a current task/rest
extern markerType currentMarker;
extern QHostAddress hostAddress;
extern int hostPort;
extern bool fullDataFlag;
extern QString comPortName;
extern int numOfReal;				/// number of a current real (starts from 2)
extern int numOfWind;				/// number of a current window
extern bool pauseFlag;				/// to detect presentation pause signal
extern int ns;						/// number of all transmissed channels
extern solveType solved;

/// consts
constexpr int eegNs = 19;
constexpr int markerChannel = 22;
constexpr int eog1 = 22;
constexpr int eog2 = 23;
constexpr int numOfSmooth = 5;
const QSet<int> dropChannels{1, 2, // Fp1, Fp2
							 3, 7, // F7, F8
							 8, 12 // T3, T4
							}; /// from 1
const QString rightKey = QObject::tr("z");
const QString wrongKey = QObject::tr("x");

const errorNetType errType = errorNetType::SME; /// how to calculate

constexpr double errorThreshold = 0.9;

const std::vector<qint16> markers{241, 247, 254};
const QStringList fileMarkers{"_241", "_247", "_254"}; /// needed?

constexpr double freq = 250.;
constexpr int fftLength = 1024;
constexpr int timeShift = 125; /// should be lower
constexpr int windowLength = 1024;

constexpr double leftFreq = 5.;
constexpr double rightFreq = 20.;

/// make GUI for these variables ?
const QString ExpName = "XXX_feedback";
#if 01
const QString workPath = "/media/Files/Data/RealTime";
#else
const QString workPath = "D:/MichaelAtanov/workData";
#endif

/// to read
const QString spectraPath = workPath + "/SpectraSmooth/winds";
const QString eyesFilePath = workPath + "/eyes.txt";

/// to write
const QString netLogPath = workPath + "/log.txt";
const QString netResPath = workPath + "/results.txt";
const QString netBadPath = workPath + "/badFiles.txt";


/// funcs
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
