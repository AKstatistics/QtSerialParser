#include "mainwindow.h"
#include "ui_mainwindow.h"


// constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    ui->splitter->setStretchFactor(0,5);
    ui->splitter->setStretchFactor(1,1);

    ui->sendButtonGroup->setId(ui->sendMessageBit, AS_BIT);
    ui->sendButtonGroup->setId(ui->sendMessageHex, AS_HEX);
    ui->sendButtonGroup->setId(ui->sendMessageChar, AS_CHAR);

    ui->outputButtonGroup->setId(ui->displayOutputBit, AS_BIT);
    ui->outputButtonGroup->setId(ui->displayOutputHex, AS_HEX);
    ui->outputButtonGroup->setId(ui->displayOutputChar, AS_CHAR);

    m_comPort = new SerialPortManager(this);
    m_writer = new FileWriter(this,QFileDialog::getExistingDirectory(this,QString("Select log file save location")));

    connect(m_comPort,SIGNAL(packetReceived(QByteArray)),this,SLOT(handlePacket(QByteArray)));
    connect(m_comPort,SIGNAL(serialNotFound()),this,SLOT(handleSerialNotFound()));
    connect(m_comPort,SIGNAL(serialPortError(QSerialPort::SerialPortError)),this,SLOT(handleSerialPortError(QSerialPort::SerialPortError)));
    connect(m_comPort,SIGNAL(connected(qint32)),this,SLOT(handleConnected(qint32)));
    connect(m_comPort,SIGNAL(disconnected()),this,SLOT(handleDisconnected()));
    connect(m_comPort,SIGNAL(packetReceived(QByteArray)),m_writer,SLOT(handlePacket(QByteArray)));

    m_comPort->setUpSerial(10400);
    m_writer->setUp();
} // constructor


// destructor
MainWindow::~MainWindow()
{
    delete m_comPort;
    delete ui;
} // destructor


// send message clicked
void MainWindow::on_sendMessage_clicked()
{
    int checkedId = ui->sendButtonGroup->checkedId();
    QString sendDataString = ui->messageTextEdit->toPlainText();
    bool sendOk = false;

    switch(checkedId){
    case AS_BIT:
        sendOk = m_comPort->sendBits(sendDataString);
        break;
    case AS_HEX:
        sendOk = m_comPort->sendHex(sendDataString);
        break;
    case AS_CHAR:
        sendOk = m_comPort->sendChar(sendDataString);
        break;
    }

    if(sendOk){
        ui->messageTextEdit->clear();
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
    int checkedId = ui->outputButtonGroup->checkedId();
//    qDebug() << packet.toHex();
    const int maxBytesPerLine = 16;
//    const int maxBitsPerLine = 4 * maxBytesPerLine;
//    const int maxCharPerLine = 20;
    static quint64 numBytes;
    static int previousCheckedId;
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
    previousCheckedId = checkedId;
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




// pause check
void MainWindow::on_pauseCheckBox_clicked(bool checked)
{
    if(checked){
        disconnect(m_comPort,SIGNAL(packetReceived(QByteArray)),this,SLOT(handlePacket(QByteArray)));
    } else{
        connect(m_comPort,SIGNAL(packetReceived(QByteArray)),this,SLOT(handlePacket(QByteArray)));
    }
} // pause check



// set baud to 9600
void MainWindow::on_actionBaud_9600_triggered()
{
    ui->actionBaud_9600->setEnabled(false);
    ui->actionBaud_10400->setEnabled(true);
    m_comPort->reconnect(9600);
} // set baud to 9600



void MainWindow::on_actionBaud_10400_triggered()
{
    ui->actionBaud_9600->setEnabled(true);
    ui->actionBaud_10400->setEnabled(false);
    m_comPort->reconnect(10400);
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    if(checked){
        disconnect(m_comPort,SIGNAL(packetReceived(QByteArray)),m_writer,SLOT(handlePacket(QByteArray)));
    } else{
        m_writer->openFile();
        connect(m_comPort,SIGNAL(packetReceived(QByteArray)),m_writer,SLOT(handlePacket(QByteArray)));
    }
}
