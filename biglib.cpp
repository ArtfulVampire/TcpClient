#include "biglib.h"

using namespace std;

QString funcName(QString in)
{
	in = in.left(in.indexOf('('));
	for(char a : {' ', '='})
	{
		int b = in.lastIndexOf(a);
		if(b < in.size())
		{
			in = in.mid(b + 1);
		}
	}
	return in;
}

std::ostream & operator<< (std::ostream &os, QString toOut)
{
	os << toOut.toStdString();
	return os;
}

template <typename Typ, template <typename> class Cont>
std::ostream & operator<< (std::ostream &os, const Cont<Typ> & toOut)
{
	for(auto in : toOut)
	{
		os << in << '\t';
	}
	return os;
}

template <typename Typ, template <typename, typename> class Cont>
std::ostream & operator<< (std::ostream &os, const Cont<Typ, std::allocator<Typ>> & toOut)
{
	for(auto in : toOut)
	{
		os << in << '\t';
	}
	return os;
}


int typeOfFileName(const QString & fileName)
{
	QStringList leest;
	int res = 0;
	for(const QString & marker : def::fileMarkers) /// each "marker" is a set of markers
	{
		leest.clear();
		leest = marker.split(QRegExp("[,; ]"), QString::SkipEmptyParts);
		for(const QString & filter : leest)
		{
			if(fileName.contains(filter))
			{
				return res;
			}
		}
		++res;
	}
	return -1;
}

std::vector<QString> contents(const QString & dirPath,
							  const QString & filter)
{
	std::vector<QString> res;
	QStringList lst = QDir(dirPath).entryList({"*" + filter + "*"});
	for(const QString & str : lst)
	{
		res.push_back(dirPath + "/" + str);
	}
	return res;
}

std::vector<std::vector<QString>> contents(const QString & dirPath,
										   const std::vector<QString> & filtersList)
{
	const int numOfClasses = filtersList.size();
	std::vector< std::vector<QString>> res(numOfClasses);
	for(int i = 0; i < numOfClasses; ++i)
	{
		QStringList lst = QDir(dirPath).entryList({"*" + filtersList[i] + "*"});
		for(const QString & str : lst)
		{
			res[i].push_back(dirPath + "/" + str);
		}
	}
	return res;
}

std::vector<std::vector<QString>> contents(const QString & dirPath,
										   const std::vector<std::vector<QString> > & filtersList)
{
	const int numOfClasses = filtersList.size();
	cout << numOfClasses << endl;
	std::vector< std::vector<QString>> res(numOfClasses);
	for(int i = 0; i < numOfClasses; ++i)
	{
		QStringList filts;
		for(const QString & str : filtersList[i])
		{
			filts << "*" + str + "*";
		}
		QStringList lst = QDir(dirPath).entryList(filts);
		for(const QString & str : lst)
		{
			res[i].push_back(dirPath + "/" + str);
		}
	}
	return res;
}

template <typename signalType>
void readFileInLine(const QString & filePath,
					signalType & result)
{
	ifstream file(filePath.toStdString());
	if(!file.good())
	{
		cout << "readFileInLine: bad file " << filePath << endl;
		return;
	}
	int rows;
	int cols;
	file.ignore(64, ' ');
	file >> rows;
	file.ignore(64, ' ');
	file >> cols;

	int len = rows * cols;
	result.resize(len);
	for(int i = 0; i < len; ++i)
	{
		file >> result[i];
	}
	file.close();
}

template <typename signalType>
void writeFileInLine(const QString & filePath,
					 const signalType & outData)
{
	ofstream file(filePath.toStdString());
	if(!file.good())
	{
		cout << "bad file" << endl;
		return;
	}
	file << "FileLen " << outData.size() << '\t';
	file << "Pewpew " << 1 << endl;
	for(auto out : outData)
	{
		file << out << '\n';
		//        file << doubleRound(out, 3) << '\n';
	}
	file << endl;
	file.close();
}


