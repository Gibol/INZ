/*  COMMUNICATION FRAME STRUCTURE
 *  [ BEG | TYP | DAT | CRC | EOF ]
 *      ->BEG   - FRAME BEGIN BYTE  [DEC 100]
 *      ->TYP   - FRAME TYPE        [DEC 66 - COMMAND FRAME, DEC 77 - RESPONSE FRAME, DEC 88 - DATA FRAME(1B WORD), DEC89 - DATA FRAME(2B WORD)]
 *      ->DAT   - OPTIONAL DATA     [0-2048 BYTES]
 *      ->CRC   - CONTROL SUM       [2 BYTES]
 *      ->EOF   - END OF FRAME BYTE [DEC 101]
*/


#ifndef ADA2_H
#define ADA2_H

#define TIMER_UPDATE_INTERVAL 1000

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimerEvent>
#include <QDebug>

class ADA2Device : public QSerialPort
{
    Q_OBJECT
public:
    explicit ADA2Device(QObject *parent = 0);
    typedef enum { F8KHZ, F11_025KHZ, F16KHZ, F22_05HZ, F32KHZ, F44_1KHZ, F48KHZ}       SampligFrequency;
    typedef enum { CompressionNone, CompressionA, CompressionMu, CompressionDigital}    CompressionType;
    typedef enum { Word8bits, Word12Bits}                                               WordLenght;
    typedef enum { AnalogInput1, AnalogInput2, TestSignal1, TestSignal2, TestSignal3}   SignalSource;
    typedef enum { Connected, Disconnected }                                            ConnectionStatus;
    typedef enum { Idle, Busy }                                                         DeviceStatus;



signals:
    void connectionStatusChanged(ConnectionStatus status);
    void newSampleData(QVector<double>);

public slots:


protected:
    void timerEvent(QTimerEvent *event);

private:
    bool setPortSettings();
    int timerID;

    DeviceStatus deviceStatus;
    ConnectionStatus connectionStatus;

private slots:

    void dataAvailable();

};

#endif // ADA2_H
