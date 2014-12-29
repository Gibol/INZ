#include "ada2.h"

ADA2Device::ADA2Device(QObject *parent) :
    QSerialPort(parent)
{
    connectionStatus = Disconnected;
    deviceStatus = Idle;
    diagnosticResponseRecieved = false;
    currentSettings.compressionType = CompressionNone;
    currentSettings.samplingFrequency = F44_1KHZ;
    currentSettings.signalSource = TestSignal1;
    currentSettings.wordLenght = Word12Bits;
    currentSettings.signalOutput = AnalogOutput1;

    timerID = startTimer(TIMER_UPDATE_INTERVAL);
    connect(this, SIGNAL(readyRead()), this, SLOT(dataAvailable()));
    connect(this, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
}

void ADA2Device::timerEvent(QTimerEvent *event)
{

    event->accept();
    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();
    qDebug()<<"timer";
    if(connectionStatus == Disconnected)
    {
        qDebug()<<"Poszukiwanie urządzenia...";
        reset();
        _flushall();
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
                        sendDignosticFrame();
                        connectionStatus = Waiting;
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
        qDebug()<<"idle";
        static int waitingCount = 0;
        // if device is idle check if its responding
        sendDignosticFrame();
        if(diagnosticResponseRecieved)
        {
            waitingCount = 0;
            diagnosticResponseRecieved = false;
        }
        else
        {
            qDebug()<<"device not responding";
            connectionStatus = Waiting;
            waitingCount++;
            if(waitingCount >= 10)
            {
                connectionStatus = Disconnected;
            }
        }
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

void ADA2Device::sendConfigurationFrame()
{
    QByteArray data;
    quint16 mod = 0;
    data.append(100);
    data.append(55);
    data.append((quint8) currentSettings.samplingFrequency);
    mod += (quint8) currentSettings.samplingFrequency;
    data.append((quint8) currentSettings.compressionType);
    mod += (quint8) currentSettings.compressionType;
    data.append((quint8) currentSettings.wordLenght);
    mod += (quint8) currentSettings.wordLenght;
    data.append((quint8) currentSettings.signalSource);
    mod += (quint8) currentSettings.signalSource;
    data.append((quint8) currentSettings.signalOutput);
    mod += (quint8) currentSettings.signalOutput;
    QByteArray modArray(2, (char)0);
    modArray[1] = (quint8) mod;
    modArray[0] = (quint8) mod >> 8;
    data.append(modArray);
    data.append(101);
    write(data);
    waitForBytesWritten(100);
}

void ADA2Device::sendDignosticFrame()
{
    QByteArray data;
    data.append(100);
    data.append(44);
    data.append((char)0);
    data.append((char)0);
    data.append(101);
    write(data);
    qDebug()<<"write";
    waitForBytesWritten(100);
    qDebug()<<"wait";
}

void ADA2Device::dataAvailable()
{
    static QByteArray buffer;
    buffer.append(readAll());

    if(buffer.size() >= 56*40 && deviceStatus == Busy)
    {

        //searching for data frame
        while(buffer.at(0) != 100 || buffer.at(55) != 101 || buffer.at(1) != 89 || buffer.at(2) != 0)
        {
            buffer.remove(0,1);
            if(buffer.size() < 56*40) return;
        }

        QVector<double> samples;

        for(int i = 0; i < 40; i++)
        {
            static int badcnt = 0;
            QByteArray frame = buffer.mid(0,56);

            // checking again (next frames can be invalid)
            if(frame.at(0) != 100 || frame.at(55) != 101 || frame.at(1) != 89)
            {
                return;
            }


            //now check for other transmission erros by checking control sum;
            quint16 controlSum = 0;
            for(int b = 0; b < 50; b++)
            {
                controlSum += (quint8)frame[b+3];
            }
            quint16 controlSumInFrame = frame[53];
            controlSumInFrame <<= 8;
            controlSumInFrame &= 0xFF00;
            controlSumInFrame |= ((quint8)frame[54] &0xFF);

            if(controlSum != controlSumInFrame)
            {
                return;
            }
            else
            {
                for(int b = 0; b < 50; b+=2)
                {
                    quint16 sample = frame[3+b];
                    sample <<= 8;
                    sample &= 0xFF00;
                    sample |= (frame[4+b] & 0xFF);
                    samples.append( (double)sample);
                }

            }

            buffer.remove(0,56);
        }

        if(samples.size() == 1000)
        {
            emit newSampleData(samples);
            static int uu = 0;
            qDebug()<<"GOOD"<<uu++;
        }
        else
        {
            qDebug()<<samples.count();
        }
    }
    else if(buffer.size() >= 5)
    {
        //searching for response frame
        while(buffer.at(0) != 100 || buffer.at(4) != 101 || (buffer.at(1) != 44 && buffer.at(1) != 55) || buffer.at(2) != 0 || buffer.at(3) != 0)
        {
            buffer.remove(0,1);
            if(buffer.size() < 5) return;
        }

        if(buffer.at(1) == 44)
        {
            // response to discivery frame
            qDebug()<<"resp disco";
            connectionStatus = Connected;
            diagnosticResponseRecieved = true;
        }
        else if(buffer.at(1) == 55)
        {
            // response to config frame
            qDebug()<<"resp config";
        }
        buffer.clear();
    }

}

void ADA2Device::handleError(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::DeviceNotFoundError)
    {
        qDebug()<<errorString()<<error;
        close();
        connectionStatus = Disconnected;
    }

}
