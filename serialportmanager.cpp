#include "serialportmanager.h"

#include <QCoreApplication>

QT_USE_NAMESPACE

// constructor
SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent),
      m_standardOutput(stdout)
{
    m_serialPort = new QSerialPort;
    m_timer.start(1000);
    m_timeoutCounter = 0;

    // Handle Timeouts
    connect(&m_timer,SIGNAL(timeout()),this,SLOT(handleTimeout()));

    // emit serial port errors externally for handling by MainWindow
    connect(m_serialPort,SIGNAL(error(QSerialPort::SerialPortError)),this,SIGNAL(serialPortError(QSerialPort::SerialPortError)));

    // deal with readyRead()
    connect(m_serialPort, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));

} // constructor



// destructor
SerialPortManager::~SerialPortManager()
{
    if(m_serialPort->isOpen()){
        m_serialPort->close();
    }
    delete m_serialPort;
} // destructor



// sendHex()
bool SerialPortManager::sendHex(const QString hexStringData)
{
    QString formattedString = hexStringData.simplified().toLower();
    formattedString.remove(QChar(' '));

    int stringLength = formattedString.length();
    int byteLength = stringLength / 2;
    if(stringLength%2){
        formattedString.append("0");
        byteLength++;
    }
    QByteArray hexData(byteLength,0x00);
    bool ok = false;
    bool allOk = true;

    for( int i = 0; i < byteLength; ++i ){
        hexData[i] = formattedString.midRef(i*2,2).toInt(&ok,16);
        allOk &= ok;
    }
    if(allOk && send(hexData) == byteLength){
        return true;
    }
    return false;
} // sendHex()



// sendChar()
//bool SerialPortManager::sendChar(const QString charStringData)
//{
//    QByteArray charData = QByteArray::fromStdString(charStringData.toStdString());
//    if( send(charData) == charStringData.length() ){
//        return true;
//    }
//    return false;
//} // sendChar()



// sendBits()
bool SerialPortManager::sendBits(const QString bitStringData)
{
    QString formattedString = bitStringData.simplified().toLower();
    formattedString.remove(QChar(' '));
    int stringLength = formattedString.length();
    int byteLength = stringLength / 8 + 1;
    QByteArray bitData(byteLength, 0x00);
    bool ok = false;
    bool allOk = false;

    for( int i = 0; i < byteLength; ++i ){
        bitData[i] = formattedString.midRef(i,8).toInt(&ok,2);
        allOk &= ok;
    }

    if( stringLength % 8 ){
        bitData[byteLength - 1] = bitData[byteLength - 1] << (8 - (stringLength % 8) );
    }

    if(allOk && send(bitData) == byteLength){
        return true;
    }

    return false;
} // sendBits()



// reconnect()
bool SerialPortManager::reconnect()
{
    qint32 baud = m_serialPort->baudRate();
    if(m_serialPort->isOpen()){
        m_serialPort->close();
        emit disconnected();
    }
    if( !setUpSerial(baud) ){
        return false;
    }
    return true;
} // reconnect()


// reconnect()
bool SerialPortManager::reconnect(const qint32 baud)
{
    if(m_serialPort->isOpen()){
        m_serialPort->close();
        emit disconnected();
    }
    if( !setUpSerial(baud) ){
        return false;
    }
    return true;
} // reconnect()



// send()
qint64 SerialPortManager::send(const QByteArray data)
{
    return m_serialPort->write(data);
} // send()



// handleTimeout()
void SerialPortManager::handleTimeout()
{
    emit connectionTimeout(m_timeoutCounter++);
    m_timer.start(1000);
} // handleTimeout()



// setUpSerial()
bool SerialPortManager::setUpSerial(const qint32 baud)
{
    const quint16 vendorId = VID;
    const quint16 productId = PID;

    QString portName("");
    bool isAvailable = false;

    // Find the correct port name
    foreach( const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts() ){
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier()){
//            qDebug() << serialPortInfo.portName();
//            qDebug() << "PID: " << serialPortInfo.productIdentifier();
//            qDebug() << "VID: " << serialPortInfo.vendorIdentifier();
            if(serialPortInfo.productIdentifier() == productId && serialPortInfo.vendorIdentifier() == vendorId){
                portName = serialPortInfo.portName();
                isAvailable = true;
//                qDebug() << portName;
            }
        }
    }

    // Future improvements: consider providing setup dialog in info box
    if(isAvailable){
        m_serialPort->setPortName(portName);
        m_serialPort->setBaudRate(baud);
        m_serialPort->setParity(QSerialPort::NoParity);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
        m_serialPort->setStopBits(QSerialPort::OneStop);

        if(m_serialPort->open(QSerialPort::ReadWrite)){
            emit connected(baud);
            return true;
        } else{
            emit failedToConnect();
            return false;
        }
    }
    emit serialNotFound();
    return false;
} // setUpSerial()



// handleReadyRead()
void SerialPortManager::handleReadyRead()
{
    // readReady() has been emitted, so now we clear the m_serialPort buffer
    m_dataBuffer = m_serialPort->readAll();
//    qDebug() << m_dataBuffer.toHex();

    emit packetReceived(m_dataBuffer);

    m_dataBuffer.clear();
} // handleReadyRead()
