#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    connect(socket, SIGNAL(readyRead()),
            this, SLOT(receiveDataSlot()));



    connect(this->ui->endPushButton, SIGNAL(clicked()),
            this, SLOT(endSlot()));

    connect(this->ui->fullDataCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(changeFullDataFlag(int)));

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
}

void MainWindow::connectSlot()
{
//    socket->abort();
    socket->connectToHost(QHostAddress(ui->serverAddressLineEdit->text()),
                          ui->serverPortSpinBox->value()); // const
    if(socket->isValid())
    {
        ui->connectToServerPushButton->setCheckable(false);
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


void MainWindow::startStopTransmisson(QDataStream & str)
{
    int var;
    str >> var;
    this->inProcess = var;
    cout << var << endl;
    QString res = (var==1)?"ON":"OFF";
    ui->textEdit->append("data transmission " + res);
    if(var == 1)
    {
        QTimer::singleShot(5000, this, SLOT(startSlot())); /// not 5 sec, but some
    }
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

void MainWindow::readStartInfo(QDataStream & str)
{
    if(fullDataFlag)
    {
        std::string patientName = readString(str); // cyrillic "net pacienta"
//        cout << patientName << endl;

        str >> numOfChannels;
//        cout << numOfChannels << endl;
        for(int i = 0; i < numOfChannels; ++i)
        {
//            cout << endl << "channel " << i << endl;

            std::string channelName = readString(str);
//            cout << channelName << endl;

            double bitWeight; str >> bitWeight;
//            cout << bitWeight << endl;

            double samplingRate; str >> samplingRate;
//            cout << samplingRate << endl;

            std::string metrica = readString(str);
//            cout << metrica << endl;

            double LFF; str >> LFF;
//            cout << LFF << endl;

            double HFF; str >> HFF;
//            cout << HFF << endl;

            int levelHFF; str >> levelHFF;
//            cout << levelHFF << endl;

            double rejector; str >> rejector;
//            cout << rejector << endl;

            double defSans; str >> defSans;
//            cout << defSans << endl;
        }
        std::string scheme = readString(str);
//        cout << scheme << endl;
//        exit(0);
    }
    else
    {

        std::string patientName = readString(str); // cyrillic "net pacienta"
//        cout << patientName << endl;

        str >> numOfChannels;
//        cout << numOfChannels << endl;

        for(int i = 0; i < numOfChannels; ++i)
        {
//            cout << endl << "channel " << i << endl;

            std::string channelName = readString(str);
//            cout << channelName << endl;

        }
        std::string scheme = readString(str);
//        cout << scheme << endl;

        str >> bitWeight;
//        cout << bitWeight << endl;

        str >> samplingRate;
//        cout << samplingRate << endl;
//        exit(0);
    }
}


void MainWindow::dataSliceCame(QDataStream & str)
{
    cout << "slice came" << endl;
    if(fullDataFlag)
    {
        int sliceNumber;
        str >> sliceNumber;

        int numOfChans;
        str >> numOfChans;

        long long numOfSlices;
        str >> numOfSlices;


        static std::vector<short> oneSlice(numOfChans);
        for(int i = 0; i < numOfSlices; ++i)
        {
            for(int j = 0; j < numOfChans; ++j)
            {
                str >> oneSlice[j];
            }
            eegData.push_back(oneSlice);
        }
        /// test
        if(eegData.size() > 1000)
        {
            socket->disconnectFromHost();
//            std::ofstream ostr("/media/Files/Data/list.txt");
            std::ofstream ostr("D:\\MichaelAtanov\\Real-time\\TcpClient\\pew.txt");
            ostr << "NumOfSlices " << eegData.size();
            ostr << "NumOfChannels " << this->numOfChannels << endl;
            for(auto slice : eegData)
            {
                for(auto val : slice)
                {
                    cout << val << '\t';
                }
                cout << '\n';
            }
            ostr.close();
            exit(0);
        }




    }

}

void MainWindow::receiveDataSlot()
{

    cout << "ready to read" << endl;
    QDataStream in(socket);
    in.setByteOrder(QDataStream::LittleEndian); // least significant bytes first
    static enc::Pack inPack;


    while(socket->bytesAvailable() < sizeof(inPack.packSize) && inPack.packSize == 0)
    {
        cout << "wait for size" << endl;
        this_thread::sleep_for(std::chrono::microseconds{100});
    }

    char tmp;
    in.device()->peek(&tmp, 1);
    in >> inPack.packSize;
    if(int(tmp) == -1) /// initial data for not fullData
    {
        inPack.packSize = 259 - sizeof(inPack.packSize);
    }
    cout << "packSize = " << inPack.packSize << endl;

//    }
//    else
//    {
//        /// happens almost never
//        cout << "pack not big enough 1" << endl;
//        return;
//    }


    while(socket->bytesAvailable() < sizeof(inPack.packId) && inPack.packId == 0)
    {
        cout << "wait for id" << endl;
        this_thread::sleep_for(std::chrono::microseconds{100});
    }

//    if(socket->bytesAvailable() >= inPack.packSize && inPack.packId == 0)

//    {
    in >> inPack.packId;
    cout << "packId = " << inPack.packId << endl;


    while(socket->bytesAvailable() < inPack.packSize - sizeof(inPack.packId))
    {
        cout << "wait for data" << endl;
        this_thread::sleep_for(std::chrono::microseconds{100});
    }

    /// protect against too many packets came - done
    inPack.packArr = in.device()->read(inPack.packSize - sizeof(inPack.packId));

    QDataStream dataStream(inPack.packArr);
    dataStream.setByteOrder(QDataStream::LittleEndian); // least significant bytes first

    switch(inPack.packId)
    {
    case 2:
    {
        /// initial data
        readStartInfo(dataStream);
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
        startStopTransmisson(dataStream);
        break;
    }
    case 9:
    {
        /// FP marker
        break;
    }
    case 8:
    {
        /// data slice
        dataSliceCame(dataStream);
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
//    cout << "startSlot: written " << bytesWritten << " bytes" << endl;

    /// get ready to have an answer
//    socket->write(initialRequest);
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
