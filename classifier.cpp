#include "classifier.h"

net::net(QObject * par)
{
	this->setParent(par);
	confusionMatrix = matrix(def::numOfClasses(), def::numOfClasses(), 0.);

	readMatrixFile(def::eyesFilePath,
				   coeff); // num eog channels

	QDir(def::workPath).mkdir("weights");

	comPort = new QSerialPort(this);
	comPort->setPortName(def::comPortName);
	comPort->open(QIODevice::WriteOnly);
	comPortDataStream.setDevice(comPort);
	if(comPort->isOpen())
	{
		std::cout << "serialPort opened: " + def::comPortName << std::endl;
		std::cout << "portName: " << comPort->portName().toStdString() << std::endl;
		std::cout << "dataBits: " << comPort->dataBits() << std::endl;
		std::cout << "baudRate: " << comPort->baudRate() << std::endl;
		std::cout << "dataTerminalReady: " << comPort->isDataTerminalReady() << std::endl;
		std::cout << "flowControl: " << comPort->flowControl() << std::endl;
		std::cout << "requestToSend: " << comPort->isRequestToSend() << std::endl;
		std::cout << "stopBits: " << comPort->stopBits() << endl << std::endl;
	}

}

net::~net()
{
	comPort->close();
}

void net::startOperate()
{
	Mode = myMode::train_test;
	successiveProcessing(def::spectraPath);
	std::cout << "classifier waits for work" << std::endl;
}

std::vector<int> net::makeLearnIndexSet()
{
	std::vector<int> mixNum;
	if(this->Mode == myMode::train_test)
	{
		for(int i = 0; i < fileNames.size(); ++i)
		{
			if(fileNames[i].contains("_train"))
			{
				mixNum.push_back(i);
			}
		}
	}
	else if(this->Mode == myMode::N_fold)
	{
		for(int i = 0; i < dataMatrix.rows(); ++i)
		{
			mixNum.push_back(i);
		}
	}
	else if(this->Mode == myMode::k_fold)
	{
		mixNum.resize(dataMatrix.rows());
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::iota(std::begin(mixNum), std::end(mixNum), 0);
		std::shuffle(std::begin(mixNum), std::end(mixNum),
					 std::default_random_engine(seed));
		mixNum.resize(mixNum.size() * (folds - 1) / folds);
	}
	else if(this->Mode == myMode::half_half)
	{
		mixNum.resize(dataMatrix.rows() / 2);
		std::iota(std::begin(mixNum), std::end(mixNum), 0);
	}
	return mixNum;
}


double net::adjustLearnRate(int lowLimit,
							int highLimit)
{
	std::cout << "adjustLearnRate: start" << std::endl;

	const std::vector<int> mixNum = makeLearnIndexSet();

	double res = 1.;
	int counter = 0;
	do
	{
		/// remake with indices
		const double currVal = this->learnRate;
		std::cout << "lrate = " << currVal << '\t';

		learnNetIndices(mixNum);

		/// check limits
		if(this->getEpoch() > highLimit
		   || this->getEpoch() < lowLimit)
		{
			this->learnRate = (currVal
							   * sqrt(this->getEpoch()
									  /  ((lowLimit + highLimit) / 2.)
									  )
							   );
		}
		else
		{
			res = currVal;
			break;
		}

		/// check lrate values
		if(this->learnRate < 0.001)
		{
			this->learnRate = 0.001; break;
		}
		else if(this->learnRate >= 1.)
		{
			this->learnRate = (1.); break;
		}
		++counter;
	} while (counter < 15);
	std::cout << std::endl;
	this->learnRate = doubleRound(this->learnRate, 3);
	return res;
}

std::pair<std::vector<int>, std::vector<int> > net::makeIndicesSetsCross(
		const std::vector<std::vector<int> > & arr,
		const int numOfFold)
{
	std::vector<int> learnInd;
	std::vector<int> tallInd;

	const int fold = this->folds;

	for(int i = 0; i < def::numOfClasses(); ++i)
	{

		for(int j = 0; j < classCount[i]; ++j)
		{
			if(j >= (classCount[i] * numOfFold / fold) &&
			   j <= (classCount[i] * (numOfFold + 1) / fold))
			{
				tallInd.push_back(arr[i][j]);
			}
			else
			{
				learnInd.push_back(arr[i][j]);
			}
		}
	}
	return std::make_pair(learnInd, tallInd);
}

