#-------------------------------------------------
#
# Project created by QtCreator 2014-11-28T13:32:20
#
#-------------------------------------------------

QT       += core gui serialport printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PCM2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    ada2.cpp \
    configwidget.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    ada2.h \
    configwidget.h

FORMS    += mainwindow.ui \
    configwidget.ui
