#ifndef DICTIONARYPHONETISER_H
#define DICTIONARYPHONETISER_H

#include <QObject>
#include "AbstractPhonetiser.h"

class DictionaryPhonetiser : public AbstractPhonetiser
{
    Q_OBJECT
public:
    explicit DictionaryPhonetiser(QObject *parent = 0);

signals:

public slots:
};

#endif // DICTIONARYPHONETISER_H
