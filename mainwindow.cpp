#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace enc;
using namespace std;
using namespace std::chrono;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<eegDataType::iterator>("eegDataType::iterator");


    /// server
    ui->serverPortSpinBox->setMaximum(65535);
    int hostCounter = 0;
    ui->serverAddressComboBox->addItem("local, 120");
    ui->serverAddressComboBox->setItemData(hostCounter++,
                                           QVariant("127.0.0.1:120"));

    ui->serverAddressComboBox->addItem("local, home");
    ui->serverAddressComboBox->setItemData(hostCounter++,
                                           QVariant("127.0.0.1:35577"));

    ui->serverAddressComboBox->addItem("Enceph");
    ui->serverAddressComboBox->setItemData(hostCounter++,
                                           QVariant("213.145.47.104:120"));

    ui->serverAddressComboBox->addItem("pew");
    ui->serverAddressComboBox->setItemData(hostCounter++,
                                           QVariant("192.168.0.104:120"));
    connect(ui->serverAddressComboBox, SIGNAL(highlighted(int)),
            this, SLOT(serverAddressSlot(int)));
    connect(ui->serverAddressComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(serverAddressSlot(int)));
#if MY_LINROWS
    ui->serverAddressComboBox->setCurrentText("pew");
#else
    ui->serverAddressComboBox->setCurrentText("Enceph");
#endif


    ui->serverAddressComboBox->setCurrentText("pew"); /// fix via router, always 192.168.0.104


    /// com
    for(int i = 0; i < 9; ++i)
    {
        ui->comPortComboBox->addItem("COM"+QString::number(i+1));
    }
    ui->comPortComboBox->setCurrentText("COM5");

#if COM_IN_MAIN
    connect(ui->connectComPortPushButton, SIGNAL(clicked()),
            this, SLOT(comPortSlot()));

    comPort = new QSerialPort(this);
    connect(comPort, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(serialPortErrorSlot(QSerialPort::SerialPortError)));
    connect(this->ui->comPortSendOnePushButton, SIGNAL(clicked()),
            this, SLOT(sendOne()));
    connect(this->ui->comPortSendTwoPushButton, SIGNAL(clicked()),
            this, SLOT(sendTwo()));
#endif


    QSerialPort * comPort;
    comPort = new QSerialPort(this);

    comPort->setPortName(ui->comPortComboBox->currentText());
    comPort->open(QIODevice::WriteOnly);

    comPortDataStream.setDevice(comPort);




    /// socket
#if SOCKET_IN_MAIN
    socket = new QTcpSocket(this);
#endif

#if !DATA_READER
    socketDataStream.setDevice(socket);
    socketDataStream.setByteOrder(QDataStream::LittleEndian); // least significant bytes first
#endif

#if SOCKET_IN_MAIN
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketErrorSlot(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(connected()),
            this, SLOT(socketConnectedSlot()));
    connect(socket, SIGNAL(disconnected()),
            this, SLOT(socketDisconnectedSlot()));

    connect(ui->connectToServerPushButton, SIGNAL(clicked()),
            this, SLOT(connectSlot()));
    connect(ui->disconnectFromServerPushButton, SIGNAL(clicked()),
            this, SLOT(disconnectSlot()));