void net::printData()
{
	std::cout << "size of learning set:" << dataMatrix.rows() << " = ";
	std::cout << std::count(std::begin(types), std::end(types), 0) << " + ";
	std::cout << std::count(std::begin(types), std::end(types), 1) << " + ";
	std::cout << std::count(std::begin(types), std::end(types), 2) << std::endl;
	std::cout << "OR = ";
	std::cout << classCount[0] << " + ";
	std::cout << classCount[1] << " + ";
	std::cout << classCount[2] << std::endl;

}


void net::autoClassification(const QString & spectraDir)
{

	QFile::remove(def::netLogPath);

	loadData(spectraDir);

#if 0
	adjustLearnRate(this->lowLimit,
					this->highLimit); // or reduce coeff ?
#endif


	confusionMatrix.fill(0.);
	if(this->Mode == myMode::k_fold)
	{
		crossClassification();
	}
	else if(this->Mode == myMode::N_fold)
	{
		leaveOneOutClassification();
	}
	else if(this->Mode == myMode::train_test)
	{
		trainTestClassification();
	}
	else if(this->Mode == myMode::half_half)
	{
		halfHalfClassification();
	}

	learnNet();
	writeWts();
}


double net::getAverageAccuracy()
{
	return this->averageAccuracy;
}

double net::getKappa()
{
	return this->kappa;
}

int net::getEpoch()
{
	return this->epoch;
}

//void net::setErrCrit(double in)
//{
//    ui->critErrorDoubleSpinBox->setValue(in);
//}

//void net::setLrate(double in)
//{
//    this->learnRate = (in);
//}

//void net::setnumOfPairs(int num)
//{
//    numOfPairsBox = num;
//}

void net::averageClassification()
{
	/// deal with confusionMatrix
	std::ofstream res;
	res.open(def::netResPath.toStdString(), std::ios_base::app);

	for(int i = 0; i < def::numOfClasses(); ++i)
	{
		const double num = confusionMatrix[i].sum();
		if(num != 0.)
		{
			res << doubleRound(confusionMatrix[i][i] * 100. / num, 2) << '\t';
		}
		else
		{
			res << "pew" << '\t';
		}
	}

	// count averages, kappas
	double corrSum = 0.;
	double wholeNum = 0.;

	for(int i = 0; i < def::numOfClasses(); ++i)
	{
		corrSum += confusionMatrix[i][i];
		wholeNum += confusionMatrix[i].sum();
	}
	averageAccuracy = corrSum * 100. / wholeNum;

	// kappa
	double pE = 0.; // for Cohen's kappa
	const double S = confusionMatrix.sum();
	for(int i = 0; i < def::numOfClasses(); ++i)
	{
		pE += (confusionMatrix[i].sum() * confusionMatrix.getCol(i).sum()) /
			  (S * S);
	}
	kappa = 1. - (1. - corrSum / wholeNum) / (1. - pE);

	res << doubleRound(averageAccuracy, 2) << '\t';
	res << doubleRound(kappa, 3) << '\t';
	res << def::ExpName.toStdString() << std::endl;
	res.close();

	confusionMatrix.print();
	std::cout << "average accuracy = " << doubleRound(averageAccuracy, 2) << std::endl;
	std::cout << "kappa = " << kappa << std::endl;
}

void net::writeWts(const QString & outPath)
{
	if(weight.size() != 1) return;
	static int wtsCounter = 0;
	QString wtsPath;
	std::cout << "writeWts: " << wtsCounter << std::endl;
	if(outPath.isEmpty())
	{
		//        while(QFile::exists(def::workPath + "/weights/" +
		//                            def::ExpName + "_" +
		//                            QString::number(wtsCounter) + ".wts"))
		//        {
		//            ++wtsCounter;
		//        }
		wtsPath = def::workPath + "/weights/" +
				  def::ExpName + "_" +
				  QString::number(wtsCounter++) + ".wts";
	}
	else
	{
		wtsPath = outPath;
	}
	writeMatrixFile(wtsPath, weight[0]);
}

void net::reset()
{
	const int numOfLayers = 2;
	dimensionality.resize(numOfLayers);
	dimensionality[0] = this->dataMatrix.cols();
	dimensionality[1] = def::numOfClasses();

	weight.resize(numOfLayers - 1);
	for(int i = 0; i < numOfLayers - 1; ++i) // weights from layer i to i+1
	{
		weight[i].resize(dimensionality[i + 1]);
		for(int j = 0; j < dimensionality[i + 1]; ++j) // to j'th in i+1 layer
		{
			// resizing std::valarray<double> -> fill zeros
			weight[i][j].resize(dimensionality[i] + 1); // from k'th in i layer
		}
	}
}

