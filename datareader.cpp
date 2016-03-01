#include "datareader.h"

using namespace std;
using namespace std::chrono;
using namespace enc;



DataReader::DataReader(QObject * inParent,
//                       QTcpSocket * inSocket,
                       bool inFullDataFlag)
{
    this->setParent(inParent);
//    this->socket = inSocket;

    /// consts
    this->socket = new QTcpSocket(this);
    this->socket->connectToHost(def::hostAddress,
                                def::hostPort);


    this->fullDataFlag = inFullDataFlag;
#if USE_DATA_STREAM
    this->socketDataStream.setDevice(this->socket);
    this->socketDataStream.setByteOrder(QDataStream::LittleEndian);
#endif
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(receiveData()));

    oneSlice.resize(def::ns); /// pewpewpew
}


DataReader::~DataReader()
{
#if USE_DATA_STREAM
    if(!socketDataStream.atEnd())
    {
//        socketDataStream.skipRawData(socketDataStream.device()->bytesAvailable());
    }
    if(!socketDataStream.atEnd())
    {
        cout << "still not at end" << endl;
    }
#endif
}

void DataReader::timerEvent(QTimerEvent *event)
{

}

void DataReader::sendStartRequest()
{
    enc::Pack startPack;
    startPack.packSize = sizeof(DWORD);
    if(this->fullDataFlag)
    {
        startPack.packId = 0x000C;
    }
    else
    {
        startPack.packId = 0x0001;
    }

    auto bytesWritten = socket->write(reinterpret_cast<char *>(&startPack),
                                      startPack.packSize + sizeof(DWORD));
    cout << "sendStartRequest: written " << bytesWritten << " bytes" << endl;
}

void DataReader::receiveData()
{
    static enc::Pack inPack;

//    static int waitCounter = 0;

    if(inPack.packSize == 0)
    {
        if(socket->bytesAvailable() < sizeof(inPack.packSize) + sizeof(inPack.packId))
        {
            return;
        }
#if !USE_DATA_STREAM

        inPack.packSize = readFromSocket<qint32>(socket);
        inPack.packId = readFromSocket<quint32>(socket);

#else
        socketDataStream >> inPack.packSize;
//        cout << inPack.packSize << endl;
        socketDataStream >> inPack.packId;

//        QByteArray arr = socket->peek(8); // try read int + uint = 8 bytes
//        QDataStream str(&arr, QIODevice::ReadOnly);
//        str.setByteOrder(QDataStream::LittleEndian);
//        str >> inPack.packSize;
//        str >> inPack.packId;
#endif
        /// deprecate
//        if(int(tmp) == -1) /// initial data for not fullData
//        {
//            inPack.packSize = 259 - sizeof(inPack.packSize);
//        }
    }

    if(inPack.packId > 12 || inPack.packId < 0 ||
       inPack.packSize > 2000 || inPack.packSize < 0)
    {
        cout << "BAD PACK: "
             << "packSize = " << inPack.packSize << "\t"
             << "packId = " << inPack.packId << endl;
        inPack = enc::Pack();
        return;
    }

    if(socket->bytesAvailable() < (inPack.packSize - sizeof(inPack.packId)))
    {
        return;
    }
//    cout << inPack.packSize << '\t' << inPack.packId << endl;



    switch(inPack.packId)
    {
    case 2:
    {
        /// initial data
        readStartInfo();
        break;
    }
    case 3:
    {
        /// button pressed
        break;
    }
    case 6:
    {
        /// start/stop data transmission
        startStopTransmisson();
        break;
    }
    case 8:
    {
        /// data slice
        dataSliceCame();
        break;
    }
    case 9:
    {
        /// FP marker
//        cout << 9 << '\t';
        markerCame();
        break;
    }
    case 10: // 0x000A
    {
        /// AVS marker
        break;
    }
    case 11: // 0x000B
    {
        /// presentation marker
//        cout << 11 << '\t';
        markerCame();
        break;
    }
    default:
    {
        cout << "unknown data-pack came, OMG " << inPack.packId << endl;
        break;
    }
    }
#if 0
    delete myBuffer;
    socketDataStream.unsetDevice();
    socketDataStream.setDevice(socket);
    socketDataStream.setByteOrder(QDataStream::LittleEndian);
#endif
    inPack = enc::Pack();
    if(socket->bytesAvailable() >= 64)
    {
        receiveData();
    }
}



