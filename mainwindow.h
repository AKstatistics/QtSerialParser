#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>

#include "serialportmanager.h"
#include "filewriter.h"

#define AS_BIT 0
#define AS_HEX 1
#define AS_CHAR 2

#define SEND_DEFAULT AS_HEX
#define OUT_DEFAULT AS_HEX

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void handlePacket(QByteArray);
    void handleSerialPortError(QSerialPort::SerialPortError);
    void handleSerialNotFound();
    void handleConnected(qint32 );
    void handleDisconnected();

    void on_actionReconnect_triggered();
    void on_actionBaud_9600_triggered();
    void on_actionBaud_10400_triggered();

    void on_pauseDisplay_clicked(bool checked);
    void on_pauseLogging_clicked(bool checked);
    void on_logTime_clicked(bool checked);
    void on_sendMessage_clicked();

    void on_escape();

private:
    Ui::MainWindow *ui;
    SerialPortManager *m_comPort;
    FileWriter * m_writer;
    QShortcut * m_escapeClear;

    void statusMessage( QString );
    void statusMessage(QString , int );
};

#endif // MAINWINDOW_H