void net::tallNet()
{
	std::vector<int> indices;
	for(int i = 0; i < dataMatrix.rows(); ++i)
	{
		indices.push_back(i);
	}
	tallNetIndices(indices);
}

void net::tallNetIndices(const std::vector<int> & indices)
{
	std::ofstream badFilesStr;
	badFilesStr.open(def::netBadPath.toStdString(), std::ios_base::app);

	matrix localConfusionMatrix(def::numOfClasses(), def::numOfClasses());
	for(int i = 0; i < indices.size(); ++i)
	{
		const int outClass = classifyDatum(indices[i]).first;
		if(types[ indices[i] ] != outClass )
		{
			badFilesStr << fileNames[ indices[i] ] << std::endl;
			if(tallCleanFlag)
			{
				QFile::remove(def::spectraPath +
							  "/" + fileNames[ indices[i] ]);
				eraseDatum(indices[i]);
			}

		}
		localConfusionMatrix[ types[ indices[i] ] ][ outClass ] += 1.;
		confusionMatrix[ types[ indices[i] ] ][ outClass ] += 1.;
	}
	badFilesStr.close();


	std::ofstream logStream;
	logStream.open(def::netLogPath.toStdString(), std::ios_base::app);

	for(int i = 0; i < def::numOfClasses(); ++i)
	{
		const double num = localConfusionMatrix[i].sum();
		if(num != 0.)
		{
			logStream << doubleRound( localConfusionMatrix[i][i] * 100. / num, 2) << '\t';
		}
		else
		{
			// no errors if there werent such files - N-fold only
			logStream << "pew" << '\t';
		}
	}

	double corrSum = 0.;
	for(int i = 0; i < def::numOfClasses(); ++i)
	{
		corrSum += localConfusionMatrix[i][i];
	}
	averageAccuracy = corrSum / indices.size() * 100.;

	logStream << doubleRound(averageAccuracy, 2) << std::endl;
	logStream.close();
}




void net::successivePreclean(const QString & spectraPath)
{
	QStringList leest;
	makeFullFileList(spectraPath, leest, {"*train*"});
	// clean from first 2 winds
	std::cout << "clean first 2 winds" << std::endl;
	for(auto str : leest)
	{
		if(str.endsWith(".00") || str.endsWith(".01"))
		{
			QFile::remove(spectraPath + "/" + str);
		}
	}

	// clean by learnSetStay
	std::cout << "clean by learnSetStay" << std::endl;
	std::vector<QStringList> leest2;
	makeFileLists(spectraPath, leest2);

	for(int j = 0; j < def::numOfClasses(); ++j)
	{
		auto it = std::begin(leest2[j]);
		for(int i = 0;
			i < leest2[j].size() - learnSetStay * 1.3; /// consts generality
			++i, ++it)
		{
			QFile::remove(spectraPath + "/" + (*it));
		}
	}
	Source = source::winds;
	Mode = myMode::N_fold;

	// N-fold cleaning
	std::cout << "N-fold cleaning" << std::endl;
	tallCleanFlag = true;
	for(int i = 0; i < 3; ++i)
	{
		autoClassification(spectraPath);
	}
	tallCleanFlag = false;
}





