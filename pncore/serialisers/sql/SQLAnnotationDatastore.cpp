#include <QDebug>
#include <QSqlError>
#include "SQLAnnotationDatastore.h"
#include "SQLSerialiserAnnotation.h"
#include "SQLSerialiserAnnotationStructure.h"
#include "SQLQueryEngineAnnotation.h"

namespace Praaline {
namespace Core {

SQLAnnotationDatastore::SQLAnnotationDatastore(QPointer<AnnotationStructure> structure, QObject *parent) :
    AnnotationDatastore(parent), m_structure(structure)
{
}

SQLAnnotationDatastore::~SQLAnnotationDatastore()
{
    closeDatastore();
}

// ==========================================================================================================================
// Datastore
// ==========================================================================================================================

bool SQLAnnotationDatastore::createDatastore(const DatastoreInfo &info)
{
    if (m_database.isOpen()) m_database.close();
    m_database = QSqlDatabase::addDatabase(info.driver, info.datasource + "_annotation");
    m_database.setHostName(info.hostname);
    m_database.setDatabaseName(info.datasource);
    m_database.setUserName(info.username);
    m_database.setPassword(info.password);
    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        qDebug() << m_lastError;
        return false;
    }
    // Initialise database
    SQLSerialiserAnnotationStructure::initialiseAnnotationStructureTables(m_database);
    return true;
}

bool SQLAnnotationDatastore::openDatastore(const DatastoreInfo &info)
{
    if (m_database.isOpen()) m_database.close();
    m_database = QSqlDatabase::addDatabase(info.driver, info.datasource + "_annotation");
    m_database.setHostName(info.hostname);
    m_database.setDatabaseName(info.datasource);
    m_database.setUserName(info.username);
    m_database.setPassword(info.password);
    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        qDebug() << m_lastError;
        return false;
    }
    return true;
}

bool SQLAnnotationDatastore::closeDatastore()
{
    if (m_database.isOpen())
        m_database.close();
    return true;
}

// ==========================================================================================================================
// Annotation Structure
// ==========================================================================================================================

bool SQLAnnotationDatastore::loadAnnotationStructure()
{
    return SQLSerialiserAnnotationStructure::loadAnnotationStructure(m_structure, m_database);
}

bool SQLAnnotationDatastore::saveAnnotationStructure()
{
    return true;
}

bool SQLAnnotationDatastore::createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel)
{
    return SQLSerialiserAnnotationStructure::createAnnotationLevel(newLevel, m_database);
}

bool SQLAnnotationDatastore::renameAnnotationLevel(const QString &levelID, const QString &newLevelID)
{
    return SQLSerialiserAnnotationStructure::renameAnnotationLevel(levelID, newLevelID, m_database);
}

bool SQLAnnotationDatastore::deleteAnnotationLevel(const QString &levelID)
{
    return SQLSerialiserAnnotationStructure::deleteAnnotationLevel(levelID, m_database);
}

bool SQLAnnotationDatastore::createAnnotationAttribute(const QString &levelID, QPointer<AnnotationStructureAttribute> newAttribute)
{
    return SQLSerialiserAnnotationStructure::createAnnotationAttribute(levelID, newAttribute, m_database);
}

bool SQLAnnotationDatastore::renameAnnotationAttribute(const QString &levelID, const QString &attributeID, const QString &newAttributeID)
{
    return SQLSerialiserAnnotationStructure::renameAnnotationAttribute(levelID, attributeID, newAttributeID, m_database);
}

bool SQLAnnotationDatastore::deleteAnnotationAttribute(const QString &levelID, const QString &attributeID)
{
    return SQLSerialiserAnnotationStructure::deleteAnnotationAttribute(levelID, attributeID, m_database);
}

bool SQLAnnotationDatastore::retypeAnnotationAttribute(const QString &levelID, const QString &attributeID, const DataType &newDatatype)
{
    return SQLSerialiserAnnotationStructure::retypeAnnotationAttribute(levelID, attributeID, newDatatype, m_database);
}

// ==========================================================================================================================
// Annotation Tiers
// ==========================================================================================================================

AnnotationTier *SQLAnnotationDatastore::getTier(const QString &annotationID, const QString &speakerID,
                                                const QString &levelID, const QStringList &attributeIDs)
{
    return SQLSerialiserAnnotation::getTier(annotationID, speakerID, levelID, attributeIDs, m_structure, m_database);
}

AnnotationTierGroup *SQLAnnotationDatastore::getTiers(const QString &annotationID, const QString &speakerID,
                                                      const QStringList &levelIDs)
{
    return SQLSerialiserAnnotation::getTiers(annotationID, speakerID, levelIDs, m_structure, m_database);
}

QMap<QString, QPointer<AnnotationTierGroup> > SQLAnnotationDatastore::getTiersAllSpeakers(const QString &annotationID, const QStringList &levelIDs)
{
    return SQLSerialiserAnnotation::getTiersAllSpeakers(annotationID, levelIDs, m_structure, m_database);
}

bool SQLAnnotationDatastore::saveTier(const QString &annotationID, const QString &speakerID, AnnotationTier *tier)
{
    return SQLSerialiserAnnotation::saveTier(annotationID, speakerID, tier, m_structure, m_database);
}