void DataReader::readStartInfo()
{
    if(fullDataFlag)
    {
#if USE_DATA_STREAM
        std::string patientName = readString(socketDataStream); // cyrillic "net pacienta"

//        cout << patientName << endl;

        socketDataStream >> numOfChannels;
//        cout << numOfChannels << endl;
        for(int i = 0; i < numOfChannels; ++i)
        {
//            cout << endl << "channel " << i << endl;

            std::string channelName = readString(socketDataStream);
//            cout << channelName << endl;

            double bitWeight;
            socketDataStream >> bitWeight;
//            cout << bitWeight << endl;

            double samplingRate;
            socketDataStream >> samplingRate;
//            cout << samplingRate << endl;

            std::string metrica = readString(socketDataStream);
//            cout << metrica << endl;

            double LFF;
            socketDataStream >> LFF;
//            cout << LFF << endl;

            double HFF;
            socketDataStream >> HFF;
//            cout << HFF << endl;

            int levelHFF;
            socketDataStream >> levelHFF;
//            cout << levelHFF << endl;

            double rejector;
            socketDataStream >> rejector;
//            cout << rejector << endl;

            double defSans;
            socketDataStream >> defSans;
//            cout << defSans << endl;
        }
        std::string scheme = readString(socketDataStream);
//        cout << scheme << endl;
//        exit(0);
#else
        std::string patientName = readString(socket); // cyrillic "net pacienta"

//        cout << patientName << endl;

        numOfChannels = readFromSocket<qint32>(socket);
//        cout << numOfChannels << endl;
        for(int i = 0; i < numOfChannels; ++i)
        {
//            cout << endl << "channel " << i << endl;

            std::string channelName = readString(socket);
//            cout << channelName << endl;

            double bitWeight = readFromSocket<double>(socket);
//            cout << bitWeight << endl;

            double samplingRate = readFromSocket<double>(socket);
//            cout << samplingRate << endl;

            std::string metrica = readString(socket);
//            cout << metrica << endl;

            double LFF = readFromSocket<double>(socket);
//            cout << LFF << endl;

            double HFF = readFromSocket<double>(socket);
//            cout << HFF << endl;

            int levelHFF = readFromSocket<qint32>(socket);
//            cout << levelHFF << endl;

            double rejector = readFromSocket<double>(socket);
//            cout << rejector << endl;

            double defSans = readFromSocket<double>(socket);
//            cout << defSans << endl;
        }
        std::string scheme = readString(socket);
//        cout << scheme << endl;
//        exit(0);
#endif
    }
    else
    {
#if USE_DATA_STREAM
        std::string patientName = readString(socketDataStream); // cyrillic "net pacienta"
//        cout << patientName << endl;

        socketDataStream >> numOfChannels;
//        cout << numOfChannels << endl;

        for(int i = 0; i < numOfChannels; ++i)
        {
//            cout << endl << "channel " << i << endl;

            std::string channelName = readString(socketDataStream);
//            cout << channelName << endl;

        }
        std::string scheme = readString(socketDataStream);
//        cout << scheme << endl;

        socketDataStream >> bitWeight;
//        cout << bitWeight << endl;

        socketDataStream >> samplingRate;
//        cout << samplingRate << endl;
//        exit(0);
#else
#endif
    }
    cout << "start info was read" << endl;
}



void DataReader::startStopTransmisson()
{
    qint32 var;
#if USE_DATA_STREAM
    socketDataStream >> var;
#else
    var = readFromSocket<qint32>(socket);
#endif

    emit startStopSignal(var);
    this->inProcess = var;

//    QString res = (var == 1)?"ON":"OFF";
//    ui->textEdit->append("data transmission " + res);


    if(var == 1) /// real-time ON signal came - should send datarequest again
    {
        this->sendStartRequest();
    }
    else //if(this->inProcess) /// finish all job
    {
#if USE_DATA_STREAM
        if(!socketDataStream.atEnd())
        {
            socketDataStream.skipRawData(socketDataStream.device()->bytesAvailable());
        }
        if(!socketDataStream.atEnd())
        {
            cout << "still not at end" << endl;
        }
#else
        socket->readAll();
#endif
    }
}
void DataReader::markerCame()
{
    qint32 sliceNumber;

#if USE_DATA_STREAM
    socketDataStream >> sliceNumber;
#else
    sliceNumber = readFromSocket<qint32>(socket);
#endif

#if USE_DATA_STREAM
    std::string name = readString(socketDataStream); /// unused
#else
    std::string name = readString(socket); /// unused
#endif


#if USE_DATA_STREAM
    socketDataStream >> def::currentMarker;
#else
    /// quint8 in real-time, quint32 in recording play
//    def::currentMarker = readFromSocket<quint8>(socket);
    def::currentMarker = readFromSocket<markerType>(socket);
#endif

    switch(def::currentMarker)
    {
    case 241:
    {
        def::currentType = 0;
        def::slicesCame = 0;
        break;
    }
    case 247:
    {
        def::currentType = 1;
        def::slicesCame = 0;
        break;
    }
    case 254:
    {
        def::currentType = 2;
        def::slicesCame = 0;
        break;
    }
    default:
    {
//        def::currentType = -1;
        break;
    }
    }
//    cout << "Marker: " << name << ", " << int(def::currentMarker) << endl;
}