void net::successiveProcessing(const QString & spectraPath)
{
	std::vector<int> eraseIndices{};

	numGoodNew = 0;
	confusionMatrix.fill(0.);
	exIndices.clear();

	/// clean from first windows
	QStringList windowsList;
	{
		auto filt = [](int in) -> QString
		{
			return "*.0" + QString::number(in) + ".psd";
		};
		windowsList = QDir(spectraPath).entryList({
													  filt(0),
													  filt(1),
													  filt(2)
												  });
	}
	for(const QString & name : windowsList)
	{
		QFile::remove(spectraPath + "/" + name);
	}

	/// load
	loadData(spectraPath, {".ps"});
	/// reduce learning set to (NumClasses * suc::learnSetStay)
	std::cout << "reduce learning set" << std::endl;
	std::vector<double> count = classCount;
	for(int i = dataMatrix.rows() - 1; i > 0; --i)
	{
		if(count[ types[i] ] > learnSetStay)
		{
			eraseIndices.push_back(i);
			count[ types[i] ] -= 1.;
		}
	}
	//	for(auto ind : eraseIndices)
	//	{
	//		std::cout << def::spectraPath + "/" + fileNames[ind] << std::endl;
	//		QFile::remove(def::spectraPath + "/" + fileNames[ind]);
	//	}
	eraseData(eraseIndices);
	eraseIndices.clear();

	/// preclean finished
	//    for(auto in : fileNames)
	//    {
	//        std::cout << in << std::endl;
	//    }
	//    exit(0);

	/// load newest weights ???

	/// consts
	errCrit = 0.05;
	learnRate = 0.005;

	adjustLearnRate(this->lowLimit,
					this->highLimit);

	std::cout << "get initial weights on train set" << std::endl;
	learnNet();

	errCrit = 0.02;
	learnRate = 0.02;

	std::cout << "successive: initial learn done" << std::endl;
	std::cout << "successive itself" << std::endl;

#if OFFLINE_SUCCESSIVE
	std::valarray<double> tempArr;
	int type = -1;
	QStringList leest = QDir(spectraPath).entryList({'*' + testMarker + '*'}); /// special generality
	for(const QString & fileNam : leest)
	{
		readFileInLine(spectraPath + "/" + fileNam,
					   tempArr);
		type = typeOfFileName(fileNam);
		successiveLearning(tempArr, type, fileNam);
	}
	averageClassification();
	exit(0);
#endif
}

void net::dataCame(eegDataType::iterator a, eegDataType::iterator b)
{
	const int type = def::currentType;
	if(type < 0)
	{
		return;
	}

	std::valarray<double> newSpectre = successiveDataToSpectre(a, b);

	const QString name = def::ExpName +
						 "." + rightNumber(def::numOfReal, 4) +
						 def::fileMarkers[type] +
						 "." + rightNumber(def::numOfWind, 2) + ".psd";

#if 01
	/// spectre
	writeFileInLine(def::workPath + "/SpectraSmooth/winds/" +
					name,
					newSpectre);
#endif

	successiveLearning(newSpectre, type, name);
	++def::numOfWind;
}

std::valarray<double> net::successiveDataToSpectre(
		const eegDataType::iterator eegDataStart,
		const eegDataType::iterator eegDataEnd)
{
	static int windowNum = 0; ++windowNum;

	matrix tmpMat(def::ns, def::windowLength, 0.);
	/// readData
	{
		auto it = eegDataStart;
		for(int j = 0; j < def::windowLength; ++j, ++it)
		{
			//// whaaaaaat
			auto itt = std::begin(*it);
			for(int i = 0; i < def::ns; ++i, ++itt)
			{
				tmpMat[i][j] = (*itt);
			}
		}
		tmpMat /= 8.; /// account for weight bit
#if 0
		/// checked - data is ok
		writePlainData(def::workPath + "/winds/" +
					   def::ExpName +
					   "." + rightNumber(def::numOfReal, 4) +
					   def::fileMarkers[def::currentType] +
				"." + rightNumber(def::numOfWind, 2) + ".scg",
				tmpMat,
				tmpMat.cols());
#endif
	}
	/// clean from eyes
	if(QFile::exists(def::eyesFilePath))
	{
		auto it2 = std::begin(tmpMat);
		for(int i = 0; i < def::eegNs; ++i, ++it2)
		{
			(*it2) -= tmpMat[def::eog1] * coeff[i][0] +
					tmpMat[def::eog2] * coeff[i][1];
		}

#if 0
		/// eyes - checked, OK
		writePlainData(def::workPath + "/winds/" +
					   def::ExpName +
					   "." + rightNumber(def::numOfReal, 4) +
					   def::fileMarkers[def::currentType] +
				"." + rightNumber(def::numOfWind, 2) +  "_ec.scg",
				tmpMat,
				tmpMat.cols());
#endif
	}
	std::valarray<double> res(0., def::eegNs * def::spLength());
	/// count spectra, take 5-20 HZ only
	{
		std::valarray<double> tmpSpec(def::spLength());
		for(int i = 0; i < def::eegNs; ++i)
		{
			tmpSpec = 0.;
			if(!def::dropChannels.contains(i + 1)) /// dropChannels from 1
			{
				calcSpectre(tmpMat[i],
							tmpSpec,
							1024, /// fftLength consts
							def::numOfSmooth
							);
				std::copy(std::begin(tmpSpec) + def::left(),
						  std::begin(tmpSpec) + def::right(),
						  std::begin(res) + i * def::spLength());
			}
		}

	}
	return res;
}

