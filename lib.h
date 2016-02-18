#ifndef SMALLFUNCS_H
#define SMALLFUNCS_H

#include <set>
#include <vector>
#include <valarray>
#include <string>
#include <list>
#include <utility>

#include <algorithm>
#include <numeric>
#include <cmath>
#include <ctime>

#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <ios>
#include <fstream>

//#include <boost/filesystem.hpp>

#include "def.h"

#define CPP_11 1
#define MY_QT 1

#if MY_QT
#include <QtCore>
#include <QDir>
#include <QFile>
#endif


typedef std::vector<double> vectType;
typedef std::valarray<double> lineType;
template <typename Typ>
class trivector : public std::vector<std::vector<std::vector<Typ> > >
{};
template <typename Typ>
class twovector : public std::vector<std::vector<Typ> >
{};

const long double pi = 3.14159265358979323846L;

static inline std::vector<std::string> makeDefFilters()
{
    std::vector<std::string> res;
    res.push_back("_241");
    res.push_back("_247");
    res.push_back("_254");
    return res;
}

#if CPP_11
const std::vector<std::vector<std::string> > defaultFilters = {{"_241"}, {"_247"}, {"_254"}};
#else
//std::initializer_list<std::string> lst("_241", "_247", "_254");
//std::vector<std::string> defaultFilters = makeDefFilters();
#endif

bool fileExists(const QString & filePath);

std::string slash();
QString qslash();

inline double doubleRound(const double & in, const int & numSigns)
{
    return int(  ceil(in * pow(10., numSigns) - 0.5)  ) / pow(10., numSigns);
}

inline double doubleRound(const double & in)
{
    return doubleRound(in, 2 - floor(log10(in))); // 2 significant numbers
}

inline double doubleRoundFraq(const double & in, const int & denom)
{
    return ceil(in * denom - 0.5) / denom;
}

inline double gaussian(const double & x, const double & sigma = 1.) //N(0,1)
{
    return 1./(sigma * sqrt(2. * pi)) * exp(-x * x / (2. * sigma * sigma) );
}

inline double sigmoid(const double & x, const double & t = 10.)
{
    return 1. / ( 1. + exp(-x/t) );
}

inline std::valarray<double> logistic(const std::valarray<double> & in, double temp)
{
    return 1. / (1. + exp(-in / temp));
}

inline std::valarray<double> softmax(const std::valarray<double> & in, double temp = 0.)
{
    // -1 for bias
    double sum = 0.;
    for(int i = 0; i < in.size() - 1; ++i)
    {
        sum += exp(in[i]);
    }
    return exp(in) / sum; // dont care about the last
}

inline int fftL(const int & in)
{
    return pow(2., ceil(log2(in)));
}



inline double prod(const lineType & in1, const lineType & in2)
{
    return (in1 * in2).sum(); // very slow

//    return std::inner_product(begin(in1),
//                              end(in1),
//                              begin(in2),
//                              0.);
}

inline double normaSq(const lineType & in)
{
    return prod(in, in);
}
inline double mean(const lineType & arr)
{
    return arr.sum() / arr.size();
}

inline double variance(const lineType & arr)
{
    return normaSq(arr - mean(arr)) / arr.size();
}

inline double sigma(const lineType & arr)
{
    return sqrt(variance(arr));
}

inline double covariance(const lineType & arr1, const lineType & arr2)
{
    return prod(arr1 - mean(arr1), arr2 - mean(arr2));
}

inline double correlation(const lineType & arr1, const lineType & arr2)
{
    return covariance(arr1, arr2) / (sigma(arr1) * sigma(arr2));
}

inline double norma(const lineType & in)
{
    return sqrt(normaSq(in));
}

inline void normalize(lineType & in)
{
    in /= norma(in);
}

inline double distance(const lineType & in1,
                       const lineType & in2)
{
    if(in1.size() != in2.size())
    {
        std::cout << "distance: lineTypes of different size" << std::endl;
        return 0.; /// exception
    }
    return norma(in1 - in2);
}

template <typename signalType>
void readFileInLine(const std::string & filePath,
                    signalType & result);

template <typename T>
void eraseItems(std::vector<T> & inVect,
                const std::vector<int> & indices);

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

template <typename signalType = lineType, typename retType = lineType>
retType spectre(const signalType & data);



#endif // SMALLFUNCS_H
