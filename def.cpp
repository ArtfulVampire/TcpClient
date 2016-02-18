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


eegDataType enc::eegData{}; /// make ring-style container
int currentType = -1;
QString currentName = QString();
int currentMarker = 200;
