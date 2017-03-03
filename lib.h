#ifndef SMALLFUNCS_H
#define SMALLFUNCS_H

#include "def.h"


template <typename Typ>
class trivector : public std::vector<std::vector<std::vector<Typ> > >
{};
template <typename Typ>
class twovector : public std::vector<std::vector<Typ> >
{};

const long double pi = 3.14159265358979323846L;

static inline std::vector<QString> makeDefFilters()
{
	std::vector<QString> res;
    res.push_back("_241");
    res.push_back("_247");
    res.push_back("_254");
    return res;
}

const std::vector<std::vector<QString>> defaultFilters = {{"_241"}, {"_247"}, {"_254"}};

inline double doubleRound(const double & in, const int & numSigns)
{
    return std::round(in * pow(10., numSigns)) / pow(10., numSigns);
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
    double sum = std::accumulate(std::begin(in),
                                 std::end(in) - 1, /// bias
                                 0.,
                                 [](double a, double b){return a + exp(b);});
    return exp(in) / sum; // dont care about the last (bias)
}

inline int fftL(const int & in)
{
    return pow(2., ceil(log2(in)));
}



inline double prod(const std::valarray<double> & in1, const std::valarray<double> & in2)
{
//    return (in1 * in2).sum(); // very slow

    return std::inner_product(begin(in1),
                              end(in1),
                              begin(in2),
                              0.);
}

inline double normaSq(const std::valarray<double> & in)
{
    return prod(in, in);
}
inline double mean(const std::valarray<double> & arr)
{
    return arr.sum() / arr.size();
}

inline double variance(const std::valarray<double> & arr)
{
    return normaSq(arr - mean(arr)) / arr.size();
}

inline double sigma(const std::valarray<double> & arr)
{
    return sqrt(variance(arr));
}

inline double covariance(const std::valarray<double> & arr1, const std::valarray<double> & arr2)
{
    return prod(arr1 - mean(arr1), arr2 - mean(arr2));
}

inline double correlation(const std::valarray<double> & arr1, const std::valarray<double> & arr2)
{
    return covariance(arr1, arr2) / (sigma(arr1) * sigma(arr2));
}

inline double norma(const std::valarray<double> & in)
{
    return sqrt(normaSq(in));
}

inline void normalize(std::valarray<double> & in)
{
    in /= norma(in);
}

inline double distance(const std::valarray<double> & in1,
					   const std::valarray<double> & in2)
{
    if(in1.size() != in2.size())
    {
		std::cout << "distance: std::valarray<double>s of different size" << std::endl;
        return 0.; /// exception
    }
    return norma(in1 - in2);
}
template <typename T>
void eraseItems(std::vector<T> & inVect,
                const std::vector<int> & indices)
{
    const int initSize = inVect.size();
    std::set<int, std::less<int> > excludeSet; // less first
    for(auto item : indices)
    {

        excludeSet.emplace(item);
    }
    std::vector<int> excludeVector;
    for(auto a : excludeSet)
    {
        excludeVector.push_back(a);
    }
    excludeVector.push_back(initSize); // for the last elements' shift

    for(int i = 0; i < excludeVector.size() - 1; ++i)
    {
        for(int j = excludeVector[i] - i; j < excludeVector[i + 1] - i - 1; ++j)
        {
            inVect[j] = std::move(inVect[j + 1 + i]);
        }
    }
    inVect.resize(initSize - excludeSet.size());
}
template void eraseItems(std::vector<std::valarray<double>> & inVect, const std::vector<int> & indices);
template void eraseItems(std::vector<int> & inVect, const std::vector<int> & indices);
template void eraseItems(std::vector<double> & inVect, const std::vector<int> & indices);
template void eraseItems(std::vector<QString> & inVect, const std::vector<int> & indices);


#endif // SMALLFUNCS_H