void DataReader::dataSliceCame()
{
    if(fullDataFlag)
    {
        qint32 sliceNumber;
#if USE_DATA_STREAM
        socketDataStream >> sliceNumber;
#else
        sliceNumber = readFromSocket<qint32>(socket);
#endif
        if(sliceNumberPrevious == 0)
        {
            sliceNumberPrevious = sliceNumber;
        }

        qint32 numOfChans;
#if USE_DATA_STREAM
        socketDataStream >> numOfChans;
#else
        numOfChans = readFromSocket<qint32>(socket);
#endif



        qint32 numOfSlices;
#if USE_DATA_STREAM
        socketDataStream >> numOfSlices;
#else
        numOfSlices = readFromSocket<qint32>(socket);
#endif




//        /// fill the lost slices with zeros
//        for(int i = sliceNumberPrevious + numOfSlices; i < sliceNumber; ++i)
//        {
//            emit sliceReady(eegSliceType(numOfChans, 0));
//        }


#if 0
//        if(sliceNumber % 250 == 0)
        cout << sliceNumber << '\t'
             << numOfChans << '\t'
             << numOfSlices
             << endl;
#endif

        for(int i = 0; i < numOfSlices; ++i)
        {
            for(int j = 0; j < numOfChans; ++j)
            {
#if USE_DATA_STREAM
                socketDataStream >> oneSlice[j];
#else
                oneSlice[j] = readFromSocket<qint16>(socket);
#endif
            }
            /// global eegData
            emit sliceReady(oneSlice);
        }
        sliceNumberPrevious = sliceNumber;
    }
}


/// DataReaderHandler
DataReaderHandler::DataReaderHandler(
//        QTcpSocket * inSocket,
                                     bool inFullDataFlag)
{
//    this->socket = inSocket;
    this->fullDataFlag = inFullDataFlag;
}

DataReaderHandler::~DataReaderHandler()
{

}

void DataReaderHandler::timerEvent(QTimerEvent *event)
{

}

void DataReaderHandler::startReadData()
{
    myReader = new DataReader(this,
//                              this->socket,
                              this->fullDataFlag);
    connect(myReader, SIGNAL(destroyed()), this, SLOT(stopReadData()));
    connect(myReader, SIGNAL(startStopSignal(int)), this, SLOT(startStopSlot(int)));
    /// global eegData
    connect(myReader, SIGNAL(sliceReady(eegSliceType)),
            this, SLOT(receiveSlice(eegSliceType)));

    connect(this, SIGNAL(finishReadData()), myReader, SLOT(deleteLater()));

    myReader->sendStartRequest();
}

void DataReaderHandler::stopReadData()
{
//    emit finishReadData();
}

void DataReaderHandler::receiveSlice(eegSliceType slic)
{
#if 1
//    static int slicesCame = 0;
    /// global eegData

    def::eegData.push_back(slic); ++def::slicesCame;
    def::eegData.pop_front();

//    cout << def::slicesCame << endl;


    if(def::slicesCame % def::timeShift == 0 &&
       def::slicesCame > def::windowLength) // not >= to delay reaction
    {
        eegDataType::iterator windowStartIterator = def::eegData.end();
        eegDataType::iterator windowEndIterator = --def::eegData.end(); /// really unused
        for(int i = 0; i < def::windowLength; ++i, --windowStartIterator)
        {}
//        cout << "receiveSlice: emit dataSend()" << endl;
        emit dataSend(windowStartIterator, windowEndIterator);
    }
#endif
}

void DataReaderHandler::startStopSlot(int var)
{
    emit this->startStopSignal(var);
}
