#ifndef FTDSERIALPORT_H
#define FTDSERIALPORT_H

#include <QObject>

#include "ftd2xx.h"

class ftdSerialPort : public QObject
{
    Q_OBJECT
public:
    explicit ftdSerialPort(QObject *parent = 0);

signals:
    void readyRead();

public slots:
};

#endif // FTDSERIALPORT_H
