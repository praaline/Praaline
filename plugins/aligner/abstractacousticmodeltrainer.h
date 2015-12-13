#ifndef ABSTRACTACOUSTICMODELTRAINER_H
#define ABSTRACTACOUSTICMODELTRAINER_H

#include <QObject>

class AbstractAcousticModelTrainer : public QObject
{
    Q_OBJECT
public:
    explicit AbstractAcousticModelTrainer(QObject *parent = 0) {}
    virtual ~AbstractAcousticModelTrainer() {}

signals:

public slots:
};

#endif // ABSTRACTACOUSTICMODELTRAINER_H
