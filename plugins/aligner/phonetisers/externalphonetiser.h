#ifndef EXTERNALPHONETISER_H
#define EXTERNALPHONETISER_H

#include <QObject>
#include <QString>
#include "pncore/annotation/intervaltier.h"
#include "alignertoken.h"

struct ExternalPhonetiserData;

class ExternalPhonetiser : public QObject
{
    Q_OBJECT
public:
    explicit ExternalPhonetiser(QObject *parent = 0);
    ~ExternalPhonetiser();

    static QList<AlignerToken> phonetiseList(QList<Interval *> tokens, const QString &attributeID = QString());
    static QList<AlignerToken> phonetiseTier(IntervalTier *tier_tokens, const QString &attributeID = QString());
    static void addPhonetisationToTokens(IntervalTier *tier_tokens, const QString &attributeIDOrthographic,
                                         const QString &attributeIDPhonetisation);

signals:

public slots:

private:
    ExternalPhonetiserData *d;
};

#endif // EXTERNALPHONETISER_H
