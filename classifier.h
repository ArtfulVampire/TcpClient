#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "biglib.h"
#include <chrono>

class net : public QObject
{
    Q_OBJECT
public:
    net(QObject * par = nullptr);
    ~net();

private:
    matrix coeff{def::eegNs, 2}; // 2 eog channels
    matrix dataMatrix; // biases and types separately
    std::vector<int> types;
    std::vector<QString> fileNames;
    std::vector<double> classCount; // really int but...

    const double loadDataNorm = 10.; // for windows, empirical
    std::valarray<double> averageDatum;
    std::valarray<double> sigmaVector;

    matrix confusionMatrix; // rows - realClass, cols - outClass

    typedef std::vector<matrix> wtsType;
    wtsType weight;
    std::vector<int> dimensionality; // for backprop

    double averageAccuracy = 0.;
    double kappa = 0.; // Cohen's

    bool tallCleanFlag  = false;

    /// previously from UI
    int epoch = 0;
    const int maxEpoch = 250;

    enum class myMode {N_fold, k_fold, train_test,  half_half};
    enum class source {winds, reals, pca, bayes};

    myMode Mode = myMode::N_fold;
	source Source = source::winds;

    const double temp = 10.;
    // softmax
    double errCrit = 0.05;
    double learnRate = 0.05;


    static const int lowLimit = 85;
    static const int highLimit = 130;

    static const int numGoodNewLimit = 5;
    static const int learnSetStay = 60;
    static constexpr double decayRate = 0.005;

    int numGoodNew = 0;

	std::vector<int> exIndices{};

    int numOfPairs = 15;
	int folds = 8;

    QSerialPort * comPort = nullptr;
    QDataStream comPortDataStream{};

signals:
    void finish();
    void sendSignal(int);

public slots:
    void dataCame(eegDataType::iterator a, eegDataType::iterator b);
    void averageClassification();


public:
    void startOperate();
    double adjustLearnRate(int lowLimit,
                           int highLimit);
	std::pair<int, double> classifyDatum(const int & vecNum);


    std::vector<int> makeLearnIndexSet();
    std::pair<std::vector<int>, std::vector<int> > makeIndicesSetsCross(const std::vector<std::vector<int> > & arr,
                                                                        const int numOfFold);
    std::valarray<double> (*activation)(const std::valarray<double> & in, double temp) = softmax;

    void autoClassification(const QString & spectraDir);
    //    void autoClassificationSimple();
    void leaveOneOut();
    void crossClassification();
    void leaveOneOutClassification();
    void halfHalfClassification();
    void trainTestClassification(const QString & trainTemplate = "_train",
                                 const QString & testTemplate = "_test");

    void learnNetIndices(std::vector<int> mixNum,
                         const bool resetFlag = true);
    void tallNetIndices(const std::vector<int> & indices);
    void learnNet(const bool resetFlag = true);
    void tallNet();
    void reset();


    double getAverageAccuracy();
	double getKappa();
    int getEpoch();
    double getLrate();

    void aaDefaultSettings();
    void printData();



    void loadData(const QString & spectraPath = QString(),
                  const QStringList & filters = {"*_train*"});
    void popBackDatum();
    void pushBackDatum(const std::valarray<double> & inDatum,
                       const int & inType,
                       const QString & inFileName);
    void eraseDatum(const int & index);
    void eraseData(const std::vector<int> & indices);




	void writeWts(const QString & outPath = QString());
	void readWts(const QString & fileName,
                 wtsType * wtsMatrix = nullptr);

    void successivePreclean(const QString & spectraPath = QString());

    void successiveProcessing(const QString & spectraPath = QString());

    std::valarray<double> successiveDataToSpectre(const eegDataType::iterator eegDataStart,
                                     const eegDataType::iterator eegDataEnd);

    void successiveLearning(const std::valarray<double> & newSpectre,
                            const int newType,
                            const QString & newFileName);
    void successiveRelearn();


};

class NetHandler : public QObject
{
    Q_OBJECT

public:
    NetHandler()
    {

    }

    ~NetHandler()
    {
    }

    void printAccuracy()
    {
        emit printAcc();
    }

public slots:
    void rethrowSlot(int a)
    {
        emit sendSignal(a);
    }
    void finishSlot()
    {
        std::cout << "finishSlot()" << std::endl;
        emit finishWork();
    }
    void startWork()
    {
        myNet = new net();
        connect(myNet, SIGNAL(finish()),
                this, SLOT(finishSlot()));
        connect(myNet, SIGNAL(sendSignal(int)),
                this, SLOT(rethrowSlot(int)));
        connect(this, SIGNAL(toProcess(eegDataType::iterator, eegDataType::iterator)),
                myNet, SLOT(dataCame(eegDataType::iterator, eegDataType::iterator)));
        connect(this, SIGNAL(printAcc()),
                myNet, SLOT(averageClassification()));
        myNet->startOperate();
    }
    void dataReceive(eegDataType::iterator a, eegDataType::iterator b)
    {
//        std::cout << "dataReceive" << std::endl;
        emit toProcess(a, b);
    }


signals:
    void sendSignal(int);
    void finishWork();
    void toProcess(eegDataType::iterator, eegDataType::iterator);
    void printAcc();

private:
    net * myNet;
};

#endif // CLASSIFIER_H
