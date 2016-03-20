#ifndef ABSTRACTPHONETISER_H
#define ABSTRACTPHONETISER_H

#include <QObject>

class AbstractPhonetiser : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPhonetiser(QObject *parent = 0) : QObject(parent) {}
    virtual ~AbstractPhonetiser() {}

signals:

public slots:
};

#endif // ABSTRACTPHONETISER_H
