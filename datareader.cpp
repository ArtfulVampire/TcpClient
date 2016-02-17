#include "datareader.h"

using namespace std;
using namespace std::chrono;
using namespace enc;




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

    oneSlice.resize(24); /// pewpewpew
}


DataReader::~DataReader()
{

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
        QByteArray arr = socket->peek(8); // try read int + uint
        QDataStream str(&arr, QIODevice::ReadOnly);
        str.setByteOrder(QDataStream::LittleEndian);
        str >> inPack.packSize;
        str >> inPack.packId;

        if(!(inPack.packSize == 1618 && inPack.packId == 2) &&
           !(inPack.packSize == 64 && inPack.packId == 8) &&
           !(inPack.packSize == 8 && inPack.packId == 6))
        {
            cout << "BAD PACK: "
                 << "packSize = " << inPack.packSize << "\t"
                 << "packId = " << inPack.packId << endl;

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

        /// deprecate
//        if(int(tmp) == -1) /// initial data for not fullData
//        {
//            inPack.packSize = 259 - sizeof(inPack.packSize);
//        }
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

    cout << inPack.packSize << '\t' << inPack.packId << '\t';

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
        cout << "disconnected readyRead()" << endl;
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

        this->thread()->msleep(500);
//        this->sendStartRequest();
//        this->thread()->msleep(2000);


        while(this->inProcess)
        {
            this->thread()->usleep(1200);
            receiveData();
        }
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

void DataReader::dataSliceCame()
{
    if(fullDataFlag)
    {
        qint32 sliceNumber;
        socketDataStream >> sliceNumber;
        if(sliceNumberPrevious == 0)
        {
            sliceNumberPrevious = sliceNumber;
        }

        qint32 numOfChans;
        socketDataStream >> numOfChans;

        qint32 numOfSlices;
        socketDataStream >> numOfSlices;

        for(int i = sliceNumberPrevious + 1; i < sliceNumber; ++i)
        {
            emit sliceReady(std::vector<qint16>(numOfChans, 0));
        }


#if 1
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
                socketDataStream >> oneSlice[j];
            }
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
    connect(myReader, SIGNAL(sliceReady(std::vector<qint16>)),
            this, SLOT(receiveSlice(std::vector<qint16>)));

    myReader->sendStartRequest();
}

void DataReaderHandler::stopReadData()
{
//    emit finishReadData();
}

void DataReaderHandler::receiveSlice(std::vector<qint16> slice)
{
    eegData.push_back(slice);
#if 0
    int timeShift = 125;
    if(eegData.size() % timeShift == 0 &&  eegData.size() > 1000)
    {
        cout << "fileNum = "
             << QString::number(eegData.size() / timeShift).toStdString() << endl;
        const QString str = "/media/Files/Data/RealTime/data_" +
                            QString::number(eegData.size() / timeShift) +
                            ".txt";
        ofstream ostr(str.toStdString());
        ostr << "NumOfSlices 1000\t" << "ns 24" << endl;
        for(uint i = eegData.size() - 1000; i < eegData.size(); ++i)
        {
            for(int j = 0; j < 24; ++j)
            {
                ostr << eegData[i][j] << '\t';
            }
            ostr << endl;
        }
        ostr.close();
    }
#endif
}

void DataReaderHandler::startStopSlot(int var)
{
    emit this->startStopSignal(var);
}
