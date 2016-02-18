#include "lib.h"

using namespace std;
//namespace fs = boost::filesystem;

bool fileExists(const QString & filePath)
{
    if(QFile::exists(filePath))
    {
        return true;
    }
    return false;
}


std::string slash()
{
    return "/";
}

QString qslash()
{
    return QString('/');
}

int typeOfFileName(const std::string & filePath)
{
    std::vector<std::string> fileMarkers = makeDefFilters();
    for(int i = 0; i < fileMarkers.size(); ++i)
    {
        if(contains(filePath, fileMarkers[i]))
        {
            return i;
        }
    }
    return -1;
}

bool endsWith(const std::string & inStr,
              const std::string & filter)
{
    int a = inStr.rfind(filter); // search from the back
    if(a != std::string::npos && (a + filter.size() == inStr.size()))
    {
        return true;
    }
    return false;
}

bool contains(const std::string & inStr,
              const std::string & filter)
{
    if(inStr.find(filter) != std::string::npos)
    {
        return true;
    }
    return false;
}

bool contains(const std::string & inStr,
              const std::vector<std::string> & filters)
{
    for(int i = 0; i < filters.size(); ++i)
    {
        if(inStr.find(filters[i]) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

std::vector<std::string> contents(const std::string & dirPath,
                                  const std::string & filter)
{
    std::vector<std::string> res;
#if !MY_QT
    fs::path myPath(dirPath);

    if(fs::is_directory(myPath))
    {
#if CPP_11
        for(const fs::directory_entry & pew : fs::directory_iterator(myPath))
        {
            const std::string pth = pew.path().native();
#else
        for(fs::directory_iterator it = fs::directory_iterator(myPath);
            it != fs::directory_iterator();
            ++it)
        {
            const std::string pth = (*it).path().native();
#endif
            if(contains(pth, filter))
            {
                res.push_back(pth);
            }
        }
    }
    else
    {
        cout << "error" << endl;
    }
#else
    const QString qDirPath = QString(dirPath.c_str());
    QStringList lst = QDir(qDirPath).entryList({"*" + QString(filter.c_str()) + "*"});
    for(const QString & str : lst)
    {
        res.push_back((qDirPath + qslash() + str).toStdString());
    }
#endif
    return res;
}

std::vector<std::vector<std::string> > contents(const std::string & dirPath,
                                  const std::vector<std::string> & filtersList)
{
    const int numOfClasses = filtersList.size();
//    cout << numOfClasses << endl;
    std::vector< std::vector<std::string> > res(numOfClasses);
#if !MY_QT
    fs::path myPath(dirPath);


    if(fs::is_directory(myPath))
    {
#if CPP_11
        for(const fs::directory_entry & pew : fs::directory_iterator(myPath))
        {
            const std::string pth = pew.path().native();
#else
        for(fs::directory_iterator it = fs::directory_iterator(myPath);
            it != fs::directory_iterator();
            ++it)
        {
            const std::string pth = (*it).path().native();
#endif
            for(int i = 0; i < numOfClasses; ++i)
            {
                if(contains(pth, filtersList[i]))
                {
                    res[i].push_back(pth);
                }
            }
        }
    }
    else
    {
        cout << "error" << endl;
    }  
#else

    const QString qDirPath = QString(dirPath.c_str());
    for(int i = 0; i < numOfClasses; ++i)
    {
        QStringList lst = QDir(qDirPath).entryList({"*" + QString(filtersList[i].c_str()) + "*"});
        for(const QString & str : lst)
        {
            res[i].push_back((qDirPath + qslash() + str).toStdString());
        }
    }
#endif
    return res;
}

std::vector<std::vector<std::string> > contents(const std::string & dirPath,
                                  const std::vector<std::vector<std::string> > & filtersList)
{
    const int numOfClasses = filtersList.size();
    cout << numOfClasses << endl;
    std::vector< std::vector<std::string> > res(numOfClasses);

#if !MY_QT
    fs::path myPath(dirPath);


    if(fs::is_directory(myPath))
    {
#if CPP_11
        for(const fs::directory_entry & pew : fs::directory_iterator(myPath))
        {
            const std::string pth = pew.path().native();
#else
        for(fs::directory_iterator it = fs::directory_iterator(myPath);
            it != fs::directory_iterator();
            ++it)
        {
            const std::string pth = (*it).path().native();
#endif
            for(int i = 0; i < numOfClasses; ++i)
            {
                if(contains(pth, filtersList[i]))
                {
                    res[i].push_back(pth);
                }
            }
        }
    }
    else
    {
        cout << "error" << endl;
    }
#else
    const QString qDirPath = QString(dirPath.c_str());
    for(int i = 0; i < numOfClasses; ++i)
    {
        QStringList filts;
        for(const std::string & str : filtersList[i])
        {
            filts << "*" + QString(str.c_str()) + "*";
        }
        QStringList lst = QDir(qDirPath).entryList(filts);
        for(const QString & str : lst)
        {
            res[i].push_back((qDirPath + qslash() + str).toStdString());
        }
    }
#endif
    return res;
}

void myIota(std::vector<int> & in)
{
    std::vector<int>::iterator it = in.begin();
    for(int i = 0;
        it != in.end();
        ++i, ++it)
    {
        *it = i;
    }
}

void myShuffle(std::vector<int> & in)
{
    srand(time(NULL));
    for(int i = 0; i < in.size() * 5; ++i)
    {
        int a = rand()%in.size();
        int b = rand()%in.size();
        std::swap(in[a], in[b]);
    }
}

int myLess(int a, int b)
{
    return (a<b)?a:b;
}

template <typename T>
void eraseItems(std::vector<T> & inVect,
                const std::vector<int> & indices)
{
#if CPP_11
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


#else
    const int initSize = inVect.size();
    std::set<int> excludeSet; // less first

    for(std::vector<int>::const_iterator it = indices.begin();
        it != indices.end();
        ++it)
    {
        excludeSet.insert(*it);
    }
    /// check size
//    std::sort(excludeSet.begin(), excludeSet.end()); // default: less first

    std::vector<int> excludeVector(excludeSet.size());
    std::copy(excludeSet.begin(), excludeSet.end(), excludeVector.begin());

    std::sort(excludeVector.begin(), excludeVector.end()); // default: less first

    excludeVector.push_back(initSize); // for the last elements' shift

    for(int i = 0; i < excludeVector.size() - 1; ++i)
    {
        for(int j = excludeVector[i] - i; j < excludeVector[i + 1] - i - 1; ++j)
        {
#if CPP_11
            inVect[j] = std::move(inVect[j + 1 + i]);
#else
            inVect[j] = inVect[j + 1 + i];
#endif
        }
    }
    inVect.resize(initSize - excludeSet.size());
#endif
}
template void eraseItems(std::vector<std::string> & inVect, const std::vector<int> & indices);
template void eraseItems(std::vector<lineType> & inVect, const std::vector<int> & indices);
template void eraseItems(std::vector<int> & inVect, const std::vector<int> & indices);
template void eraseItems(std::vector<double> & inVect, const std::vector<int> & indices);


template <typename signalType>
void readFileInLine(const std::string & filePath,
                    signalType & result)
{
//    cout << filePath << endl;

    ifstream file(filePath.c_str());
    if(!file.good())
    {
        cout << "readFileInLine: bad file " << filePath << endl;
        return;
    }
    vectType outData;
    double tmp;
    int num = 0;
    while(!file.eof())
    {
        file >> tmp;
        outData.push_back(tmp);
        ++num;
    }
    outData.pop_back(); ///// prevent doubling last item (eof) bicycle
    file.close();

    result.resize(outData.size());
#if CPP_11
    std::copy(outData.begin(),
              outData.end(),
              begin(result));
#else
    int i = 0;
    for(vectType::iterator it = outData.begin();
        it != outData.end();
        ++i, ++it)
    {
        result[i] = *it;
    }
#endif

}


#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
void four1(double * dataF, int nn, int isign)
{
    int n,mmax,m,j,istep,i;
    double wtemp,wr,wpr,wpi,wi,theta;
    double tempr,tempi;

    n = nn << 1; //n = 2 * fftLength
    j = 1;
    for (i = 1; i < n; i += 2)
    {
        if (j > i)
        {
            SWAP(dataF[j], dataF[i]);
            SWAP(dataF[j+1],dataF[i+1]);
        }
        m = n >> 1; // m = n / 2;
        while (m >= 2 && j > m)
        {
            j -= m;
            m >>= 1; //m /= 2;
        }
        j += m;
    }
    mmax = 2;
    while (n > mmax)
    {
        istep = mmax << 1; //istep = mmax * 2;
        theta = isign * (2 * pi / mmax);
        wtemp = sin(0.5 * theta);

        wpr = - 2.0 * wtemp * wtemp;
        wpi = sin(theta);

        wr = 1.0;
        wi = 0.0;
        for(m = 1; m < mmax; m += 2)
        {
            for(i = m; i <= n; i += istep)
            {
                j = i + mmax;

                // tempCompl = wCompl * dataFCompl[j/2]
                tempr = wr * dataF[j] - wi * dataF[j + 1];
                tempi = wr * dataF[j + 1] + wi * dataF[j];

                // dataFCompl[j/2] = dataFCompl[i/2] - tempCompl
                dataF[j] = dataF[i] - tempr;
                dataF[j + 1] = dataF[i + 1] - tempi;

                // dataFCompl[i/2] += tempCompl
                dataF[i] += tempr;
                dataF[i + 1] += tempi;
            }
            wr = (wtemp = wr) * wpr - wi * wpi + wr;
            wi = wi * wpr + wtemp * wpi + wi;
        }
        mmax = istep;
    }
}
#undef SWAP

template <typename signalType, typename retType>
retType spectre(const signalType & data)
{
    int length = data.size();
    int fftLen = fftL(length); // nearest exceeding power of 2
    double norm = sqrt(fftLen / double(length));

    vectType tempSpectre(2 * fftLen, 0.);
    for(int i = 0; i < length; ++i)
    {
        tempSpectre[ 2 * i + 0 ] = data[i] * norm;
    }
    four1(tempSpectre.data() - 1, fftLen, 1);

    retType spectr(fftLen / 2);
    norm = 2. / (250. * fftLen); /// def::freq
    for(int i = 0; i < fftLen / 2; ++i )      //get the absolute value of FFT
    {
        spectr[ i ] = (pow(tempSpectre[ i * 2 ], 2.)
                      + pow(tempSpectre[ i * 2 + 1 ], 2.)) * norm;
    }
    return spectr;
}

template <typename signalType, typename retType>
retType smoothSpectre(const signalType & inSpectre, const int numOfSmooth)
{
    retType result = inSpectre;
    double help1, help2;
    for(int num = 0; num < numOfSmooth; ++num)
    {
        help1 = result[0];
        for(int i = 1; i < result.size() - 1; ++i)
        {
            help2 = result[i];
            result[i] = (help1 + help2 + result[i+1]) / 3.;
            help1 = help2;
        }
    }
    return result;
}


template lineType smoothSpectre(const lineType & inSpectre, const int numOfSmooth);

template void readFileInLine(const std::string & filePath, lineType & outData);
template void readFileInLine(const std::string & filePath, vectType & outData);


template lineType spectre(const vectType & data);
template lineType spectre(const lineType & data);

