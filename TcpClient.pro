#-------------------------------------------------
#
# Project created by QtCreator 2016-02-04T23:12:00
#
#-------------------------------------------------

QT       += core gui svg
QT += network
QT += serialport
QMAKE_CXXFLAGS += -Wno-sign-compare
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-result
QMAKE_CXXFLAGS += -std=c++14
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TcpClient
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    datareader.cpp \
    def.cpp \
    classifier.cpp \
    matrix.cpp \
    biglib.cpp

HEADERS  += mainwindow.h \
    def.h \
    datareader.h \
    classifier.h \
    lib.h \
    matrix.h \
    biglib.h

FORMS    += mainwindow.ui
