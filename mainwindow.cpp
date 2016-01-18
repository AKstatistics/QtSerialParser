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
    ui->sendButtonGroup->setId(ui->sendMessageHex, AS_HEX);
    ui->sendButtonGroup->setId(ui->sendMessageChar, AS_CHAR);
    ui->sendButtonGroup->setId(ui->sendMessageBit, AS_BIT);

    ui->displayButtonGroup->setId(ui->displayOutputHex, AS_HEX);
    ui->displayButtonGroup->setId(ui->displayOutputChar, AS_CHAR);

    // allocate
    m_comPort = new SerialPortManager(this);
    m_portSettings.portName = "";
    m_portSettings.baud = DEFAULT_BAUD;
    m_portSettings.dataBits = DEFAULT_DATABITS;
    m_portSettings.flowControl = DEFAULT_FLOWCONTROL;
    m_portSettings.parity = DEFAULT_PARITY;
    m_portSettings.stopBits = DEFAULT_STOPBITS;

    connect(this, SIGNAL(send(QByteArray)), m_comPort, SLOT(send(QByteArray)));

    // handlePacket
    connect(m_comPort,SIGNAL(packetReceived(QByteArray)),SLOT(handlePacket(QByteArray)));

    connect(m_comPort,SIGNAL(serialPortError(QSerialPort::SerialPortError)),SLOT(handleSerialPortError(QSerialPort::SerialPortError)));
    connect(m_comPort,SIGNAL(connected()), SLOT(handleConnected()));
    connect(m_comPort,SIGNAL(failedToConnect()), SLOT(handleFailedToConnect()));

    qRegisterMetaType<PortSettings>("PortSettings");
    qRegisterMetaType<QSerialPort::SerialPortError>("QSerialPort::SerialPortError");

    connect(this, SIGNAL(updatePortSettings(PortSettings)), m_comPort, SLOT(changePortSettings(PortSettings)));
    connect(m_comPort,SIGNAL(portSettingsChanged(PortSettings)),SLOT(handlePortSettingsChanged(PortSettings)));
    connect(m_comPort,SIGNAL(sent(qint64)), SLOT(handleSent(qint64)));

    connect(ui->menuSelect_Port,SIGNAL(aboutToShow()),SLOT(populatePortMenu()));
    connect(this, SIGNAL(reconnect()), m_comPort, SLOT(reconnect()));
    QList<QAction *> baudRateActions = ui->menuBaud_Rate->actions();
    baudRateActions.pop_back(); // remove custom baud action
    foreach(QAction * action, baudRateActions){
        connect(action, SIGNAL(triggered(bool)), this, SLOT(changeBaud()));
    }
    connect(ui->menuData_Bits,SIGNAL(triggered(QAction*)),this,SLOT(changeDataBits(QAction*)));
    connect(ui->menuStop_Bits,SIGNAL(triggered(QAction*)),this,SLOT(changeStopBits(QAction*)));
    connect(ui->menuFlow_Control,SIGNAL(triggered(QAction*)),this,SLOT(changeFlowControl(QAction*)));
    connect(ui->menuParity,SIGNAL(triggered(QAction*)),this,SLOT(changeParity(QAction*)));

    connect(ui->displayOutputChar, SIGNAL(toggled(bool)), this, SLOT(changeDisplayFormat(bool)));
    connect(ui->plainTextEdit, SIGNAL(textChanged()), this, SLOT(handleSendMessageFormatting()));
    connect(ui->serialTextBrowser,SIGNAL(textChanged()), this, SLOT(enableClearButton()));
    m_numBytesDisplayed = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sendMessage_clicked()
{
    if(m_connected){
        int checkedId = ui->sendButtonGroup->checkedId();
        // clean input
        QString formattedString = ui->plainTextEdit->toPlainText().simplified().toLower();
        formattedString.remove(QChar(' '));

        switch(checkedId){
            case AS_HEX:
            {
                // get number of bytes
                int stringLength = formattedString.length();
                int byteLength = stringLength / 2;
                // add leading zeros to incomplete bytes,
                if(stringLength%2){
                    formattedString.insert(0,"0");
                    byteLength++;
                }

                QByteArray hexData(byteLength,0x00);
                // used to check for valid conversion
                bool ok = false;
                bool allOk = true;

                // convert to bytes
                for( int i = 0; i < byteLength; ++i ){
                    hexData[i] = formattedString.midRef(i*2,2).toInt(&ok,16);
                    allOk &= ok;
                }

                emit send(hexData);
                break;
            }
            case AS_CHAR:
            {
                QByteArray charData = QByteArray::fromStdString(ui->plainTextEdit->toPlainText().toStdString());
                emit send(charData);
                break;
            }
            case AS_BIT:
            {
                // get number of bytes
                int stringLength = formattedString.length();
                int byteLength = stringLength / 8;
                // add leading zeros to incomplete bytes,
                if(stringLength%8){
                    while(stringLength%8){
                        formattedString.insert(0,"0");
                        stringLength++;
                    }
                    byteLength++;
                }

                QByteArray bitData(byteLength, 0x00);
                // used to check for valid conversion
                bool ok = false;
                bool allOk = false;

                // convert to bytes
                for( int i = 0; i < byteLength; ++i ){
                    bitData[i] = formattedString.midRef(i,8).toInt(&ok,2);
                    allOk &= ok;
                }

                emit send(bitData);
                break;
            }
        }
    } else{
        statusMessage(QString("No open port. "), 5000);
    }
}



