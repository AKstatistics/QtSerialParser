#include "mainwindow.h"
#include "ui_mainwindow.h"



// constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // establishes initial proportions of splitter
    ui->splitter->setStretchFactor(0,5);
    ui->splitter->setStretchFactor(1,1);

    // group buttons and assign indexes
    ui->sendButtonGroup->setId(ui->sendMessageBit, AS_BIT);
    ui->sendButtonGroup->setId(ui->sendMessageHex, AS_HEX);
    ui->sendButtonGroup->setId(ui->sendMessageChar, AS_CHAR);

    ui->displayButtonGroup->setId(ui->displayOutputBit, AS_BIT);
    ui->displayButtonGroup->setId(ui->displayOutputHex, AS_HEX);
    ui->displayButtonGroup->setId(ui->displayOutputChar, AS_CHAR);

    // keyboard shortcuts
    m_escapeClear = new QShortcut(QKeySequence(Qt::Key_Escape),this);


    // allocate
    m_comPort = new SerialPortManager(this);

    // shortcut key connects

    connect(ui->messageLineEdit,SIGNAL(returnPressed()),SLOT(on_sendMessage_clicked()));
    connect(m_escapeClear, SIGNAL(activated()),SLOT(on_escape()));

    // handlePacket
    connect(m_comPort,SIGNAL(packetReceived(QByteArray)),SLOT(handlePacket(QByteArray)));

    // handleSerialNotFound
    connect(m_comPort,SIGNAL(serialNotFound()),SLOT(handleSerialNotFound()));

    // handleSerialPortError
    connect(m_comPort,SIGNAL(serialPortError(QSerialPort::SerialPortError)),SLOT(handleSerialPortError(QSerialPort::SerialPortError)));

    // handleConnected
    connect(m_comPort,SIGNAL(connected(qint32)),SLOT(handleConnected(qint32)));

    // handleDisconnected
    connect(m_comPort,SIGNAL(disconnected()),SLOT(handleDisconnected()));

    // file and serial setup
    m_comPort->setUpSerial(57600);

} // constructor



// destructor
MainWindow::~MainWindow()
{
    delete m_comPort;
    delete m_escapeClear;
    delete ui;
} // destructor



// send message clicked
void MainWindow::on_sendMessage_clicked()
{
    int checkedId = ui->sendButtonGroup->checkedId();
    QString sendDataString = ui->messageLineEdit->text();
    bool sendOk = false;

    switch(checkedId){
    case AS_BIT:
        sendOk = m_comPort->sendBits(sendDataString);
        break;
    case AS_HEX:
        sendOk = m_comPort->sendHex(sendDataString);
        break;
    case AS_CHAR:
//        sendOk = m_comPort->sendChar(sendDataString);
        break;
    }

    if(sendOk){
        //ui->messageTextEdit->clear();
        statusMessage(QString("Sent"), 5000);
    } else{
        statusMessage(QString("Failed to send!"), 5000);
    }
} // send message clicked



// handlePacket
void MainWindow::handlePacket(QByteArray packet)
{
    QString currentText = ui->serialTextBrowser->toPlainText();
    QString packetAsHex;
    int checkedId = ui->displayButtonGroup->checkedId();
//    qDebug() << packet.toHex();
    const int maxBytesPerLine = 6;
//    const int maxBitsPerLine = 4 * maxBytesPerLine;
//    const int maxCharPerLine = 20;
    static quint64 numBytes;
    static int counter = 0;
    static bool endOfByte = false;

    QString timeString("");

    switch(checkedId){
    case AS_BIT:

        break;

    case AS_HEX:
        packetAsHex = packet.toHex().toUpper();
        for(int i = 0; i < packetAsHex.length(); ++i){
            if(counter == 0){
                if(!endOfByte){
                    currentText.append(QDateTime::currentDateTime().toString(QString("hh:mm:ss:zzz ")));
//                    currentText.append(QString("(%1):  ").arg(numBytes));
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

        break;

    case AS_CHAR:

        break;
    }

    QStringList lines = currentText.split('\n');
    while(lines.length() > 60){
        lines.pop_front();
    }

    currentText = lines.join('\n');

    ui->serialTextBrowser->setText(currentText);
    ui->serialTextBrowser->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
} // handlePacket



// handleSerialPortError()
void MainWindow::handleSerialPortError(QSerialPort::SerialPortError error)
{
    QString errorText = QString("Serial port error: ").append(error).append(" ").append(QSerialPort(sender()).errorString());
    statusMessage(errorText);

} // handleSerialPortError()



// handleSerialNotFound()
void MainWindow::handleSerialNotFound()
{
    statusMessage("Serial device not found!");

} // handleSerialNotFound()



// handleConnected()
void MainWindow::handleConnected(qint32 baud)
{
    statusMessage(QString("Connected. Baud %1").arg(baud), 5000);

} // handleConnected()

// handleDisconnected()
void MainWindow::handleDisconnected()
{
    statusMessage(QString("Disonnected."), 5000);
} // handleDisconnected()

// statusMessage()
void MainWindow::statusMessage(QString message)
{
    QString dateString("dd/mm/yyyy  hh:mm:ss  '");
    message = dateString.append(message).append("'");
    ui->statusBar->showMessage(QDateTime::currentDateTime().toString(message));
} // statusMessage()



// statusMessage with timeout
void MainWindow::statusMessage(QString message, int timeout)
{
    QString dateString("dd/mm/yyyy  hh:mm:ss  '");
    message = dateString.append(message).append("'");
    ui->statusBar->showMessage(QDateTime::currentDateTime().toString(message),timeout);
} // status message with timeout



// reconnect triggered
void MainWindow::on_actionReconnect_triggered()
{
   m_comPort->reconnect();
} // reconnect triggered

// escape key
void MainWindow::on_escape()
{
    ui->messageLineEdit->clear();
} // escape key

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog settings;
    settings.show();
}

void MainWindow::on_pauseScrolling_clicked(bool checked)
{

}
