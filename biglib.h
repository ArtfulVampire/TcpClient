#ifndef BIGLIB_H
#define BIGLIB_H

#include "matrix.h"



#define TIME(arg)\
    do{\
        auto t0 = std::chrono::high_resolution_clock::now();\
        arg;\
        auto t1 = std::chrono::high_resolution_clock::now();\
        std::cout << funcName(#arg) \
        << ": time elapsed = "\
        << std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count()\
        << " mcsec" << std::endl;\
    } while(false)

std::string funcName(std::string in);

std::ostream & operator<< (std::ostream &os, QString toOut);

template <typename Typ, template <typename> class Cont>
std::ostream & operator<< (std::ostream &os, const Cont<Typ> & toOut);


template <typename Typ, template <typename, typename = std::allocator<Typ>> class Cont>
std::ostream & operator<< (std::ostream &os, const Cont<Typ> & toOut);

QString rightNumber(const unsigned int input, int N);


template <typename signalType>
void readFileInLine(const QString & filePath,
                    signalType & result);

template <typename signalType>
void writeFileInLine(const QString & filePath,
                     const signalType & outData);

//template <typename T>
//void eraseItems(std::vector<T> & inVect,
//                const std::vector<int> & indices);

std::vector<std::string> contents(const std::string & dirPath,
                                  const std::string & filter);
std::vector<std::vector<std::string> > contents(const std::string & dirPath,
                                  const std::vector<std::vector<std::string> > & filtersList);

std::vector<std::vector<std::string> > contents(const std::string & dirPath,
                                  const std::vector<std::string> & filtersList);
bool endsWith(const std::string & inStr,
              const std::string & filter);
bool contains(const std::string & inStr,
              const std::string & filter);
bool contains(const std::string & inStr,
              const std::vector<std::string> & filters);

int typeOfFileName(const std::string & filePath);
int typeOfFileName(const QString & fileName);

void myIota(std::vector<int> & in);
void myShuffle(std::vector<int> & in);
int myLess(int a, int b); // for sort in eraseItems

void four1(double * dataF, int nn, int isign);


void makeFullFileList(const QString & path,
                      QStringList & lst,
                      const QStringList & auxFilters = QStringList());

void makeFileLists(const QString & path,
                   std::vector<QStringList> & lst,
                   const QStringList & auxFilters = QStringList());

void readPlainData(const QString & inPath,
                   matrix & data,
                   int & numOfSlices,
                   const int & start = 0);

void writePlainData(const QString outPath,
                    const matrix & data,
                    int numOfSlices,
                    const int & start = 0);

void readMatrixFile(const QString & filePath,
                    matrix & outData);

void writeMatrixFile(const QString & filePath,
                     const matrix & outData,
                     const QString & rowsString = "NumOfRows",
                     const QString & colsString = "NumOfCols");

void fixFilesSlashN(const QString & path);

template <typename signalType = lineType, typename retType = lineType>
retType spectre(const signalType & data);

template <typename signalType = lineType>
void calcSpectre(const signalType & inSignal,
                 signalType & outSpectre,
                 const int & fftLength = def::fftLength,
                 const int & NumOfSmooth = 5.,
                 const int & Eyes = 0.,
                 const double & powArg = 1.);

template <typename Container>
int indexOfMax(const Container & cont);


void resizeValar(lineType & in, int num);


template <typename signalType = lineType>
signalType four2(const signalType & inRealData, int nn = def::fftLength, int isign = 1);

bool fileExists(const QString & filePath);

std::string readString(QDataStream & in);
std::string readString(QTcpSocket * inSocket);

template <typename Typ>
Typ readFromSocket(QTcpSocket * inSocket);

template <typename Typ>
Typ peekFromSocket(QTcpSocket * inSocket);

template <typename Typ>
std::bitset<8 * sizeof(Typ)> bits(Typ in);

std::string slash();
QString qslash();


#endif // BIGLIB_H
