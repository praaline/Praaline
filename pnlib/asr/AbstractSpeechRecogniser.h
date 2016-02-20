#ifndef ABSTRACTRECOGNISER_H
#define ABSTRACTRECOGNISER_H

#include <QObject>

class AbstractRecogniser : public QObject
{
    Q_OBJECT
public:
    explicit AbstractRecogniser(QObject *parent = 0) {}
    virtual ~AbstractRecogniser() {}

signals:

public slots:
};

#endif // ABSTRACTRECOGNISER_H
