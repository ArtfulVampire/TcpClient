#include "def.h"

namespace def
{
eegDataType eegData{}; /// make ring-style container OR NOT
int currentType = -1;
markerType currentMarker = 200;
int slicesCame = 0;
QHostAddress hostAddress("127.0.0.1");
int hostPort{120};
bool fullDataFlag{true};
QString comPortName{"COM5"};
int numOfReal = 0;
int numOfWind = 0;
bool pauseFlag = 0;
int ns = 24;
solveType solved = solveType::notYet;
}
