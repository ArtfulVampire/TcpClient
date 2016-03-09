#include "def.h"

namespace def
{
eegDataType eegData{}; /// make ring-style container OR NOT
int currentType = -1;
QString currentName = QString();
markerType currentMarker = 200;
int slicesCame = 0;
QHostAddress hostAddress("127.0.0.1");
int hostPort{120};
bool fullDataFlag{true};
QString comPortName{"COM5"};

//double freq = 250.;
//int fftLength = 1024;
//double leftFreq = 5.;
//double rightFreq = 20.;
//QStringList fileMarkers{"_241", "_247", "_254"};

//int left()  {return fftLimit(def::leftFreq);}
//int right() {return fftLimit(def::rightFreq) + 1;}
}
