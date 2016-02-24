#include "datareader.h"

using namespace std;
using namespace std::chrono;
using namespace enc;

//template <typename Typ>
//Typ readFromSocket(QTcpSocket * inSocket)
//{
//    int siz = sizeof(Typ);
//    char * tmp = new char[siz];
//    inSocket->read(tmp, siz);
//    Typ res = *(reinterpret_cast<Typ*>(tmp));
//    delete[] tmp;
//    return res;
//}


DataReader::DataReader(QObject * inParent, QTcpSocket * inSocket,
                       bool inFullDataFlag)
{
    this->setParent(inParent);
    this->socket = inSocket;
    this->fullDataFlag = inFullDataFlag;
    this->socketDataStream.setDevice(this->socket);
    this->socketDataStream.setByteOrder(QDataStream::LittleEndian);
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(receiveData()));

    oneSlice.resize(def::ns); /// pewpewpew
}


DataReader::~DataReader()
{
    if(!socketDataStream.atEnd())
    {
        socketDataStream.skipRawData(socketDataStream.device()->bytesAvailable());
    }
    if(!socketDataStream.atEnd())
    {
        cout << "still not at end" << endl;
    }
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
//    receiveData();
}

void DataReader::receiveData()
{
    static enc::Pack inPack;

    static int waitCounter = 0;

    if(inPack.packSize == 0)
    {
        if(socket->bytesAvailable() < sizeof(inPack.packSize) + sizeof(inPack.packId))
        {
            ++waitCounter;
            return;
        }

        if(waitCounter != 0)
        {
//            cout << "size waitCounter = " << waitCounter << endl;
            waitCounter = 0;
        }

//        char tmp;
//        socketDataStream.device()->peek(&tmp, 1);

//        socketDataStream >> inPack.packSize;
#if !USE_DATA_STREAM ||1

        inPack.packSize = readFromSocket<qint32>(socket);
        inPack.packId = readFromSocket<quint32>(socket);

#else
        QByteArray arr = socket->peek(8); // try read int + uint = 8 bytes
        QDataStream str(&arr, QIODevice::ReadOnly);
        str.setByteOrder(QDataStream::LittleEndian);
        str >> inPack.packSize;
        str >> inPack.packId;

#if 1 //
        socketDataStream.skipRawData(8);
#else
        if(!(inPack.packSize == 1618 && inPack.packId == 2) &&
           !(inPack.packSize == 64 && inPack.packId == 8) &&
           !(inPack.packSize == 8 && inPack.packId == 6))
        {
            cout << "BAD PACK: "
                 << "packSize = " << inPack.packSize
                 << "\t"
                 << "packId = " << inPack.packId
//                 << '\t'
//                 << "packSize = " << bits(inPack.packSize)
//                 << "\t"
//                 << "packId = " << bits(inPack.packId)
                 << endl;

            inPack.packSize = 0;
            inPack.packId = 0;


            if(int sk = socketDataStream.skipRawData(2) != 2)
            {
                cout << "sk != 2, sk == " << sk << endl;
            }
            return;
        }
        else
        {
            if(int sk = socketDataStream.skipRawData(8) != 8)
            {
                cout << "sk != 8, sk == " << sk << endl;
            }

            /// prevent some seldom situations
            if(inPack.packSize == 8 && inPack.packId == 6)
            {

                QByteArray arr = socket->peek(8); // try read int + uint

                QDataStream str(&arr, QIODevice::ReadOnly);
                str.setByteOrder(QDataStream::LittleEndian);
                qint32 a;
                str >> a;
                quint32 b;
                str >> b;
                if(a == 8 && b == 6)
                {
                    if(int sk = socketDataStream.skipRawData(8) != 8)
                    {
                        cout << "DOUBLE KILL: sk != 8, sk == " << sk << endl;
                    }
                }
            }


#if 0
            if(inPack.packId == 8)
            {
                qint32 sliceNum;
                socketDataStream >> sliceNum;
                cout << sliceNum << endl;
            }
            else
            {
                socketDataStream.skipRawData(4);
            }
            socketDataStream.skipRawData(inPack.packSize - sizeof(inPack.packId) - 4); // skip for peek
            inPack.packSize = 0;
            inPack.packId = 0;
            return;
#endif
        }

#endif
        /// deprecate
//        if(int(tmp) == -1) /// initial data for not fullData
//        {
//            inPack.packSize = 259 - sizeof(inPack.packSize);
//        }
#endif
    }

#if 1
//    if(inPack.packId == 0)
//    {
//        if(socket->bytesAvailable() < sizeof(inPack.packId))
//        {
//            ++waitCounter;
//            return;
//        }

//        if(waitCounter != 0)
//        {
////            cout << "id waitCounter = " << waitCounter << endl;
//            waitCounter = 0;
//        }

//        //    {
//        socketDataStream >> inPack.packId;
//    }



//    cout << "packSize = " << inPack.packSize << "\t"
//         << "packId = " << inPack.packId << endl;

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
//        cout << "wait for data\t";
//        cout << socket->bytesAvailable() << "\t" << inPack.packSize - sizeof(inPack.packId) << endl;
        ++waitCounter;
        return;
    }
    if(waitCounter != 0)
    {
//        cout << "data waitCounter = " << waitCounter << endl;
        waitCounter = 0;
    }
