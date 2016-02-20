#ifndef ABSTRACTSYLLABIFIER_H
#define ABSTRACTSYLLABIFIER_H

#include <QObject>

class AbstractSyllabifier : public QObject
{
    Q_OBJECT
public:
    explicit AbstractSyllabifier(QObject *parent = 0) {}
    ~AbstractSyllabifier() {}

signals:

public slots:
};

#endif // ABSTRACTSYLLABIFIER_H
