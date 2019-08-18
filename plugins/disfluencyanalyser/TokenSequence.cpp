#include "TokenSequence.h"

TokenSequence::TokenSequence(IntervalTier *tierTokens, int start, int end, const QString &tag, QObject *parent) :
    QObject(parent), m_tierTokens(tierTokens), m_start(start), m_end(end), m_tag(tag)
{
}

QList<Interval *> TokenSequence::contextIntervals(int delta) const
{
    if (!m_tierTokens) return QList<Interval *>();
    return m_tierTokens->getContext(m_start, delta);
}

QList<Interval *> TokenSequence::contextIntervals(RealTime delta) const
{
    if (!m_tierTokens) return QList<Interval *>();
    return m_tierTokens->getContext(m_start, delta);
}

QString TokenSequence::contextText(int delta) const
{
    QString ret;
    if (!m_tierTokens) return ret;
    int startingPoint = m_start;
    if (delta > 0) startingPoint = m_end;
    foreach (Interval *intv, m_tierTokens->getContext(startingPoint, delta))
        if (intv) ret.append(intv->text()).append(" ");
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

QString TokenSequence::contextText(RealTime delta) const
{
    QString ret;
    if (!m_tierTokens) return ret;
    int startingPoint = m_start;
    if (delta > RealTime(0, 0)) startingPoint = m_end;
    foreach (Interval *intv, m_tierTokens->getContext(startingPoint, delta))
        if (intv) ret.append(intv->text()).append(" ");
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}
