#include "classifier.h"

using namespace std;

/// defaults???

net::net(QObject * par)
{
    this->setParent(par);
    confusionMatrix = matrix(this->numOfClasses, this->numOfClasses, 0.);
//    workPath = "/media/Files/Data/RealTime";
}

net::~net()
{
}

void net::startOperate()
{
    ExpName = "GAS_train";
    successivePreclean(enc::spectraPath.toStdString());


    ExpName = "GAS_test";
    Mode = myMode::train_test;
    successiveProcessing(enc::spectraPath.toStdString());
    cout << "classifier waits for work" << endl;

}

std::vector<int> net::makeLearnIndexSet()
{
    std::vector<int> mixNum;
    if(this->Mode == myMode::train_test)
    {
        for(int i = 0; i < fileNames.size(); ++i)
        {
            if(contains(fileNames[i], "_train"))
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
        if(this->learnRate < 0.005)
        {
            this->learnRate = (0.005); break;
        }
        else if(this->learnRate >= 1.)
        {
            this->learnRate = (1.); break;
        }
        ++counter;
    } while (counter < 15);
    return res;
}

//void net::autoClassificationSimple()
//{

//    std::string helpString;
//    helpString = this->workPath + slash() + "SpectraSmooth";

//    if(this->Source == source::winds) //generality
//    {
//        helpString += slash() + "windows";
//    }
//    else if(this->Source == source::bayes)
//    {
//        helpString += slash() + "Bayes";
//    }
//    else if(this->Source == source::pca)
//    {
//        helpString += slash() + "PCA";
//    }
//    if(!helpString.empty()) autoClassification(helpString);
//}


std::pair<std::vector<int>, std::vector<int> > net::makeIndicesSetsCross(
        const std::vector<std::vector<int> > & arr,
        const int numOfFold)
{
    std::vector<int> learnInd;
    std::vector<int> tallInd;

    const int fold = this->folds;

    for(int i = 0; i < this->numOfClasses; ++i)
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

void net::autoClassification(const std::string & spectraDir)
{
//    std::string helpString = this->workPath + slash() + "log.txt";
//    remove(helpString.c_str());
    QFile::remove(enc::netLogPath);

    loadData(spectraDir);

#if 0
        adjustLearnRate(this->lowLimit,
                        this->highLimit); // or reduce coeff ?
#endif


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
//    cout <<  "AutoClass: time elapsed = " << myTime.elapsed()/1000. << " sec" << endl;
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

//    std::string helpString = this->workPath + slash() + "results.txt";
    std::ofstream res;
    res.open(enc::netResPath.toStdString(), std::ios_base::app);

    for(int i = 0; i < this->numOfClasses; ++i)
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

    for(int i = 0; i < this->numOfClasses; ++i)
    {
        corrSum += confusionMatrix[i][i];
        wholeNum += confusionMatrix[i].sum();
    }
    averageAccuracy = corrSum * 100. / wholeNum;

    // kappa
    double pE = 0.; // for Cohen's kappa
    const double S = confusionMatrix.sum();
    for(int i = 0; i < this->numOfClasses; ++i)
    {
        pE += (confusionMatrix[i].sum() * confusionMatrix.getCol(i).sum()) /
              (S * S);
    }
    kappa = 1. - (1. - corrSum / wholeNum) / (1. - pE);

    res << doubleRound(averageAccuracy, 2) << '\t';
    res << doubleRound(kappa, 3) << '\t';
    res << this->ExpName << endl;
    res.close();

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
    twovector<lineType> tempWeights;
    readWtsByName(wtsPath, &tempWeights);

    matrix drawWts; // 3 arrays of weights
#if 0
    vec tempVec;
    for(int i = 0; i < this->numOfClasses; ++i)
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


void net::writeWts(const std::string & wtsPath)
{
    std::ofstream weightsFile;
    weightsFile.open(wtsPath.c_str());

    if(!weightsFile.good())
    {
        cout << "saveWts: cannot open file = " << wtsPath << endl;
        return;
    }

    for(int i = 0; i < dimensionality.size() - 1; ++i) // numOfLayers
    {
        for(int j = 0; j < dimensionality[i + 1]; ++j) // NetLength+1 for bias
        {
            for(int k = 0; k < dimensionality[i] + 1; ++k) // NumOfClasses
            {
                weightsFile << weight[i][j][k] << '\n';
            }
            weightsFile << '\n';
        }
        weightsFile << '\n';
    }
    weightsFile.close();
}

void net::reset()
{
    const int numOfLayers = 2;
    dimensionality.resize(numOfLayers);
    dimensionality[0] = this->dataMatrix.cols();
    dimensionality[1] = this->numOfClasses;

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
//    std::string helpString = this->workPath + slash() + "badFiles.txt";
    std::ofstream badFilesStr;
    badFilesStr.open(enc::netBadPath.toStdString(), std::ios_base::app);

    matrix localConfusionMatrix(this->numOfClasses, this->numOfClasses);
    for(int i = 0; i < indices.size(); ++i)
    {
        const int outClass = classifyDatum(indices[i]).first;
        if(types[ indices[i] ] != outClass )
        {
            badFilesStr << fileNames[ indices[i] ] << endl;
            if(tallCleanFlag)
            {
                remove((enc::spectraPath.toStdString() +
                       slash() + fileNames[ indices[i] ]).c_str());

                /// pewpew
                if(this->Source == source::reals)
                {
//                    remove((this->workPath
//                            + slash() + "SpectraSmooth"
//                            + slash() + fileNames[ indices[i] ]).c_str());
                }
                else if(this->Source == source::winds)
                {
//                    remove((this->workPath
//                            + slash() + "SpectraSmooth"
//                            + slash() + "windows"
//                            + slash() + fileNames[ indices[i] ]).c_str());
                }
                eraseDatum(indices[i]);
            }

        }
        localConfusionMatrix[ types[ indices[i] ] ][ outClass ] += 1.;
        confusionMatrix[ types[ indices[i] ] ][ outClass ] += 1.;
    }
    badFilesStr.close();


//    helpString = this->workPath + slash() + "log.txt";
    std::ofstream logStream;
//    logStream.open(helpString.c_str(), std::ios_base::app);
    logStream.open(enc::netLogPath.toStdString(), std::ios_base::app);

    for(int i = 0; i < this->numOfClasses; ++i)
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
    for(int i = 0; i < this->numOfClasses; ++i)
    {
        corrSum += localConfusionMatrix[i][i];
    }
    averageAccuracy = corrSum / indices.size() * 100.;

    logStream << doubleRound(averageAccuracy, 2) << endl;
    logStream.close();
}




void net::successivePreclean(const std::string & spectraPath)
{
    std::vector<std::vector<std::string> > leest = contents(spectraPath,
                                                            makeDefFilters());
    // clean from first 2 winds
    cout << "clean from first 2 winds" << endl;

    for(int i = 0; i < leest.size(); ++i)
    {
        for(int j = 0; j < leest[i].size(); ++j)
        {
//            cout << leest[i][j] << endl; continue;
            if(contains(leest[i][j], "_train") && // generality train
               (endsWith(leest[i][j], ".00") ||
                endsWith(leest[i][j], ".01")))
            {
//                cout << leest[i][j] << endl; continue;
                remove(leest[i][j].c_str());
            }
        }
    }

    // cleen by learnSetStay
    cout << "cleen by learnSetStay" << endl;
    std::vector<std::string> leest2 = contents(spectraPath, "_train"); // all train files
    cout << leest2.size() << endl;

//    emit finish();
    return;

    for(int i = 0; i < leest2.size() - learnSetStay * numOfClasses * 2; ++i)
    {
//        remove(leest2[i].c_str());
    }
    Source = source::winds;
    Mode = myMode::N_fold;

    // N-fold cleaning
    cout << "N-fold cleaning" << endl;
    tallCleanFlag = true;
    for(int i = 0; i < 1; ++i)
    {
        this->autoClassification(spectraPath);
    }
    tallCleanFlag = false;
}


#if CPP_11
std::vector<int> exIndices{};
#else
std::vector<int> exIndices;
#endif
int numGoodNew;

void net::successiveProcessing(const std::string & spectraPath)
{


#if CPP_11
    std::vector<int> eraseIndices{};
#else
    std::vector<int> eraseIndices;
#endif

    numGoodNew = 0;
    confusionMatrix.fill(0.);
    exIndices.clear();

    /// check for no test items
    cout << "check for no test items" << endl;
    loadData(spectraPath);
    for(int i = 0; i < dataMatrix.rows(); ++i)
    {
        if(contains(fileNames[i], "_test"))
        {
            eraseIndices.push_back(i);
        }
    }
    eraseData(eraseIndices);
    eraseIndices.clear();



    /// reduce learning set to (NumClasses * suc::learnSetStay)
    /// move to preclean?
    cout << "reduce learning set AGAIN" << endl;
    std::vector<double> count = classCount;
    for(int i = 0; i < dataMatrix.rows(); ++i)
    {
        if(count[ types[i] ] > this->learnSetStay)
        {
            eraseIndices.push_back(i);
            count[ types[i] ] -= 1.;
        }
    }
    eraseData(eraseIndices);
    eraseIndices.clear();

    /// consts
    this->errCrit = 0.05;
    this->learnRate = 0.05;
    cout << "get initial weights on train set" << endl;
    learnNet(); /// get initial weights on train set
    this->errCrit = 0.01;
    this->learnRate = 0.01;

    cout << "successive: initial learn done" << endl;
    lineType tempArr;
    int type = -1;

#if CPP_11
    std::vector<std::string> leest = contents(spectraPath, "_test"); /// special generality
#else
    std::vector<std::string> leest = contents(spectraPath, "_test"); /// special generality
#endif



    cout << "successive itself" << endl;
    /// make slot
#if 0

#if CPP_11
    for(const std::string & filePath : leest)
    {
#else
    for(int i = 0; i < leest.size(); ++i)
    {
        const std::string & filePath = leest[i];
#endif

        readFileInLine(filePath, tempArr);
        type = typeOfFileName(filePath);

        int a = filePath.rfind(slash());
        std::string fileName = filePath.substr(a + 1, filePath.size() - a);

        successiveLearning(tempArr, type,
                           fileName);
    }
    averageClassification();
#endif
}

void net::dataCame(eegDataType::iterator a)
{
    lineType newSpectre = successiveDataToSpectre(a);

    std::string name = "pew";
//    int type = typeOfFileName(enc::currentName.toStdString());
//    successiveLearning(newSpectre, type, name);
}

lineType net::successiveDataToSpectre(
        const eegDataType::iterator eegDataStart)
{
    matrix tmpMat(enc::ns, enc::windowLength);
    auto it = eegDataStart;
    for(int j = 0; j < enc::windowLength; ++j, ++it)
    {
        for(int i = 0; i < enc::ns; ++i) // 19 EEG channels
        {
            tmpMat[i][j] = (*it)[i];
        }
    }
}

void net::successiveLearning(const lineType & newSpectre,
                             const int newType,
                             const std::string & newFileName)
{
    /// consider loaded wts
    /// dataMatrix is learning matrix


//    const double errorThreshold = 0.8; /// add to learning set or not - logistic
    const double errorThreshold = 0.5; /// add to learning set or not - softmax, 0.5 = take all

    lineType newData = (newSpectre - averageDatum) / (sigmaVector * loadDataNorm);

    pushBackDatum(newData, newType, newFileName);

    const std::pair<int, double> outType = classifyDatum(dataMatrix.rows() - 1); // take the last
    confusionMatrix[newType][outType.first] += 1.;
    if(outType.first == newType && outType.second < errorThreshold) /// if good coincidence
    {
        const int num = std::find(types.begin(), types.end(), newType) - types.begin();
        eraseDatum(num);
        ++numGoodNew;
    }
    else
    {
        popBackDatum();
    }

    if(numGoodNew == this->numGoodNewLimit)
    {
        successiveRelearn();
        numGoodNew = 0;
    }
}

void net::successiveRelearn()
{
    // decay weights
    const double rat = this->decayRate;
    for(int i = 0; i < dimensionality.size() - 1; ++i)
    {
#if CPP_11
        std::for_each(weight[i].begin(),
                      weight[i].end(),
                      [rat](lineType & in)
        {
            in *= 1. - rat;
        });
#else
        for(std::vector<lineType>::iterator it = weight[i].begin();
            it != weight[i].end();
            ++it)
        {
            (*it) *= 1. - rat;
        }
#endif
    }
    learnNet(false); // relearn w/o weights reset
}

void net::readWts(const std::string & fileName,
                  twovector<lineType> * wtsMatrix)
{
    std::ifstream wtsStr(fileName.c_str());
    if(!wtsStr.good())
    {
        cout << "readWtsByName: wtsStr is not good() " << endl;
        return;
    }
    if(wtsMatrix == NULL)
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
    arr.resize(this->numOfClasses, std::vector<int>());
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
        for(int i = 0; i < this->numOfClasses; ++i)
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
//            if(tallCleanFlag)
//            {
//                /// remake for k-fold tallCleanFlag
//                arr.resize(this->numOfClasses, {});
//                for(int i = 0; i < dataMatrix.rows(); ++i)
//                {
//                    arr[ types[i] ].push_back(i);
//                }
//                for(int i = 0; i < this->numOfClasses; ++i)
//                {
//                    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//                    std::shuffle(arr[i].begin(),
//                                 arr[i].end(),
//                                 std::default_random_engine(seed));
//                }
//            }

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
        cout << "teainTest: indicesArray empty, return" << endl;
        return;
    }
    learnNetIndices(learnIndices);
    tallNetIndices(tallIndices);
    cout << "half-half classification - ";
    averageClassification();
}

void net::trainTestClassification(const std::string & trainTemplate,
                                  const std::string & testTemplate)
{
    std::vector<int> learnIndices;
    std::vector<int> tallIndices;
    for(int i = 0; i < dataMatrix.rows(); ++i)
    {
        if(contains(fileNames[i], trainTemplate))
        {
            learnIndices.push_back(i);
        }
        if(contains(fileNames[i], testTemplate))
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
                      const std::string & inFileName)
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
void net::loadData(const std::string & spectraPath)
{
    std::vector<std::vector<std::string> > leest = contents(spectraPath, makeDefFilters());


    dataMatrix = matrix();
    classCount.resize(this->numOfClasses, 0.);
    types.clear();
    fileNames.clear();

    lineType tempArr;
//    cout << leest.size() << endl;
    for(int i = 0; i < leest.size(); ++i)
    {
//        cout << leest[i].size() << endl;
        classCount[i] = 0.;
#if CPP_11
        for(std::string fileName : leest[i])
        {
#else
        for(int j = 0; j < leest[i].size(); ++j)
        {
            std::string fileName = leest[i][j];
#endif
            /// generality path-name
            fileName = fileName.substr(fileName.rfind(slash()) + 1);

            readFileInLine(spectraPath + slash() + fileName,
                           tempArr);


            pushBackDatum(tempArr, i, fileName);
        }
    }
//    cout << "loadDataNorm = " << loadDataNorm << endl;
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
            // to equal variance, 10 for reals, 5 winds
            dataMatrix[i] /= sigmaVector[i] * loadDataNorm;
        }
    }
    dataMatrix.transpose();
#endif
#if 0
    // to range [-0.5; 0.5]
    dataMatrix.transpose();
    for(int i = 0; i < dataMatrix.rows(); ++i)
    {
        const double a = dataMatrix[i].max();
        const double b = dataMatrix[i].min();
        dataMatrix[i] -= (a + b) / 2.;
        dataMatrix[i] /= (a - b);
    }
    dataMatrix.transpose();

#endif
}

void net::learnNet(const bool resetFlag)
{
    std::vector<int> mixNum(dataMatrix.rows());
#if CPP_11
    std::iota(mixNum.begin(), mixNum.end(), 0);
#else
    myIota(mixNum);
#endif
    learnNetIndices(mixNum, resetFlag);
}

void net::learnNetIndices(std::vector<int> mixNum,
                          const bool resetFlag)
{

//    cout << "pew" << endl;
    if(resetFlag)
    {
        reset();
    }

//    cout << "pewpew" << endl;

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
    std::vector<double> normCoeff;
    const double helpMin = *std::min_element(classCount.begin(),
                                             classCount.end());
    for(int i = 0; i < this->numOfClasses; ++i)
    {
        normCoeff.push_back(helpMin / classCount[i]);
    }
    epoch = 0;
    while(currentError > this->errCrit && epoch < this->maxEpoch)
    {
#if CPP_11
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        // mix the sequence of input vectors
        std::shuffle(mixNum.begin(),
                     mixNum.end(),
                     std::default_random_engine(seed));
#else
        myShuffle(mixNum);
#endif


#if CPP_11
        for(const int & index : mixNum)
        {
#else
        for(std::vector<int>::iterator it = mixNum.begin();
            it != mixNum.end();
            ++it)
        {
            const int index = *it;
#endif

#if CPP_11
            // add data
            std::copy(begin(dataMatrix[index]),
                      end(dataMatrix[index]),
                      begin(output[0]));
#else
            output[0] = dataMatrix[index];
#endif



            // add bias
            output[0][output[0].size() - 1] = 1.;
            type = types[index];

            //obtain outputs
            for(int i = 1; i < numOfLayers; ++i)
            {
                for(int j = 0; j < dimensionality[i]; ++j)
                {
                    output[i][j] = prod(weight[i - 1][j], output[i-1]); // bias included
                }
                output[i] = activation(output[i], temp);

                output[i][ dimensionality[i] ] = 1.; //bias, unused for the highest layer
            }

            //error in the last layer
            double err1 = 0.;
            for(int j = 0; j < dimensionality.back(); ++j)
            {
                err1 += pow((output.back()[j] - int(type == j) ), 2.);
            }
            err1 = sqrt(err1);
            currentError += err1;
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

            for(int j = 0; j < this->numOfClasses; ++j)
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
        //count error
        currentError /= mixNum.size();

//        cout << "epoch = " << epoch << "\terror = " << doubleRound(currentError, 4) << endl;
    }
    cout << "epoch = " << epoch << "\terror = " << doubleRound(currentError, 4) << endl;
}



std::pair<int, double> net::classifyDatum(const int & vecNum)
{
    const int type = types[vecNum]; // true class
    const int numOfLayers = dimensionality.size();

    std::vector<std::valarray<double> > output(numOfLayers);
    output[0].resize(dimensionality[0] + 1); // +1 for biases

#if CPP_11
    std::copy(begin(dataMatrix[vecNum]),
              end(dataMatrix[vecNum]),
              begin(output[0]));
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

    /// effect on successive procedure
    double res = 0.;
    if(activation == logistic)
    {
        for(int i = 0; i < this->numOfClasses; ++i)
        {
            res += pow((output.back()[i] - (i == type)), 2);
        }
        res = sqrt(res / this->numOfClasses);
    }
    else if(activation == softmax)
    {
        res = 1. - output[numOfLayers - 1][type]; // crutch
    }

    // return number of maximal element in output.back()
#if CPP_11
    return std::make_pair(std::max_element(begin(output.back()),
                                           end(output.back()) - 1)  // -bias
                          - begin(output.back()),
                          res);
#else
    int maxNum = -1;
    double maxVal = 0.;
    for(int i = 0; i < output.back().size() - 1; ++i)
    {
        if(output.back()[i] > maxVal)
        {
            maxVal = output.back()[i];
            maxNum = i;
        }
    }
    return std::make_pair(maxNum, res);
#endif


    // more general
//    return std::distance(output.back().begin(),
//                         std::max_element(output.back().begin(),
//                                          output.back().end()));

//    for(int k = 0; k < dimensionality[numOfLayers - 1]; ++k)
//    {
//        if(k != type && output[numOfLayers - 1] [k] >= output[numOfLayers - 1] [type])
//        {
//            return false;
//        }
//    }
//    return true;
}