void net::successiveLearning(const std::valarray<double> & newSpectre,
							 const int newType,
							 const QString & newFileName)
{
	static std::vector<int> succOldIndices{};
	static auto findIt = std::begin(types);

	/// dataMatrix is learning matrix
	std::valarray<double> newData = (newSpectre - averageDatum) / (sigmaVector * loadDataNorm);

	pushBackDatum(newData, newType, newFileName);

	const std::pair<int, double> outType = classifyDatum(dataMatrix.rows() - 1); // take the last
	confusionMatrix[newType][outType.first] += 1.;

	/// if correct classification
	if(newType == 0 || newType == 1)
	{
		if(outType.first == newType)
		{
			comPortDataStream << qint8(1);
		}
		else
		{
			comPortDataStream << qint8(2);
		}
	}

	static std::vector<int> passed(3, 0);

	if((outType.first == newType && outType.second < def::errorThreshold)
	   || passed[newType] < learnSetStay
	   )
	{
#if NEW_SUCC
		findIt = std::find(findIt, std::end(types), newType);
		int num = findIt - std::begin(types);
		succOldIndices.push_back(num);
		++findIt;
#else
		const int num = std::find(std::begin(types), std::end(types),
								  newType)
						- std::begin(types);
		eraseDatum(num);
		++numGoodNew;
#endif

	}
	else
	{
		popBackDatum();
	}

	++passed[newType];


#if NEW_SUCC
	if(def::solved == def::solveType::right)
	{
		std::cout << "excluded inds : " << succOldIndices << std::endl;

		eraseData(succOldIndices);
		successiveRelearn();

		succOldIndices.clear();
		def::solved = def::solveType::notYet;
		findIt = std::begin(types);
	}
	else if(def::solved == def::solveType::wrong)
	{
		/// erase all last added
		std::cout << "size before: " << dataMatrix.size() << std::endl;
		eraseData(range<std::vector<int>>(3 * this->learnSetStay, dataMatrix.rows() - 1));
		std::cout << "size after: " << dataMatrix.size() << std::endl;

		succOldIndices.clear();
		def::solved = def::solveType::notYet;
		findIt = std::begin(types);
	}
#else
	if(numGoodNew == numGoodNewLimit)
	{
		successiveRelearn();
		numGoodNew = 0;
	}
#endif
}

void net::successiveRelearn()
{
	// decay weights
	const double rat = decayRate;
	for(int i = 0; i < dimensionality.size() - 1; ++i)
	{
		std::for_each(std::begin(weight[i]),
					  std::end(weight[i]),
					  [rat](std::valarray<double> & in)
		{
			in *= 1. - rat;
		});
	}
	learnNet(false); // relearn w/o weights reset
}

void net::readWts(const QString & fileName,
				  wtsType * wtsMatrix)
{
	std::ifstream wtsStr(fileName.toStdString());
	if(!wtsStr.good())
	{
		std::cout << "readWtsByName: wtsStr is not good() " << std::endl;
		return;
	}
	if(wtsMatrix == nullptr)
	{
		wtsMatrix = &(this->weight);
	}
	else
	{
		(*wtsMatrix).resize(dimensionality.size() - 1);
		for(int i = 0; i < dimensionality.size() - 1; ++i)
		{
			(*wtsMatrix)[i].resize(dimensionality[i + 1]);
			for(int j = 0; j < dimensionality[i + 1]; ++j)
			{
				(*wtsMatrix)[i][j].resize(dimensionality[i] + 1);
			}
		}
	}

	for(int i = 0; i < dimensionality.size() - 1; ++i)
	{
		for(int j = 0; j < dimensionality[i + 1]; ++j)
		{
			for(int k = 0; k < dimensionality[i] + 1; ++k)
			{
				wtsStr >> (*wtsMatrix)[i][j][k];
			}
		}
	}
	wtsStr.close();
}

