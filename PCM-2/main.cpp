#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    translator.load(QString("pcm_pl"));
    if(a.arguments().count() > 1)
    {
        if(a.arguments().at(1) == "-polish")
        {
            a.installTranslator(&translator);
        }
    }


    qRegisterMetaType<ADA2Device::ADASettings>("ADA2Device::ADASettings");
    qRegisterMetaType<ADA2Device::StartStopCommand>("ADA2Device::StartStopCommand");
    qRegisterMetaType<ADA2Device::ConnectionStatus>("ADA2Device::ConnectionStatus");
    qRegisterMetaType<ADA2Device::DeviceStatus>("ADA2Device::DeviceStatus");
    qRegisterMetaType<QVector<double> > ("QVector<double>") ;
    MainWindow w;


    w.show();

    return a.exec();
}
