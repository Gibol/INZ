#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<ADA2Device::ADASettings>("ADA2Device::ADASettings");
    qRegisterMetaType<ADA2Device::StartStopCommand>("ADA2Device::StartStopCommand");
    qRegisterMetaType<ADA2Device::ConnectionStatus>("ADA2Device::ConnectionStatus");
    qRegisterMetaType<ADA2Device::DeviceStatus>("ADA2Device::DeviceStatus");
    qRegisterMetaType<QVector<double> > ("QVector<double>") ;
    MainWindow w;


    w.show();

    return a.exec();
}