// handlePacket
void MainWindow::handlePacket(QByteArray packet)
{
    if(m_numBytesDisplayed > 60000){
        on_clearDisplayButton_clicked();
    }
    QString addedText = packet.toHex();
    m_numBytesDisplayed += packet.length();
    addedText = addedText.toUpper();
    int len = addedText.length();
    for(int i = 0; i < len; i++){
        m_textAsHex.append(addedText[i]);
        if(i%2){
            m_textAsHex.append(" ");
        }
    }

    m_text.append(QString::fromStdString(packet.toStdString()));

    int sliderPosition = ui->serialTextBrowser->verticalScrollBar()->sliderPosition();
    if(ui->displayOutputHex->isChecked()){
        ui->serialTextBrowser->setText(m_textAsHex);
    } else{
        ui->serialTextBrowser->setText(m_text);
    }
    if(!ui->pauseScrolling->isChecked()){
        ui->serialTextBrowser->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
    } else{
        ui->serialTextBrowser->verticalScrollBar()->setSliderPosition(sliderPosition);
    }
    statusMessage(QString("%1 bytes received.").arg(m_numBytesDisplayed));
}

void MainWindow::handleSerialPortError(QSerialPort::SerialPortError error)
{
    QString errorText;
    switch(error){
    case 0:
        errorText = "No Error";
        return;
        break;
    case 1:
        errorText = "Device Not Found";
        break;
    case 2:
        errorText = "Permission Error";
        break;
    case 3:
        errorText = "Open Error";
        break;
    case 4:
        errorText = "Parity Error";
        break;
    case 5:
        errorText = "Framing Error";
        break;
    case 6:
        errorText = "Break Condition Error";
        break;
    case 7:
        errorText = "Write Error";
        break;
    case 8:
        errorText = "Read Error";
        break;
    case 9:
        errorText = "Resource Error, device may be disconnected";
        break;
    case 10:
        errorText = "Unsupported Operation Error";
        break;
    case 11:
        errorText = "Unknown Error";
        break;
    case 12:
        errorText = "Timeout Error";
        break;
    case 13:
        errorText = "Not Open Error";
        break;
    }

    errorText = QString("Serial port error %1:  %2").arg(error).arg(errorText);
    statusMessage(errorText);
    if(error > 1){
        m_connected = false;
    }
}

