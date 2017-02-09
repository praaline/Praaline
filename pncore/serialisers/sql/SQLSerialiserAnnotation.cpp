#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "datastore/CorpusRepository.h"
#include "annotation/AnnotationElement.h"
#include "annotation/Point.h"
#include "annotation/Interval.h"
#include "annotation/Sequence.h"
#include "annotation/Relation.h"
#include "annotation/PointTier.h"
#include "annotation/IntervalTier.h"
#include "annotation/SequenceTier.h"
#include "annotation/RelationTier.h"
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
    if (selection.annotationID.isEmpty()) {
        if (level->isLevelTypePrimary())    q.append(" ORDER BY annotationID, speakerID, tMin ");
        else                                q.append(" ORDER BY annotationID, speakerID, intervalNoLeft ");
    } else {
        if (level->isLevelTypePrimary())    q.append(" ORDER BY tMin, speakerID ");
        else                                q.append(" ORDER BY intervalNoLeft, speakerID ");
    }
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

bool prepareInsertQuery(QSqlQuery &query, AnnotationStructureLevel *level, const QStringList &effectiveAttributeIDs)
{
    if (!level) return false;
    QString q1, q2;
    q1 = QString("INSERT INTO %1 (annotationID, speakerID ").arg(level->ID());
    q2 = "VALUES (:annotationID, :speakerID ";
    switch (level->levelType()) {
    case AnnotationStructureLevel::IndependentPointsLevel:
    case AnnotationStructureLevel::IndependentIntervalsLevel:
    case AnnotationStructureLevel::GroupingLevel:
        q1.append(", intervalNo, tMin, tMax, xText");
        q2.append(", :intervalNo, :tMin, :tMax, :xText");
        break;
    case AnnotationStructureLevel::SequencesLevel:
    case AnnotationStructureLevel::TreeLevel:
    case AnnotationStructureLevel::RelationsLevel:
        q1.append(", intervalNoLeft, intervalNoRight, xText");
        q2.append(", :intervalNoLeft, :intervalNoRight, :xText");
        break;
    }
    // User-defined attributes
    foreach (QString attributeID, effectiveAttributeIDs) {
        q1.append(", ").append(attributeID);
        q2.append(", :").append(attributeID);
    }
    q1.append(") "); q2.append(") ");
    query.prepare(q1 + q2);
    // qDebug() << q1 + q2;
    return true;
}

bool prepareUpdateQuery(QSqlQuery &query, AnnotationStructureLevel *level, const QStringList &effectiveAttributeIDs)
{
    if (!level) return false;
    QString q;
    q = QString("UPDATE %1 SET xText = :xText").arg(level->ID());
    // Update timecodes if need be
    if ((level->levelType() == AnnotationStructureLevel::IndependentPointsLevel) ||
        (level->levelType() == AnnotationStructureLevel::IndependentIntervalsLevel) ||
        (level->levelType() == AnnotationStructureLevel::GroupingLevel)) {
        q.append(", tMin = :tMin, tMax = :tMax");
    }
    // User-defined attributes
    foreach (QString attributeID, effectiveAttributeIDs) {
        q.append(", ").append(attributeID);
        q.append(" = :").append(attributeID);
    }
    q.append(" WHERE annotationID = :annotationID AND speakerID = :speakerID ");
    switch (level->levelType()) {
    case AnnotationStructureLevel::IndependentPointsLevel:
    case AnnotationStructureLevel::IndependentIntervalsLevel:
    case AnnotationStructureLevel::GroupingLevel:
        q.append("AND intervalNo = :intervalNo");
        break;
    case AnnotationStructureLevel::SequencesLevel:
    case AnnotationStructureLevel::TreeLevel:
    case AnnotationStructureLevel::RelationsLevel:
        q.append("AND intervalNoLeft = :intervalNoLeft, intervalNoRight = :intervalNoRight");
        break;
    }
    query.prepare(q);
    // qDebug() << q;
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
        if (query.value("intervalNo").isValid())        attributes.insert("indexNo", query.value("intervalNo"));
        if (query.value("intervalNoLeft").isValid())    attributes.insert("indexFrom", query.value("intervalNoLeft"));
        if (query.value("intervalNoRight").isValid())   attributes.insert("indexTo", query.value("intervalNoRight"));
        if (query.value("tMin").isValid())              attributes.insert("tMin_nsec", query.value("tMin"));
        if (query.value("tMax").isValid())              attributes.insert("tMax_nsec", query.value("tMax"));
        AnnotationElement *element = new AnnotationElement(xText, attributes);
        setClean(element);
        elements << element;
    }
    return elements;
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
        Point *point = new Point(time, xText, attributes);
        setClean(point);
        points << point;
    }
    return points;
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
        Interval *interval = new Interval(tMin, tMax, xText, attributes);
        setClean(interval);
        intervals << interval;
    }
    return intervals;
}

