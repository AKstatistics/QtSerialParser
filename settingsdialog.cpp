#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    // Set up selectable port list
    QTableWidget * portList = ui->portList; // for convenience
    portList->setColumnCount(3);

    // create headers
    QTableWidgetItem * portName = new QTableWidgetItem("Port Name");
    portList->setHorizontalHeaderItem(0, portName);

    // selection settings: select entire rows, only one row at a time
    portList->setSelectionBehavior(QAbstractItemView::SelectRows);
    portList->setSelectionMode(QAbstractItemView::SingleSelection);

    QTableWidgetItem * pidHeader = new QTableWidgetItem("PID");
    pidHeader->setToolTip("Product ID #");
    portList->setHorizontalHeaderItem(1, pidHeader);

    QTableWidgetItem * vidHeader = new QTableWidgetItem("VID");
    vidHeader->setToolTip("Vendor ID #");
    portList->setHorizontalHeaderItem(2, vidHeader);

    populatePortList();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::populatePortList()
{
    QTableWidget * portList = ui->portList; // for convenience

    portList->clearContents();
    foreach( const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts() ){
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier()){
            portList->insertRow(portList->rowCount());
            // port name
            QTableWidgetItem * portNameItem = new QTableWidgetItem(serialPortInfo.portName());
            portList->setItem(portList->rowCount()-1, 0, portNameItem);
            // pid
            QTableWidgetItem * pidItem = new QTableWidgetItem(serialPortInfo.productIdentifier());
            portList->setItem(portList->rowCount()-1, 1, pidItem);
            // vid
            QTableWidgetItem * vidItem = new QTableWidgetItem(serialPortInfo.vendorIdentifier());
            portList->setItem(portList->rowCount()-1, 2, vidItem);
        }
    }
    portList->sortItems(0);
}
