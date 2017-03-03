#include "datareader.h"

using namespace std;
using namespace std::chrono;
using namespace enc;



DataReader::DataReader(QObject * inParent)
{
    this->setParent(inParent);


    this->socket = new QTcpSocket(this);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketErrorSlot()));
    connect(socket, SIGNAL(connected()),
            this, SLOT(socketConnectedSlot()));
    connect(socket, SIGNAL(disconnected()),
            this, SLOT(socketDisconnectedSlot()));

    this->socket->connectToHost(def::hostAddress,
                                def::hostPort);

    this->fullDataFlag = def::fullDataFlag;

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
//        socketDataStream.skipRawData(socketDataStream.device()->bytesAvailable());
    }
    if(!socketDataStream.atEnd())
    {
        cout << "still not at end" << endl;
	}
    socket->disconnectFromHost();
    socket->close();
}

void DataReader::sendStartRequest()
{
    if(!socket->isOpen())
    {
        emit retranslateMessage("socket not opened!");
        return;
    }

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

void DataReader::socketConnectedSlot()
{
    emit retranslateMessage("socket connected = "
                            + socket->peerAddress().toString()
                            + ":" + QString::number(socket->peerPort()));

}

void DataReader::socketErrorSlot()
{
    emit retranslateMessage("socket error: "
                            + QString::number(socket->error())
                            + " " + socket->errorString());
}

void DataReader::socketDisconnectedSlot()
{
    emit retranslateMessage("socket disconnected from host");
}

void DataReader::receiveData()
{
    static enc::Pack inPack;

//    static int waitCounter = 0;

    if(inPack.packSize == 0)
	{
        socketDataStream >> inPack.packSize;
		socketDataStream >> inPack.packId;
    }

    if(inPack.packId > 12 ||
//            inPack.packId < 0 ||
            inPack.packSize > 2000 ||
            inPack.packSize < 0)
    {
        cout << "BAD PACK: "
             << "packSize = " << inPack.packSize << "\t"
             << "packId = " << inPack.packId << endl;
//        inPack = enc::Pack();
//        return;
    }

    /// wait for data to come (namely, initial 1618 bytes)
    if(socket->bytesAvailable() < (inPack.packSize - sizeof(inPack.packId)))
    {
        return;
    }
#if (VERBOSE_OUTPUT >= 3) || 0
    if(inPack.packId != 8)
    cout << inPack.packSize << '\t' << inPack.packId << endl;
#endif

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
        markerCameAVS();
        break;
    }
    case 11: // 0x000B
    {
        /// presentation marker
        markerCame();
        break;
    }
    default:
    {
        cout << "unknown data-pack came, OMG " << inPack.packId << endl;
        exit(0);
        socketDataStream.readRawData(tmpData, inPack.packSize);
        break;
    }
    }
    inPack = enc::Pack();
    /// pewpew crutch
    if(socket->bytesAvailable() >= 64)
    {
        receiveData();
    }
}



void DataReader::readStartInfo()
{
    if(fullDataFlag)
	{
		QString patientName = readString(socketDataStream); // cyrillic "net pacienta"

//        cout << patientName << endl;

        socketDataStream >> numOfChannels;
//        cout << numOfChannels << endl;
        for(int i = 0; i < numOfChannels; ++i)
        {
//            cout << endl << "channel " << i << endl;

			QString channelName = readString(socketDataStream);
//			cout << channelName << endl;

            double bitWeight;
            socketDataStream >> bitWeight;
//            cout << bitWeight << endl;

            double samplingRate;
            socketDataStream >> samplingRate;
//            cout << samplingRate << endl;

			QString metrica = readString(socketDataStream);
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
//            cout << endl;
        }
		QString scheme = readString(socketDataStream);
    }
    else
	{
		QString patientName = readString(socketDataStream); // cyrillic "net pacienta"
//        cout << patientName << endl;

        socketDataStream >> numOfChannels;
//        cout << numOfChannels << endl;

        for(int i = 0; i < numOfChannels; ++i)
        {
//            cout << endl << "channel " << i << endl;

			QString channelName = readString(socketDataStream);
//            cout << channelName << endl;

        }
		QString scheme = readString(socketDataStream);
//        cout << scheme << endl;

        socketDataStream >> bitWeight;
//        cout << bitWeight << endl;

        socketDataStream >> samplingRate;
//        cout << samplingRate << endl;
//        exit(0);
    }
	cout << "start info was read" << endl;
}



