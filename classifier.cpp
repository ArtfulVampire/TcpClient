#include "classifier.h"

using namespace std;


net::net(QObject * par)
{
    this->setParent(par);
    confusionMatrix = matrix(def::numOfClasses(), def::numOfClasses(), 0.);

    readMatrixFile(def::eyesFilePath,
                   coeff); // num eog channels

    comPort = new QSerialPort(this);
    comPort->setPortName(def::comPortName);
    comPort->open(QIODevice::WriteOnly);
    comPortDataStream.setDevice(comPort);
    if(comPort->isOpen())
    {
        cout << "serialPort opened: " + def::comPortName << endl;
        cout << "portName: " << comPort->portName().toStdString() << endl;
        cout << "dataBits: " << comPort->dataBits() << endl;
        cout << "baudRate: " << comPort->baudRate() << endl;
        cout << "dataTerminalReady: " << comPort->isDataTerminalReady() << endl;
        cout << "flowControl: " << comPort->flowControl() << endl;
        cout << "requestToSend: " << comPort->isRequestToSend() << endl;
        cout << "stopBits: " << comPort->stopBits() << endl << endl;
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
    cout << "classifier waits for work" << endl;
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
#if CPP_11
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::iota(mixNum.begin(), mixNum.end(), 0);
        std::shuffle(mixNum.begin(), mixNum.end(),
                     std::default_random_engine(seed));
#else
        myIota(mixNum);
        myShuffle(mixNum);

#endif
        mixNum.resize(mixNum.size() * (folds - 1) / folds);
    }
    else if(this->Mode == myMode::half_half)
    {
        mixNum.resize(dataMatrix.rows() / 2);
#if CPP_11
        std::iota(mixNum.begin(), mixNum.end(), 0);
#else
        myIota(mixNum);
#endif
    }
    return mixNum;
}


