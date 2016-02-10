#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace enc;
using namespace std;
using namespace std::chrono;


static int WholeNumOfSlices = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // defaults
    ui->fullDataCheckBox->setChecked(this->fullDataFlag);

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
                                           QVariant("127.0.0.1:35577"));
    connect(ui->serverAddressComboBox, SIGNAL(highlighted(int)),
            this, SLOT(serverAddressSlot(int)));
    connect(ui->serverAddressComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(serverAddressSlot(int)));
    ui->serverAddressComboBox->setCurrentText("Enceph");

    /// com
    for(int i = 0; i < 10; ++i)
    {
        ui->comPortComboBox->addItem("COM"+QString::number(i+1));
    }
    connect(ui->connectComPortPushButton, SIGNAL(clicked()),
            this, SLOT(comPortSlot()));


    /// socket
    socket = new QTcpSocket(this);
    socketDataStream.setDevice(socket);
    socketDataStream.setByteOrder(QDataStream::LittleEndian); // least significant bytes first

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

    connect(this->ui->startPushButton, SIGNAL(clicked()),
            this, SLOT(startSlot()));





    connect(this->ui->endPushButton, SIGNAL(clicked()),
            this, SLOT(endSlot()));

    connect(this->ui->fullDataCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(changeFullDataFlag(int)));

    connect(this->ui->startRealTimePushButton, SIGNAL(clicked()),
            this, SLOT(dataReadThread()));

//    cout << char(-1) << endl; exit(0);


    ui->fullDataCheckBox->setChecked(true);
    connectSlot();
    startSlot();


    /// custom



//    comPort = new QSerialPort(this);
//    connect(comPort, SIGNAL(error(QSerialPort::SerialPortError)),
//            this, SLOT(serialPortErrorSlot(QSerialPort::SerialPortError)));
//    comPort->setPortName(ui->comPortComboBox->currentText());
//    comPort->open(QIODevice::WriteOnly);
//    // cout comPortInfo
//    if(comPort->isOpen())
//    {
//        cout << "portName: " << comPort->portName().toStdString() << endl;
//        cout << "dataBits: " << comPort->dataBits() << endl;
//        cout << "baudRate: " << comPort->baudRate() << endl;
//        cout << "dataTerminalReady: " << comPort->isDataTerminalReady() << endl;
//        cout << "flowControl: " << comPort->flowControl() << endl;
//        cout << "requestToSend: " << comPort->isRequestToSend() << endl;
//        cout << "stopBits: " << comPort->stopBits() << endl;
//    }
//    char ch(1);
//    const char * tmp = &ch;
//    comPort->write(tmp);


}

MainWindow::~MainWindow()
{
    delete ui;
//    socket->close();
//    delete socket;
}

void MainWindow::changeFullDataFlag(int a)
{
    this->fullDataFlag = a;
}

void MainWindow::serialPortErrorSlot(QSerialPort::SerialPortError)
{
    ui->textEdit->append("serialPort error: "
                         + QString::number(comPort->error())
                         + " " + comPort->errorString());
}

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
    disconnect(socket, SIGNAL(readyRead()),
            this, SLOT(receiveDataSlot()));
}

void MainWindow::connectSlot()
{
//    socket->abort();
    socket->connectToHost(QHostAddress(ui->serverAddressLineEdit->text()),
                          ui->serverPortSpinBox->value()); // const

    if(socket->isValid())
    {
        ui->connectToServerPushButton->setCheckable(false);
        connect(socket, SIGNAL(readyRead()),
                this, SLOT(receiveDataSlot()));
    }
    else
    {
        socket->abort();
    }
}

void MainWindow::serverAddressSlot(int a)
{
    const QString & alias = ui->serverAddressComboBox->itemData(a).toString();
    const int semicol = alias.indexOf(':');

    ui->serverAddressLineEdit->setText(alias.left(semicol));
    ui->serverPortSpinBox->setValue(alias.right(alias.size() - semicol - 1).toInt());

}


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
        dataThread.detach();
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
    }
    this->inProcess = var;
}