bool SQLAnnotationDatastore::saveTiers(const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers)
{
    return SQLSerialiserAnnotation::saveTiers(annotationID, speakerID, tiers, m_structure, m_database);
}

bool SQLAnnotationDatastore::saveTiersAllSpeakers(const QString &annotationID, QMap<QString, QPointer<AnnotationTierGroup> > &tiersAllSpeakers)
{
    bool success = true;
    foreach (QString speakerID, tiersAllSpeakers.keys()) {
        success = success && saveTiers(annotationID, speakerID, tiersAllSpeakers.value(speakerID));
    }
    return success;
}

bool SQLAnnotationDatastore::deleteTier(const QString &annotationID, const QString &speakerID, const QString &levelID)
{
    return SQLSerialiserAnnotation::deleteTier(annotationID, speakerID, levelID, m_structure, m_database);
}

bool SQLAnnotationDatastore::deleteTiers(const QString &annotationID, const QString &speakerID, const QStringList &levelIDs)
{
    bool success = true;
    foreach (QString levelID, levelIDs) {
        success = success && deleteTier(annotationID, speakerID, levelID);
    }
    return success;
}

bool SQLAnnotationDatastore::deleteAllTiersAllSpeakers(const QString &annotationID)
{
    return SQLSerialiserAnnotation::deleteAllTiersAllSpeakers(annotationID, m_structure, m_database);
}

// ==========================================================================================================================
// Annotation Elements
// ==========================================================================================================================
QList<AnnotationElement *> SQLAnnotationDatastore::getAnnotationElements(const Selection &selection)
{
    return SQLSerialiserAnnotation::getAnnotationElements(selection, m_structure, m_database);
}

QList<Interval *> SQLAnnotationDatastore::getIntervals(const Selection &selection)
{
    return SQLSerialiserAnnotation::getIntervals(selection, m_structure, m_database);
}

QList<Point *> SQLAnnotationDatastore::getPoints(const Selection &selection)
{
    return SQLSerialiserAnnotation::getPoints(selection, m_structure, m_database);
}

// ==========================================================================================================================
// Speakers and Timeline
// ==========================================================================================================================

QList<QString> SQLAnnotationDatastore::getSpeakersInLevel(const QString &annotationID, const QString &levelID)
{
    return SQLSerialiserAnnotation::getSpeakersInLevel(annotationID, levelID, m_structure, m_database);
}

QList<QString> SQLAnnotationDatastore::getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID)
{
    return SQLSerialiserAnnotation::getSpeakersActiveInLevel(annotationID, levelID, m_structure, m_database);
}

QList<QString> SQLAnnotationDatastore::getSpeakersInAnnotation(const QString &annotationID)
{
    return SQLSerialiserAnnotation::getSpeakersInAnnotation(annotationID, m_structure, m_database);
}

QList<QString> SQLAnnotationDatastore::getSpeakersActiveInAnnotation(const QString &annotationID)
{
    return SQLSerialiserAnnotation::getSpeakersActiveInAnnotation(annotationID, m_structure, m_database);
}

IntervalTier *SQLAnnotationDatastore::getSpeakerTimeline(const QString &communicationID,const QString &annotationID,
                                                         const QString &levelID, bool detailed)
{
    return SQLSerialiserAnnotation::getSpeakerTimeline(communicationID, annotationID, levelID, detailed, m_structure, m_database);
}

// ==========================================================================================================================
// Queries
// ==========================================================================================================================

QList<QueryOccurrencePointer *> SQLAnnotationDatastore::runQuery(QueryDefinition *qdef)
{
    return SQLQueryEngineAnnotation::runQuery(qdef, m_structure, m_database);
}

QueryOccurrence *SQLAnnotationDatastore::getOccurrence(QueryOccurrencePointer *pointer, QueryDefinition *qdef)
{
    return SQLQueryEngineAnnotation::getOccurrence(pointer, qdef, m_structure, m_database);
}

bool SQLAnnotationDatastore::updateAnnotationsFromQueryOccurrences(const QList<QueryOccurrence *> &occurrences)
{
    return SQLQueryEngineAnnotation::updateAnnotationsFromQueryOccurrences(occurrences, m_structure, m_database);
}

// ==========================================================================================================================
// Batch Processing
// ==========================================================================================================================

QList<QPair<QList<QVariant>, long> > SQLAnnotationDatastore::getDistinctLabels(const QString &levelID, const QStringList &attributeIDs)
{
    return SQLSerialiserAnnotation::getDistinctLabels(levelID, attributeIDs, m_structure, m_database);
}

bool SQLAnnotationDatastore::batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                                         const QList<QPair<QString, QVariant> > &criteria)
{
    return SQLSerialiserAnnotation::batchUpdate(levelID, attributeID, newValue, criteria, m_structure, m_database);
}

// ==========================================================================================================================
// Statistics
// ==========================================================================================================================

QList<QPair<QList<QVariant>, long> > SQLAnnotationDatastore::countItems(const QString &levelID, const QStringList &groupByAttributeIDs)
{
    return SQLSerialiserAnnotation::countItems(levelID, groupByAttributeIDs, m_structure, m_database);
}

} // namespace Core
} // namespace Praaline
