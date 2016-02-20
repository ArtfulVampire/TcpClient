#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#define DRAWS 0

#include "biglib.h"

#if CPP_11
#include <chrono>
#endif



class net : public QObject
{
    Q_OBJECT
public:
    net(QObject * par = nullptr);
    ~net();

private:
    matrix dataMatrix; // biases and types separately
    std::vector<int> types;
    std::vector<std::string> fileNames;
    std::vector<double> classCount; // really int but...

    double loadDataNorm = 10.; // for windows, empirical
    lineType averageDatum;
    lineType sigmaVector;

    matrix confusionMatrix; // rows - realClass, cols - outClass

    twovector<lineType> weight;
    std::vector<int> dimensionality; // for backprop

    double averageAccuracy = 0.;
    double kappa = 0.; // Cohen's


    bool tallCleanFlag  = false;

    /// previously from UI
    int epoch = 0;
    const int maxEpoch = 250;

#if CPP_11
    enum class myMode {N_fold, k_fold, train_test,  half_half};
    enum class source {winds, reals, pca, bayes};

    myMode Mode = myMode::N_fold;
    source Source = source::winds;
#else
    enum myMode{N_fold, k_fold, train_test,  half_half};
    enum source{winds, reals, pca, bayes};

    myMode Mode = N_fold;
    source Source = winds;
#endif

    const double temp = 10.;
    // softmax
    double errCrit = 0.05;
    double learnRate = 0.05;

    static const int lowLimit = 70;
    static const int highLimit = 110;

    static const int learnSetStay = 100;
    static const int numGoodNewLimit = 5;
    static constexpr double decayRate = 0.01;


    int numOfPairs = 15;
    int folds = 8;
    double rdcCoeff = 1.; // deprecated

    /// previously from namespace def
    std::string workPath; // def::dir->absolutePath()
    std::string ExpName; /// really unused
    static const int numOfClasses = 3;

signals:
    void finish();
    void sendSignal(int);

public slots:
    void dataCame(eegDataType::iterator a, eegDataType::iterator b);

public:
    void startOperate();
    double adjustLearnRate(int lowLimit,
                           int highLimit);
    std::pair<int, double> classifyDatum(const int & vecNum);


    std::vector<int> makeLearnIndexSet();
    std::pair<std::vector<int>, std::vector<int> > makeIndicesSetsCross(const std::vector<std::vector<int> > & arr,
                                                                        const int numOfFold);
    std::valarray<double> (*activation)(const std::valarray<double> & in, double temp) = softmax;

    void autoClassification(const std::string & spectraDir);
    //    void autoClassificationSimple();
    void leaveOneOut();
    void crossClassification();
    void leaveOneOutClassification();
    void halfHalfClassification();
    void trainTestClassification(const std::string & trainTemplate = "_train",
                                 const std::string & testTemplate = "_test");

    void learnNetIndices(std::vector<int> mixNum,
                         const bool resetFlag = true);
    void tallNetIndices(const std::vector<int> & indices);
    void learnNet(const bool resetFlag = true);
    void tallNet();
    void reset();

    void averageClassification();

    double getAverageAccuracy();
    double getKappa();
    double getReduceCoeff();
    int getEpoch();
    double getLrate();

    void aaDefaultSettings();



    void loadData(const std::string & spectraPath = std::string());
    void popBackDatum();
    void pushBackDatum(const lineType & inDatum,
                       const int & inType,
                       const std::string & inFileName);
    void eraseDatum(const int & index);
    void eraseData(const std::vector<int> & indices);




    void writeWts(const std::string &wtsPath = std::string());
#if DRAWS
    void drawWts(std::string wtsPath = std::string(),
                 std::string picPath = std::string());
#endif
    void readWts(const std::string & fileName,
                 twovector<lineType> * wtsMatrix = NULL);


    void successivePreclean(const std::string & spectraPath = std::string());

    void successiveProcessing(const std::string & spectraPath = std::string());

    lineType successiveDataToSpectre(const eegDataType::iterator eegDataStart,
                                     const eegDataType::iterator eegDataEnd);

    void successiveLearning(const lineType & newSpectre,
                            const int newType,
                            const std::string & newFileName);
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
        myNet->startOperate();
    }
    void dataReceive(eegDataType::iterator a, eegDataType::iterator b)
    {
        emit toProcess(a, b);
    }


signals:
    void sendSignal(int);
    void finishWork();
    void toProcess(eegDataType::iterator, eegDataType::iterator);

private:
    net * myNet;
};

#endif // CLASSIFIER_H
