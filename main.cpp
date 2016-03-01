#include "mainwindow.h"
#include <QApplication>



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    for(double ecr : {0.5, 0.01, 0.02, 0.03, 0.04, 0.05})
//    {
//        for(double lr : {0.5, 0.01, 0.02, 0.03, 0.04, 0.05})
//        {
//            for(double err = 1.0; err >= 0.0; err -= 0.1)
//            {
//                def::tempErrcrit = ecr;
//                def::tempLrate = lr;
//                def::tempError = err;
//                std::cout << ecr << std::endl;

//                MainWindow * w =  new MainWindow();
//                delete w;
//            }
//        }
//    }
    MainWindow w;
    w.show();

    return a.exec();
}
