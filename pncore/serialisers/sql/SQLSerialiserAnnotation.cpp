#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "SQLSerialiserAnnotation.h"

namespace Praaline {
namespace Core {

SQLSerialiserAnnotation::SQLSerialiserAnnotation()
{
}

bool updateTier(const QString &annotationID, const QString &speakerID,
                AnnotationTier *tier, AnnotationStructureLevel *level, QSqlDatabase &db) {
    QSqlQuery query(db);
    QString q1, q2;

    q1 = QString("DELETE FROM %1 WHERE annotationID=:annotationID AND speakerID=:speakerID").arg(level->ID());
    query.prepare(q1);
    query.bindValue(":annotationID", annotationID);
    query.bindValue(":speakerID", speakerID);
    if (!query.exec()) { qDebug() << query.lastError(); return false; }

    q1 = QString("INSERT INTO %1 (annotationID, speakerID, intervalNo, tMin, tMax, xText").arg(level->ID());
    q2 = "VALUES (:annotationID, :speakerID, :intervalNo, :tMin, :tMax, :xText";
    foreach (QString attributeID, level->attributeIDs()) {
        q1.append(", ").append(attributeID);
        q2.append(", :").append(attributeID);
    }
    q1.append(") "); q2.append(") ");
    query.prepare(q1 + q2);
    query.bindValue(":annotationID", annotationID);
    query.bindValue(":speakerID", speakerID);
    int itemNo = 1;
    if (tier->tierType() == AnnotationTier::TierType_Intervals) {
        IntervalTier *tierI = qobject_cast<IntervalTier *>(tier);
        foreach (Interval *intv, tierI->intervals()) {
            query.bindValue(":intervalNo", itemNo);
            query.bindValue(":tMin", intv->tMin().toNanoseconds());
            query.bindValue(":tMax", intv->tMax().toNanoseconds());
            query.bindValue(":xText", intv->text());
            foreach (QString attributeID, level->attributeIDs()) {
                query.bindValue(QString(":%1").arg(attributeID), intv->attribute(attributeID));
            }
            if (!query.exec()) {
                qDebug() << "Error in updateTier (intervals): " << annotationID << speakerID << tier->name() << query.lastError(); return false;
            }
            ++itemNo;
        }
    }
    else if (tier->tierType() == AnnotationTier::TierType_Points) {
        PointTier *tierP = qobject_cast<PointTier *>(tier);
        foreach (Point *pnt, tierP->points()) {
            query.bindValue(":intervalNo", itemNo);
            query.bindValue(":tMin", pnt->time().toNanoseconds());
            query.bindValue(":tMax", pnt->time().toNanoseconds());
            query.bindValue(":xText", pnt->text());
            foreach (QString attributeID, level->attributeIDs()) {
                query.bindValue(QString(":%1").arg(attributeID), pnt->attribute(attributeID));
            }
            if (!query.exec()) {
                qDebug() << "Error in updateTier (points): " << annotationID << speakerID << tier->name() << query.lastError(); return false;
            }
            ++itemNo;
        }
    }
    return true;
}


// static
bool SQLSerialiserAnnotation::saveTiers(const QString &annotationID, const QString &speakerID,
                                        AnnotationTierGroup *tiers, AnnotationStructure *structure, QSqlDatabase &db)
{
    db.transaction();
    foreach (AnnotationTier *tier, tiers->tiers()) {
        AnnotationStructureLevel *level = structure->level(tier->name().replace("-", "_"));
        if (!level) continue;
        if (!updateTier(annotationID, speakerID, tier, level, db)) { db.rollback(); return false; }
    }
    db.commit();
    return true;
}

// static
bool SQLSerialiserAnnotation::saveTier(const QString &annotationID, const QString &speakerID,
                                       AnnotationTier *tier, AnnotationStructure *structure, QSqlDatabase &db)
{
    AnnotationStructureLevel *level = structure->level(tier->name().replace("-", "_"));
    if (!level) return false;
    db.transaction();
    if (!updateTier(annotationID, speakerID, tier, level, db)) { db.rollback(); return false; }
    db.commit();
    return true;
}


// ====================================================================================================================

// static
QMap<QString, QPointer<AnnotationTierGroup> > SQLSerialiserAnnotation::getTiersAllSpeakers(
        const QString &annotationID,
        const AnnotationStructure *structure, QSqlDatabase &db,
        const QStringList &levelIDs)
{
    QStringList effectiveLevelIDs;
    if (levelIDs.isEmpty())
        effectiveLevelIDs = structure->levelIDs();
    else {
        foreach (QString levelID, levelIDs) {
            if (structure->hasLevel(levelID))
                effectiveLevelIDs << levelID;
        }
    }

    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QString levelID, effectiveLevelIDs) {
        QString q = QString("SELECT DISTINCT speakerID FROM %1 WHERE annotationID = :annotationID").arg(levelID);
        QSqlQuery query(db);
        query.setForwardOnly(true);
        query.prepare(q);
        query.bindValue(":annotationID", annotationID);
        query.exec();
        while (query.next()) {
            QString speakerID = query.value(0).toString();
            QPointer<AnnotationTierGroup> tiersSpk;
            if (tiersAll.contains(speakerID))
                tiersSpk = tiersAll.value(speakerID);
            else {
                tiersSpk = new AnnotationTierGroup();
                tiersAll.insert(speakerID, tiersSpk);
            }
            AnnotationTier *tier = getTier(annotationID, speakerID, structure, db, levelID);
            if (tier) tiersSpk->addTier(tier);
        }
    }
    return tiersAll;
}

// static
AnnotationTierGroup *SQLSerialiserAnnotation::getTiers(const QString &annotationID, const QString &speakerID,
                                                       const AnnotationStructure *structure, QSqlDatabase &db,
                                                       const QStringList &levelIDs)
{
    QStringList effectiveLevelIDs;
    if (levelIDs.isEmpty())
        effectiveLevelIDs = structure->levelIDs();
    else {
        foreach (QString levelID, levelIDs) {
            if (structure->hasLevel(levelID))
                effectiveLevelIDs << levelID;
        }
    }
    AnnotationTierGroup *group = new AnnotationTierGroup();
    foreach (QString levelID, effectiveLevelIDs) {
        AnnotationTier *tier = getTier(annotationID, speakerID, structure, db, levelID);
        if (tier) group->addTier(tier);
    }
    return group;
}

bool getIntervalsList(QList<Interval *> &intervals, QList<Point *> &points,
                      const QString &annotationID, const QString &speakerID,
                      const AnnotationStructure *structure, QSqlDatabase &db,
                      const QString &levelID, const QStringList &attributeIDs,
                      int intervalNoMin, int intervalNoMax, RealTime tMin, RealTime tMax)
{
    // Check that the requested level exists in the annotation structure
    AnnotationStructureLevel *level = structure->level(levelID);
    if (!level) return false;
    // Derive the list of attributes that will be requested from the database
    QStringList effectiveAttributeIDs;
    if (attributeIDs.isEmpty())
        effectiveAttributeIDs = level->attributeIDs();
    else {
        foreach (QString attributeID, attributeIDs) {
            if (level->hasAttribute(attributeID))
                effectiveAttributeIDs << attributeID;
        }
    }
    // TODO: IntervalTier vs. PointTier from structure
    // Run query
    QSqlQuery query(db);
    QString q("SELECT tMin, tMax, xText");
    foreach (QString attribute, effectiveAttributeIDs) {
        q.append(", ").append(attribute);
    }
    q.append(QString(" FROM %1 WHERE 1=1 ").arg(levelID));
    if (!annotationID.isEmpty()) q.append(" AND annotationID = :annotationID ");
    if (!speakerID.isEmpty()) q.append(" AND speakerID = :speakerID ");
    if (intervalNoMin >= 0) q.append(" AND intervalNo >= :intervalNoMin ");
    if (intervalNoMax >= 0) q.append(" AND intervalNo <= :intervalNoMax ");
    if (tMin >= RealTime(0, 0)) q.append(" AND tMin >= :tMinMin ");
    if (tMax >= RealTime(0, 0)) q.append(" AND tMax <= :tMaxMax ");
    q.append(" ORDER BY tMin ");
    // Criteria
    query.setForwardOnly(true);
    query.prepare(q);
    if (!annotationID.isEmpty()) query.bindValue(":annotationID", annotationID);
    if (!speakerID.isEmpty()) query.bindValue(":speakerID", speakerID);
    if (intervalNoMin >= 0) query.bindValue(":intervalNoMin", intervalNoMin);
    if (intervalNoMax >= 0) query.bindValue(":intervalNoMax", intervalNoMax);
    if (tMin >= RealTime(0, 0)) query.bindValue(":tMinMin", tMin.toNanoseconds());
    if (tMax >= RealTime(0, 0)) query.bindValue(":tMaxMax", tMax.toNanoseconds());
    query.exec();
    // Read intervals
    while (query.next()) {
        long long tMin_nsec = query.value(0).toLongLong();
        long long tMax_nsec = query.value(1).toLongLong();
        QString xText = query.value(2).toString();
        int i = 3;
        QHash<QString, QVariant> attributes;
        foreach (QString attribute, effectiveAttributeIDs) {
            attributes.insert(attribute, query.value(i));
            i++;
        }
        if (levelID == "smooth") {
            points << new Point(RealTime::fromNanoseconds(tMin_nsec), xText, attributes);
        } else {
            intervals << new Interval(RealTime::fromNanoseconds(tMin_nsec), RealTime::fromNanoseconds(tMax_nsec),
                                      xText, attributes);
        }
//        RealTime r = RealTime::fromNanoseconds(tMin_nsec);
//        RealTime r2 = RealTime::fromNanoseconds(tMax_nsec);
//        double d = (r2-r).toDouble();
//        qDebug() << tMin_nsec << tMax_nsec << xText << d;
    }
    return true;
}

// static
QList<Interval *> SQLSerialiserAnnotation::getIntervals(const QString &annotationID, const QString &speakerID,
                                                        const AnnotationStructure *structure, QSqlDatabase &db,
                                                        const QString &levelID, int intervalNoMin, int intervalNoMax,
                                                        const QStringList &attributeIDs)
{
    QList<Interval *> intervals; QList<Point *> points;
    getIntervalsList(intervals, points, annotationID, speakerID, structure, db, levelID, attributeIDs,
                     intervalNoMin, intervalNoMax, RealTime(-1, 0), RealTime(-1, 0));
    return intervals;
}

// static
QList<Interval *> SQLSerialiserAnnotation::getIntervals(const QString &annotationID, const QString &speakerID,
                                                        const AnnotationStructure *structure, QSqlDatabase &db,
                                                        const QString &levelID, RealTime tMin, RealTime tMax,
                                                        const QStringList &attributeIDs)
{
    QList<Interval *> intervals; QList<Point *> points;
    getIntervalsList(intervals, points, annotationID, speakerID, structure, db, levelID, attributeIDs, -1, -1, tMin, tMax);
    return intervals;
}

// static
AnnotationTier *SQLSerialiserAnnotation::getTier(const QString &annotationID, const QString &speakerID,
                                                 const AnnotationStructure *structure, QSqlDatabase &db,
                                                 const QString &levelID, const QStringList &attributeIDs)
{
    QList<Interval *> intervals; QList<Point *> points;
    getIntervalsList(intervals, points, annotationID, speakerID, structure, db, levelID, attributeIDs, -1, -1,
                     RealTime(-1, 0), RealTime(-1, 0));

    if (intervals.count() > 0) {
        return new IntervalTier(levelID, intervals);
    } else if (points.count() > 0) {
        return new PointTier(levelID, points);
    }
    return 0;
}

// static
QList<QString> SQLSerialiserAnnotation::getSpeakersInLevel(const QString &annotationID, const QString &levelID,
                                                           const AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<QString> speakerIDs;
    // Check that the requested level exists in the annotation structure
    if (!structure) return speakerIDs;
    AnnotationStructureLevel *level = structure->level(levelID);
    if (!level) return speakerIDs;
    // Run query
    QSqlQuery query(db);
    QString q = QString("SELECT DISTINCT speakerID FROM %1 WHERE annotationID = :annotationID").arg(levelID);
    query.setForwardOnly(true);
    query.prepare(q);
    query.bindValue(":annotationID", annotationID);
    query.exec();
    while (query.next()) {
        speakerIDs << query.value(0).toString();
    }
    return speakerIDs;
}

// static
QList<QString> SQLSerialiserAnnotation::getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID,
                                                                 const AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<QString> speakerIDs;
    // Check that the requested level exists in the annotation structure
    if (!structure) return speakerIDs;
    AnnotationStructureLevel *level = structure->level(levelID);
    if (!level) return speakerIDs;
    // Run query
    QSqlQuery query(db);
    QString q = QString("SELECT DISTINCT speakerID FROM %1 WHERE annotationID = :annotationID AND xText IS NOT NULL"
                        " AND xText <> '_' AND xText <> ''").arg(levelID);
    query.setForwardOnly(true);
    query.prepare(q);
    query.bindValue(":annotationID", annotationID);
    query.exec();
    while (query.next()) {
        speakerIDs << query.value(0).toString();
    }
    return speakerIDs;
}

