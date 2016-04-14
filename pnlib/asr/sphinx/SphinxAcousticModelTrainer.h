#ifndef SPHINXACOUSTICMODELTRAINER_H
#define SPHINXACOUSTICMODELTRAINER_H

#include <QObject>

class SphinxAcousticModelTrainer : public QObject
{
    Q_OBJECT
public:
    explicit SphinxAcousticModelTrainer(QObject *parent = 0);

signals:

public slots:
};

#endif // SPHINXACOUSTICMODELTRAINER_H