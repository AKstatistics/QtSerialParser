#include "filewriter.h"

FileWriter::FileWriter(QObject *parent, QString fileLocation) : QObject(parent)
{
    m_fileName = QDateTime::currentDateTime().toString(QString("dd-MM-yyyy_hh-mm_'byteLog.txt'"));
    m_fileLocation = fileLocation;

    m_file = new QFile(m_fileLocation.append(m_fileName));
    m_stream = new QTextStream(m_file);
}

FileWriter::~FileWriter()
{
    m_stream->flush();
    if(m_file->isOpen()){
        m_file->close();
    }
    delete m_file;
    delete m_stream;
}

void FileWriter::closeFile()
{
    if(m_stream != 0){
        m_stream->flush();
    }
    if(m_file->isOpen()){
        m_file->close();
    }
    if(m_stream != 0){
        delete m_stream;
        m_stream = 0;
    }
    if(m_file != 0){
        delete m_file;
        m_file = 0;
    }
}

void FileWriter::openFile()
{
    closeFile();
    m_fileName = QDateTime::currentDateTime().toString(QString("dd-MM-yyyy_hh-mm_'byteLog.txt'"));
    m_file = new QFile(m_fileLocation.append(m_fileName));
    if(!m_file->open(QIODevice::WriteOnly)){
        emit failedToOpen(m_fileLocation.append(m_fileName));
    }
    else{
        emit successfulOpen();
        m_stream = new QTextStream(m_file);
    }
}

void FileWriter::setUp(){
    if(!m_file->open(QIODevice::WriteOnly)){
        emit failedToOpen(m_fileLocation.append(m_fileName));
    } else{
        emit successfulOpen();
    }
}

void FileWriter::handlePacket(QByteArray packet)
{
    QString currentText;
    QString packetAsHex;
//    qDebug() << packet.toHex();
    const int maxBytesPerLine = 16;
//    const int maxBitsPerLine = 4 * maxBytesPerLine;
//    const int maxCharPerLine = 20;
    static quint64 numBytes;
    static int counter = 0;
    static bool endOfByte = false;

    QString timeString("");

    packetAsHex = packet.toHex().toUpper();
    for(int i = 0; i < packetAsHex.length(); ++i){
        if(counter == 0){
            if(!endOfByte){
                currentText.append(QDateTime::currentDateTime().toString(QString("hh:mm:ss ")));
                currentText.append(QString("(%1):  ").arg(numBytes));
                currentText.append(packetAsHex[i]);
                endOfByte = true;
            } else{
                currentText.append(packetAsHex[i]).append("  ");
                endOfByte = false;
                counter++;
            }
        } else if(counter == maxBytesPerLine - 1){
            if(!endOfByte){
                currentText.append(packetAsHex[i]);
                endOfByte = true;
            } else{
                currentText.append(packetAsHex[i]).append('\n');
                endOfByte = false;
                counter = 0;
            }

        } else{
            if(!endOfByte){
                currentText.append(packetAsHex[i]);
                endOfByte = true;
            } else{
                currentText.append(packetAsHex[i]).append("  ");
                endOfByte = false;
                counter++;
            }
        }
        if(!endOfByte){
            numBytes++;
        }
    }
    *m_stream << currentText;
    m_stream->flush();
}




