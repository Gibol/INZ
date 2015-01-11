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
    initStructure.compressionType = ADA2Device::None;
    initStructure.samplingFrequency = ADA2Device::F8KHZ;
    initStructure.signalSource = ADA2Device::AnalogInput1;
    initStructure.wordLenght = ADA2Device::Word12bits;
    initStructure.analogCompressionParam = 10; //1.0;

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
        //qDebug()<<"Poszukiwanie urządzenia...";
        emit message(tr("Searching for device..."));
        reset();
        _flushall();
        foreach(QSerialPortInfo i, availablePorts)
        {
            if(i.productIdentifier() == 22336 && i.vendorIdentifier() == 1155)
            {
                //qDebug()<<"Znaleziono urządzenie ADA-2, otwieranie połączenia na"<<i.description();
                emit message(tr("Found ADA-2 device, opening connection on ")+i.description());
                setPort(i);
                if(open(QIODevice::ReadWrite))
                {
                    //qDebug()<<"port otwarty";
                    emit message(tr("Port Opened."));
                    if(setPortSettings())
                    {
                        //qDebug()<<"Connection parameters set correctly.";
                        emit message(tr("Connection parameters set correctly."));
                        sendDignosticFrame();
                        connectionStatus = Waiting;
                    }
                }
                else
                {
                    //qDebug()<<"błąd otwarcia portu";
                    emit message(tr("Error opening port"));
                    close();
                }
            }

        }
    }
    else if(deviceStatus == Idle || deviceStatus == Configured)
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
            //qDebug()<<"device not responding";
            connectionStatus = Waiting;
            waitingCount++;
            if(waitingCount >= 10)
            {
                connectionStatus = Disconnected;
                deviceStatus = Idle;
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

    emit deviceStatusChanged(deviceStatus);
    emit connectionStatusChanged(connectionStatus);

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
    buffer.clear();

    QByteArray data;
    quint16 mod = 0;
    data.append(100);
    data.append(55);
    data.append((quint8) currentSettings.samplingFrequency);
    mod += (quint8) currentSettings.samplingFrequency;
    data.append((quint8) currentSettings.compressionType);
    mod += (quint8) currentSettings.compressionType;
    data.append(((quint8) currentSettings.wordLenght));
    mod += (quint8) currentSettings.wordLenght;
    data.append((quint8) currentSettings.signalSource);
    mod += (quint8) currentSettings.signalSource;
    data.append((quint8) currentSettings.signalOutput);
    mod += (quint8) currentSettings.signalOutput;
    data.append((quint8) currentSettings.bitError);
    mod += (quint8) currentSettings.bitError;
    data.append((quint8) (currentSettings.analogCompressionParam /10));
    mod += ((quint8) (currentSettings.analogCompressionParam /10));
    data.append((quint8) (currentSettings.analogCompressionParam % 10));
    mod += ((quint8) (currentSettings.analogCompressionParam %10));
    QByteArray modArray(2, (char)0);
    modArray[1] = (quint8) mod;
    modArray[0] = (quint8) (mod >> 8);
    data.append(modArray);
    data.append(101);
    write(data);
    deviceStatus = Idle;
    waitForBytesWritten(100);
}

