#include "ada2.h"
#include <QThread>

ADA2Device::ADA2Device(QObject *parent) :
    QSerialPort(parent)
{
    connectionStatus = Disconnected;
    deviceStatus = Idle;
    diagnosticResponseRecieved = false;
    currentSettings = initSettingsStructure();
    frameTimeoutCounter = 0;

    timerID = startTimer(TIMER_UPDATE_INTERVAL);
    connect(this, SIGNAL(readyRead()), this, SLOT(dataAvailable()));
    connect(this, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
}

ADA2Device::ADASettings ADA2Device::initSettingsStructure()
{
    ADASettings initStructure;

    initStructure.signalOutput = ADA2Device::AnalogOutput1;
    initStructure.bitError = ADA2Device::BitNone;
    initStructure.compressionType = ADA2Device::CompressionNone;
    initStructure.samplingFrequency = ADA2Device::F44_1KHZ;
    initStructure.signalSource = ADA2Device::AnalogInput1;
    initStructure.wordLenght = ADA2Device::Word12Bits;

    return initStructure;
}

void ADA2Device::newSettings(ADA2Device::ADASettings settings)
{
    currentSettings = settings;
    sendConfigurationFrame();
}

void ADA2Device::timerEvent(QTimerEvent *event)
{

    event->accept();

    QList<QSerialPortInfo> availablePorts = QSerialPortInfo::availablePorts();

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
    else if(deviceStatus == Busy)
    {
        frameTimeoutCounter++;
        if(frameTimeoutCounter == 10)
        {
            sendStartStopCommand(Stop);
            frameTimeoutCounter = 0;
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
    data.append((quint8) currentSettings.bitError);
    mod += (quint8) currentSettings.bitError;
    QByteArray modArray(2, (char)0);
    modArray[1] = (quint8) mod;
    modArray[0] = (quint8) mod >> 8;
    data.append(modArray);
    data.append(101);
    write(data);
    qDebug()<<"write config";
    waitForBytesWritten(100);
}

void ADA2Device::sendStartStopCommand(ADA2Device::StartStopCommand cmd)
{
    QByteArray data;
    data.append(100);
    data.append(66);
    data.append((char) cmd);
    data.append((char) 0);
    data.append((char) cmd);
    data.append(101);
    write(data);
    waitForBytesWritten(100);
    if(cmd == Start) deviceStatus = Busy;
    else deviceStatus = Idle;
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
    waitForBytesWritten(100);
}

void ADA2Device::dataAvailable()
{
    static QByteArray buffer;
    buffer.append(readAll());

    if(buffer.size() >= 56*((currentSettings.wordLenght == Word12Bits) ? SAMPLE_BUFFER_SIZE : (SAMPLE_BUFFER_SIZE/2))/50 && deviceStatus == Busy)
    {
        qDebug()<<buffer.toHex()<<buffer.size();
        //searching for data frame
        while(buffer.at(0) != 100 || buffer.at(55) != 101 || buffer.at(1) != 89 || buffer.at(2) != 0)
        {
            buffer.remove(0,1);
            if(buffer.size() < 56*((currentSettings.wordLenght == Word12Bits) ? SAMPLE_BUFFER_SIZE : (SAMPLE_BUFFER_SIZE/2))/50) return;
        }

        QVector<double> samples;

        for(int i = 0; i < ((currentSettings.wordLenght == Word12Bits) ? SAMPLE_BUFFER_SIZE : (SAMPLE_BUFFER_SIZE/2))/50; i++)
        {
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
            controlSumInFrame |= ((quint8)frame[54]);

            if(controlSum != controlSumInFrame)
            {
                return;
            }
            else
            {
                for(int b = 0; b < 50; b++)
                {

                    if(currentSettings.wordLenght == Word12Bits)
                    {
                        quint16 sample = (quint8)frame[3+b];
                        sample <<= 8;
                        sample &= 0xFF00;
                        sample |= (quint8)(frame[4+b]);
                        b++;
                        samples.append( (double)((qint16)sample));
                    }
                    else
                    {
                        samples.append( (double)((qint8)frame[3+b]));
                    }

                }

            }

            buffer.remove(0,56);
        }

        if(samples.size() == SAMPLE_BUFFER_SIZE/2)
        {
            emit newSampleData(samples);
            frameTimeoutCounter = 0;
        }
        else
        {
            qDebug()<<samples.count();
        }
    }
    else if(buffer.size() >= 5 && deviceStatus == Idle)
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
