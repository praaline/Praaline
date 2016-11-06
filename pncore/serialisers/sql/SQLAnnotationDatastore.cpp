#include <QDebug>
#include <QSqlError>
#include "SQLAnnotationDatastore.h"
#include "SQLSerialiserAnnotation.h"
#include "SQLSerialiserAnnotationStructure.h"
#include "SQLSchemaProxyAnnotation.h"
#include "SQLQueryEngineAnnotation.h"

namespace Praaline {
namespace Core {

SQLAnnotationDatastore::SQLAnnotationDatastore(QPointer<AnnotationStructure> structure, QObject *parent) :
    AbstractAnnotationDatastore(parent), m_structure(structure)
{
}

SQLAnnotationDatastore::~SQLAnnotationDatastore()
{
    closeDatastore();
}

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
    SQLSchemaProxyAnnotation::createSchema(m_structure, m_database);
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

// ------------------------------------------------------------------------------------------------------------------------------

bool SQLAnnotationDatastore::loadAnnotationStructure()
{
    return SQLSerialiserAnnotationStructure::read(m_structure, m_database);
}

bool SQLAnnotationDatastore::saveAnnotationStructure()
{
    return SQLSerialiserAnnotationStructure::write(m_structure, m_database);
}

bool SQLAnnotationDatastore::createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel)
{
    return SQLSchemaProxyAnnotation::createAnnotationLevel(newLevel, m_database);
}

bool SQLAnnotationDatastore::renameAnnotationLevel(QString levelID, QString newLevelID)
{
    return SQLSchemaProxyAnnotation::renameAnnotationLevel(levelID, newLevelID, m_database);
}

bool SQLAnnotationDatastore::deleteAnnotationLevel(QString levelID)
{
    return SQLSchemaProxyAnnotation::deleteAnnotationLevel(levelID, m_database);
}

bool SQLAnnotationDatastore::createAnnotationAttribute(QString levelID, QPointer<AnnotationStructureAttribute> newAttribute)
{
    return SQLSchemaProxyAnnotation::createAnnotationAttribute(levelID, newAttribute, m_database);
}

bool SQLAnnotationDatastore::renameAnnotationAttribute(QString levelID, QString attributeID, QString newAttributeID)
{
    return SQLSchemaProxyAnnotation::renameAnnotationAttribute(levelID, attributeID, newAttributeID, m_database);
}

bool SQLAnnotationDatastore::deleteAnnotationAttribute(QString levelID, QString attributeID)
{
    return SQLSchemaProxyAnnotation::deleteAnnotationAttribute(levelID, attributeID, m_database);
}

bool SQLAnnotationDatastore::retypeAnnotationAttribute(QString levelID, QString attributeID, QString newDatatype, int newDatalength)
{
    return SQLSchemaProxyAnnotation::retypeAnnotationAttribute(levelID, attributeID, newDatatype, newDatalength, m_database);
}

// ------------------------------------------------------------------------------------------------------------------------------

bool SQLAnnotationDatastore::saveTiersAllSpeakers(const QString &annotationID, QMap<QString, QPointer<AnnotationTierGroup> > &tiersAllSpeakers)
{
    foreach (QString speakerID, tiersAllSpeakers.keys()) {
        saveTiers(annotationID, speakerID, tiersAllSpeakers.value(speakerID));
    }
    return true;
}

bool SQLAnnotationDatastore::saveTiers(const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers)
{
    return SQLSerialiserAnnotation::saveTiers(annotationID, speakerID, tiers, m_structure, m_database);
}

bool SQLAnnotationDatastore::saveTier(const QString &annotationID, const QString &speakerID, AnnotationTier *tier)
{
    return SQLSerialiserAnnotation::saveTier(annotationID, speakerID, tier, m_structure, m_database);
}

AnnotationTierGroup *SQLAnnotationDatastore::getTiers(const QString &annotationID, const QString &speakerID,
                                                      const QStringList &levelIDs)
{
    return SQLSerialiserAnnotation::getTiers(annotationID, speakerID, m_structure, m_database, levelIDs);
}

QMap<QString, QPointer<AnnotationTierGroup> > SQLAnnotationDatastore::getTiersAllSpeakers(const QString &annotationID, const QStringList &levelIDs)
{
    return SQLSerialiserAnnotation::getTiersAllSpeakers(annotationID, m_structure, m_database, levelIDs);
}

AnnotationTier *SQLAnnotationDatastore::getTier(const QString &annotationID, const QString &speakerID,
                                                const QString &levelID, const QStringList &attributeIDs)
{
    return SQLSerialiserAnnotation::getTier(annotationID, speakerID, m_structure, m_database, levelID, attributeIDs);
}

QList<Interval *> SQLAnnotationDatastore::getIntervals(const QString &annotationID, const QString &speakerID,
                                                       const QString &levelID, int intervalNoMin, int intervalNoMax,
                                                       const QStringList &attributeIDs)
{
    return SQLSerialiserAnnotation::getIntervals(annotationID, speakerID, m_structure, m_database,
                                                 levelID, intervalNoMin, intervalNoMax, attributeIDs);
}

QList<Interval *> SQLAnnotationDatastore::getIntervals(const QString &annotationID, const QString &speakerID,
                                                       const QString &levelID, RealTime tMin, RealTime tMax,
                                                       const QStringList &attributeIDs)
{
    return SQLSerialiserAnnotation::getIntervals(annotationID, speakerID, m_structure, m_database,
                                                 levelID, tMin, tMax, attributeIDs);
}

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

bool SQLAnnotationDatastore::deleteAllForAnnotationID(QString annotationID)
{
    return SQLSerialiserAnnotation::deleteAllForAnnotationID(annotationID, m_structure, m_database);
}

// Queries

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

// Batch editing

QList<QPair<QList<QVariant>, long> > SQLAnnotationDatastore::getDistinctLabels(const QString &levelID, const QStringList &attributeIDs)
{
    return SQLSerialiserAnnotation::getDistinctLabels(levelID, attributeIDs, m_structure, m_database);
}

bool SQLAnnotationDatastore::batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                                         const QList<QPair<QString, QVariant> > &criteria)
{
    return SQLSerialiserAnnotation::batchUpdate(levelID, attributeID, newValue, criteria, m_structure, m_database);
}

// Counting
QList<QPair<QList<QVariant>, long> > SQLAnnotationDatastore::countItems(const QString &levelID, const QStringList &groupByAttributeIDs)
{
    return SQLSerialiserAnnotation::countItems(levelID, groupByAttributeIDs, m_structure, m_database);
}

} // namespace Core
} // namespace Praaline