// static
QList<QString> SQLSerialiserAnnotation::getSpeakersInAnnotation(const QString &annotationID,
                                                                const AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<QString> allSpeakerIDs;
    foreach (QString levelID, structure->levelIDs()) {
        foreach (QString speakerID, getSpeakersInLevel(annotationID, levelID, structure, db)) {
            if (!allSpeakerIDs.contains(speakerID))
                allSpeakerIDs << speakerID;
        }
    }
    return allSpeakerIDs;
}

// static
QList<QString> SQLSerialiserAnnotation::getSpeakersActiveInAnnotation(const QString &annotationID,
                                                                      const AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<QString> allSpeakerIDs;
    foreach (QString levelID, structure->levelIDs()) {
        foreach (QString speakerID, getSpeakersActiveInLevel(annotationID, levelID, structure, db)) {
            if (!allSpeakerIDs.contains(speakerID))
                allSpeakerIDs << speakerID;
        }
    }
    return allSpeakerIDs;
}

// static
IntervalTier * SQLSerialiserAnnotation::getSpeakerTimeline(const QString &communicationID, const QString &annotationID,
                                                           const QString &levelID, bool detailed,
                                                           const AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<RealTime> timelineBoundaries;
    QMap<QString, QList<Interval *> > speakingIntervals;
    QMap<QString, IntervalTier * > speakingTiers;

    if (!structure) return 0;
    AnnotationStructureLevel *level = structure->level(levelID);
    if (!level) return 0;
    if (communicationID.isEmpty() && annotationID.isEmpty()) return 0;

    QSqlQuery query(db);
    QString q = QString("SELECT tMin, tMax, speakerID, xText FROM %1 ").arg(levelID);
    if (!communicationID.isEmpty()) q.append(QString(" INNER JOIN annotation ON %1.annotationID=annotation.annotationID ").arg(levelID));
    q.append(" WHERE 1=1 ");
    if (!communicationID.isEmpty()) q.append(" AND annotation.communicationID=:communicationID ");
    if (!annotationID.isEmpty()) q.append(" AND annotation.annotationID = :annotationID ");
    q.append(" ORDER BY tMin ");
    // qDebug() << q;
    query.setForwardOnly(true);
    query.prepare(q);
    if (!communicationID.isEmpty()) query.bindValue(":communicationID", communicationID);
    if (!annotationID.isEmpty()) query.bindValue(":annotationID", annotationID);
    query.exec();
    while (query.next()) {
        RealTime tMin = RealTime::fromNanoseconds(query.value(0).toLongLong());
        RealTime tMax = RealTime::fromNanoseconds(query.value(1).toLongLong());
        QString speakerID = query.value(2).toString();
        QString xText = query.value(3).toString();
        if (!timelineBoundaries.contains(tMin)) timelineBoundaries << tMin;
        if (!timelineBoundaries.contains(tMax)) timelineBoundaries << tMax;
        speakingIntervals[speakerID].append(new Interval(tMin, tMax, xText));
    }
    if (timelineBoundaries.isEmpty()) return 0;
    qSort(timelineBoundaries);
    QList<Interval *> timelineIntervals;
    for (int i = 0; i < timelineBoundaries.count() - 1; ++i) {
        timelineIntervals << new Interval(timelineBoundaries.at(i), timelineBoundaries.at(i + 1), "");
    }
    IntervalTier *timeline = new IntervalTier(QString("%1_timeline").arg(levelID), timelineIntervals,
                                              timelineBoundaries.first(), timelineBoundaries.last());
    foreach (QString speakerID, speakingIntervals.keys()) {
        speakingTiers.insert(speakerID, new IntervalTier(speakerID, speakingIntervals[speakerID],
                                                         timeline->tMin(), timeline->tMax()));
    }
    foreach (Interval *segment, timeline->intervals()) {
        QString speakers;
        foreach (QString speakerID, speakingTiers.keys()) {
            Interval *intv = speakingTiers.value(speakerID)->intervalAtTime(segment->tCenter());
            if (!intv) continue;
            if (intv->isPauseSilent()) continue;
            speakers.append(speakerID).append("+");
        }
        if (!speakers.isEmpty()) speakers.chop(1);
        segment->setText(speakers);
    }
    qDeleteAll(speakingTiers);
    if (!detailed) timeline->mergeIdenticalAnnotations();
    return timeline;
}