void makeFileLists(const QString & path,
				   vector<QStringList> & lst,
				   const QStringList & auxFilters)
{
	QDir localDir(path);
	QStringList nameFilters, leest;
	QString helpString;
	for(const QString & fileMark : def::fileMarkers)
	{
		nameFilters.clear();
		leest.clear();
		leest = fileMark.split(QRegExp("[,; ]"), QString::SkipEmptyParts);
		for(const QString & filter : leest)
		{
			helpString = "*" + filter + "*";
			if(!auxFilters.isEmpty())
			{
				for(const QString & aux : auxFilters)
				{
					//                    nameFilters << QString(def::ExpName.left(3) + "*" + aux + helpString);
					nameFilters << QString("*" + aux + helpString);
					nameFilters << QString(helpString + aux + "*");
				}
			}
			else
			{
				//                nameFilters << QString(def::ExpName.left(3) + helpString);
				nameFilters << helpString;
			}
		}
		lst.push_back(localDir.entryList(nameFilters,
										 QDir::Files,
										 QDir::Time)); /// Time ~ order
	}
}

void makeFullFileList(const QString & path,
					  QStringList & lst,
					  const QStringList & auxFilters)
{
	QDir localDir(path);
	QStringList nameFilters, leest;
	QString helpString;
	for(const QString & fileMark : def::fileMarkers)
	{
		leest = fileMark.split(QRegExp("[,; ]"), QString::SkipEmptyParts);
		for(const QString & filter : leest)
		{
			helpString = "*" + filter + "*";
			if(!auxFilters.isEmpty())
			{
				for(const QString & aux : auxFilters)
				{
					//                    nameFilters << QString(def::ExpName.left(3) + "*" + aux + helpString);
					nameFilters << QString("*" + aux + helpString);
				}
			}
			else
			{
				//                nameFilters << QString(def::ExpName.left(3) + helpString);
				nameFilters << helpString;
			}

		}
	}
	lst = localDir.entryList(nameFilters,
							 QDir::Files,
							 QDir::Time); /// Time ~ order
}

void readMatrixFile(const QString & filePath,
					matrix & outData)
{
	ifstream file(filePath.toStdString());
	if(!file.good())
	{
		cout << "readMatrixFile: bad input file " << filePath << endl;
		return;
	}
	int rows;
	int cols;
	file.ignore(64, ' ');
	file >> rows;
	file.ignore(64, ' ');
	file >> cols;

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
					 const QString & rowsString,
					 const QString & colsString)
{
	ofstream file(filePath.toStdString());
	if(!file.good())
	{
		cout << "writeMatrixFile: bad output file\n" << filePath.toStdString() << endl;
		return;
	}

	file << rowsString << " " << outData.rows() << '\t';
	file << colsString << " " << outData.cols() << endl;

	for(int i = 0; i < outData.rows(); ++i)
	{
		for(int j = 0; j < outData.cols(); ++j)
		{
			file << doubleRound(outData[i][j], 3) << '\t';
		}
		file << endl;
	}
	file.close();
}


/// in file and in matrix - transposed
void writePlainData(const QString outPath,
					const matrix & data,
					int numOfSlices,
					const int & start)
{
	numOfSlices = min(numOfSlices,
					  data.cols() - start);

	//    if(numOfSlices < 250) return; /// used for sliceWindFromReal() but Cut::cut() ...

	ofstream outStr;
	outStr.open(outPath.toStdString());
	outStr << "NumOfSlices " << numOfSlices << '\t';
	outStr << "NumOfChannels " << data.rows() << endl;
	for (int i = 0; i < numOfSlices; ++i)
	{
		for(int j = 0; j < data.rows(); ++j)
		{
			outStr << doubleRound(data[j][i + start], 3) << '\t';
		}
		outStr << endl;
	}
	outStr.flush();
	outStr.close();
}


