#ifndef DICTIONARYPHONETISER_H
#define DICTIONARYPHONETISER_H

#include <QObject>
#include "Phonetiser.h"

class DictionaryPhonetiser : public AbstractPhonetiser
{
    Q_OBJECT
public:
    explicit DictionaryPhonetiser(QObject *parent = 0);

signals:

public slots:
};

#endif // DICTIONARYPHONETISER_H