#else
    if(socket->bytesAvailable() < inPack.packSize)
    {
        ++waitCounter;
        return;
    }
    else
    {
        socketDataStream >> inPack.packId;


        inPack.packData = socket->read(inPack.packSize);
        myBuffer = new QBuffer(&(inPack.packData));
        myBuffer->open(QIODevice::ReadOnly);

        socketDataStream.unsetDevice();
        socketDataStream.setDevice(myBuffer);
        socketDataStream.setByteOrder(QDataStream::LittleEndian);
    }
    cout << "packSize = " << inPack.packSize << "\t" << "packId = " << inPack.packId << endl;
#endif



//    cout << inPack.packSize << '\t' << inPack.packId << endl;
//    socket->read(inPack.packSize - 4);
//    return;

//    cout << socket->bytesAvailable() << endl;

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
    }
    else
    {

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
    }
    cout << "start info was read" << endl;
    if(this->inProcess)
    {
//        disconnect(socket, SIGNAL(readyRead()),
//                   this, SLOT(receiveData()));
//        cout << "disconnected readyRead()" << endl;
    }
}



void DataReader::startStopTransmisson()
{


    int var;
    socketDataStream >> var;
    emit startStopSignal(var);
    this->inProcess = var;

//    QString res = (var == 1)?"ON":"OFF";
//    ui->textEdit->append("data transmission " + res);


    if(var == 1) /// real-time ON signal came - should send datarequest again
    {
//        cout << "pew" << endl;
//        QObject::disconnect(socket, SIGNAL(readyRead()),
//                            this, SLOT(receiveData()));
//        this->thread()->sleep(8);
        this->sendStartRequest();
//        std::thread p([this](){
//        while(this->inProcess)
//        {
//            receiveData();
//        }
//        }); p.detach();
    }
    else //if(this->inProcess) /// finish all job
    {

        if(!socketDataStream.atEnd())
        {
            socketDataStream.skipRawData(socketDataStream.device()->bytesAvailable());
        }
        if(!socketDataStream.atEnd())
        {
            cout << "still not at end" << endl;
        }
//        connect(socket, SIGNAL(readyRead()),
//                this, SLOT(receiveData()));
//        this->~DataReader();
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

    std::string name = readString(socketDataStream); /// unused


#if USE_DATA_STREAM
    socketDataStream >> def::currentMarker;
#else
    def::currentMarker = readFromSocket<quint8>(socket);
#endif

    switch(def::currentMarker)
    {
    case 241:
    {
        def::currentType = 0;
        break;
    }
    case 247:
    {
        def::currentType = 1;
    }
    case 254:
    {
        def::currentType = 2;
    }
    default:
    {
        def::currentType = -1;
        break;
    }
    }
    def::slicesCame = 0;
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




        /// fill the lost slices with zeros
        for(int i = sliceNumberPrevious + 1; i < sliceNumber; ++i)
        {
            emit sliceReady(eegSliceType(numOfChans, 0));
        }


#if 1
        if(sliceNumber % 250 == 0)
        cout << sliceNumber << '\t'
             << numOfChans << '\t'
             << numOfSlices
             << endl;
#endif
//        socketDataStream.skipRawData(numOfSlices * numOfChans * sizeof(qint16));

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
DataReaderHandler::DataReaderHandler(QTcpSocket * inSocket,
                                     bool inFullDataFlag)
{
    this->socket = inSocket;
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
                              this->socket,
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

//    cout << slicesCame << endl;


    if(def::slicesCame % def::timeShift == 0 &&
       def::slicesCame > def::windowLength) // not >= to delay reaction
    {
        eegDataType::iterator windowStartIterator = def::eegData.end();
        eegDataType::iterator windowEndIterator = --def::eegData.end(); /// really unused
        for(int i = 0; i < def::windowLength; ++i, --windowStartIterator)
        {}
        cout << "receiveSlice: emit dataSend()" << endl;
        emit dataSend(windowStartIterator, windowEndIterator);
    }
#endif
}

void DataReaderHandler::startStopSlot(int var)
{
    emit this->startStopSignal(var);
}