void readPlainData(const QString & inPath,
				   matrix & data,
				   int & numOfSlices,
				   const int & start)
{
	ifstream inStr;
	inStr.open(inPath.toStdString());
	if(!inStr.good())
	{
		cout << "readPlainData: cannot open file\n" << inPath << endl;
		return;
	}
	int localNs;
	inStr.ignore(64, ' '); // "NumOfSlices "
	inStr >> numOfSlices;
	inStr.ignore(64, ' '); // "NumOfChannels "
	inStr >> localNs;

	data.resize(localNs, numOfSlices + start);

	for (int i = 0; i < numOfSlices; ++i)
	{
		for(int j = 0; j < localNs; ++j)
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

	std::vector<double> tempSpectre(2 * fftLen, 0.);
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

template <typename Container>
int indexOfMax(const Container & cont)
{
	int res = 0;
	int ans = 0;
	auto val = *(std::begin(cont));

	for(auto it = std::begin(cont);
		it != std::end(cont);
		++it, ++res)
	{
		if(*it > val)
		{
			ans = res;
			val = *it;
		}
	}
	return ans;
}

void resizeValar(std::valarray<double> & in, int num)
{
	std::valarray<double> temp = in;
	in.resize(num);
	std::copy(begin(temp),
			  begin(temp) + min(in.size(), temp.size()),
			  begin(in));
}

template <typename signalType = std::valarray<double>>
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

QString rightNumber(const unsigned int input, int N) // prepend zeros
{
	QString h;
	h.setNum(input);
	for(int i = 0; i < N; ++i)
	{
		h.prepend("0");
	}
	return h.right(N);
}

void fixFilesSlashN(const QString & path)
{
	for(const QString & fileNam : QDir(path).entryList(QDir::Files))
	{
		QFile fil;

		fil.setFileName(path + fileNam);
		fil.open(QFile::ReadOnly);
		auto dat = fil.readAll().replace("\n", "\r\n");
		fil.close();

		fil.open(QFile::WriteOnly);
		fil.write(dat);
		fil.close();
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

/// check
QString readString(QDataStream & in)
{
	qint32 numOfChars;
	in >> numOfChars;
	cout << "numChars = " << numOfChars << endl;

	char * res = new char [numOfChars + 1];
	for(int i = 0; i < numOfChars; ++i)
	{
		in.readRawData(&(res[i]), 1);
	}
	res[numOfChars] = '\0';

	QString ret(res);
	delete[] res;
	return ret;
}

QString readString(QTcpSocket * inSocket)
{
	int numOfChars = readFromSocket<qint32>(inSocket);
	char * res = new char [numOfChars + 1];
	for(int i = 0; i < numOfChars; ++i)
	{
		inSocket->read(&(res[i]), 1);
	}
	res[numOfChars] = '\0';

	QString ret(res);
	delete[] res;
	return ret;
}
template std::ostream & operator<< (std::ostream &os, const std::valarray<double> & toOut);
template std::ostream & operator<< (std::ostream &os, const std::vector<int> & toOut);
template std::ostream & operator<< (std::ostream &os, const std::vector<double> & toOut);

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

template int indexOfMax(const std::vector<double> & cont);
template int indexOfMax(const std::valarray<double> & cont);
template int indexOfMax(const std::list<double> & cont);

template std::valarray<double> smoothSpectre(const std::valarray<double> & inSpectre, const int numOfSmooth);

template void readFileInLine(const QString & filePath, std::valarray<double> & outData);

template void writeFileInLine(const QString & filePath, const std::valarray<double> & outData);

template std::valarray<double> spectre(const std::vector<double> & data);
template std::valarray<double> spectre(const std::valarray<double> & data);

template void calcSpectre(const std::valarray<double> & inSignal, std::valarray<double> & outSpectre,
const int & fftLength, const int & NumOfSmooth, const int & Eyes, const double & powArg);

template std::valarray<double> four2(const std::valarray<double> & inRealData, int fftLen, int isign);
