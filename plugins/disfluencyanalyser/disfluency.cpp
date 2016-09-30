#include "disfluency.h"

Disfluency::Disfluency(IntervalTier *tierTokens, int start, int end, const QString &globalTag,
                       int interruptionPoint, int reparansStart, int editTermStart, int editTermEnd,
                       QObject *parent) :
    TokenSequence(tierTokens, start, end, globalTag, parent),
    m_interruptionPoint(interruptionPoint), m_reparansStart(reparansStart),
    m_editTermStart(editTermStart), m_editTermEnd(editTermEnd)
{
    if (!m_tierTokens) {
        // invalid tokens tier
        m_start = m_end = m_interruptionPoint = m_reparansStart = m_editTermStart = m_editTermEnd = -1;
        return;
    }
    if ((m_start > m_end) || (m_start < 0) || (m_end >= m_tierTokens->countItems())) {
        // invalid data
        m_start = m_end = m_interruptionPoint = m_reparansStart = m_editTermStart = m_editTermEnd = -1;
        return;
    }
    if (!(m_interruptionPoint >= m_start) || !(m_interruptionPoint <= m_end)) {
        // invalid interruption point
        m_interruptionPoint = -1;
    }
    if (!(m_reparansStart >= m_start) || !(m_reparansStart <= m_end)) {
        // invalid reparans starting point
        m_reparansStart = -1;
    }
    if ((m_editTermStart > m_editTermEnd) || (m_editTermStart < m_start) || (m_editTermEnd > m_end)) {
        // invalid editing term
        m_editTermStart = m_editTermEnd = -1;
    }
}

RealTime Disfluency::timeStart() const
{
    if ((!m_tierTokens) || (m_start < 0) || (m_start >= m_tierTokens->countItems()))
        return RealTime(-1, 0);
    return m_tierTokens->interval(m_start)->tMin();
}
RealTime Disfluency::timeEnd() const
{
    if ((!m_tierTokens) || (m_end < 0) || (m_end >= m_tierTokens->countItems()))
        return RealTime(-1, 0);
    return m_tierTokens->interval(m_end)->tMax();
}
RealTime Disfluency::timeInterruptionPoint() const
{
    if ((!m_tierTokens) || (m_interruptionPoint < 0) || (m_interruptionPoint >= m_tierTokens->countItems()))
        return RealTime(-1, 0);
    return m_tierTokens->interval(m_interruptionPoint)->tMax();
}
RealTime Disfluency::timeReparansStart() const
{
    if ((!m_tierTokens) || (m_reparansStart < 0) || (m_reparansStart >= m_tierTokens->countItems()))
        return RealTime(-1, 0);
    return m_tierTokens->interval(m_reparansStart)->tMin();
}
RealTime Disfluency::timeEditTermStart() const
{
    if ((!m_tierTokens) || (m_editTermStart < 0) || (m_editTermStart >= m_tierTokens->countItems()))
        return RealTime(-1, 0);
    return m_tierTokens->interval(m_editTermStart)->tMin();
}
RealTime Disfluency::timeEditTermEnd() const
{
    if ((!m_tierTokens) || (m_editTermEnd < 0) || (m_editTermEnd >= m_tierTokens->countItems()))
        return RealTime(-1, 0);
    return m_tierTokens->interval(m_editTermEnd)->tMax();
}

Disfluency::DisfluencyStructureTypes Disfluency::structureType() const
{
    if ((!m_tierTokens) || (m_start < 0))
        return Disfluency::InvalidData;
    if (m_start == m_end)
        return Disfluency::Simple;
    return Disfluency::Structured;
}

QList<Interval *> Disfluency::reparandumIntervals() const
{
    QList<Interval *> ret;
    if (!m_tierTokens) return ret;
    int start = m_start;
    int end = (m_interruptionPoint < 0) ? m_end : m_interruptionPoint;
    for (int i = start; i <= end; i++) {
        Interval *intv = m_tierTokens->interval(i);
        if (intv) ret << intv;
    }
    return ret;
}

QList<Interval *> Disfluency::interregnumIntervals() const
{
    QList<Interval *> ret;
    if (!m_tierTokens) return ret;
    if (m_interruptionPoint < 0) return ret;
    int start = m_interruptionPoint + 1;
    int end = (m_reparansStart < 0) ? m_end : m_reparansStart;
    for (int i = start; i < end; i++) {
        Interval *intv = m_tierTokens->interval(i);
        if (intv) ret << intv;
    }
    return ret;
}

QList<Interval *> Disfluency::editTermIntervals() const
{
    QList<Interval *> ret;
    if (!m_tierTokens) return ret;
    if (m_editTermStart < 0) return ret;
    for (int i = m_editTermStart; i <= m_editTermEnd; i++) {
        Interval *intv = m_tierTokens->interval(i);
        if (intv) ret << intv;
    }
    return ret;
}

QList<Interval *> Disfluency::reparansIntervals(int startFrom) const
{
    QList<Interval *> ret;
    if (!m_tierTokens) return ret;
    if (m_start < 0) return ret;
    int start = (m_reparansStart < 0) ? m_end : m_reparansStart;
    if (startFrom > 0) start = startFrom;
    for (int i = start; i <= m_end; i++) {
        Interval *intv = m_tierTokens->interval(i);
        if (intv) ret << intv;
    }
    return ret;
}

QString Disfluency::reparandumText() const
{
    QString ret;
    foreach (Interval *intv, reparandumIntervals()) {
        ret.append(intv->text()).append(" ");
    }
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

QString Disfluency::interregnumText() const
{
    QString ret;
    foreach (Interval *intv, interregnumIntervals()) {
        ret.append(intv->text()).append(" ");
    }
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

QString Disfluency::editTermText() const
{
    QString ret;
    foreach (Interval *intv, editTermIntervals()) {
        ret.append(intv->text()).append(" ");
    }
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

QString Disfluency::reparansText() const
{
    QString ret;
    foreach (Interval *intv, reparansIntervals()) {
        ret.append(intv->text()).append(" ");
    }
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

QString Disfluency::formatted(const QString &leftS, const QString &rightS, const QString &intpt,
                              const QString &leftE, const QString &rightE)
{
    QString ret;
    if (!m_tierTokens) return ret;
    if (m_start < 0) return ret;
    ret.append(leftS);
    ret.append(reparandumText());
    if (m_interruptionPoint > 0) ret.append(intpt);
//    if (m_editTermStart > 0) {
//        ret.append(leftE);
//        ret.append(editTermText());
//        ret.append(rightE);
//    }
    if (!interregnumText().isEmpty())
        ret.append(interregnumText()).append(" |");
    ret.append(reparansText());
    ret.append(rightS);
    return ret;
}

QList<QList<Interval *> > Disfluency::backtracking() const
{
    QList<QList<Interval *> > ret;
    if (structureType() == Disfluency::Simple) {
        ret << reparandumIntervals();
    }
    else if (structureType() == Disfluency::Structured) {
        QList<Interval *> line1;
        QList<Interval *> line2;
        QList<Interval *> reparandum = reparandumIntervals();
        QList<Interval *> reparans = reparansIntervals();
        int i = 0, j = 0;
        while (i < reparandum.count() || j < reparans.count()) {
            QString A = (i < reparandum.count()) ? reparandum.at(i)->text() : "";
            QString B = (j < reparandum.count()) ? reparandum.at(j)->text() : "";
            i++; j++;
        }
    }
    return ret;
}

QString Disfluency::backtrackingFormatted() const
{

}