#endif
    /// "static" COM-port sending
    connect(this->ui->comPortSendOnePushButton, SIGNAL(clicked()),
            this, SLOT(sendOne()));
    connect(this->ui->comPortSendTwoPushButton, SIGNAL(clicked()),
            this, SLOT(sendTwo()));

    connect(this->ui->startPushButton, SIGNAL(clicked()),
            this, SLOT(startSlot()));
    connect(this->ui->endPushButton, SIGNAL(clicked()),
            this, SLOT(endSlot()));
    ui->fullDataCheckBox->setChecked(true);


    /// DataProcessor

    if(1)
    {
        def::eegData.resize(10 * def::windowLength);
//        cout << "list size = " << def::eegData.size() << endl;
        def::comPortName = ui->comPortComboBox->currentText();
        myNetThread = new QThread;
        myNetHandler = new NetHandler();

        myNetHandler->moveToThread(myNetThread);
        connect(myNetThread, SIGNAL(started()),
                myNetHandler, SLOT(startWork()));

        connect(myNetHandler, SIGNAL(finishWork()),
                myNetThread, SLOT(quit()));

        connect(myNetHandler, SIGNAL(finishWork()),
                myNetHandler, SLOT(deleteLater()));

        connect(myNetHandler, SIGNAL(finishWork()),
                myNetThread, SLOT(deleteLater()));

        myNetThread->start(QThread::HighestPriority); // veru fast
    }
    if(0)
    {
        myNetHandler->finishWork();
        myNetThread->wait();
        delete myNetThread;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

#if COM_IN_MAIN
void MainWindow::comPortSend(int a)
{
    comPortDataStream << qint8(a);
}

void MainWindow::serialPortErrorSlot(QSerialPort::SerialPortError)
{
    if(comPort->error() != 0)
    {
        ui->textEdit->append("serialPort error: "
                             + QString::number(comPort->error())
                             + " " + comPort->errorString());
    }
}

void MainWindow::comPortSlot()
{
    comPort->setPortName(ui->comPortComboBox->currentText());
    comPort->open(QIODevice::WriteOnly);
    comPortDataStream.setDevice(comPort);
    // cout comPortInfo
    if(comPort->isOpen())
    {
        ui->textEdit->append("serialPort opened: "
                             + ui->comPortComboBox->currentText());
        cout << "portName: " << comPort->portName().toStdString() << endl;
        cout << "dataBits: " << comPort->dataBits() << endl;
        cout << "baudRate: " << comPort->baudRate() << endl;
        cout << "dataTerminalReady: " << comPort->isDataTerminalReady() << endl;
        cout << "flowControl: " << comPort->flowControl() << endl;
        cout << "requestToSend: " << comPort->isRequestToSend() << endl;
        cout << "stopBits: " << comPort->stopBits() << endl << endl;
    }
}

#endif
void MainWindow::sendOne()
{
    comPortDataStream << qint8(1);
}
void MainWindow::sendTwo()
{
    comPortDataStream << qint8(2);
}


#if SOCKET_IN_MAIN
void MainWindow::socketErrorSlot(QAbstractSocket::SocketError)
{
    ui->textEdit->append("socket error: "
                         + QString::number(socket->error())
                         + " " + socket->errorString());
}

void MainWindow::socketDisconnectedSlot()
{
    ui->textEdit->append("socket disconnected from host");
}

void MainWindow::socketConnectedSlot()
{
    ui->textEdit->append("socket connected = "
                         + socket->peerAddress().toString()
                         + ":" + QString::number(socket->peerPort()));
}


void MainWindow::disconnectSlot()
{

    socket->disconnectFromHost();
}

void MainWindow::connectSlot()
{

//    socket->abort();
    socket->connectToHost(QHostAddress(ui->serverAddressLineEdit->text()),
                          ui->serverPortSpinBox->value()); // const

    if(socket->isValid())
    {
        ui->connectToServerPushButton->setCheckable(false);
//        connect(socket, SIGNAL(readyRead()),
//                this, SLOT(receiveDataSlot()));
    }
    else
    {
        socket->abort();
    }
}

#endif

void MainWindow::serverAddressSlot(int a)
{
    const QString & alias = ui->serverAddressComboBox->itemData(a).toString();
    const int semicol = alias.indexOf(':');

    def::hostAddress = QHostAddress(alias.left(semicol));
    def::hostPort = alias.right(alias.size() - semicol - 1).toInt();

    ui->serverAddressLineEdit->setText(def::hostAddress.toString());
    ui->serverPortSpinBox->setValue(def::hostPort);
}

void MainWindow::startSlot()
{
    def::fullDataFlag = ui->fullDataCheckBox->isChecked();
    myDataThread = new QThread;
    myDataReaderHandler = new DataReaderHandler();

    myDataReaderHandler->moveToThread(myDataThread);

//    connect(myDataReaderHandler, SIGNAL(finishReadData()),
//            myNetHandler, SLOT(finishSlot())); /// finish, when data read finishes;
    connect(myDataReaderHandler, SIGNAL(dataSend(eegDataType::iterator, eegDataType::iterator)),
            myNetHandler, SLOT(dataReceive(eegDataType::iterator, eegDataType::iterator)));


    connect(myDataThread, SIGNAL(started()),
            myDataReaderHandler, SLOT(startReadData()));
    connect(myDataReaderHandler, SIGNAL(finishReadData()),
            myDataThread, SLOT(quit()));
    connect(ui->endPushButton, SIGNAL(clicked()),
            myDataReaderHandler, SLOT(stopReadData()));

    connect(myDataReaderHandler, SIGNAL(finishReadData()),
            myDataReaderHandler, SLOT(deleteLater()));
    connect(myDataReaderHandler, SIGNAL(finishReadData()),
            myDataThread, SLOT(deleteLater()));
    connect(myDataThread, SIGNAL(finished()),
            myDataReaderHandler, SLOT(deleteLater()));


    connect(myDataReaderHandler, SIGNAL(retranslateMessage(QString)),
            this, SLOT(retranslateMessageSlot(QString)));

//    myDataThread->start(QThread::TimeCriticalPriority); // veru fast
    myDataThread->start(QThread::HighestPriority); // veru fast
}

void MainWindow::retranslateMessageSlot(QString a)
{
    ui->textEdit->append(a);
}

void MainWindow::endSlot() /// dont click twice anyway
{
    if(myDataReaderHandler)
    {
        myDataReaderHandler->finishReadData();
    }
    if(myDataThread)
    {
        myDataThread->wait();
    }
    myNetHandler->printAccuracy();
}



#if !DATA_READER

void MainWindow::startStopTransmisson()
{
    int var;
    socketDataStream >> var;

    QString res = (var==1)?"ON":"OFF";
    ui->textEdit->append("data transmission " + res);

    if(var == 1)
    {
        startSlot();
    }
    else if(this->inProcess)
    {
//        dataThread.join();
//        dataThread.~thread();
        connect(socket, SIGNAL(readyRead()),
                this, SLOT(receiveDataSlot()));
        if(!socketDataStream.atEnd())
        {
            socketDataStream.skipRawData(socketDataStream.device()->bytesAvailable());
        }
        if(!socketDataStream.atEnd())
        {
            cout << "still not at end" << endl;
        }

        std::ofstream ostr("D:\\MichaelAtanov\\Real-time\\TcpClient\\pew.txt");
        ostr << "NumOfSlices " << eegData.size() << endl;
        ostr << "NumOfChannels " << this->numOfChannels << endl;
        for(auto slice : eegData)
        {
            for(auto val : slice)
            {
                ostr << val << '\t';
            }
            ostr << '\n';
        }
        ostr.close();
    }
    this->inProcess = var;
}

void MainWindow::readStartInfo()
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


void MainWindow::dataSliceCame()
{
//    cout << "slice came" << '\t';
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
//               << "\t{";

//        static std::vector<short> oneSlice(numOfChans);
        short tmp;
        for(int i = 0; i < numOfSlices; ++i)
        {
            for(int j = 0; j < numOfChans; ++j)
            {
//                socketDataStream >> oneSlice[j];
                socketDataStream >> tmp;
//                cout << tmp << '\t';
            }
//            eegData.push_back(oneSlice);
//            ++WholeNumOfSlices;
        }
//        cout << "}";
//        cout << endl;

        /// test
#if 0
        if(eegData.size() > 10000)
        {
//            socket->disconnectFromHost();
//            std::ofstream ostr("/media/Files/Data/list.txt");
            std::ofstream ostr("D:\\MichaelAtanov\\Real-time\\TcpClient\\pew.txt");
            ostr << "NumOfSlices " << eegData.size();
            ostr << "NumOfChannels " << this->numOfChannels << endl;
            for(auto slice : eegData)
            {
                for(auto val : slice)
                {
                    ostr << val << '\t';
                }
                ostr << '\n';
            }
            ostr.close();
            exit(0);
        }
#endif


    }

}

void MainWindow::receiveDataSlot()
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

    /// protect against too many packets came - done
//    inPack.packArr = socketDataStream(inPack.packSize - sizeof(inPack.packId));

//    QDataStream dataStream(inPack.packArr);
//    dataStream.setByteOrder(QDataStream::LittleEndian); // least significant bytes first

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



#endif


