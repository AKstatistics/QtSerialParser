#include "setcustombaud.h"
#include "ui_setcustombaud.h"

SetCustomBaud::SetCustomBaud(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetCustomBaud)
{
    ui->setupUi(this);
}

SetCustomBaud::~SetCustomBaud()
{
    delete ui;
}

void SetCustomBaud::on_buttonBox_accepted()
{
    QString baudString = ui->lineEdit->text();
    int baudRate = baudString.toInt();
    emit setBaudRate(baudRate);
}
