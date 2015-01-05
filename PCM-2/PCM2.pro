#-------------------------------------------------
#
# Project created by QtCreator 2014-11-28T13:32:20
#
#-------------------------------------------------

QT       += core gui serialport printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PCM2
TEMPLATE = app

RC_FILE = icon.rc

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    ada2.cpp \
    configwidget.cpp \
    g711.c \
    statusindicator.cpp \
    simpleindicator.cpp \
    aboutwidget.cpp \
    didacticswidget.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    configwidget.h \
    ada2.h \
    g711.h \
    statusindicator.h \
    simpleindicator.h \
    aboutwidget.h \
    didacticswidget.h

FORMS    += mainwindow.ui \
    configwidget.ui \
    statusindicator.ui \
    aboutwidget.ui \
    didacticswidget.ui

RESOURCES += \
    images.qrc

OTHER_FILES += \
    icon.rc
