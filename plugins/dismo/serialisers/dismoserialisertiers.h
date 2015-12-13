#ifndef DISMOSERIALISERTIERS_H
#define DISMOSERIALISERTIERS_H

#include <QObject>
#include "annotation/intervaltier.h"
#include "dismo/tokenlist.h"

namespace DisMoAnnotator {

class DisMoSerialiserTiers : public QObject
{
    Q_OBJECT
public:

    explicit DisMoSerialiserTiers(QObject *parent = 0);
    void serializeToTiers(TokenList &TL, IntervalTier *tierMin, IntervalTier *tierMWU);

signals:

public slots:

};

}

#endif // DISMOSERIALISERTIERS_H
