#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H

// THIS CONTROLS PID VID, SET TO FTDI FOR DEFAULTS, UNO FOR ARDUINO UNO, SPARK FOR SPARKFUN FTDI_BASIC CHIP
#define UNO


#include <QtSerialPort/QtSerialPort>
#include <QtWidgets>
#include <QTextStream>
#include <QByteArray>
#include <QObject>

#define FT232_PID 6001
#define FT232_VID 0403

#define SPARK_PID 24577
#define SPARK_VID 1027

#define UNO_PID 67
#define UNO_VID 9025

#ifdef UNO
#define PID UNO_PID
#define VID UNO_VID
#endif

#ifdef SPARK
#define PID SPARK_PID
#define VID SPARK_VID
#endif

#ifdef FTDI
#define PID FT232_PID
#define VID FT232_VID
#endif

QT_USE_NAMESPACE

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    SerialPortManager(QObject *parent = 0);
    ~SerialPortManager();

    bool sendHex( const QString );
    bool sendBits( const QString );
//    bool sendChar( const QString );

    bool setUpSerial(const qint32 baud);
    bool reconnect();
    bool reconnect(const qint32 );

signals:
    void packetReceived(const QByteArray);

    void serialPortError(QSerialPort::SerialPortError);
    void connected(qint32);
    void disconnected();
    void connectionTimeout(const int);
    void serialNotFound();
    void failedToConnect();

private slots:
    void handleReadyRead();
    void handleTimeout();

private:
    QSerialPort *m_serialPort;
    QByteArray   m_dataBuffer;
    QTextStream  m_standardOutput;
    QTimer       m_timer;
    int          m_timeoutCounter;

    qint64 send(const QByteArray);

    // timing
};

#endif // SERIALPORTREADER_H
