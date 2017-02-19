#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "datastore/CorpusRepository.h"
#include "datastore/DatastoreInfo.h"
#include "SQLAnnotationDatastore.h"
#include "SQLSerialiserAnnotation.h"
#include "SQLSerialiserAnnotationStructure.h"
#include "SQLSerialiserNameValueList.h"
#include "SQLQueryEngineAnnotation.h"

namespace Praaline {
namespace Core {

struct SQLAnnotationDatastoreData {
    QSqlDatabase database;
    QPointer<AnnotationStructure> structure;
    QPointer<CorpusRepository> repository;
};

SQLAnnotationDatastore::SQLAnnotationDatastore(AnnotationStructure *structure, CorpusRepository *repository, QObject *parent) :
    AnnotationDatastore(repository, parent), d(new SQLAnnotationDatastoreData)
{
    d->structure = structure;
    d->repository = repository;
}

SQLAnnotationDatastore::~SQLAnnotationDatastore()
{
    closeDatastore();
    delete d;
}

// ==========================================================================================================================
// Datastore
// ==========================================================================================================================

bool SQLAnnotationDatastore::createDatastore(const DatastoreInfo &info)
{
    if (d->database.isOpen()) d->database.close();
    d->database = QSqlDatabase::addDatabase(info.driver, info.datasource + "_annotation");
    d->database.setHostName(info.hostname);
    d->database.setUserName(info.username);
    d->database.setPassword(info.password);
    if (info.driver == "QSQLITE") {
        d->database.setDatabaseName(info.datasource);
    }
    // Test if the connection is valid
    if (!d->database.isValid()){
        d->repository->setLastError(d->database.lastError().text());
        qDebug() << d->database.lastError().text();
        return false;
    }
    // Test if we can open the database
    if (!d->database.open()) {
        d->repository->setLastError(d->database.lastError().text());
        qDebug() << d->database.lastError().text();
        return false;
    }
    // Try creating database
    if (info.driver != "QSQLITE") {
        d->database.exec(QString("CREATE DATABASE IF NOT EXISTS %1").arg(info.datasource));
        if (d->database.lastError().isValid()) {
            d->repository->setLastError(d->database.lastError().text());
            qDebug() << d->database.lastError().text();
            return false;
        }
        d->database.setDatabaseName(info.datasource);
    }
    // Initialise database
    SQLSerialiserAnnotationStructure::initialiseAnnotationStructureTables(d->database);
    SQLSerialiserNameValueList::initialiseNameValueListSchema(d->database);
    return true;
}

bool SQLAnnotationDatastore::openDatastore(const DatastoreInfo &info)
{
    if (d->database.isOpen()) d->database.close();
    d->database = QSqlDatabase::addDatabase(info.driver, info.datasource + "_annotation");
    d->database.setHostName(info.hostname);
    d->database.setDatabaseName(info.datasource);
    d->database.setUserName(info.username);
    d->database.setPassword(info.password);
    if (!d->database.open()) {
        d->repository->setLastError(d->database.lastError().text());
        qDebug() << d->database.lastError().text();
        return false;
    }
    // Upgrade database if needed
    SQLSerialiserBase::upgradeSchema(d->database);
    SQLSerialiserNameValueList::upgradeNameValueListSchema(d->database);
    return true;
}

bool SQLAnnotationDatastore::closeDatastore()
{
    if (d->database.isOpen())
        d->database.close();
    return true;
}

// ==========================================================================================================================
// Annotation Structure
// ==========================================================================================================================

bool SQLAnnotationDatastore::loadAnnotationStructure()
{
    return SQLSerialiserAnnotationStructure::loadAnnotationStructure(d->structure, d->database);
}

bool SQLAnnotationDatastore::saveAnnotationStructure()
{
    return SQLSerialiserAnnotationStructure::saveAnnotationStructure(d->structure, d->database);
}

bool SQLAnnotationDatastore::createAnnotationLevel(AnnotationStructureLevel *newLevel)
{
    return SQLSerialiserAnnotationStructure::createAnnotationLevel(newLevel, d->database);
}

bool SQLAnnotationDatastore::renameAnnotationLevel(const QString &levelID, const QString &newLevelID)
{
    return SQLSerialiserAnnotationStructure::renameAnnotationLevel(levelID, newLevelID, d->database);
}

bool SQLAnnotationDatastore::deleteAnnotationLevel(const QString &levelID)
{
    return SQLSerialiserAnnotationStructure::deleteAnnotationLevel(levelID, d->database);
}

bool SQLAnnotationDatastore::createAnnotationAttribute(const QString &levelID, AnnotationStructureAttribute *newAttribute)
{
    return SQLSerialiserAnnotationStructure::createAnnotationAttribute(levelID, newAttribute, d->database);
}

bool SQLAnnotationDatastore::renameAnnotationAttribute(const QString &levelID, const QString &attributeID, const QString &newAttributeID)
{
    return SQLSerialiserAnnotationStructure::renameAnnotationAttribute(levelID, attributeID, newAttributeID, d->database);
}

bool SQLAnnotationDatastore::deleteAnnotationAttribute(const QString &levelID, const QString &attributeID)
{
    return SQLSerialiserAnnotationStructure::deleteAnnotationAttribute(levelID, attributeID, d->database);
}

bool SQLAnnotationDatastore::retypeAnnotationAttribute(const QString &levelID, const QString &attributeID, const DataType &newDatatype)
{
    return SQLSerialiserAnnotationStructure::retypeAnnotationAttribute(levelID, attributeID, newDatatype, d->database);
}

// ==========================================================================================================================
// Name-value lists
// ==========================================================================================================================

NameValueList *SQLAnnotationDatastore::getNameValueList(const QString &listID)
{
    return SQLSerialiserNameValueList::getNameValueList(listID, SQLSerialiserNameValueList::Annotation, d->database);
}

QStringList SQLAnnotationDatastore::getAllNameValueListIDs()
{
    return SQLSerialiserNameValueList::getAllNameValueListIDs(SQLSerialiserNameValueList::Annotation, d->database);
}

QMap<QString, QPointer<NameValueList> > SQLAnnotationDatastore::getAllNameValueLists()
{
    return SQLSerialiserNameValueList::getAllNameValueLists(SQLSerialiserNameValueList::Annotation, d->database);
}

bool SQLAnnotationDatastore::createNameValueList(NameValueList *list)
{
    return SQLSerialiserNameValueList::createNameValueList(list, SQLSerialiserNameValueList::Annotation, d->database);
}

bool SQLAnnotationDatastore::updateNameValueList(NameValueList *list)
{
    return SQLSerialiserNameValueList::updateNameValueList(list, SQLSerialiserNameValueList::Annotation, d->database);
}

bool SQLAnnotationDatastore::deleteNameValueList(const QString &listID)
{
    return SQLSerialiserNameValueList::deleteNameValueList(listID, SQLSerialiserNameValueList::Annotation, d->database);
}

// ==========================================================================================================================
// Annotation Elements
// ==========================================================================================================================
QList<AnnotationElement *> SQLAnnotationDatastore::getAnnotationElements(const Selection &selection)
{
    return SQLSerialiserAnnotation::getAnnotationElements(selection, d->structure, d->database);
}

QList<Point *> SQLAnnotationDatastore::getPoints(const Selection &selection)
{
    return SQLSerialiserAnnotation::getPoints(selection, d->structure, d->database);
}

QList<Interval *> SQLAnnotationDatastore::getIntervals(const Selection &selection)
{
    return SQLSerialiserAnnotation::getIntervals(selection, d->structure, d->database);
}

QList<Sequence *> SQLAnnotationDatastore::getSequences(const Selection &selection)
{
    return SQLSerialiserAnnotation::getSequences(selection, d->structure, d->database);
}

bool SQLAnnotationDatastore::saveAnnotationElements(
        const QList<AnnotationElement *> &elements, const QString &levelID, const QStringList &attributeIDs)
{
    return SQLSerialiserAnnotation::saveAnnotationElements(elements, levelID, attributeIDs, d->structure, d->database);
}

// ==========================================================================================================================
// Annotation Tiers
// ==========================================================================================================================

AnnotationTier *SQLAnnotationDatastore::getTier(const QString &annotationID, const QString &speakerID,
                                                const QString &levelID, const QStringList &attributeIDs)
{
    return SQLSerialiserAnnotation::getTier(annotationID, speakerID, levelID, attributeIDs, d->structure, d->database);
}

AnnotationTierGroup *SQLAnnotationDatastore::getTiers(const QString &annotationID, const QString &speakerID,
                                                      const QStringList &levelIDs)
{
    return SQLSerialiserAnnotation::getTiers(annotationID, speakerID, levelIDs, d->structure, d->database);
}

QMap<QString, QPointer<AnnotationTierGroup> > SQLAnnotationDatastore::getTiersAllSpeakers(const QString &annotationID, const QStringList &levelIDs)
{
    return SQLSerialiserAnnotation::getTiersAllSpeakers(annotationID, levelIDs, d->structure, d->database);
}

bool SQLAnnotationDatastore::saveTier(const QString &annotationID, const QString &speakerID, AnnotationTier *tier)
{
    return SQLSerialiserAnnotation::saveTier(annotationID, speakerID, tier, d->structure, d->database);
}

bool SQLAnnotationDatastore::saveTiers(const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers)
{
    return SQLSerialiserAnnotation::saveTiers(annotationID, speakerID, tiers, d->structure, d->database);
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
    return SQLSerialiserAnnotation::deleteTier(annotationID, speakerID, levelID, d->structure, d->database);
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
    return SQLSerialiserAnnotation::deleteAllTiersAllSpeakers(annotationID, d->structure, d->database);
}

// ==========================================================================================================================
// Speakers and Timeline
// ==========================================================================================================================

QStringList SQLAnnotationDatastore::getSpeakersInLevel(const QString &annotationID, const QString &levelID)
{
    return SQLSerialiserAnnotation::getSpeakersInLevel(annotationID, levelID, d->structure, d->database);
}

QStringList SQLAnnotationDatastore::getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID)
{
    return SQLSerialiserAnnotation::getSpeakersActiveInLevel(annotationID, levelID, d->structure, d->database);
}

QStringList SQLAnnotationDatastore::getSpeakersInAnnotation(const QString &annotationID)
{
    return SQLSerialiserAnnotation::getSpeakersInAnnotation(annotationID, d->structure, d->database);
}

QStringList SQLAnnotationDatastore::getSpeakersActiveInAnnotation(const QString &annotationID)
{
    return SQLSerialiserAnnotation::getSpeakersActiveInAnnotation(annotationID, d->structure, d->database);
}

IntervalTier *SQLAnnotationDatastore::getSpeakerTimeline(const QString &communicationID,const QString &annotationID,
                                                         const QString &levelID, bool detailed)
{
    return SQLSerialiserAnnotation::getSpeakerTimeline(communicationID, annotationID, levelID, detailed, d->structure, d->database);
}

// ==========================================================================================================================
// Queries
// ==========================================================================================================================

QList<QueryOccurrencePointer *> SQLAnnotationDatastore::runQuery(QueryDefinition *qdef)
{
    return SQLQueryEngineAnnotation::runQuery(qdef, d->structure, d->database);
}

QueryOccurrence *SQLAnnotationDatastore::getOccurrence(QueryOccurrencePointer *pointer, QueryDefinition *qdef)
{
    return SQLQueryEngineAnnotation::getOccurrence(pointer, qdef, d->structure, d->database);
}

bool SQLAnnotationDatastore::updateAnnotationsFromQueryOccurrences(const QList<QueryOccurrence *> &occurrences)
{
    return SQLQueryEngineAnnotation::updateAnnotationsFromQueryOccurrences(occurrences, d->structure, d->database);
}

// ==========================================================================================================================
// Batch Processing
// ==========================================================================================================================

QList<QPair<QList<QVariant>, long> > SQLAnnotationDatastore::getDistinctLabels(const QString &levelID, const QStringList &attributeIDs)
{
    return SQLSerialiserAnnotation::getDistinctLabels(levelID, attributeIDs, d->structure, d->database);
}

bool SQLAnnotationDatastore::batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                                         const QList<QPair<QString, QVariant> > &criteria)
{
    return SQLSerialiserAnnotation::batchUpdate(levelID, attributeID, newValue, criteria, d->structure, d->database);
}

// ==========================================================================================================================
// Statistics
// ==========================================================================================================================

QList<QPair<QList<QVariant>, long long> > SQLAnnotationDatastore::countItems(const QString &levelID, const QStringList &groupByAttributeIDs)
{
    return SQLSerialiserAnnotation::countItems(levelID, groupByAttributeIDs, d->structure, d->database);
}

} // namespace Core
} // namespace Praaline