QList<Sequence *> SQLSerialiserAnnotation::getSequences(
        const AnnotationDatastore::Selection &selection, AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<Sequence *> sequences;
    // Check that the requested level exists in the annotation structure
    AnnotationStructureLevel *level = structure->level(selection.levelID);
    if (!level) return sequences;
    // Derive the list of attributes that will be requested from the database
    QStringList effectiveAttributeIDs = getEffectiveAttributeIDs(level, selection.attributeIDs);
    // Run query
    QSqlQuery query(db);
    prepareSelectQuery(query, level, effectiveAttributeIDs, selection);
    query.exec();
    // Read annotation elements
    while (query.next()) {
        int indexFrom = query.value("intervalNoLeft").toInt();
        int indexTo = query.value("intervalNoRight").toInt();
        QString xText = query.value("xText").toString();
        QHash<QString, QVariant> attributes;
        foreach (QString attributeID, effectiveAttributeIDs) {
            attributes.insert(attributeID, query.value(attributeID));
        }
        if (selection.annotationID.isEmpty()) attributes.insert("annotationID", query.value("annotationID"));
        if (selection.speakerID.isEmpty())    attributes.insert("speakerID", query.value("speakerID"));
        Sequence *sequence = new Sequence(indexFrom, indexTo, xText, attributes);
        setClean(sequence);
        sequences << sequence;
    }
    return sequences;
}