void net::leaveOneOutClassification()
{
	if(this->Source == source::pca)
	{
#if 0
		std::ofstream outStr;
		outStr.open((this->workPath
					 + "/" + "pcaRes.txt"));
		// auto pca classification
		for(int i = ui->autoPCAMaxSpinBox->value();
			i >= ui->autoPCAMinSpinBox->value();
			i -= ui->autoPCAStepSpinBox->value())
		{
			std::cout << "numOfPc = " << i  << " \t";
			dataMatrix.resizeCols(i);

			adjustLearnRate(this->lowLimit,
							this->highLimit);

			leaveOneOut();
			outStr << i << "\t" << averageAccuracy << std::endl;
		}
		outStr.close();
#endif
	}
	else
	{
		std::cout << "Net: autoclass (max " << dataMatrix.rows() << "):" << std::endl;
		leaveOneOut();
	}

	return;
}

void net::crossClassification()
{
	std::vector<std::vector<int> > arr;
	arr.resize(def::numOfClasses(), std::vector<int>());
	for(int i = 0; i < dataMatrix.rows(); ++i)
	{
		arr[ types[i] ].push_back(i);
	}
	std::cout << "Net: autoclass (max " << this->numOfPairs << "):" << std::endl;

	for(int i = 0; i < this->numOfPairs; ++i)
	{
		std::cout << i + 1;
		std::cout << " "; std::cout.flush();

		// mix arr for one "pair"-iteration
		for(int i = 0; i < def::numOfClasses(); ++i)
		{
			unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
			std::shuffle(std::begin(arr[i]),
						 std::end(arr[i]),
						 std::default_random_engine(seed));
		}

		/// new with indices
		for(int numFold = 0; numFold < this->folds; ++numFold)
		{
			std::pair<std::vector<int>, std::vector<int> > sets
					= makeIndicesSetsCross(arr, numFold);
			learnNetIndices(sets.first);
			tallNetIndices(sets.second);
		}
	}
	std::cout << std::endl;
	std::cout << "cross classification - ";
	averageClassification();
}

void net::halfHalfClassification()
{
	std::vector<int> learnIndices;
	std::vector<int> tallIndices;

	for(int i = 0; i < dataMatrix.rows() / 2; ++i)
	{
		learnIndices.push_back(i);
		tallIndices.push_back(i + dataMatrix.rows() / 2);
	}
	if(learnIndices.empty() || tallIndices.empty())
	{
		std::cout << "trainTest: indicesArray empty, return" << std::endl;
		return;
	}
	learnNetIndices(learnIndices);
	tallNetIndices(tallIndices);
	std::cout << "half-half classification - ";
	averageClassification();
}

void net::trainTestClassification(const QString & trainTemplate,
								  const QString & testTemplate)
{
	std::vector<int> learnIndices;
	std::vector<int> tallIndices;
	for(int i = 0; i < dataMatrix.rows(); ++i)
	{
		if(fileNames[i].contains(trainTemplate))
		{
			learnIndices.push_back(i);
		}
		if(fileNames[i].contains(testTemplate))
		{
			tallIndices.push_back(i);
		}
	}
	if(learnIndices.empty() || tallIndices.empty())
	{
		std::cout << "teainTest: indicesArray empty, return" << std::endl;
		return;
	}
	learnNetIndices(learnIndices);
	tallNetIndices(tallIndices);
	std::cout << "train-test classification - ";
	averageClassification();
}

void net::leaveOneOut()
{
	std::vector<int> learnIndices;
	int i = 0;
	while(i < dataMatrix.rows())
	{
		std::cout << i + 1;
		std::cout << " "; std::cout.flush();

		/// iota ?
		learnIndices.clear();
		for(int j = 0; j < dataMatrix.rows(); ++j)
		{
			if(j == i) continue;
			learnIndices.push_back(j);
		}
		learnNetIndices(learnIndices);
		tallNetIndices({i});

		/// not so fast
		/// what with softmax/logistic ?
#if 0
		if(tallCleanFlag && epoch < this->lowLimit)
		{
			adjustLearnRate(this->lowLimit,
							this->highLimit);
		}
#endif
		++i;

	}
	std::cout << std::endl;
	std::cout << "N-fold cross-validation:" << std::endl;
	averageClassification();
}


void net::pushBackDatum(const std::valarray<double> & inDatum,
						const int & inType,
						const QString & inFileName)
{
	dataMatrix.push_back(inDatum);
	classCount[inType] += 1.;
	types.push_back(inType);
	fileNames.push_back(inFileName);
}

void net::popBackDatum()
{
	dataMatrix.pop_back();
	classCount[types.back()] -= 1.;
	types.pop_back();
	fileNames.pop_back();
}

