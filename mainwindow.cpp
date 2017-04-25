#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QShortcut>

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
	QObject::connect(ui->serverAddressComboBox, SIGNAL(highlighted(int)),
					 this, SLOT(serverAddressSlot(int)));
	QObject::connect(ui->serverAddressComboBox, SIGNAL(currentIndexChanged(int)),
					 this, SLOT(serverAddressSlot(int)));

	ui->serverAddressComboBox->setCurrentText("pew"); /// fix via router, always 192.168.0.104


	/// com
	for(int i = 0; i < 9; ++i)
	{
		ui->comPortComboBox->addItem("COM"+QString::number(i+1));
	}
	ui->comPortComboBox->setCurrentText("COM5");


	ui->fullDataCheckBox->setChecked(true);

#if 0
	/// COM test
	QSerialPort * comPort;
	comPort = new QSerialPort(this);

	comPort->setPortName(ui->comPortComboBox->currentText());
	comPort->open(QIODevice::WriteOnly);

	if(comPort->isOpen())
	{
		std::cout << comPort->error() << std::endl;
		std::cout << comPort->errorString() << std::endl;
		std::cout << "serialPort opened: " + def::comPortName << std::endl;
//		std::cout << "portName: " << comPort->portName().toStdString() << std::endl;
//		std::cout << "dataBits: " << comPort->dataBits() << std::endl;
//		std::cout << "baudRate: " << comPort->baudRate() << std::endl;
//		std::cout << "dataTerminalReady: " << comPort->isDataTerminalReady() << std::endl;
//		std::cout << "flowControl: " << comPort->flowControl() << std::endl;
//		std::cout << "requestToSend: " << comPort->isRequestToSend() << std::endl;
//		std::cout << "stopBits: " << comPort->stopBits() << endl << std::endl;
	}

	comPortDataStream.setDevice(comPort);
	ui->comPortSpinBox->setMaximum(def::numFbGradation);


	/// "static" COM-port sending
	QObject::connect(this->ui->comPortSendOnePushButton, &QPushButton::clicked,
					 [this](){this->comPortDataStream << qint8(1);});
	QObject::connect(this->ui->comPortSendTwoPushButton, &QPushButton::clicked,
					 [this](){this->comPortDataStream << qint8(2);});
	QObject::connect(this->ui->comPortSendPushButton, &QPushButton::clicked,
					 [this](){this->comPortDataStream << qint8(ui->comPortSpinBox->value());});

#endif




	/// right/wrong shortcuts
	QShortcut * rightShortcut = new QShortcut(QKeySequence(def::rightKey), this);
	QShortcut * wrongShortcut = new QShortcut(QKeySequence(def::wrongKey), this);

	QObject::connect(rightShortcut,
					 &QShortcut::activated,
					 []()
	{
		std::cout << "right" << std::endl;
		def::solved = def::solveType::right;
	});

	QObject::connect(wrongShortcut,
					 &QShortcut::activated,
					 []()
	{
		std::cout << "wrong" << std::endl;
		def::solved = def::solveType::wrong;
	});


	/// start/stop
	QObject::connect(this->ui->startPushButton, SIGNAL(clicked()),
					 this, SLOT(startSlot()));
	QObject::connect(this->ui->endPushButton, SIGNAL(clicked()),
					 this, SLOT(endSlot()));



	/// DataProcessor
	if(1)
	{
		def::eegData.resize(10 * def::windowLength);
		def::comPortName = ui->comPortComboBox->currentText();
		myNetThread = new QThread;
		myNetHandler = new NetHandler();

		myNetHandler->moveToThread(myNetThread);
		QObject::connect(myNetThread, SIGNAL(started()),
						 myNetHandler, SLOT(startWork()));

		QObject::connect(myNetHandler, SIGNAL(finishWork()),
						 myNetThread, SLOT(quit()));

		QObject::connect(myNetHandler, SIGNAL(finishWork()),
						 myNetHandler, SLOT(deleteLater()));

		QObject::connect(myNetHandler, SIGNAL(finishWork()),
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

//	QObject::connect(myDataReaderHandler, SIGNAL(finishReadData()),
//					 myNetHandler, SLOT(finishSlot())); /// finish, when data read finishes;
	QObject::connect(myDataReaderHandler, SIGNAL(dataSend(eegDataType::iterator, eegDataType::iterator)),
					 myNetHandler, SLOT(dataReceive(eegDataType::iterator, eegDataType::iterator)));


	QObject::connect(myDataThread, SIGNAL(started()),
					 myDataReaderHandler, SLOT(startReadData()));
	QObject::connect(myDataReaderHandler, SIGNAL(finishReadData()),
					 myDataThread, SLOT(quit()));
	QObject::connect(ui->endPushButton, SIGNAL(clicked()),
					 myDataReaderHandler, SLOT(stopReadData()));

	QObject::connect(myDataReaderHandler, SIGNAL(finishReadData()),
					 myDataReaderHandler, SLOT(deleteLater()));
	QObject::connect(myDataReaderHandler, SIGNAL(finishReadData()),
					 myDataThread, SLOT(deleteLater()));
	QObject::connect(myDataThread, SIGNAL(finished()),
					 myDataReaderHandler, SLOT(deleteLater()));


	QObject::connect(myDataReaderHandler, SIGNAL(retranslateMessage(QString)),
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
