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

// ==========================================================================================================================
// Helper Functions
// ==========================================================================================================================

// Derives the list of attributes that will be requested from the database, based on the requested attribute IDs and
// the attribute IDs actually in the database.
QStringList getEffectiveAttributeIDs(AnnotationStructureLevel *level, const QStringList &attributeIDs)
{
    QStringList effectiveAttributeIDs;
    if (!level) return effectiveAttributeIDs;
    if (attributeIDs.isEmpty())
        effectiveAttributeIDs = level->attributeIDs();
    else {
        foreach (QString attributeID, attributeIDs) {
            if (level->hasAttribute(attributeID))
                effectiveAttributeIDs << attributeID;
        }
    }
    return effectiveAttributeIDs;
}

bool prepareSelectQuery(QSqlQuery &query, AnnotationStructureLevel *level, const QStringList &effectiveAttributeIDs,
                        const AnnotationDatastore::Selection &selection)
{
    if (!level) return false;
    // Select basic data
    QString q("SELECT annotationID, speakerID ");
    if (level->isLevelTypePrimary())
        q.append(", intervalNo, tMin, tMax, xText ");
    else
        q.append(", intervalNoLeft, intervalNoRight, xText ");
    // Select attributes
    foreach (QString attribute, effectiveAttributeIDs) {
        q.append(", ").append(attribute);
    }
    // Criteria
    q.append(QString(" FROM %1 WHERE 1=1 ").arg(level->ID()));
    if (!selection.annotationID.isEmpty())          q.append(" AND annotationID = :annotationID ");
    if (!selection.speakerID.isEmpty())             q.append(" AND speakerID = :speakerID ");
    if (selection.indexMin >= 0) {
        if (level->isLevelTypePrimary())            q.append(" AND intervalNo >= :indexMin ");
        else                                        q.append(" AND intervalNoLeft >= :indexMin ");
    }
    if (selection.indexMax >= 0) {
        if (level->isLevelTypePrimary())            q.append(" AND intervalNo <= :indexMax ");
        else                                        q.append(" AND intervalNoRight <= :indexMax ");
    }
    if (level->isLevelTypePrimary()) {
        if (selection.timeMin >= RealTime(0, 0))    q.append(" AND tMin >= :timeMin ");
        if (selection.timeMax >= RealTime(0, 0))    q.append(" AND tMax <= :timeMax ");
    }
    // Sorting
    if (level->isLevelTypePrimary())    q.append(" ORDER BY tMin, annotationID, speakerID ");
    else                                q.append(" ORDER BY intervalNoLeft, annotationID, speakerID ");
    // Prepare query
    query.setForwardOnly(true);
    query.prepare(q);
    // Bind parameters
    if (!selection.annotationID.isEmpty())  query.bindValue(":annotationID", selection.annotationID);
    if (!selection.speakerID.isEmpty())     query.bindValue(":speakerID", selection.speakerID);
    if (selection.indexMin >= 0)            query.bindValue(":indexMin", selection.indexMin);
    if (selection.indexMax >= 0)            query.bindValue(":indexMax", selection.indexMax);
    if (level->isLevelTypePrimary()) {
        if (selection.timeMin >= RealTime(0, 0)) query.bindValue(":timeMin", selection.timeMin.toNanoseconds());
        if (selection.timeMax >= RealTime(0, 0)) query.bindValue(":timeMax", selection.timeMax.toNanoseconds());
    }
    return true;
}

// ==========================================================================================================================
// Annotation Elements
// ==========================================================================================================================
QList<AnnotationElement *> SQLSerialiserAnnotation::getAnnotationElements(
        const AnnotationDatastore::Selection &selection, AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<AnnotationElement *> elements;
    // Check that the requested level exists in the annotation structure
    AnnotationStructureLevel *level = structure->level(selection.levelID);
    if (!level) return elements;
    // Derive the list of attributes that will be requested from the database
    QStringList effectiveAttributeIDs = getEffectiveAttributeIDs(level, selection.attributeIDs);
    // Run query
    QSqlQuery query(db);
    prepareSelectQuery(query, level, effectiveAttributeIDs, selection);
    query.exec();
    // Read annotation elements
    while (query.next()) {
        QString xText = query.value("xText").toString();
        QHash<QString, QVariant> attributes;
        foreach (QString attributeID, effectiveAttributeIDs) {
            attributes.insert(attributeID, query.value(attributeID));
        }
        if (selection.annotationID.isEmpty()) attributes.insert("annotationID", query.value("annotationID"));
        if (selection.speakerID.isEmpty())    attributes.insert("speakerID", query.value("speakerID"));
        elements << new AnnotationElement(xText, attributes);
    }
    return elements;
}

