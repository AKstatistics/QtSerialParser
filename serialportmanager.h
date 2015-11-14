#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H

// THIS CONTROLS PID VID, SET TO FTDI FOR DEFAULTS, UNO FOR ARDUINO UNO, SPARK FOR SPARKFUN FTDI_BASIC CHIP
#define UNO


#include <QtSerialPort/QtSerialPort>
#include <QtWidgets>
#include <QTextStream>
#include <QByteArray>
#include <QObject>

QT_USE_NAMESPACE

#define DEFAULT_BAUD 57600
#define DEFAULT_DATABITS QSerialPort::Data8
#define DEFAULT_FLOWCONTROL QSerialPort::NoFlowControl
#define DEFAULT_PARITY QSerialPort::NoParity
#define DEFAULT_STOPBITS QSerialPort::OneStop

struct PortSettings{
    QString portName;
    qint32 baud;
    QSerialPort::DataBits dataBits;
    QSerialPort::FlowControl flowControl;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
};

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    SerialPortManager(QObject *parent = 0);
    ~SerialPortManager();

public slots:
    void send(const QByteArray data);

    void reconnect();
    void changePortSettings(PortSettings newSettings);

signals:
    void packetReceived(const QByteArray);
    void sent(qint64);

    void serialPortError(QSerialPort::SerialPortError);
    void connected();
    void failedToConnect();
    void portSettingsChanged(PortSettings);

private slots:
    void handleReadyRead();

private:
    QSerialPort  * m_serialPort;
    PortSettings m_settings;
};

#endif // SERIALPORTREADER_H
