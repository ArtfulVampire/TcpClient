#include "datareader.h"

using namespace std;
using namespace std::chrono;
using namespace enc;




DataReader::DataReader(QObject * inParent,
                       QTcpSocket * inSocket,
                       bool inFullDataFlag)
{
    this->setParent(inParent);
    this->socket = inSocket;
    this->fullDataFlag = inFullDataFlag;
    this->socketDataStream.setDevice(this->socket);
    this->socketDataStream.setByteOrder(QDataStream::LittleEndian);

    connect(socket, SIGNAL(readyRead()),
            this, SLOT(receiveData()));

}


DataReader::~DataReader()
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
    cout << "startSlot: written " << bytesWritten << " bytes" << endl;
}

void DataReader::receiveData()
{
    static enc::Pack inPack; /// maybe bad with parallel slots

    static int waitCounter = 0;

    if(inPack.packSize == 0)
    {
        if(socket->bytesAvailable() < sizeof(inPack.packSize))
        {
            ++waitCounter;
            return;
        }

        if(waitCounter != 0)
        {
            cout << "size waitCounter = " << waitCounter << endl;
            waitCounter = 0;
        }

        char tmp;
        socketDataStream.device()->peek(&tmp, 1);
        socketDataStream >> inPack.packSize;
        if(int(tmp) == -1) /// initial data for not fullData
        {
            inPack.packSize = 259 - sizeof(inPack.packSize);
        }

    }


    if(inPack.packId == 0)
    {
        if(socket->bytesAvailable() < sizeof(inPack.packId))
        {
            ++waitCounter;
            return;
        }

        if(waitCounter != 0)
        {
            cout << "id waitCounter = " << waitCounter << endl;
            waitCounter = 0;
        }

        //    {
        socketDataStream >> inPack.packId;
    }



    cout << "packSize = " << inPack.packSize << "\t"
         << "packId = " << inPack.packId << endl;

    if(inPack.packId > 12 || inPack.packId < 0 ||
            inPack.packSize > 2000 || inPack.packSize < 0)
    {
        exit(9);
    }

    if(socket->bytesAvailable() < (inPack.packSize - sizeof(inPack.packId)))
    {
        cout << "wait for data\t";
        cout << socket->bytesAvailable() << "\t" << inPack.packSize - sizeof(inPack.packId) << endl;
        ++waitCounter;
        return;
    }
    if(waitCounter != 0)
    {
        cout << "data waitCounter = " << waitCounter << endl;
        waitCounter = 0;
    }
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
        cout << "unknown data-pack came, OMG" << endl;
        break;
    }
    }
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

            double bitWeight; socketDataStream >> bitWeight;
//            cout << bitWeight << endl;

            double samplingRate; socketDataStream >> samplingRate;
//            cout << samplingRate << endl;

            std::string metrica = readString(socketDataStream);
//            cout << metrica << endl;

            double LFF; socketDataStream >> LFF;
//            cout << LFF << endl;

            double HFF; socketDataStream >> HFF;
//            cout << HFF << endl;

            int levelHFF; socketDataStream >> levelHFF;
//            cout << levelHFF << endl;

            double rejector; socketDataStream >> rejector;
//            cout << rejector << endl;

            double defSans; socketDataStream >> defSans;
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
    if(this->inProcess)
    {
        disconnect(socket, SIGNAL(readyRead()),
                this, SLOT(receiveDataSlot()));
        cout << "disconnected readyRead()" << endl;
    }
}



void DataReader::startStopTransmisson()
{
    int var;
    socketDataStream >> var;

    QString res = (var == 1)?"ON":"OFF";
//    ui->textEdit->append("data transmission " + res);

    if(var == 1) /// real-time ON signal came - should send datarequest again
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{200});
        this->sendStartRequest();
    }
    else if(this->inProcess) /// finish all job
    {
        disconnect(socket, SIGNAL(readyRead()),
                   this, SLOT(receiveData()));

        if(!socketDataStream.atEnd())
        {
            socketDataStream.skipRawData(socketDataStream.device()->bytesAvailable());
        }
        if(!socketDataStream.atEnd())
        {
            cout << "still not at end" << endl;
        }
        this->~DataReader();
    }
    this->inProcess = var;
    emit startStopSignal(var);
}

void DataReader::dataSliceCame()
{
    if(fullDataFlag)
    {
        qint32 sliceNumber;
        socketDataStream >> sliceNumber;

        qint32 numOfChans;
        socketDataStream >> numOfChans;

        qint32 numOfSlices;
        socketDataStream >> numOfSlices;

        cout << sliceNumber << '\t'
             << numOfChans << '\t'
             << numOfSlices
             << endl;

        static std::vector<short> oneSlice(numOfChans);
        for(int i = 0; i < numOfSlices; ++i)
        {
            for(int j = 0; j < numOfChans; ++j)
            {
                socketDataStream >> oneSlice[j];
            }
            emit sliceReady(oneSlice);
        }
    }
}


/// DataReaderHandler
DataReaderHandler::DataReaderHandler(QObject * inParent,
                                     QTcpSocket * inSocket,
                                     bool inFullDataFlag)
{
    this->setParent(inParent);
    this->socket = inSocket;
    this->fullDataFlag = inFullDataFlag;
}

DataReaderHandler::~DataReaderHandler()
{

}

void DataReaderHandler::startReadData()
{
    myReader = new DataReader(this->parent(), this->socket, this->fullDataFlag);
    connect(myReader, SIGNAL(destroyed()), this, SLOT(stopReadData()));
    myReader->sendStartRequest();
}

void DataReaderHandler::stopReadData()
{
    emit this->finishReadData();
}

void DataReaderHandler::receiveSlice(const std::vector<short> & slice)
{
    eegData.push_back(slice);
}