void DataReader::startStopTransmisson()
{
	qint32 var;
	socketDataStream >> var;

    this->inProcess = var;

    QString res = (var == 1)?"ON":"OFF";
    emit retranslateMessage("data transmission " + res);


    if(var == 1) /// real-time ON signal came - should send datarequest again
    {
        this->sendStartRequest();
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
    }
}

void DataReader::markerCame()
{
	qint32 sliceNumber;
	socketDataStream >> sliceNumber;
	QString name = readString(socketDataStream);

	socketDataStream >> def::currentMarker;

    switch(def::currentMarker)
    {
    case 201:
    {
        def::pauseFlag = 1;
        break;
    }
    case 202:
    {
        def::pauseFlag = 0;
        break;
    }
    case 241:
    {
        def::currentType = 0;
        def::slicesCame = 0;
        ++def::numOfReal;
        def::numOfWind = 0;
        break;
    }
    case 247:
    {
        def::currentType = 1;
        def::slicesCame = 0;
        ++def::numOfReal;
        def::numOfWind = 0;
        break;
    }
    case 254:
    {
        def::currentType = 2;
        def::slicesCame = 0;
        ++def::numOfReal;
        def::numOfWind = 0;
        break;
    }
    case 200:
    {
        def::currentType = 2;
        def::slicesCame = 0;
        def::numOfReal = 2; /// right after "sta"
        def::numOfWind = 0;
        break;
    }
    default:
    {
//        def::currentType = -1;
        break;
    }
    }
    cout << "Marker: " << name << ", " << int(def::currentMarker) << endl;
}


void DataReader::markerCameAVS()
{
	qint32 sliceNumber;
	socketDataStream >> sliceNumber;
	QString name = readString(socketDataStream);
    cout << "AVS Marker: " << name << endl;
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

//        /// fill the lost slices with zeros
//        for(int i = sliceNumberPrevious + numOfSlices; i < sliceNumber; ++i)
//        {
//            emit sliceReady(eegSliceType(numOfChans, 0));
//        }


#if VERBOSE_OUTPUT >= 2
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
				socketDataStream >> oneSlice[j];
            }
            /// global eegData
            emit sliceReady(oneSlice);
        }
        sliceNumberPrevious = sliceNumber;
    }
}





/// DataReaderHandler
DataReaderHandler::DataReaderHandler()
{
}

DataReaderHandler::~DataReaderHandler()
{

}

void DataReaderHandler::timerEvent(QTimerEvent *event)
{

}


void DataReaderHandler::retranslateMessageSlot(QString a)
{
    emit retranslateMessage(a);
}

void DataReaderHandler::startReadData()
{
    myReader = new DataReader(this);
    connect(myReader, SIGNAL(destroyed()), this, SLOT(stopReadData()));
    connect(myReader, SIGNAL(retranslateMessage(QString)),
            this, SLOT(retranslateMessageSlot(QString)));

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
    /// global eegData

    def::eegData.push_back(slic); ++def::slicesCame;
    def::eegData.pop_front();

//    cout << def::slicesCame << endl;

    /// old variant
//    if(def::slicesCame % def::timeShift == 0 &&
//    def::slicesCame > def::windowLength) // not >= to delay reaction
    if(!def::pauseFlag
       && ((def::slicesCame - def::windowLength) % def::timeShift == 0)
       && def::slicesCame >= def::windowLength)
    {
        eegDataType::iterator windowStartIterator = def::eegData.end();
        eegDataType::iterator windowEndIterator = --def::eegData.end(); /// really unused
        for(int i = 0; i < def::windowLength; ++i, --windowStartIterator)
        {}
//        cout << "receiveSlice: emit dataSend()" << endl;
        emit dataSend(windowStartIterator, windowEndIterator);
	}
}