void MainWindow::handleSerialNotFound()
{
    statusMessage("Serial device not found!");

}

void MainWindow::handleConnected()
{
    m_connected = true;
    statusMessage(QString("Connected. Port Name: \"%1\", Baud: %2")
                  .arg(m_portSettings.portName).arg(m_portSettings.baud));
}

void MainWindow::handleFailedToConnect()
{
    m_connected = false;
    statusMessage(QString("Failed To Connect. Port Name: \"%1\", Baud: %2")
                  .arg(m_portSettings.portName).arg(m_portSettings.baud));
}

void MainWindow::handleSendMessageFormatting()
{
    //psuedo code:
    //  for each char sym in inputText:
    //      if sym is a valid hex/bit character or space,
    //              add it to the formattedText
    //      else
    //              print a status message explaining why sym
    //              wasn't able to be typed

    //  at the end of this function the send and clear buttons
    //  are disabled/enabled based on whether the input is empty/!empty

    if(ui->sendButtonGroup->checkedId() == AS_HEX){
        QString inputText = ui->plainTextEdit->toPlainText();
        QString formattedText = "";
        for(int i = 0; i < inputText.length(); i++){
            char sym = QChar(inputText[i]).unicode();
            if(isdigit(sym) || sym == ' '
                    ||('a' <= sym && sym <= 'f')
                    ||('A' <= sym && sym <= 'F')){
                formattedText.append(sym);
            } else{
                statusMessage("Invalid hex symbol. ", 3000);
            }
        }
        ui->plainTextEdit->blockSignals(true);
        QTextCursor cursor = ui->plainTextEdit->textCursor();
        ui->plainTextEdit->setPlainText(formattedText);
        ui->plainTextEdit->setTextCursor(cursor);
        ui->plainTextEdit->blockSignals(false);
    } else if(ui->sendButtonGroup->checkedId() == AS_BIT){
        QString inputText = ui->plainTextEdit->toPlainText();
        QString formattedText = "";
        for(int i = 0; i < inputText.length(); i++){
            char sym = QChar(inputText[i]).unicode();
            if( sym == '0' || sym == '1' || sym == ' ' ){
                formattedText.append(sym);
            } else{
                statusMessage("Invalid binary symbol. ", 3000);
            }
        }
        ui->plainTextEdit->blockSignals(true);
        QTextCursor cursor = ui->plainTextEdit->textCursor();
        ui->plainTextEdit->setPlainText(formattedText);
        ui->plainTextEdit->setTextCursor(cursor);
        ui->plainTextEdit->blockSignals(false);
    }

    // enable/disable send and clear buttons
    // based on whether there is text in the plainTextEdit
    if(ui->plainTextEdit->toPlainText().length() > 0){
        ui->sendMessage->setEnabled(true);
        ui->clear->setEnabled(true);
    } else{
        ui->sendMessage->setEnabled(false);
        ui->clear->setEnabled(false);
    }
}

void MainWindow::changeDisplayFormat(bool asChar)
{
    if(asChar == true){
        ui->serialTextBrowser->setText(m_text);
    } else{
        ui->serialTextBrowser->setText(m_textAsHex);
    }
}

void MainWindow::enableClearButton()
{
    if( ui->serialTextBrowser->toPlainText().length() > 0){
        ui->clearDisplayButton->setEnabled(true);
        disconnect(ui->serialTextBrowser,SIGNAL(textChanged()),this,SLOT(enableClearButton()));
    }
}

