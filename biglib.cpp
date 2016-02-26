#include "biglib.h"

using namespace std;
//namespace fs = boost::filesystem;

std::string funcName(std::string in)
{
    in.resize(in.find('('));
    for(char a : {' ', '='})
    {
        auto b = in.rfind(a);
        if(b >= 0)
        {
            in = in.substr(b + 1);
        }
    }
    return in;
}

std::ostream & operator << (std::ostream &os, QString toOut)
{
    os << toOut.toStdString();
    return os;
}

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

//template <typename T>
//void eraseItems(std::vector<T> & inVect,
//                const std::vector<int> & indices)
//{
//#if CPP_11
//    const int initSize = inVect.size();
//    std::set<int, std::less<int> > excludeSet; // less first
//    for(auto item : indices)
//    {

//        excludeSet.emplace(item);
//    }
//    std::vector<int> excludeVector;
//    for(auto a : excludeSet)
//    {
//        excludeVector.push_back(a);
//    }
//    excludeVector.push_back(initSize); // for the last elements' shift

//    for(int i = 0; i < excludeVector.size() - 1; ++i)
//    {
//        for(int j = excludeVector[i] - i; j < excludeVector[i + 1] - i - 1; ++j)
//        {
//            inVect[j] = std::move(inVect[j + 1 + i]);
//        }
//    }
//    inVect.resize(initSize - excludeSet.size());


//#else
//    const int initSize = inVect.size();
//    std::set<int> excludeSet; // less first

//    for(std::vector<int>::const_iterator it = indices.begin();
//        it != indices.end();
//        ++it)
//    {
//        excludeSet.insert(*it);
//    }
//    /// check size
////    std::sort(excludeSet.begin(), excludeSet.end()); // default: less first

//    std::vector<int> excludeVector(excludeSet.size());
//    std::copy(excludeSet.begin(), excludeSet.end(), excludeVector.begin());

//    std::sort(excludeVector.begin(), excludeVector.end()); // default: less first

//    excludeVector.push_back(initSize); // for the last elements' shift

//    for(int i = 0; i < excludeVector.size() - 1; ++i)
//    {
//        for(int j = excludeVector[i] - i; j < excludeVector[i + 1] - i - 1; ++j)
//        {
//#if CPP_11
//            inVect[j] = std::move(inVect[j + 1 + i]);
//#else
//            inVect[j] = inVect[j + 1 + i];
//#endif
//        }
//    }
//    inVect.resize(initSize - excludeSet.size());
//#endif
//}

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

void readMatrixFile(const QString & filePath,
                     matrix & outData,
                     int rows,
                     int cols)
{
    ifstream file(filePath.toStdString());
    if(!file.good())
    {
        cout << "readSpectreFile: bad input file " << filePath << endl;
        return;
    }
    outData.resize(rows, cols);

    for(int i = 0; i < rows; ++i)
    {
        for(int j = 0; j < cols; ++j)
        {
            file >> outData[i][j];
        }
    }
    file.close();
}

void writeMatrixFile(const QString & filePath,
                      const matrix & outData,
                      int rows,
                      int cols)

{
    if(rows > outData.rows() ||
       cols > outData.cols())
    {
        cout << "bad inputs while writing matrix" << endl;
        return;
    }

    ofstream file(filePath.toStdString());
    if(!file.good())
    {
        cout << "bad output file:\n" << filePath.toStdString() << endl;
        return;
    }

    for(int i = 0; i < rows; ++i)
    {
        for(int j = 0; j < cols; ++j)
        {
            file << doubleRound(outData[i][j], 4) << '\t';
        }
        file << endl;
    }
    file.close();
}


void writePlainData(const QString outPath,
                    const matrix & data,
                    const int & ns,
                    int numOfSlices,
                    const int & start)
{
    numOfSlices = min(numOfSlices,
                      data.cols() - start);

//    if(numOfSlices < 250) return; /// used for sliceWindFromReal() but Cut::cut() ...

    ofstream outStr;
    outStr.open(outPath.toStdString());
    outStr << "NumOfSlices " << numOfSlices;
    outStr << "\tNumOfChannels " << ns;
    outStr << endl;
    for (int i = 0; i < numOfSlices; ++i)
    {
        for(int j = 0; j < ns; ++j)
        {
            outStr << doubleRound(data[j][i + start], 3) << '\t';
        }
        outStr << '\n';
    }
    outStr.flush();
    outStr.close();
}


