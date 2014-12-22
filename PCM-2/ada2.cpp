#include "ada2.h"

ADA2Device::ADA2Device(QObject *parent) :
    QSerialPort(parent)
{
    connectionStatus = Disconnected;
    timerID = startTimer(TIMER_UPDATE_INTERVAL);
    connect(this, SIGNAL(readyRead()), this, SLOT(dataAvailable()));
}

void ADA2Device::timerEvent(QTimerEvent *event)
{

    event->accept();
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();

    if(connectionStatus == Disconnected)
    {
        qDebug()<<"Poszukiwanie urządzenia...";
        foreach(QSerialPortInfo i, availablePorts)
        {
            if(i.productIdentifier() == 22336 && i.vendorIdentifier() == 1155)
            {
                qDebug()<<"Znaleziono urządzenie ADA-2, otwieranie połączenia na"<<i.description();
                setPort(i);
                if(open(QIODevice::ReadWrite))
                {
                    qDebug()<<"port otwarty";
                    if(setPortSettings())
                    {
                        qDebug()<<"Ustawienie parametrow transmisji powiodło się.";
                        connectionStatus = Connected;
                        // send frame to device for confirmation;
                    }
                }
                else
                {
                    qDebug()<<"błąd otwarcia portu";
                    close();
                }
            }

        }
    }
    else if(deviceStatus == Idle)
    {
        // if device is idle check if its responding
        // send frame
    }

}

bool ADA2Device::setPortSettings()
{
    bool ok = true;
    ok &= setBaudRate(Baud115200);
    ok &= setParity(QSerialPort::NoParity);
    ok &= setFlowControl(QSerialPort::HardwareControl);
    ok &= setDataBits(QSerialPort::Data8);
    ok &= setStopBits(QSerialPort::OneStop);

    return ok;
}

void ADA2Device::dataAvailable()
{
    static QByteArray buffer;
    buffer.append(readAll());

    if(buffer.size() >= 56*40)
    {

        //searching for frame
        while(buffer.at(0) != 100 || buffer.at(55) != 101 || buffer.at(1) != 89 || buffer.at(2) != 0)
        {
            buffer.remove(0,1);
            if(buffer.size() < 56*40) return;
        }

        qDebug()<<buffer.toHex();
        buffer.clear();
        return;


        for(int i = 0; i < buffer.size()/56; i++)
        {
            static int badcnt = 0;
            QByteArray candidate = buffer.mid(0,56);

            // checking again (next frames can be invalid)
            if(candidate.at(0) != 100 || candidate.at(55) != 101 || candidate.at(1) != 89)
            {

                qDebug()<<badcnt++<<"BAD FRAME!"<<candidate.toHex();
                return;
            }
            //data was properly alligned, we can remove it;
            buffer.remove(0,56);

            //now check for other transmission erros by checking control sum;
            quint16 controlSum = 0;
            for(int b = 0; b < 50; b++)
            {
                controlSum += (quint8)candidate[b+3];
                //qDebug()<<QString::number(controlSum, 16);
            }
            quint16 controlSumInFrame = candidate[53];
                    controlSumInFrame <<= 8;
                    controlSumInFrame &= 0xFF00;
                    controlSumInFrame |= ((quint8)candidate[54] &0xFF);

            if(controlSum != controlSumInFrame)
            {
                qDebug()<<badcnt++<<"BAD FRAME (CRC)!"<<controlSum<<controlSumInFrame<<candidate.mid(53).toHex();
                //qDebug()<<candidate.mid(2,2000).toHex();
            }
            else
            {
//                static int goodCnt = 0;
//                qDebug()<<"GOOD CNT"<<goodCnt++;
                qDebug()<<QString::number(candidate[2]);
                if(((quint8)candidate[2]) == 39) buffer.clear();

                QVector<double> samples;
                for(int b = 0; b < 50; b+=2)
                {
                    quint16 sample = candidate[2+b];
                    sample <<= 8;
                    sample &= 0xFF00;
                    sample |= (candidate[3+b] & 0xFF);
                    samples.append( (double)sample);
                }
                emit newSampleData(samples);
            }
        }


    }

}