std::string readString(QDataStream & in)
{
    int numOfChars;
    in >> numOfChars;
    std::string res;
    res.resize(numOfChars);
    for(int i = 0; i < numOfChars; ++i)
    {
        in.readRawData(&(res[i]), 1);
    }
    return res;
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
        int sliceNumber;
        socketDataStream >> sliceNumber;

        int numOfChans;
        socketDataStream >> numOfChans;

        qint32 numOfSlices; // not really 'long'
        socketDataStream >> numOfSlices;

        cout << sliceNumber << '\t'
             << numOfChans << '\t'
             << numOfSlices << endl;
//        "\t{";

//        std::vector<short> oneSlice(numOfChans);
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
//        cout << "}" << endl;

        /// test
#if 0
        if(eegData.size() > 1000)
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
//    this_thread::sleep_for(milliseconds{2.5});
    this_thread::sleep_for(microseconds{3000});

    static enc::Pack inPack; /// maybe bad with parallel slots
    static int waitCounter = 0;

    if(inPack.packSize == 0)
    {
        if(socket->bytesAvailable() < sizeof(inPack.packSize))
        {
            return;
        }

        char tmp;
        socketDataStream.device()->peek(&tmp, 1);
        socketDataStream >> inPack.packSize;
        if(int(tmp) == -1) /// initial data for not fullData
        {
            inPack.packSize = 259 - sizeof(inPack.packSize);
        }
        if(inPack.packSize > 2000 || inPack.packSize < 0)
        {
            cout << "bad pack size " << inPack.packSize << " , will exit" << endl;
            exit(9);
        }
    }


    if(inPack.packId == 0)
    {
        if(socket->bytesAvailable() < sizeof(inPack.packId))
        {
            cout << "wait for id" << endl;
            ++waitCounter;
//            if(waitCounter == 1000)
//            {
//                cout << "too much wait id" << endl;
//                exit(7);
//            }
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
    if(socket->bytesAvailable() < (inPack.packSize - sizeof(inPack.packId)))
    {
        cout << "wait for data\t";
        cout << socket->bytesAvailable() << "\t" << inPack.packSize - sizeof(inPack.packId) << endl;
//        ++waitCounter;
//        if(waitCounter == 1000)
//        {
//            cout << "too much wait data" << endl;
//            exit(7);
//        }
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

void MainWindow::startSlot()
{
//    QByteArray initialRequest;
//    QDataStream out(&initialRequest, QIODevice::WriteOnly);

    enc::Pack startPack;
    startPack.packSize = sizeof(DWORD);
    if(ui->fullDataCheckBox->isChecked())
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



    /// get ready to have an answer
//    socket->write(initialRequest);
}

void MainWindow::dataReadThread()
{
//    DataThread * thr = new DataThread(this->socket, this->fullDataFlag);
//    connect(thr, SIGNAL(finished()), thr, SLOT(deleteLater()));
//    cout << "pew" << endl;
//    /// reconnect again
//    thr->start(QThread::TimeCriticalPriority); // highest priority


    cout << "before thread start" << endl;

    dataThread = std::thread([this]()
    {
        while(1)
        {
            this->receiveDataSlot();
        }
    });
//    while(1)
//    {
//        this->receiveDataSlot();
//    }
    cout << "after thread start" << endl;
}

void MainWindow::endSlot()
{

}

void MainWindow::comPortSlot()
{

}

void MainWindow::askServer()
{
    cout << "ask" << endl;
    blockSize = 0;
    socket->abort();
    socket->connectToHost(ui->serverAddressLineEdit->text(),
                          ui->serverPortSpinBox->value());
}

void MainWindow::react()
{
    cout << "react" << endl;
    QDataStream in(socket);

    if (blockSize == 0)
    {
        cout << "blockSize == 0" << endl;
        if (socket->bytesAvailable() < (int)sizeof(quint16))
        {
            cout << "wait for size" << endl;
            return;
        }
        in >> blockSize;
        cout << "blockSize = " << blockSize << endl;
    }

    if (socket->bytesAvailable() < blockSize)
    {
        cout << "wait for data" << endl;
        return;
    }

//    QString gotString;
//    in >> gotString;
//    cout << gotString.toStdString() << endl;

    char * gotString = new char[blockSize + 2];
    uint amountReadBytes;
    in.readBytes(gotString, amountReadBytes);
    cout << amountReadBytes << "\t" << gotString << endl;
    delete[] gotString;
}
