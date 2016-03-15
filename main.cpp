#include "mainwindow.h"
#include <QApplication>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    fixFilesSlashN("F:/AAU/Spectra_windows/"); exit(0);
    MainWindow w;
    w.show();

    return a.exec();
}
