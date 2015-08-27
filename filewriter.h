#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QFileDialog>

class FileWriter : public QObject
{
    Q_OBJECT
public:
    // CHANGE DEFAULT SAVE FILE LOCATION FOR WINDOWS
    explicit FileWriter(QObject *parent = 0,
                        QString fileLocation = QString("/Users/adamlevy/Qtprojects/SerialData%1"));
    ~FileWriter();

    void openFile();
    void setUp();

signals:
    void failedToOpen(QString);
    void successfulOpen();

public slots:
    void handlePacket(QByteArray);

private:
    QFile * m_file;
    QTextStream * m_stream;
    QString m_fileName;
    QString m_fileLocation;
    void closeFile();
};

#endif // FILEWRITER_H