void MainWindow::handlePortSettingsChanged(PortSettings newSettings)
{
    // copy new settings
    m_portSettings = newSettings;

    // update which items are disabled in the serial menus

    // Select Port, port name
    foreach(QAction * action, ui->menuSelect_Port->actions()){
        QString name = action->text();
        action->setEnabled((name != m_portSettings.portName));
    }

    // Baud Rate
    foreach(QAction * action, ui->menuBaud_Rate->actions()){
        QString baudString = action->text();
        baudString.chop(5);
        int baud = baudString.toInt();
        action->setEnabled((baud != m_portSettings.baud));
    }

    // NOTE the below is a mess. I use the index position of the QAction in the
    // menu to map to the QSerialPort enum values. Casting to the enum makes it
    // extra messy.

    // Data bits
    foreach(QAction * action, ui->menuData_Bits->actions()){
        QString dataBitString = action->text();
        dataBitString.chop(5);
        int dataBits = dataBitString.toInt();
        action->setEnabled((((QSerialPort::DataBits)dataBits) != m_portSettings.dataBits));
    }

    // Flow control
    QList<QAction *> actions = ui->menuFlow_Control->actions();
    for(int i = 0; i < actions.length(); i++){
        actions[i]->setEnabled((((QSerialPort::FlowControl)i)!=m_portSettings.flowControl));
    }

    // Parity
    actions = ui->menuParity->actions();
    for(int i = 0; i < actions.length(); i++){
        // look at the values for QSerialPort::Parity and the position
        // of the action in the menu to make sense of the following.
        int parity = i;
        if(i > 0){
            parity++;
        }
        actions[i]->setEnabled((((QSerialPort::Parity)parity)!=m_portSettings.parity));
    }

    // Stop bits
    actions = ui->menuStop_Bits->actions();
    for(int i = 0; i < actions.length(); i++){
        QSerialPort::StopBits stopBits;
        switch(i){
        case 0:
            stopBits = QSerialPort::OneStop;
            break;
        case 1:
            stopBits = QSerialPort::OneAndHalfStop;
            break;
        case 2:
            stopBits = QSerialPort::TwoStop;
        }

        actions[i]->setEnabled((stopBits!=m_portSettings.stopBits));
    }
}

void MainWindow::handleSent(qint64 numBytes)
{
    statusMessage(QString("%1 bytes sent").arg(numBytes));
}

void MainWindow::populatePortMenu()
{
    ui->menuSelect_Port->clear();

    QList<QAction *> ports;
    // list through the available ports
    foreach( const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts() ){
        // create an action named appropriately..
        QString portName = serialPortInfo.portName();
        QAction * portAction = new QAction(portName,0/*ui->menuSelect_Por*/);
        portAction->setData(portName);
        // check if connected, it'll be disabled so we don't need to connect it
        if(portName == m_portSettings.portName && m_connected){
            portName.append(" (connected)");
            portAction->setText(portName);
            ports.insert(0,portAction);
        } else{
            portAction->setText(portName);
            connect(portAction, SIGNAL(triggered(bool)), this, SLOT(changePort()));
            ports.push_back(portAction);
        }
    }
    // toggle hidden No available ports menu item
    if(ports.isEmpty()){
        ui->actionNo_available_ports->setVisible(true);
    } else{
        ui->actionNo_available_ports->setVisible(false);
        ui->menuSelect_Port->addActions(ports);
    }
}

void MainWindow::statusMessage(QString message)
{
    QString dateString("dd/mm/yyyy  hh:mm:ss  '");
    message = dateString.append(message).append("'");
    ui->statusBar->showMessage(QDateTime::currentDateTime().toString(message));
}

void MainWindow::statusMessage(QString message, int timeout)
{
    QString dateString("dd/mm/yyyy  hh:mm:ss  '");
    message = dateString.append(message).append("'");
    ui->statusBar->showMessage(QDateTime::currentDateTime().toString(message), timeout);

}

void MainWindow::changePort()
{
    QString portName = qobject_cast<QAction *>(sender())->data().toString();
    PortSettings newSettings = m_portSettings;
    newSettings.portName = portName;
    emit updatePortSettings(newSettings);
}

void MainWindow::changeBaud()
{
    QString baudText = qobject_cast<QAction *>(sender())->text();
    baudText.chop(5);
    PortSettings newSettings = m_portSettings;
    newSettings.baud = baudText.toInt();
    statusMessage(QString("Baud rate set: %1").arg(newSettings.baud));
    emit updatePortSettings(newSettings);
}

