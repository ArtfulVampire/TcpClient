#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>



#include "def.h"
#include "datareader.h"
#include "classifier.h"
#include "biglib.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	bool eventFilter(QObject * obj, QEvent * ev);

public slots:
	// ui slots
	void serverAddressSlot(int a);
	void startSlot();
	void endSlot(); /// is needed?

	void retranslateMessageSlot(QString); /// from dataReader


private:
	Ui::MainWindow * ui;

	QThread * myDataThread = nullptr;
	DataReaderHandler * myDataReaderHandler = nullptr;

	QThread * myNetThread = nullptr;
	NetHandler * myNetHandler = nullptr;

	QDataStream comPortDataStream; /// needed for comPort test in main
};

#endif // MAINWINDOW_H
