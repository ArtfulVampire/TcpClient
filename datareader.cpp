#include "datareader.h"

DataReader::DataReader(QObject * inParent)
{
	this->setParent(inParent);


	this->socket = new QTcpSocket(this);
	QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
					 this, SLOT(socketErrorSlot()));
	QObject::connect(socket, SIGNAL(connected()),
					 this, SLOT(socketConnectedSlot()));
	QObject::connect(socket, SIGNAL(disconnected()),
					 this, SLOT(socketDisconnectedSlot()));

	this->socket->connectToHost(def::hostAddress,
								def::hostPort);

	this->fullDataFlag = def::fullDataFlag;

	this->socketDataStream.setDevice(this->socket);
	this->socketDataStream.setByteOrder(QDataStream::LittleEndian);

	QObject::connect(socket, SIGNAL(readyRead()),
					 this, SLOT(receiveData()));

	oneSlice.resize(def::ns); /// whaaaat
}


DataReader::~DataReader()
{
	if(!socketDataStream.atEnd())
	{
		//        socketDataStream.skipRawData(socketDataStream.device()->bytesAvailable());
	}
	if(!socketDataStream.atEnd())
	{
		std::cout << "still not at end" << std::endl;
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
	startPack.packSize = sizeof(enc::DWORD);
	if(this->fullDataFlag)
	{
		startPack.packId = 0x000C;
	}
	else
	{
		startPack.packId = 0x0001;
	}

	auto bytesWritten = socket->write(reinterpret_cast<char *>(&startPack),
									  startPack.packSize + sizeof(enc::DWORD));
	std::cout << "sendStartRequest: written " << bytesWritten << " bytes" << std::endl;
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
		std::cout << "BAD PACK: "
			 << "packSize = " << inPack.packSize << "\t"
			 << "packId = " << inPack.packId << std::endl;
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
		std::cout << inPack.packSize << '\t' << inPack.packId << std::endl;
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
			std::cout << "unknown data-pack came, OMG " << inPack.packId << std::endl;
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

		//        std::cout << patientName << std::endl;

		socketDataStream >> numOfChannels;
		def::ns = numOfChannels; /// whaaaaat
		oneSlice.resize(def::ns); /// whaaaat
		//        std::cout << numOfChannels << std::endl;
		for(int i = 0; i < numOfChannels; ++i)
		{
			//            std::cout << endl << "channel " << i << std::endl;

			QString channelName = readString(socketDataStream);
			//			std::cout << channelName << std::endl;

			double bitWeight;
			socketDataStream >> bitWeight;
			//            std::cout << bitWeight << std::endl;

			double samplingRate;
			socketDataStream >> samplingRate;
			//            std::cout << samplingRate << std::endl;

			QString metrica = readString(socketDataStream);
			//            std::cout << metrica << std::endl;

			double LFF;
			socketDataStream >> LFF;
			//            std::cout << LFF << std::endl;

			double HFF;
			socketDataStream >> HFF;
			//            std::cout << HFF << std::endl;

			int levelHFF;
			socketDataStream >> levelHFF;
			//            std::cout << levelHFF << std::endl;

			double rejector;
			socketDataStream >> rejector;
			//            std::cout << rejector << std::endl;

			double defSans;
			socketDataStream >> defSans;
			//            std::cout << defSans << std::endl;
			//            std::cout << std::endl;
		}
		QString scheme = readString(socketDataStream);
	}
	else
	{
		QString patientName = readString(socketDataStream); // cyrillic "net pacienta"
		//        std::cout << patientName << std::endl;

		socketDataStream >> numOfChannels;
		def::ns = numOfChannels; /// whaaaaat
		oneSlice.resize(def::ns); /// whaaaat
		//        std::cout << numOfChannels << std::endl;

		for(int i = 0; i < numOfChannels; ++i)
		{
			//            std::cout << endl << "channel " << i << std::endl;

			QString channelName = readString(socketDataStream);
			//            std::cout << channelName << std::endl;

		}
		QString scheme = readString(socketDataStream);
		//        std::cout << scheme << std::endl;

		socketDataStream >> bitWeight;
		//        std::cout << bitWeight << std::endl;

		socketDataStream >> samplingRate;
		//        std::cout << samplingRate << std::endl;
		//        exit(0);
	}
	std::cout << "start info was read" << std::endl;
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
			std::cout << "still not at end" << std::endl;
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
//		case 203:
//		{
//			def::pauseFlag = 1;
//			break;
//		}
		case 241:
		{
			def::setTask(0);
			def::solved = def::solveType::right;
			break;
		}
		case 247:
		{
			def::setTask(1);
			def::solved = def::solveType::right;
			break;
		}
		case 254:
		{
			def::setTask(2);
			break;
		}
		case 200:
		{
			def::currentType = 2;
			def::slicesCame = 0;
			def::numOfReal = 1; /// right after "sta"
			def::numOfWind = 0;
			break;
		}
		default:
		{
//			def::currentType = -1;
			break;
		}
	}
	std::cout << "Marker: " << name << ", " << int(def::currentMarker) << std::endl;
}


void DataReader::markerCameAVS()
{
	qint32 sliceNumber;
	socketDataStream >> sliceNumber;
	QString name = readString(socketDataStream);
	std::cout << "AVS Marker: " << name << std::endl;
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
		std::cout << sliceNumber << '\t'
			 << numOfChans << '\t'
			 << numOfSlices
			 << std::endl;
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
	QObject::connect(myReader, SIGNAL(destroyed()), this, SLOT(stopReadData()));
	QObject::connect(myReader, SIGNAL(retranslateMessage(QString)),
					 this, SLOT(retranslateMessageSlot(QString)));

	QObject::connect(myReader, SIGNAL(sliceReady(eegSliceType)),
					 this, SLOT(receiveSlice(eegSliceType)));

	QObject::connect(this, SIGNAL(finishReadData()), myReader, SLOT(deleteLater()));

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

	if(!def::pauseFlag
	   && ((def::slicesCame - def::windowLength) % def::timeShift == 0)
	   && def::slicesCame >= def::windowLength)
	{
		eegDataType::iterator windowStartIterator = std::end(def::eegData);
		eegDataType::iterator windowEndIterator = std::end(def::eegData); /// really unused
		for(int i = 0; i < def::windowLength; ++i, --windowStartIterator){}
		emit dataSend(windowStartIterator, windowEndIterator);
	}
}