void MainWindow::changeDataBits(QAction * action)
{
    QString dataBitsText = action->text();
    dataBitsText.chop(5);
    int dataBits = dataBitsText.toInt();
    PortSettings newSettings = m_portSettings;
    newSettings.dataBits = (QSerialPort::DataBits)dataBits;
    statusMessage(QString("Data bits set: %1 bits ").arg(dataBits));
    emit updatePortSettings(newSettings);
}

void MainWindow::changeFlowControl(QAction * action)
{
    QString flowControlText = action->text();

    QList<QAction *> actions = ui->menuFlow_Control->actions();
    PortSettings newSettings = m_portSettings;
    for(int i = 0; i < actions.length(); i++){
        if(actions[i]->text() == flowControlText){
            newSettings.flowControl = (QSerialPort::FlowControl)i;
            break;
        }
    }
    statusMessage(QString("Flow control set: %1 ").arg(flowControlText));
    emit updatePortSettings(newSettings);
}

void MainWindow::changeStopBits(QAction * action)
{
    QString stopBitsText = action->text();
    int correctIndex = 0;
    QList<QAction *> actions = ui->menuStop_Bits->actions();
    for(int i = 0; i < actions.length(); i++){
        if(actions[i]->text() == stopBitsText){
            correctIndex = i;
            break;
        }
    }
    QSerialPort::StopBits stopBits;
    switch(correctIndex){
    case 0:
        stopBits = QSerialPort::OneStop;
        break;
    case 1:
        stopBits = QSerialPort::OneAndHalfStop;
        break;
    case 2:
        stopBits = QSerialPort::TwoStop;
    }
    PortSettings newSettings = m_portSettings;
    newSettings.stopBits = stopBits;
    statusMessage(QString("Stop bits set: %1 ").arg(stopBitsText));
    emit updatePortSettings(newSettings);
}

void MainWindow::changeParity(QAction * action)
{
    QString parityText = action->text();
    QList<QAction *> actions = ui->menuParity->actions();
    QSerialPort::Parity parity = QSerialPort::NoParity;
    for(int i = 0; i < actions.length(); i++){
        if(actions[i]->text() == parityText){
            if(i > 0){
                parity = (QSerialPort::Parity)(i+1);
            } else{
                parity = (QSerialPort::Parity)i;
            }
        }
    }
    PortSettings newSettings = m_portSettings;
    newSettings.parity = parity;
    statusMessage(QString("Parity set: %1 ").arg(parityText));
    emit updatePortSettings(newSettings);
}

void MainWindow::customBaud(int baud)
{
    PortSettings newSettings = m_portSettings;
    newSettings.baud = baud;
    statusMessage(QString("Baud rate set: %1").arg(baud));
    emit updatePortSettings(newSettings);
}

void MainWindow::on_actionReconnect_triggered()
{
    if(m_portSettings.portName != ""){
        emit reconnect();
    } else{
        statusMessage(QString("No port selected. "), 5000);
    }
}

void MainWindow::on_actionCustom_baud_triggered()
{
    SetCustomBaud * customBaudDialog = new SetCustomBaud(this);
    connect(customBaudDialog,SIGNAL(setBaudRate(int)),SLOT(customBaud(int)));
    customBaudDialog->show();
}

void MainWindow::on_clear_clicked()
{
    ui->plainTextEdit->clear();
}

void MainWindow::on_clearDisplayButton_clicked()
{
    ui->serialTextBrowser->clear();
    m_text.clear();
    m_textAsHex.clear();
    m_numBytesDisplayed = 0;
    ui->clearDisplayButton->setEnabled(false);
    connect(ui->serialTextBrowser,SIGNAL(textChanged()), this, SLOT(enableClearButton()));
    ui->statusBar->clearMessage();
}
