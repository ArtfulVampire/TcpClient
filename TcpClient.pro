#-------------------------------------------------
#
# Project created by QtCreator 2016-02-04T23:12:00
#
#-------------------------------------------------

QT       += core gui
QT += network
QT += serialport
QMAKE_CXXFLAGS = -std=c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TcpClient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    def.h

FORMS    += mainwindow.ui