void readPlainData(const QString & inPath,
                   matrix & data,
                   const int & ns,
                   int & numOfSlices,
                   const int & start)
{
    ifstream inStr;
    inStr.open(inPath.toStdString());
    if(!inStr.good())
    {
        cout << "readPlainData: cannot open file" << endl;
        return;
    }
    int inNs = -1;
    inStr.ignore(64, ' '); // "NumOfSlices "
    inStr >> numOfSlices;
    inStr.ignore(64, ' '); // "NumOfChannels N\n"
    inStr >> inNs;
    inStr.ignore(64, '\n');
    if(inNs != -1 && inNs != ns)
    {
        cout << "readPlainData: not real ns" << endl;
    }

    data.resize(ns, numOfSlices + start);

    for (int i = 0; i < numOfSlices; ++i)
    {
        for(int j = 0; j < ns; ++j)
        {
            inStr >> data[j][i + start];
        }
    }
    inStr.close();
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


template <typename signalType>
signalType four2(const signalType & inRealData, int fftLen, int isign)
{
    double * pew = new double [2 * fftLen];
    for(int i = 0; i < fftLen; ++i)
    {
        pew[2 * i] = inRealData[i];
        pew[2 * i + 1] = 0.;
    }
//    for(int i = inRealData.size(); i < 2 * fftLen; ++i)
//    {
//        pew[i] = 0.;
//    }

    four1(pew - 1, fftLen, isign);

    signalType res(fftLen);
    for(int i = 0; i < fftLen; ++i)
    {
        res[i] = (pew[2 * i] * pew[2 * i] + pew[2 * i + 1] * pew[2 * i + 1]);
    }
    delete []pew;
    return res;
}

template <typename signalType = lineType>
void calcSpectre(const signalType & inSignal,
                 signalType & outSpectre,
                 const int & fftLength,
                 const int & NumOfSmooth,
                 const int & Eyes,
                 const double & powArg)
{
    if(inSignal.size() != fftLength)
    {
        cout << "calcSpectre: inappropriate signal length" << endl;
        return;
    }

    const double norm1 = sqrt(fftLength / double(fftLength - Eyes));
#if 0
    const double norm2 = 2. / (def::freq * fftLength);
    vector<double> spectre (fftLength * 2, 0.); // can be valarray, but not important
    for(int i = 0; i < fftLength; ++i)
    {
        spectre[ i * 2 ] = inSignal[ i ] * norm1;
    }
    four1(spectre, fftLength, 1);
    for(int i = 0; i < fftLength / 2; ++i )
    {
        outSpectre[ i ] = (pow(spectre[ i * 2 ], 2) + pow(spectre[ i * 2 + 1 ], 2)) * norm2;
//        outSpectre[ i ] = pow ( outSpectre[ i ], powArg );
    }
#else
    const double nrm = 2. / (double(fftLength - Eyes) * def::freq);
    outSpectre = four2(inSignal, fftLength, 1) * nrm;
#endif
//    outSpectre = pow(four2(inSignal, fftLength, 1) * nrm, powArg);



    //smooth spectre
    const int leftSmoothLimit = 1;
    const int rightSmoothLimit = fftLength / 2 - 1;
    double help1, help2;
    for(int a = 0; a < (int)(NumOfSmooth / norm1); ++a)
    {
        help1 = outSpectre[leftSmoothLimit - 1];
        for(int k = leftSmoothLimit; k < rightSmoothLimit; ++k)
        {
            help2 = outSpectre[k];
            outSpectre[k] = (help1 + help2 + outSpectre[k + 1]) / 3.;
            help1 = help2;
        }
    }
}

template <typename Typ>
Typ peekFromSocket(QTcpSocket * inSocket)
{
    int siz = sizeof(Typ);
    char * tmp = new char[siz];
    inSocket->peek(tmp, siz);
    Typ res = *(reinterpret_cast<Typ*>(tmp));
    delete[] tmp;
    return res;
}
template <typename Typ>
Typ readFromSocket(QTcpSocket * inSocket)
{
    int siz = sizeof(Typ);
    char * tmp = new char[siz];
    inSocket->read(tmp, siz);
    Typ res = *(reinterpret_cast<Typ*>(tmp));
    delete[] tmp;
    return res;
}

template <typename Typ>
std::bitset<8 * sizeof(Typ)> bits(Typ in)
{
    return std::bitset<8 * sizeof(Typ)>(in);
}

std::string readString(QDataStream &in)
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

std::string readString(QTcpSocket * inSocket)
{
    int numOfChars = readFromSocket<qint32>(inSocket);
    char * res = new char [numOfChars + 1];
    cout << "readString: numOfChars = " << numOfChars << endl;

//    inSocket->read(res, numOfChars);
    for(int i = 0; i < numOfChars; ++i)
    {
        inSocket->read(&(res[i]), 1);
    }
    res[numOfChars] = '\0';
    std::string ret(res);
    delete[] res;
    return ret;
}


template qint8 readFromSocket(QTcpSocket * inSocket);
template quint8 readFromSocket(QTcpSocket * inSocket);
template qint16 readFromSocket(QTcpSocket * inSocket);
template quint16 readFromSocket(QTcpSocket * inSocket);
template qint32 readFromSocket(QTcpSocket * inSocket);
template quint32 readFromSocket(QTcpSocket * inSocket);
template qint64 readFromSocket(QTcpSocket * inSocket);
template quint64 readFromSocket(QTcpSocket * inSocket);
template double readFromSocket(QTcpSocket * inSocket);

template qint8 peekFromSocket(QTcpSocket * inSocket);
template quint8 peekFromSocket(QTcpSocket * inSocket);
template qint16 peekFromSocket(QTcpSocket * inSocket);
template quint16 peekFromSocket(QTcpSocket * inSocket);
template qint32 peekFromSocket(QTcpSocket * inSocket);
template quint32 peekFromSocket(QTcpSocket * inSocket);

template std::bitset<8> bits(qint8 in);
template std::bitset<8> bits(quint8 in);
template std::bitset<16> bits(qint16 in);
template std::bitset<16> bits(quint16 in);
template std::bitset<32> bits(qint32 in);
template std::bitset<32> bits(quint32 in);

//template void eraseItems(std::vector<std::string> & inVect, const std::vector<int> & indices);
//template void eraseItems(std::vector<lineType> & inVect, const std::vector<int> & indices);
//template void eraseItems(std::vector<int> & inVect, const std::vector<int> & indices);
//template void eraseItems(std::vector<double> & inVect, const std::vector<int> & indices);

template lineType smoothSpectre(const lineType & inSpectre, const int numOfSmooth);

template void readFileInLine(const std::string & filePath, lineType & outData);
template void readFileInLine(const std::string & filePath, vectType & outData);

template lineType spectre(const vectType & data);
template lineType spectre(const lineType & data);

template void calcSpectre(const lineType & inSignal, lineType & outSpectre,
const int & fftLength, const int & NumOfSmooth, const int & Eyes, const double & powArg);

template lineType four2(const lineType & inRealData, int fftLen, int isign);