void ADA2Device::sendStartStopCommand(ADA2Device::StartStopCommand cmd)
{
    buffer.clear();

    QByteArray data;
    data.append(100);
    data.append(66);
    data.append((char) cmd);
    data.append((char) 0);
    data.append((char) cmd);
    data.append(101);
    write(data);
    waitForBytesWritten(100);

    if(cmd == Start)
    {
        deviceStatus = Busy;
    }
    else
    {
        deviceStatus = Configured;
    }


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
    buffer.append(readAll());

    static const quint8 dataFrameSize = 56;
    quint16 expextedPayload = (dataFrameSize*SAMPLE_BUFFER_SIZE)/((currentSettings.wordLenght > Word8bits) ? 25 : 50);

    if(buffer.size() >= expextedPayload && deviceStatus == Busy)
    {
        //searching for data frame
        while(buffer.at(0) != 100 || buffer.at(55) != 101 || !(buffer.at(1) == 89 || buffer.at(1) == 88 )|| buffer.at(2) != 0)
        {
            buffer.remove(0,1);
            if(buffer.size() < expextedPayload) return;
        }

        QVector<double> samples;
        //qDebug()<<buffer.size();
        for(int i = 0; i < expextedPayload/dataFrameSize; i++)
        {
            QByteArray frame = buffer.mid(0,dataFrameSize);

            // checking again (next frames can be invalid)
            if(frame.at(0) != 100 || frame.at(55) != 101)
            {
                return;
            }
            if(frame.at(1) != 88 && currentSettings.wordLenght <= Word8bits)
            {
                return;
            }
            if(frame.at(1) != 89 && currentSettings.wordLenght > Word8bits)
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

            for(int b = 0; b < 50; b++)
            {

                if(currentSettings.wordLenght > Word8bits)
                {
                    quint16 sample = (quint8)frame[3+b];
                    sample <<= 8;
                    sample &= 0xFF00;
                    sample |= (quint8)(frame[4+b]);
                    b++;
                    samples.append( (double)((qint16)sample)- (1 << (currentSettings.wordLenght -1)));
                }
                else if(currentSettings.compressionType == None)
                {
                    samples.append( (double)((quint8)frame[3+b])- (1 << (currentSettings.wordLenght -1)) );
                }
                else if(currentSettings.compressionType == MuDigital)
                {
                    samples.append( (double)(( ulaw2linear(((quint8)frame[3+b]))/16)));
                }
                else if(currentSettings.compressionType == ADigital)
                {
                    samples.append( (double)(( alaw2linear(((quint8)frame[3+b]))/16)));
                }
                else if(currentSettings.compressionType == MuAnalog)
                {
                    samples.append( (double)((u_expand( (quint8)frame[3+b], 12, ((float)currentSettings.analogCompressionParam/10.0f)))));
//                    uint8_t val = ((quint8)frame[3+b]);

//                    if(val & 128) samples.append( (double)(-(val&127)));
//                    else samples.append( (double)(val));
                }
                else if(currentSettings.compressionType == AAnalog)
                {
                    samples.append( (double)((A_expand( (quint8)frame[3+b], 12, ((float)currentSettings.analogCompressionParam/10.0f)))));
//                    uint8_t val = ((quint8)frame[3+b]);

//                    if(val & 128) samples.append( (double)(-(val&127)));
//                    else samples.append( (double)(val));
                }
                else if(currentSettings.compressionType == Approx13seg)
                {
                    samples.append( (double)((seg13_expand( (quint8)frame[3+b], 12))));
//                    uint8_t val = ((quint8)frame[3+b]);

//                    if(val & 128) samples.append( (double)(-(val&127)));
//                    else samples.append( (double)(val));
                }

            }
            buffer.remove(0, dataFrameSize);
        }

        if(samples.size() == SAMPLE_BUFFER_SIZE)
        {
            emit newSampleData(samples);
            frameTimeoutCounter = 0;
        }
    }
    else if(buffer.size() >= 5 && (deviceStatus == Idle || deviceStatus == Configured) )
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
            //qDebug()<<"resp disco";
            connectionStatus = Connected;
            diagnosticResponseRecieved = true;
        }
        else if(buffer.at(1) == 55)
        {
            // response to config frame
            //qDebug()<<"resp config";
            deviceStatus = Configured;
        }
        buffer.clear();
    }

}

void ADA2Device::handleError(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::DeviceNotFoundError)
    {
        //qDebug()<<errorString()<<error;
        close();
        connectionStatus = Disconnected;
        deviceStatus = Idle;
        emit message(tr("Connection error."));
    }

}