void net::eraseDatum(const int & index)
{
	dataMatrix.eraseRow(index);
	classCount[ types[index] ] -= 1.;
	types.erase(std::begin(types) + index);
	fileNames.erase(std::begin(fileNames) + index);
}

void net::eraseData(const std::vector<int> & indices)
{
	dataMatrix.eraseRows(indices);
	eraseItems(fileNames, indices);

	for(int index : indices)
	{
		classCount[ types[index] ] -= 1.; /// doubling indices not prevented
	}

	eraseItems(types, indices);
}


// like readPaFile from library.cpp
void net::loadData(const QString & spectraPath,
				   const QStringList & filters)
{
	std::vector<QStringList> leest;
	makeFileLists(spectraPath, leest, filters);

	dataMatrix = matrix();
	classCount.resize(def::numOfClasses(), 0.);
	types.clear();
	fileNames.clear();

	std::valarray<double> tempArr;
	for(int i = 0; i < leest.size(); ++i)
	{
		classCount[i] = 0.;
		for(QString fileName : leest[i])
		{

			readFileInLine(spectraPath + "/" + fileName,
						   tempArr);


			pushBackDatum(tempArr, i, fileName);
		}
	}
#if 1
	averageDatum = dataMatrix.averageRow();
	for(int i = 0; i < dataMatrix.rows(); ++i)
	{
		dataMatrix[i] -= averageDatum;
	}
	dataMatrix.transpose();
	sigmaVector.resize(dataMatrix.rows());
	for(int i = 0; i < dataMatrix.rows(); ++i)
	{
		sigmaVector[i] = sigma(dataMatrix[i]);
		if(sigmaVector[i] != 0.)
		{
			dataMatrix[i] /= sigmaVector[i] * loadDataNorm;
		}
	}

	dataMatrix.transpose();
#endif
}

void net::learnNet(const bool resetFlag)
{
	std::vector<int> mixNum(dataMatrix.rows());
	std::iota(std::begin(mixNum), std::end(mixNum), 0);
	learnNetIndices(mixNum, resetFlag);
}

void net::learnNetIndices(std::vector<int> mixNum,
						  const bool resetFlag)
{

	if(resetFlag)
	{
		reset();
	}

	const int numOfLayers = dimensionality.size();
	std::vector<std::valarray<double> > deltaWeights(numOfLayers);
	std::vector<std::valarray<double> > output(numOfLayers);
	for(int i = 0; i < numOfLayers; ++i)
	{
		deltaWeights[i].resize(dimensionality[i]); // fill zeros
		output[i].resize(dimensionality[i] + 1); // for biases
	}

	double currentError = this->errCrit + 0.1;
	int type = 0;

	/// edit due to Indices
	std::vector<int> localClassCount(def::numOfClasses(), 0);
	for(int index : mixNum)
	{
		++localClassCount[types[index]];
	}
	const double helpMin = *std::min_element(std::begin(localClassCount),
											 std::end(localClassCount));
	std::vector<double> normCoeff;
	for(uint i = 0; i < def::numOfClasses(); ++i)
	{
		normCoeff.push_back(helpMin / double(localClassCount[i]));
	}

	epoch = 0;
	while(currentError > this->errCrit && epoch < this->maxEpoch)
	{
		/// mix the sequence of input vectors
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle(std::begin(mixNum), std::end(mixNum),
					 std::default_random_engine(seed));


		currentError = 0.;

		for(const int & index : mixNum)
		{
			/// add data
			std::copy(std::begin(dataMatrix[index]),
					  std::end(dataMatrix[index]),
					  std::begin(output[0]));


			/// add bias
			output[0][output[0].size() - 1] = 1.;
			type = types[index];

			/// obtain outputs
			for(int i = 1; i < numOfLayers; ++i)
			{
				for(int j = 0; j < dimensionality[i]; ++j)
				{
					output[i][j] = prod(weight[i - 1][j], output[i-1]); // bias included
				}
				output[i] = activation(output[i], temp);

				output[i][ dimensionality[i] ] = 1.; //bias, unused for the highest layer
			}

			/// error in the last layer
			{
				double err = 0.;
				for(int j = 0; j < dimensionality.back(); ++j)
				{
					err += pow((output.back()[j] - int(type == j) ), 2.);
				}
				err = sqrt(err);
				if(def::errType == def::errorNetType::SME)
				{
					currentError += err;
				}
				else if(def::errType == def::errorNetType::maxDist)
				{
					currentError = std::max(err, currentError);
				}
			}

#if 0
			/// check weight
			if(!deltaFlag) /// enum !
			{
				//count deltaweights (used for backprop only)
				//for the last layer
				for(int j = 0; j < dimensionality[numOfLayers-1]; ++j)
				{
					deltaWeights[numOfLayers-1][j] = -1. / temp
													 * output[numOfLayers-1][j]
													 * (1. - output[numOfLayers-1][j])
							* ((type == j) - output[numOfLayers-1][j]); //~0.1
				}

				//for the other layers, besides the input one, upside->down
				for(int i = numOfLayers - 2; i >= 1; --i)
				{
					for(int j = 0; j < dimensionality[i] + 1; ++j) //+1 for bias
					{
						deltaWeights[i][j] = 0.;
						for(int k = 0; k < dimensionality[i + 1]; ++k) //connected to the higher-layer
						{
							deltaWeights[i][j] += deltaWeights[i + 1][k] * weight[i][j][k];
						}
						deltaWeights[i][j] *= 1. / temp
											  * output[i][j]
											  * (1. - output[i][j]);
					}
				}
			}
#endif



			// numOfLayers = 2 and i == 0 in this case
			// simplified
			for(int j = 0; j < def::numOfClasses(); ++j)
			{
				weight[0][j] += output[0]
						* (learnRate * normCoeff[type]
						   * ((type == j) - output[1][j])
						//                        * (output[1][j] * (1. - output[1][j])) * 6. // derivative
						);
			}


#if 0
			else /// backprop check weight
			{

				// count new weights using deltas
				for(int i = 0; i < numOfLayers - 1; ++i)
				{
					for(int j = 0; j < dimensionality[i] + 1; ++j) // +1 for bias? 01.12.2014
					{
						for(int k = 0; k < dimensionality[i + 1]; ++k)
						{
							weight[i][j][k] -= learnRate
											   * normCoeff[type]
											   * output[i][j]
											   * deltaWeights[i + 1][k];
						}
					}
				}
			}
#endif
		}
		++epoch;
		if(def::errType == def::errorNetType::SME)
		{
			currentError /= mixNum.size();
		}
	}
	//    std::cout << "epoch = " << epoch << "\terror = " << doubleRound(currentError, 4) << std::endl;

	writeWts();

	//    printData();
}


