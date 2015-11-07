#-------------------------------------------------
#
# Project created by QtCreator 2015-08-25T11:00:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = AdamsQuickAndDirtySerialReadWrite
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    serialportmanager.cpp \
    filewriter.cpp \
    settingsdialog.cpp

HEADERS  += mainwindow.h \
    serialportmanager.h \
    filewriter.h \
    settingsdialog.h

FORMS    += mainwindow.ui \
    settingsdialog.ui
