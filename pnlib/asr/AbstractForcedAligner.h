#ifndef ABSTRACTFORCEDALIGNER_H
#define ABSTRACTFORCEDALIGNER_H

#include <QObject>

class AbstractForcedAligner : public QObject
{
    Q_OBJECT
public:
    explicit AbstractForcedAligner(QObject *parent = 0) : QObject(parent) {}
    virtual ~AbstractForcedAligner() {}

signals:

public slots:
};

#endif // ABSTRACTFORCEDALIGNER_H