std::pair<int, double> net::classifyDatum(const int & vecNum)
{
	const int type = types[vecNum]; // true class
	const int numOfLayers = dimensionality.size();

	std::vector<std::valarray<double> > output(numOfLayers);
	output[0].resize(dimensionality[0] + 1); // +1 for biases

	std::copy(std::begin(dataMatrix[vecNum]),
			  std::end(dataMatrix[vecNum]),
			  std::begin(output[0]));

	output[0][output[0].size() - 1] = 1.; //bias

	for(int i = 1; i < numOfLayers; ++i)
	{
		output[i].resize(dimensionality[i] + 1);
		for(int j = 0; j < dimensionality[i]; ++j) //higher level, w/o bias
		{
			output[i][j] = prod(weight[i-1][j], output[i-1]); // bias included
		}
		output[i] = activation(output[i], temp);
		output[i][ dimensionality[i] ] = 1.; //bias, unused for the highest layer
	}

	resizeValar(output.back(), def::numOfClasses());
	int outClass = indexOfMax(output.back());

	/// effect on successive procedure
	double res = 0.;
	for(int j = 0; j < dimensionality.back(); ++j)
	{
		res += pow((output.back()[j] - int(type == j) ), 2.);
	}
	res = sqrt(res);

#if VERBOSE_OUTPUT >= 1
	/// std::cout results

	std::ofstream resFile;
	resFile.open((def::workPath + "/class.txt").toStdString(),
				 std::ios_base::app);
	//    auto tmp = std::cout.rdbuf();
	//    cout.rdbuf(resFile.rdbuf());


	std::cout << "type = " << type << '\t' << "(";
	for(int i = 0; i < def::numOfClasses(); ++i)
	{
		std::cout << doubleRound(output.back()[i], 3) << '\t';
	}
	std::cout << ") " << fileNames[vecNum] << "   ";
	std::cout << ((type == outClass) ? "+" : "-");
	std::cout << "\t" << doubleRound(res, 2);
	std::cout << std::endl;


	//    cout.rdbuf(tmp);
	resFile.close();
#endif

	return std::make_pair(outClass,
						  res);
}