QList<Interval *> SQLSerialiserAnnotation::getIntervals(
        const AnnotationDatastore::Selection &selection, AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<Interval *> intervals;
    // Check that the requested level exists in the annotation structure
    AnnotationStructureLevel *level = structure->level(selection.levelID);
    if (!level) return intervals;
    // Derive the list of attributes that will be requested from the database
    QStringList effectiveAttributeIDs = getEffectiveAttributeIDs(level, selection.attributeIDs);
    // Run query
    QSqlQuery query(db);
    prepareSelectQuery(query, level, effectiveAttributeIDs, selection);
    query.exec();
    // Read annotation elements
    while (query.next()) {
        RealTime tMin(RealTime::fromNanoseconds(query.value("tMin").toLongLong()));
        RealTime tMax(RealTime::fromNanoseconds(query.value("tMax").toLongLong()));
        QString xText = query.value("xText").toString();
        QHash<QString, QVariant> attributes;
        foreach (QString attributeID, effectiveAttributeIDs) {
            attributes.insert(attributeID, query.value(attributeID));
        }
        if (selection.annotationID.isEmpty()) attributes.insert("annotationID", query.value("annotationID"));
        if (selection.speakerID.isEmpty())    attributes.insert("speakerID", query.value("speakerID"));
        intervals << new Interval(tMin, tMax, xText, attributes);
    }
    return intervals;
}

QList<Point *> SQLSerialiserAnnotation::getPoints(
        const AnnotationDatastore::Selection &selection, AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<Point *> points;
    // Check that the requested level exists in the annotation structure
    AnnotationStructureLevel *level = structure->level(selection.levelID);
    if (!level) return points;
    // Derive the list of attributes that will be requested from the database
    QStringList effectiveAttributeIDs = getEffectiveAttributeIDs(level, selection.attributeIDs);
    // Run query
    QSqlQuery query(db);
    prepareSelectQuery(query, level, effectiveAttributeIDs, selection);
    query.exec();
    // Read annotation elements
    while (query.next()) {
        RealTime time(RealTime::fromNanoseconds(query.value("tMin").toLongLong()));
        QString xText = query.value("xText").toString();
        QHash<QString, QVariant> attributes;
        foreach (QString attributeID, effectiveAttributeIDs) {
            attributes.insert(attributeID, query.value(attributeID));
        }
        if (selection.annotationID.isEmpty()) attributes.insert("annotationID", query.value("annotationID"));
        if (selection.speakerID.isEmpty())    attributes.insert("speakerID", query.value("speakerID"));
        points << new Point(time, xText, attributes);
    }
    return points;
}

// ==========================================================================================================================
// Get annotation tiers
// ==========================================================================================================================
AnnotationTier *SQLSerialiserAnnotation::getTier(
        const QString &annotationID, const QString &speakerID, const QString &levelID, const QStringList &attributeIDs,
        AnnotationStructure *structure, QSqlDatabase &db)
{
    if (!structure) return Q_NULLPTR;
    AnnotationStructureLevel *level = structure->level(levelID);
    if (!level) return Q_NULLPTR;
    if (level->levelType() == AnnotationStructureLevel::IndependentPointsLevel) {
        QList<Point *> points = getPoints(AnnotationDatastore::Selection(annotationID, speakerID, levelID, attributeIDs), structure, db);
        if (!points.isEmpty()) return new PointTier(levelID, points);
    }
    else if ((level->levelType() == AnnotationStructureLevel::IndependentIntervalsLevel) ||
             (level->levelType() == AnnotationStructureLevel::GroupingLevel)) {
        QList<Interval *> intervals = getIntervals(AnnotationDatastore::Selection(annotationID, speakerID, levelID, attributeIDs), structure, db);
        if (!intervals.isEmpty()) return new IntervalTier(levelID, intervals);
    }
    return Q_NULLPTR;
}

AnnotationTierGroup *SQLSerialiserAnnotation::getTiers(
        const QString &annotationID, const QString &speakerID, const QStringList &levelIDs,
        AnnotationStructure *structure, QSqlDatabase &db)
{
    AnnotationTierGroup *tiers = new AnnotationTierGroup();
    foreach (QString levelID, levelIDs) {
        AnnotationTier *tier = getTier(annotationID, speakerID, levelID, QStringList(), structure, db);
        if (tier) tiers->addTier(tier);
    }
    return tiers;
}

QMap<QString, QPointer<AnnotationTierGroup> > SQLSerialiserAnnotation::getTiersAllSpeakers(
        const QString &annotationID, const QStringList &levelIDs, AnnotationStructure *structure, QSqlDatabase &db)
{
    // Check which levels exist in the database (out of those requested)
    QStringList effectiveLevelIDs;
    if (levelIDs.isEmpty())
        effectiveLevelIDs = structure->levelIDs();
    else {
        foreach (QString levelID, levelIDs) {
            if (structure->hasLevel(levelID)) effectiveLevelIDs << levelID;
        }
    }
    // Collect data
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
            AnnotationTier *tier = getTier(annotationID, speakerID, levelID, QStringList(), structure, db);
            if (tier) tiersSpk->addTier(tier);
        }
    }
    return tiersAll;
}

// ==========================================================================================================================
// Update annotation tiers
// ==========================================================================================================================

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

bool SQLSerialiserAnnotation::saveTier(
        const QString &annotationID, const QString &speakerID, AnnotationTier *tier, AnnotationStructure *structure, QSqlDatabase &db)
{
    AnnotationStructureLevel *level = structure->level(tier->name().replace("-", "_"));
    if (!level) return false;
    db.transaction();
    if (!updateTier(annotationID, speakerID, tier, level, db)) { db.rollback(); return false; }
    db.commit();
    return true;
}

