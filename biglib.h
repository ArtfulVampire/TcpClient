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
        << std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count()/1000.\
        << " msec" << std::endl;\
    } while(false)

std::string funcName(std::string in);

std::ostream & operator<< (std::ostream &os, QString toOut);


template <typename signalType>
void readFileInLine(const std::string & filePath,
                    signalType & result);

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

void myIota(std::vector<int> & in);
void myShuffle(std::vector<int> & in);
int myLess(int a, int b); // for sort in eraseItems

void four1(double * dataF, int nn, int isign);

void readMatrixFile(const QString & filePath,
                     matrix & outData,
                     int rows,
                     int cols);

void writeMatrixFile(const QString & filePath,
                      const matrix & outData,
                      int rows,
                      int cols);

void readPlainData(const QString & inPath,
                   matrix & data,
                   const int & ns,
                   int & numOfSlices,
                   const int & start = 0);

void writePlainData(const QString outPath,
                    const matrix &data,
                    const int & ns,
                    int numOfSlices,
                    const int & start = 0);

template <typename signalType = lineType, typename retType = lineType>
retType spectre(const signalType & data);

template <typename signalType = lineType>
void calcSpectre(const signalType & inSignal,
                 signalType & outSpectre,
                 const int & fftLength = def::fftLength,
                 const int & NumOfSmooth = 5.,
                 const int & Eyes = 0.,
                 const double & powArg = 1.);


template <typename signalType = lineType>
signalType four2(const signalType & inRealData, int nn = def::fftLength, int isign = 1);


bool fileExists(const QString & filePath);

std::string slash();
QString qslash();
#endif // BIGLIB_H
