#ifndef ABSTRACTLANGUAGEMODELTRAINER_H
#define ABSTRACTLANGUAGEMODELTRAINER_H

#include <QObject>

class AbstractLanguageModelTrainer : public QObject
{
    Q_OBJECT
public:
    explicit AbstractLanguageModelTrainer(QObject *parent = 0) {}
    virtual ~AbstractLanguageModelTrainer() {}

signals:

public slots:
};

#endif // ABSTRACTLANGUAGEMODELTRAINER_H
