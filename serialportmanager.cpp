#include "serialportmanager.h"

#include <QCoreApplication>

QT_USE_NAMESPACE

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
{
    m_serialPort = new QSerialPort(this);

    m_settings.portName = "";
    m_settings.baud = DEFAULT_BAUD;
    m_settings.dataBits = DEFAULT_DATABITS;
    m_settings.flowControl = DEFAULT_FLOWCONTROL;
    m_settings.parity = DEFAULT_PARITY;
    m_settings.stopBits = DEFAULT_STOPBITS;


    // forward emit serial port errors
//    qRegisterMetaType<QSerialPort::SerialPortError>("QSerialPort::SerialPortError");
    connect(m_serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SIGNAL(serialPortError(QSerialPort::SerialPortError)));

    // deal with readyRead()
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortManager::handleReadyRead);

    m_serialPort->clear();
    emit portSettingsChanged(m_settings);
}

SerialPortManager::~SerialPortManager()
{
    if(m_serialPort->isOpen()){
        m_serialPort->close();
    }
}

void SerialPortManager::send(const QByteArray data)
{
    if(m_serialPort->isOpen()){
        emit sent(m_serialPort->write(data));
    }
}

void SerialPortManager::changePortSettings(PortSettings newSettings)
{
    if( m_settings.portName != newSettings.portName ||
        m_settings.baud != newSettings.baud ||
        m_settings.dataBits != newSettings.dataBits ||
        m_settings.flowControl != newSettings.flowControl ||
        m_settings.parity != newSettings.parity ||
        m_settings.stopBits != newSettings.stopBits ){
        m_settings = newSettings;
        emit portSettingsChanged(m_settings);
        if(m_settings.portName != ""){
            reconnect();
        }
    }
}

void SerialPortManager::reconnect()
{
    if(m_serialPort->isOpen()){
        m_serialPort->close();
    }

    // apply saved settings
    m_serialPort->setPortName(m_settings.portName);
    m_serialPort->setBaudRate(m_settings.baud);
    m_serialPort->setParity(m_settings.parity);
    m_serialPort->setDataBits(m_settings.dataBits);
    m_serialPort->setFlowControl(m_settings.flowControl);
    m_serialPort->setStopBits(m_settings.stopBits);

    if(m_serialPort->open(QSerialPort::ReadWrite)){
        emit connected();
    } else{
        emit failedToConnect();
    }
}

void SerialPortManager::handleReadyRead()
{
    // readReady() has been emitted, so now we clear the m_serialPort buffer
//    QByteArray data = m_serialPort->readAll();
    QByteArray data;
    while(!m_serialPort->atEnd()){
        data+=m_serialPort->readAll();
    }
    emit packetReceived(data);
}