bool SQLSerialiserAnnotation::saveAnnotationElements(
        const QList<AnnotationElement *> &elements, const QString &levelID, const QStringList &attributeIDs,
        AnnotationStructure *structure, QSqlDatabase &db)
{
    if (!structure) return false;
    AnnotationStructureLevel *level = structure->level(levelID);
    if (!level) return false;
    // Derive the list of attributes that will be requested from the database
    QStringList effectiveAttributeIDs = getEffectiveAttributeIDs(level, attributeIDs);
    QSqlQuery queryInsert(db), queryUpdate(db);
    prepareInsertQuery(queryInsert, level, effectiveAttributeIDs);
    prepareUpdateQuery(queryUpdate, level, effectiveAttributeIDs);
    bool success = true;
    foreach (AnnotationElement *element, elements) {
        if (element->isClean()) continue;
        QSqlQuery &query(queryUpdate);
        if (element->isNew()) query = queryInsert;
        // Bind parameters
        query.bindValue(":annotationID", element->attribute("annotationID"));
        query.bindValue(":speakerID", element->attribute("speakerID"));
        switch (level->levelType()) {
        case AnnotationStructureLevel::IndependentPointsLevel:
        case AnnotationStructureLevel::IndependentIntervalsLevel:
        case AnnotationStructureLevel::GroupingLevel:
            query.bindValue(":intervalNo", element->attribute("indexNo"));
            query.bindValue(":tMin", element->attribute("tMin_nsec"));
            query.bindValue(":tMax", element->attribute("tMax_nsec"));
            break;
        case AnnotationStructureLevel::SequencesLevel:
        case AnnotationStructureLevel::TreeLevel:
        case AnnotationStructureLevel::RelationsLevel:
            query.bindValue(":intervalNoLeft", element->attribute("indexFrom"));
            query.bindValue(":intervalNoRight", element->attribute("indexTo"));
            break;
        }
        query.bindValue(":xText", element->attribute("text"));
        foreach (QString attributeID, effectiveAttributeIDs) {
            query.bindValue(QString(":%1").arg(attributeID), element->attribute(attributeID));
        }
        if (query.exec()) {
            setClean(element);
        } else {
            qDebug() << query.lastError();
            success = false;
        }
    }
    return success;
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
    AnnotationDatastore::Selection selection(annotationID, speakerID, levelID, attributeIDs);
    if (level->levelType() == AnnotationStructureLevel::IndependentPointsLevel) {
        QList<Point *> points = getPoints(selection, structure, db);
        if (!points.isEmpty()) return new PointTier(levelID, points);
    }
    else if ((level->levelType() == AnnotationStructureLevel::IndependentIntervalsLevel) ||
             (level->levelType() == AnnotationStructureLevel::GroupingLevel)) {
        QList<Interval *> intervals = getIntervals(selection, structure, db);
        if (!intervals.isEmpty()) return new IntervalTier(levelID, intervals);
    }
    else if ((level->levelType() == AnnotationStructureLevel::SequencesLevel)) {
        QList<Sequence *> sequences = getSequences(selection, structure, db);
        if (!sequences.isEmpty()) return new SequenceTier(levelID, sequences);
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
    if (!level) return false;

    QSqlQuery query(db);
    QString q;

    // Prepare and execute delete query that removes all annotation elements
    q = QString("DELETE FROM %1 WHERE annotationID=:annotationID AND speakerID=:speakerID").arg(level->ID());
    query.prepare(q);
    query.bindValue(":annotationID", annotationID);
    query.bindValue(":speakerID", speakerID);
    if (!query.exec()) { qDebug() << query.lastError(); return false; }

    // Prepare, bind and execute insert query to re-insert all annotation elements
    if (!prepareInsertQuery(query, level, level->attributeIDs())) return false;
    query.bindValue(":annotationID", annotationID);
    query.bindValue(":speakerID", speakerID);
    int itemNo = 1;
    for (int i = 0; i < tier->count(); ++i) {
        AnnotationElement *element = tier->at(i);
        switch (level->levelType()) {
        case AnnotationStructureLevel::IndependentPointsLevel:
            query.bindValue(":intervalNo", itemNo);
            query.bindValue(":tMin", element->attribute("timeNanoseconds"));
            query.bindValue(":tMax", element->attribute("timeNanoseconds"));
            break;
        case AnnotationStructureLevel::IndependentIntervalsLevel:
        case AnnotationStructureLevel::GroupingLevel:
            query.bindValue(":intervalNo", itemNo);
            query.bindValue(":tMin", element->attribute("tMinNanoseconds"));
            query.bindValue(":tMax", element->attribute("tMaxNanoseconds"));
            break;
        case AnnotationStructureLevel::SequencesLevel:
        case AnnotationStructureLevel::TreeLevel:
        case AnnotationStructureLevel::RelationsLevel:
            query.bindValue(":intervalNoLeft", element->attribute("indexFrom").toInt());
            query.bindValue(":intervalNoRight", element->attribute("indexTo").toInt());
            break;
        }
        query.bindValue(":xText", element->text());
        foreach (QString attributeID, level->attributeIDs()) {
            query.bindValue(QString(":%1").arg(attributeID), element->attribute(attributeID));
        }
        if (!query.exec()) {
            qDebug() << "Error in updateTier: " << annotationID << speakerID << tier->name() << itemNo << query.lastError();
            return false;
        }
        ++itemNo;
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
QList<QPair<QList<QVariant>, long long> > SQLSerialiserAnnotation::countItems(
        const QString &levelID, const QStringList &groupByAttributeIDs, AnnotationStructure *structure, QSqlDatabase &db)
{
    QList<QPair<QList<QVariant>, long long> > list;
    AnnotationStructureLevel *level = structure->level(levelID);
    if (!level) return list;
    QStringList groupfields;
    if (groupByAttributeIDs.contains("annotationID")) groupfields << "annotationID";
    if (groupByAttributeIDs.contains("speakerID")) groupfields << "speakerID";
    groupfields << getEffectiveAttributeIDs(level, groupByAttributeIDs);

    QString q1 = "SELECT ";
    QString q2 = "GROUP BY ";
    foreach (QString groupfield, groupfields) {
        q1.append(groupfield).append(", ");
        q2.append(groupfield).append(", ");
    }
    q1.append(QString("COUNT(*) AS praaline_count FROM %1 ").arg(levelID));
    if (q2.endsWith(", ")) q2.chop(2);

    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare(q1 + q2);

    q.exec();
    while (q.next()) {
        QList<QVariant> groupvalues;
        foreach (QString groupfield, groupfields) {
            groupvalues << q.value(groupfield);
        }
        long long count = q.value("praaline_count").toLongLong();
        QPair<QList<QVariant>, long long> item(groupvalues, count);
        list << item;
    }

    return list;
}

} // namespace Core
} // namespace Praaline