// static
bool SQLSerialiserAnnotation::deleteAllForAnnotationID(QString annotationID, const AnnotationStructure *structure, QSqlDatabase &db)
{
    if (!structure) return false;
    foreach (QString levelID, structure->levelIDs()) {
        QSqlQuery query(db);
        QString q = QString("DELETE FROM %1 WHERE annotationID = :annotationID").arg(levelID);
        query.prepare(q);
        query.bindValue(":annotationID", annotationID);
        query.exec();
    }
    return true;
}

// static
QList<QPair<QList<QVariant>, long> > SQLSerialiserAnnotation::getDistinctLabels(const QString &levelID, const QStringList &attributeIDs,
                                                                        const AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<QPair<QList<QVariant>, long> > list;
    if (!structure) return list;
    if (!structure->hasLevel(levelID)) return list;

    QSqlQuery query(db);
    QString attributesList;
    foreach (QString attributeID, attributeIDs) {
        attributesList.append((attributeID.isEmpty()) ? "xText" : attributeID).append(", ");
    }
    if (attributesList.length() > 2) attributesList.chop(2);
    QString q = QString("SELECT DISTINCT %1, COUNT(*) FROM %2 GROUP BY %1").arg(attributesList).arg(levelID);
    query.setForwardOnly(true);
    query.prepare(q);
    query.exec();
    while (query.next()) {
        QPair<QList<QVariant>, int> row;
        QList<QVariant> values;
        for (int i = 0; i < attributeIDs.count(); ++i) {
            values << query.value(i);
        }
        int count = query.value(attributeIDs.count()).toInt();
        row.first = values;
        row.second = count;
        list << row;
    }
    return list;
}

