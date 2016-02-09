#ifndef DEF_H
#define DEF_H

#include <QByteArray>

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

//static const DWORD buttonChecked = 0x0001;
//static const DWORD buttonDisabled = 0x0002;

struct Pack
{
    int packSize = 0;
    DWORD packId = 0;
    QByteArray packArr{};
//    char * packData = nullptr;
};

}

#endif // DEF_H
