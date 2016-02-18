#-------------------------------------------------
#
# Project created by QtCreator 2016-02-04T23:12:00
#
#-------------------------------------------------

QT       += core gui svg
QT += network
QT += serialport
QMAKE_CXXFLAGS = -std=c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TcpClient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    datareader.cpp \
    def.cpp \
    classifier.cpp \
    lib.cpp \
    matrix.cpp

HEADERS  += mainwindow.h \
    def.h \
    datareader.h \
    classifier.h \
    lib.h \
    matrix.h

FORMS    += mainwindow.ui