double net::adjustLearnRate(int lowLimit,
                            int highLimit)
{
    cout << "adjustLearnRate: start" << endl;

    const std::vector<int> mixNum = makeLearnIndexSet();

    double res = 1.;
    int counter = 0;
    do
    {
        /// remake with indices
        const double currVal = this->learnRate;
        cout << "lrate = " << currVal << '\t';

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
    cout << endl;
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
    cout << "size of learning set:" << dataMatrix.rows() << " = ";
    cout << std::count(types.begin(), types.end(), 0) << " + ";
    cout << std::count(types.begin(), types.end(), 1) << " + ";
    cout << std::count(types.begin(), types.end(), 2) << endl;
    cout << "OR = ";
    cout << classCount[0] << " + ";
    cout << classCount[1] << " + ";
    cout << classCount[2] << endl;

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

double net::getReduceCoeff()
{
    return this->rdcCoeff;
}

int net::getEpoch()
{
    return this->epoch;
}

//void net::setReduceCoeff(double coeff)
//{
//    this->ui->rdcCoeffSpinBox->setValue(coeff);
//}

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
    res << def::ExpName.toStdString() << endl;
    res.close();

    confusionMatrix.print();
    cout << "average accuracy = " << doubleRound(averageAccuracy, 2) << endl;
    cout << "kappa = " << kappa << endl;
}

#if DRAWS
void net::drawWts(std::string wtsPath,
                  std::string picPath)  //generality
{
    if( dimensionality.size() != 2 ) return;

    if(!fileExists(wtsPath))
    {
        wtsPath = this->workPath
                  + slash() + this->ExpName + ".wts";
        if(!fileExists(wtsPath))
        {
            cout << "drawWts: bad filePath" << endl;
            return;
        }
    }
    wtsType tempWeights;
    readWtsByName(wtsPath, &tempWeights);

    matrix drawWts; // 3 arrays of weights
#if 0
    vec tempVec;
    for(int i = 0; i < def::numOfClasses(); ++i)
    {
        tempVec.clear();
        for(int j = 0; j < dataMatrix.cols(); ++j)
        {
            tempVec.push_back(tempWeights[0][j][i]); // 0 is for 2 layers
        }
        drawWts.push_back(tempVec);
    }
#else
    drawWts = tempWeights[0];
    drawWts.resizeCols(drawWts.cols() - 1); // fck the bias?
#endif

    if(picPath.empty())
    {
        picPath = wtsPath;
        picPath.replace(".wts", "_wts.jpg"); /// make default suffixes
    }
    drawTemplate(picPath);
    drawArrays(picPath,
               drawWts,
               true);
}
#endif


void net::writeWts(const QString & outPath)
{
    if(weight.size() != 1) return;
    static int wtsCounter = 0;
    QString wtsPath;
    cout << "writeWts: " << wtsCounter << endl;
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
            // resizing lineType -> fill zeros
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
            badFilesStr << fileNames[ indices[i] ] << endl;
            if(tallCleanFlag)
            {
                QFile::remove(def::spectraPath +
                              qslash() + fileNames[ indices[i] ]);
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

    logStream << doubleRound(averageAccuracy, 2) << endl;
    logStream.close();
}




void net::successivePreclean(const QString & spectraPath)
{
    QStringList leest;
    makeFullFileList(spectraPath, leest, {"*train*"});
    // clean from first 2 winds
    cout << "clean first 2 winds" << endl;
    for(auto str : leest)
    {
        if(str.endsWith(".00") || str.endsWith(".01"))
        {
            QFile::remove(spectraPath + qslash() + str);
        }
    }

    // clean by learnSetStay
    cout << "clean by learnSetStay" << endl;
    vector<QStringList> leest2;
    makeFileLists(spectraPath, leest2);

    for(int j = 0; j < def::numOfClasses(); ++j)
    {
        auto it = leest2[j].begin();
        for(int i = 0;
            i < leest2[j].size() - learnSetStay * 1.3; /// consts generality
            ++i, ++it)
        {
            QFile::remove(spectraPath + qslash() + (*it));
        }
    }
    Source = source::winds;
    Mode = myMode::N_fold;

    // N-fold cleaning
    cout << "N-fold cleaning" << endl;
    tallCleanFlag = true;
    for(int i = 0; i < 3; ++i)
    {
        autoClassification(spectraPath);
    }
    tallCleanFlag = false;
}





void net::successiveProcessing(const QString & spectraPath)
{


#if CPP_11
    std::vector<int> eraseIndices{};
#else
    std::vector<int> eraseIndices;
#endif

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
        QFile::remove(spectraPath + qslash() + name);
    }

    /// load
    loadData(spectraPath, {".ps"});
    /// reduce learning set to (NumClasses * suc::learnSetStay)
    cout << "reduce learning set" << endl;
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
//        cout << in << endl;
//    }
//    exit(0);

    /// load newest weights ???

    /// consts
    errCrit = 0.05;
    learnRate = 0.005;

    adjustLearnRate(this->lowLimit,
                    this->highLimit);

    cout << "get initial weights on train set" << endl;
    learnNet();

    errCrit = 0.02;
    learnRate = 0.02;

    cout << "successive: initial learn done" << endl;
    cout << "successive itself" << endl;

#if OFFLINE_SUCCESSIVE
    lineType tempArr;
    int type = -1;
    QStringList leest = QDir(spectraPath).entryList({'*' + testMarker + '*'}); /// special generality
    for(const QString & fileNam : leest)
    {
        readFileInLine(spectraPath + qslash() + fileNam,
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

    lineType newSpectre = successiveDataToSpectre(a, b);

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

lineType net::successiveDataToSpectre(
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
            auto itt = (*it).begin();
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
        auto it2 = tmpMat.begin();
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
	lineType res(0., def::eegNs * def::spLength());
    /// count spectra, take 5-20 HZ only
    {
		lineType tmpSpec(def::spLength());
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

void net::successiveLearning(const lineType & newSpectre,
                             const int newType,
                             const QString & newFileName)
{
    /// dataMatrix is learning matrix
    lineType newData = (newSpectre - averageDatum) / (sigmaVector * loadDataNorm);

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
        const int num = std::find(types.begin(),
                                  types.end(),
                                  newType)
                - types.begin();
        eraseDatum(num);
        ++numGoodNew;
    }
    else
    {
        popBackDatum();
    }
    ++passed[newType];

    if(numGoodNew == numGoodNewLimit)
    {
        successiveRelearn();
        numGoodNew = 0;
    }
}

void net::successiveRelearn()
{
    // decay weights
    const double rat = decayRate;
    for(int i = 0; i < dimensionality.size() - 1; ++i)
    {
        std::for_each(weight[i].begin(),
                      weight[i].end(),
                      [rat](lineType & in)
        {
            in *= 1. - rat;
        });
    }
    learnNet(false); // relearn w/o weights reset
}

void net::readWts(const std::string & fileName,
                  wtsType * wtsMatrix)
{
    std::ifstream wtsStr(fileName.c_str());
    if(!wtsStr.good())
    {
        cout << "readWtsByName: wtsStr is not good() " << endl;
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
        ofstream outStr;
        outStr.open((this->workPath
                    + slash() + "pcaRes.txt"));
        // auto pca classification
        for(int i = ui->autoPCAMaxSpinBox->value();
            i >= ui->autoPCAMinSpinBox->value();
            i -= ui->autoPCAStepSpinBox->value())
        {
            cout << "numOfPc = " << i  << " \t";
            dataMatrix.resizeCols(i);

            adjustLearnRate(this->lowLimit,
                            this->highLimit);

            leaveOneOut();
            outStr << i << "\t" << averageAccuracy << endl;
        }
        outStr.close();
#endif
    }
    else
    {
        cout << "Net: autoclass (max " << dataMatrix.rows() << "):" << endl;
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
    cout << "Net: autoclass (max " << this->numOfPairs << "):" << endl;

    for(int i = 0; i < this->numOfPairs; ++i)
    {
        cout << i + 1;
        cout << " "; cout.flush();

        // mix arr for one "pair"-iteration
        for(int i = 0; i < def::numOfClasses(); ++i)
        {
#if CPP_11
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::shuffle(arr[i].begin(),
                         arr[i].end(),
                         std::default_random_engine(seed));
#else
            myShuffle(arr[i]);
#endif
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
    cout << endl;
    cout << "cross classification - ";
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
        cout << "trainTest: indicesArray empty, return" << endl;
        return;
    }
    learnNetIndices(learnIndices);
    tallNetIndices(tallIndices);
    cout << "half-half classification - ";
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
        cout << "teainTest: indicesArray empty, return" << endl;
        return;
    }
    learnNetIndices(learnIndices);
    tallNetIndices(tallIndices);
    cout << "train-test classification - ";
    averageClassification();
}

void net::leaveOneOut()
{
    std::vector<int> learnIndices;
    int i = 0;
    while(i < dataMatrix.rows())
    {
        cout << i + 1;
        cout << " "; cout.flush();

        /// iota ?
        learnIndices.clear();
        for(int j = 0; j < dataMatrix.rows(); ++j)
        {
            if(j == i) continue;
            learnIndices.push_back(j);
        }
        learnNetIndices(learnIndices);
#if CPP_11
        tallNetIndices({i});
#else
        tallNetIndices(std::vector<int>(1, i));
#endif

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
    cout << endl;
    cout << "N-fold cross-validation:" << endl;
    averageClassification();
}


void net::pushBackDatum(const lineType & inDatum,
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
    types.erase(types.begin() + index);
    fileNames.erase(fileNames.begin() + index);
}

void net::eraseData(const std::vector<int> & indices)
{
    dataMatrix.eraseRows(indices);
    eraseItems(fileNames, indices);
#if CPP_11
    for(int index : indices)
    {
        classCount[ types[index] ] -= 1.;
    }
#else
    for(std::vector<int>::const_iterator i = indices.begin();
        i != indices.end();
        ++i)
    {
        classCount[ types[*i] ] -= 1.;
    }
#endif
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

    lineType tempArr;
    for(int i = 0; i < leest.size(); ++i)
    {
        classCount[i] = 0.;
        for(QString fileName : leest[i])
        {

            readFileInLine(spectraPath + qslash() + fileName,
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
    std::iota(mixNum.begin(), mixNum.end(), 0);
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
        std::shuffle(mixNum.begin(),
                     mixNum.end(),
                     std::default_random_engine(seed));


        currentError = 0.;

        for(const int & index : mixNum)
        {
            /// add data
            std::copy(begin(dataMatrix[index]),
                      end(dataMatrix[index]),
                      begin(output[0]));


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
                if(def::errType == errorNetType::SME)
                {
                    currentError += err;
                }
                else if(def::errType == errorNetType::maxDist)
                {
                    currentError = max(err, currentError);
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
        if(def::errType == errorNetType::SME)
        {
            currentError /= mixNum.size();
        }
    }
//    cout << "epoch = " << epoch << "\terror = " << doubleRound(currentError, 4) << endl;

    writeWts();

//    printData();
}

double net::errorNet()
{
//    double tmp;
//    for(int j = 0; j < dimensionality.back(); ++j)
//    {
//        tmp = pow((output.back()[j] - int(type == j) ), 2.);
//    }
//    if(activation == logistic)
//    {
//        err1 = sqrt(tmp);
//        currentError += err1;
//    }
//    if(activation == softmax)
//    {
//        err1 = max(err1, tmp);
//    }
//    return tmp;
}



std::pair<int, double> net::classifyDatum(const int & vecNum)
{
    const int type = types[vecNum]; // true class
    const int numOfLayers = dimensionality.size();

    std::vector<std::valarray<double> > output(numOfLayers);
    output[0].resize(dimensionality[0] + 1); // +1 for biases

#if CPP_11
    std::copy(std::begin(dataMatrix[vecNum]),
              std::end(dataMatrix[vecNum]),
              std::begin(output[0]));
#else
    output[0] = dataMatrix[vecNum];
#endif


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
    /// cout results

    std::ofstream resFile;
    resFile.open((def::workPath + "/class.txt").toStdString(),
                 ios_base::app);
//    auto tmp = std::cout.rdbuf();
//    cout.rdbuf(resFile.rdbuf());


    cout << "type = " << type << '\t' << "(";
    for(int i = 0; i < def::numOfClasses(); ++i)
    {
        cout << doubleRound(output.back()[i], 3) << '\t';
    }
    cout << ") " << fileNames[vecNum] << "   ";
    cout << ((type == outClass) ? "+" : "-");
    cout << "\t" << doubleRound(res, 2);
    cout << endl;


//    cout.rdbuf(tmp);
    resFile.close();
#endif
    return std::make_pair(outClass,
                          res);
}

