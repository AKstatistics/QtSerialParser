#ifndef FILEWRITER_H
#define FILEWRITER_H



// CHANGE DEFAULT SAVE FILE LOCATION HERE, do not add a trailing forward slash
#define BRIANS_DEFAULT_DIR "your/dir/here/in/quotes/no/trailing/slash"
#define ADAM // CHANGE ADAM TO BRIAN


#define ADAMS_DEFAULT_DIR "/Users/adamlevy/Qtprojects/SerialData"
#define FARASIS_TABLET_DEFAULT_DIR


#ifdef ADAM
#define DEFAULT_DIR ADAMS_DEFAULT_DIR
#endif

#ifdef FARASIS
#define DEFAULT_DIR FARASIS_TABLET_DEFAULT_DIR
#endif

#ifdef BRIAN
#define DEFAULT_DIR BRIANS_DEFAULT_DIR
#endif

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QFileDialog>

class FileWriter : public QObject
{
    Q_OBJECT
public:
    explicit FileWriter(QObject *parent = 0,
                        QString fileLocation = QString(DEFAULT_DIR).append("%1"));
    ~FileWriter();

    void openFile();
    void setUp();
    void logTimeStamps(bool);

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
    bool m_logTimeStamps;
};

#endif // FILEWRITER_H
