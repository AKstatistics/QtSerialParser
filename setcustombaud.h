#ifndef SETCUSTOMBAUD_H
#define SETCUSTOMBAUD_H

#include <QDialog>

namespace Ui {
class SetCustomBaud;
}

class SetCustomBaud : public QDialog
{
    Q_OBJECT

public:
    explicit SetCustomBaud(QWidget *parent = 0);
    ~SetCustomBaud();
    
signals:
    void setBaudRate(int);

private slots:
    void on_buttonBox_accepted();
    
private:
    Ui::SetCustomBaud *ui;
};

#endif // SETCUSTOMBAUD_H
