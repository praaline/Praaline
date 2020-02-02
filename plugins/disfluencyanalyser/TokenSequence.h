#ifndef TOKENSEQUENCE_H
#define TOKENSEQUENCE_H

#include <QObject>
#include <QString>
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/Interval.h"
using namespace Praaline::Core;

class TokenSequence : public QObject
{
    Q_OBJECT
public:
    TokenSequence(IntervalTier *tierTokens, int start, int end, const QString &tag, QObject *parent = nullptr);

    int indexStart() const { return m_start; }
    int indexEnd() const { return m_end; }

    QList<Interval *> contextIntervals(int delta) const;
    QList<Interval *> contextIntervals(RealTime delta) const;
    QString contextText(int delta) const;
    QString contextText(RealTime delta) const;

signals:

public slots:

protected:
    // Basic data
    IntervalTier *m_tierTokens;
    int m_start;
    int m_end;
    QString m_tag;
};

#endif // TOKENSEQUENCE_H
