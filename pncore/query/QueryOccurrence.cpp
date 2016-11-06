#include "QueryOccurrence.h"

namespace Praaline {
namespace Core {

QueryOccurrence::QueryOccurrence(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                 const QMap<QString, QList<ResultInterval> > &resultIntervals,
                                 QObject *parent) :
    QObject(parent), m_corpusID(corpusID), m_communicationID(communicationID), m_annotationID(annotationID),
    m_resultIntervals(resultIntervals)
{
}

QueryOccurrence::~QueryOccurrence()
{
}

RealTime QueryOccurrence::tMin() const
{
    RealTime tMin;
    foreach (QString annotationLevelID, m_resultIntervals.keys()) {
        if (m_resultIntervals.value(annotationLevelID).isEmpty())
            continue;
        if ((m_resultIntervals.value(annotationLevelID).first().interval->tMin() < tMin) || (tMin == RealTime(0, 0)))
            tMin = m_resultIntervals.value(annotationLevelID).first().interval->tMin();
    }
    return tMin;
}

RealTime QueryOccurrence::tMax() const
{
    RealTime tMax;
    foreach (QString annotationLevelID, m_resultIntervals.keys()) {
        if (m_resultIntervals.value(annotationLevelID).isEmpty())
            continue;
        if (m_resultIntervals.value(annotationLevelID).last().interval->tMax() > tMax)
            tMax = m_resultIntervals.value(annotationLevelID).last().interval->tMax();
    }
    return tMax;
}

QStringList QueryOccurrence::annotationLevelIDs() const
{
    return m_resultIntervals.keys();
}

QStringList QueryOccurrence::speakerIDs() const
{
    QStringList speakerIDs;
    foreach (QString annotationLevelID, m_resultIntervals.keys()) {
        foreach (ResultInterval intv, m_resultIntervals.value(annotationLevelID)) {
            if (!speakerIDs.contains(intv.speakerID))
                speakerIDs << intv.speakerID;
        }
    }
    return speakerIDs;
}

QList<QueryOccurrence::ResultInterval> QueryOccurrence::resultIntervals(const QString &annotationLevelID)
{
    QList<QueryOccurrence::ResultInterval> empty;
    if (m_resultIntervals.contains(annotationLevelID)) {
        return m_resultIntervals.value(annotationLevelID);
    }
    return empty;
}

int QueryOccurrence::resultIntervalsCount(const QString &annotationLevelID) const
{
    if (m_resultIntervals.contains(annotationLevelID))
        return m_resultIntervals.value(annotationLevelID).count();
    return 0;
}

QList<QueryOccurrence::ResultInterval> QueryOccurrence::leftContext(const QString &annotationLevelID)
{
    QList<QueryOccurrence::ResultInterval> ret;
    if (!m_resultIntervals.contains(annotationLevelID)) return ret;
    foreach (ResultInterval intv, m_resultIntervals.value(annotationLevelID)) {
        if (intv.type == ResultInterval::LeftContext)
            ret << intv;
    }
    return ret;
}

QList<QueryOccurrence::ResultInterval> QueryOccurrence::target(const QString &annotationLevelID)
{
    QList<QueryOccurrence::ResultInterval> ret;
    if (!m_resultIntervals.contains(annotationLevelID)) return ret;
    foreach (ResultInterval intv, m_resultIntervals.value(annotationLevelID)) {
        if (intv.type == ResultInterval::Target)
            ret << intv;
    }
    return ret;
}

QList<QueryOccurrence::ResultInterval> QueryOccurrence::rightContext(const QString &annotationLevelID)
{
    QList<QueryOccurrence::ResultInterval> ret;
    if (!m_resultIntervals.contains(annotationLevelID)) return ret;
    foreach (ResultInterval intv, m_resultIntervals.value(annotationLevelID)) {
        if (intv.type == ResultInterval::RightContext)
            ret << intv;
    }
    return ret;
}

} // namespace Core
} // namespace Praaline
