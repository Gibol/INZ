/*  COMMUNICATION FRAME STRUCTURE
 *  [ BEG | TYP | DAT | MOD | EOF ]
 *      ->BEG   - FRAME BEGIN BYTE  [DEC 100]
 *      ->TYP   - FRAME TYPE        [DEC 44 - DIAGNOSTIC FRAME, 55 - CONFIG FRAME,  66 - CONVERSION FRAME, DEC 88 - DATA FRAME(1B WORD), DEC89 - DATA FRAME(2B WORD)]
 *      ->DAT   - OPTIONAL DATA     [0-2048 BYTES]
 *      ->MOD   - CONTROL SUM       [2 BYTES]
 *      ->EOF   - END OF FRAME BYTE [DEC 101]
 */

/* DIAGNOSTIC FRAME - to check if device is connected and responding
 * TYPE: DIAGNOSTIC
 * DATA: 0 BYTES
 * RESPONSE: YES
 * RESPONSE DATA: 0 BYTES
 */

/* CONFIGURATION FRAME - send configuraton parameters to the device
 * TYPE: CONFIG
 * DATA: 6 BYTES  (0 - FREQUENCY; 1 - COMPRESSION; 2 - WORDLENGTH; 3 - INPUT; 4 - OUTPUT; 5 - BITERROR )
 * RESPONSE: YES
 * RESPONSE DATA: 0 BYTES
 */

/* CONVERSION START STOP FRAME
 * TYPE: CONVERSION
 * DATA: 1 BYTE (0 - stop conversion; 255 - start conversion)
 * RESPONSE: NONE (device starts to send samples data)
 */

/* SAMPLES DATA FRAME
 * DATA: 51 BYTES (0 - sequence number, 1:50 samples data (50 or 25 samples per frame))
 */


#ifndef ADA2_H
#define ADA2_H

#define TIMER_UPDATE_INTERVAL 1000
#define SAMPLE_BUFFER_SIZE 1000

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimerEvent>
#include <QDebug>

class ADA2Device : public QSerialPort
{
    Q_OBJECT

public:
    explicit ADA2Device(QObject *parent = 0);
    typedef enum { F8KHZ, F11_025KHZ, F16KHZ, F22_05KHZ, F32KHZ, F44_1KHZ}                                          SampligFrequency;
    typedef enum { CompressionNone, CompressionA, CompressionMu, CompressionDigital}                                CompressionType;
    typedef enum { Word8bits, Word12Bits}                                                                           WordLenght;
    typedef enum { AnalogInput1, AnalogInput2, TestSignal1, TestSignal2 }                                           SignalSource;
    typedef enum { AnalogOutput1, AnalogOutput2}                                                                    SignalOutput;
    typedef enum { Bit0, Bit1, Bit2, Bit3, Bit4, Bit5, Bit6, Bit7, Bit8, Bit9, Bit10, Bit11, BitRandom, BitNone }   BitError;
    typedef enum { Connected, Disconnected, Waiting }                                                               ConnectionStatus;
    typedef enum { Idle, Busy }                                                                                     DeviceStatus;
    typedef enum { Start = 255, Stop = 0 }                                                                          StartStopCommand;

    typedef struct {
        SampligFrequency samplingFrequency;
        WordLenght wordLenght;
        CompressionType compressionType;
        SignalSource signalSource;
        SignalOutput signalOutput;
        BitError bitError;
    } ADASettings;

    static ADASettings initSettingsStructure();

signals:
    void connectionStatusChanged(ConnectionStatus status);
    void deviceStatusChanged(DeviceStatus status);
    void newSampleData(QVector<double>);

public slots:
    void newSettings(ADA2Device::ADASettings settings);
    void sendStartStopCommand(ADA2Device::StartStopCommand cmd);

protected:
    void timerEvent(QTimerEvent *event);

private:
    bool setPortSettings();
    int timerID;
    bool diagnosticResponseRecieved;
    DeviceStatus deviceStatus;
    ConnectionStatus connectionStatus;
    ADASettings currentSettings;
    int frameTimeoutCounter;

    void sendDignosticFrame();
    void sendConfigurationFrame();

private slots:
    void handleError(QSerialPort::SerialPortError error);
    void dataAvailable();

};

#endif // ADA2_H
