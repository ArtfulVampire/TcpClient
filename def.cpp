#include "def.h"

std::string enc::readString(QDataStream &in)
{
    int numOfChars;
    in >> numOfChars;
    std::string res;
    res.resize(numOfChars);
    for(int i = 0; i < numOfChars; ++i)
    {
        in.readRawData(&(res[i]), 1);
    }
    return res;
}




namespace def
{
eegDataType eegData{}; /// make ring-style container OR NOT
int currentType = -1;
QString currentName = QString();
int currentMarker = 200;
//double freq = 250.;
//int fftLength = 1024;
//double leftFreq = 5.;
//double rightFreq = 20.;
//QStringList fileMarkers{"_241", "_247", "_254"};

//int left()  {return fftLimit(def::leftFreq);}
//int right() {return fftLimit(def::rightFreq) + 1;}
}