// static
bool SQLSerialiserAnnotation::batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                                          const QList<QPair<QString, QVariant> > &criteria,
                                          const AnnotationStructure *structure, QSqlDatabase &db)
{
    if (!structure) return false;
    AnnotationStructureLevel *level = structure->level(levelID);
    if (!level) return false;
    if ((!attributeID.isEmpty()) && (!level->hasAttribute(attributeID))) return false;

    QSqlQuery query(db);

    QString upd = (attributeID.isEmpty()) ? "xText" : attributeID;
    QString q = QString("UPDATE %1 SET %2 = :newValue WHERE ").arg(level->ID()).arg(upd);
    for (int i = 0; i < criteria.count(); ++i) {
        upd = (criteria.at(i).first.isEmpty()) ? "xText" : criteria.at(i).first;
        q.append(QString("%1 = :%1").arg(upd));
        if (i < criteria.count() - 1) q.append(" AND ");
    }
    query.prepare(q);
    query.bindValue(":newValue", newValue);
    for (int i = 0; i < criteria.count(); ++i) {
        upd = (criteria.at(i).first.isEmpty()) ? "xText" : criteria.at(i).first;
        query.bindValue(QString(":%1").arg(upd), criteria.at(i).second);
    }
    return query.exec();
}

// static
QList<QPair<QList<QVariant>, long> > SQLSerialiserAnnotation::countItems(const QString &levelID, const QStringList &groupByAttributeIDs,
                                                                         const AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<QPair<QList<QVariant>, long> > list;
    if (!structure) return list;
    if (!structure->hasLevel(levelID)) return list;

    return list;
}

} // namespace Core
} // namespace Praaline