bool SQLSerialiserAnnotation::saveTiers(
        const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers, AnnotationStructure *structure, QSqlDatabase &db)
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

// ==========================================================================================================================
// Delete annotation tiers
// ==========================================================================================================================

bool SQLSerialiserAnnotation::deleteTier(
        const QString &annotationID, const QString &speakerID, const QString &levelID, AnnotationStructure *structure, QSqlDatabase &db)
{
    if (!structure) return false;
    if (!structure->hasLevel(levelID)) return false;
    QSqlQuery query(db);
    QString q = QString("DELETE FROM %1 WHERE annotationID = :annotationID AND speakerID = :speakerID").arg(levelID);
    query.prepare(q);
    query.bindValue(":annotationID", annotationID);
    query.bindValue(":speakerID", speakerID);
    return query.exec();
}

bool SQLSerialiserAnnotation::deleteAllTiersAllSpeakers(const QString &annotationID, AnnotationStructure *structure, QSqlDatabase &db)
{
    if (!structure) return false;
    db.transaction();
    foreach (QString levelID, structure->levelIDs()) {
        QSqlQuery query(db);
        QString q = QString("DELETE FROM %1 WHERE annotationID = :annotationID").arg(levelID);
        query.prepare(q);
        query.bindValue(":annotationID", annotationID);
        if (!query.exec()) { db.rollback(); return false; }
    }
    db.commit();
    return true;
}

// ==========================================================================================================================
// Speakers and Timeline
// ==========================================================================================================================
QList<QString> SQLSerialiserAnnotation::getSpeakersInLevel(
        const QString &annotationID, const QString &levelID, AnnotationStructure *structure, QSqlDatabase &db,
        bool filter)
{
    QList<QString> speakerIDs;
    // Check that the requested level exists in the annotation structure
    if (!structure) return speakerIDs;
    AnnotationStructureLevel *level = structure->level(levelID);
    if (!level) return speakerIDs;
    // Run query
    QSqlQuery query(db);
    QString q = QString("SELECT DISTINCT speakerID FROM %1 WHERE annotationID = :annotationID ").arg(levelID);
    if (filter) q.append(" AND xText IS NOT NULL AND xText <> '_' AND xText <> ''");
    query.setForwardOnly(true);
    query.prepare(q);
    query.bindValue(":annotationID", annotationID);
    query.exec();
    while (query.next()) {
        speakerIDs << query.value(0).toString();
    }
    return speakerIDs;
}

QList<QString> SQLSerialiserAnnotation::getSpeakersActiveInLevel(
        const QString &annotationID, const QString &levelID, AnnotationStructure *structure, QSqlDatabase &db)
{
    return getSpeakersInLevel(annotationID, levelID, structure, db, true);
}

QList<QString> SQLSerialiserAnnotation::getSpeakersInAnnotation(const QString &annotationID, AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<QString> allSpeakerIDs;
    foreach (QString levelID, structure->levelIDs()) {
        foreach (QString speakerID, getSpeakersInLevel(annotationID, levelID, structure, db)) {
            if (!allSpeakerIDs.contains(speakerID)) allSpeakerIDs << speakerID;
        }
    }
    return allSpeakerIDs;
}

QList<QString> SQLSerialiserAnnotation::getSpeakersActiveInAnnotation(
        const QString &annotationID, AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<QString> allSpeakerIDs;
    foreach (QString levelID, structure->levelIDs()) {
        foreach (QString speakerID, getSpeakersActiveInLevel(annotationID, levelID, structure, db)) {
            if (!allSpeakerIDs.contains(speakerID)) allSpeakerIDs << speakerID;
        }
    }
    return allSpeakerIDs;
}

IntervalTier *SQLSerialiserAnnotation::getSpeakerTimeline(
        const QString &communicationID, const QString &annotationID, const QString &levelID, bool detailed,
        AnnotationStructure *structure, QSqlDatabase &db)
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

// ==========================================================================================================================
// Batch Processing
// ==========================================================================================================================
QList<QPair<QList<QVariant>, long> > SQLSerialiserAnnotation::getDistinctLabels(
        const QString &levelID, const QStringList &attributeIDs, AnnotationStructure *structure, QSqlDatabase &db)
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

bool SQLSerialiserAnnotation::batchUpdate(
        const QString &levelID, const QString &attributeID, const QVariant &newValue, const QList<QPair<QString, QVariant> > &criteria,
        AnnotationStructure *structure, QSqlDatabase &db)
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

// ==========================================================================================================================
// Statistics
// ==========================================================================================================================
QList<QPair<QList<QVariant>, long> > SQLSerialiserAnnotation::countItems(
        const QString &levelID, const QStringList &groupByAttributeIDs, AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<QPair<QList<QVariant>, long> > list;
    if (!structure) return list;
    if (!structure->hasLevel(levelID)) return list;
    return list;
}

} // namespace Core
} // namespace Praaline
