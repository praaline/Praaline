#ifndef DISFLUENCYANALYSERTOOL_H
#define DISFLUENCYANALYSERTOOL_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "PraalineCore/Base/RealTime.h"
#include "PraalineCore/Annotation/Interval.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "Disfluency.h"

class DisfluencyAnalyserTool : public QObject
{
    Q_OBJECT
public:
    DisfluencyAnalyserTool(IntervalTier *tierTokens, QObject *parent = nullptr);

    Disfluency* disfluency(int index) const;
    QList<Disfluency *> disfluencies() const;

    bool readFromTier(IntervalTier *tierDisfluency, const QString &attribute = QString());

signals:

public slots:

private:
    IntervalTier *m_tierTokens;
    QList<Disfluency *> m_disfluencies;

    static QStringList m_simpleDisfluencyTags;
    static QStringList m_structuredDisfluecnyTags;

};

#endif // DISFLUENCYANALYSERTOOL_H
