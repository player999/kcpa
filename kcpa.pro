#-------------------------------------------------
#
# Project created by QtCreator 2016-01-19T11:11:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += serialport
TARGET = kcpa
TEMPLATE = app

LIBS += -lmodbus

SOURCES += main.cpp\
        kcpa.cpp

HEADERS  += kcpa.h

#INCLUDEPATH += /usr/include/x86_64-linux-gnu/qt5
INCLUDEPATH += "D:\home\taras\modbus\include"

LIBPATH += "D:\home\taras\modbus\lib"

FORMS    += kcpa.ui
