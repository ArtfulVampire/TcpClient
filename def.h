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

struct Pack
{
    int packSize = 0;
    DWORD packId = 0;
    QByteArray packArr{};
};

}

#endif // DEF_H
