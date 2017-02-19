#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "corpus/CorpusObject.h"
#include "corpus/CorpusObjectInfo.h"
#include "structure/MetadataStructure.h"
#include "structure/MetadataStructureAttribute.h"
#include "datastore/DatastoreInfo.h"
#include "datastore/CorpusRepository.h"
#include "SQLSerialiserMetadata.h"
#include "SQLSerialiserMetadataStructure.h"
#include "SQLSerialiserNameValueList.h"
#include "SQLMetadataDatastore.h"

namespace Praaline {
namespace Core {

struct SQLMetadataDatastoreData {
    QSqlDatabase database;
    QPointer<MetadataStructure> structure;
    QPointer<CorpusRepository> repository;
};

SQLMetadataDatastore::SQLMetadataDatastore(MetadataStructure *structure, CorpusRepository *repository, QObject *parent) :
    MetadataDatastore(repository, parent), d(new SQLMetadataDatastoreData)
{
    d->structure = structure;
    d->repository = repository;
}

SQLMetadataDatastore::~SQLMetadataDatastore()
{
    closeDatastore();
    delete d;
}

// ==========================================================================================================================
// Datastore
// ==========================================================================================================================
bool SQLMetadataDatastore::createDatastore(const DatastoreInfo &info)
{
    if (d->database.isOpen()) d->database.close();
    d->database = QSqlDatabase::addDatabase(info.driver, info.datasource + "_metadata");
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
    SQLSerialiserMetadataStructure::initialiseMetadataStructureSchema(d->database);
    SQLSerialiserMetadataStructure::initialiseMetadataSchema(d->structure, d->database);
    SQLSerialiserNameValueList::initialiseNameValueListSchema(d->database);
    return true;
}

bool SQLMetadataDatastore::openDatastore(const DatastoreInfo &info)
{
    if (d->database.isOpen()) d->database.close();
    d->database = QSqlDatabase::addDatabase(info.driver, info.datasource + "_metadata");
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
    SQLSerialiserMetadataStructure::upgradeMetadataStructureSchema(d->database);
    SQLSerialiserMetadataStructure::upgradeMetadataSchema(d->structure, d->database);
    SQLSerialiserNameValueList::upgradeNameValueListSchema(d->database);
    return true;
}

bool SQLMetadataDatastore::closeDatastore()
{
    if (d->database.isOpen())
        d->database.close();
    return true;
}

// ==========================================================================================================================
// Metadata Structure
// ==========================================================================================================================
bool SQLMetadataDatastore::createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute)
{
    return SQLSerialiserMetadataStructure::createMetadataAttribute(type, newAttribute, d->database);
}

bool SQLMetadataDatastore::renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID)
{
    return SQLSerialiserMetadataStructure::renameMetadataAttribute(type, attributeID, newAttributeID, d->database);
}

bool SQLMetadataDatastore::deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID)
{
    return SQLSerialiserMetadataStructure::deleteMetadataAttribute(type, attributeID, d->database);
}

bool SQLMetadataDatastore::retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const DataType &newDatatype)
{
    return SQLSerialiserMetadataStructure::retypeMetadataAttribute(type, attributeID, newDatatype, d->database);
}

bool SQLMetadataDatastore::loadMetadataStructure()
{
    return SQLSerialiserMetadataStructure::loadMetadataStructure(d->structure, d->database);
}

bool SQLMetadataDatastore::saveMetadataStructure()
{
    return SQLSerialiserMetadataStructure::saveMetadataStructure(d->structure, d->database);
}

// ==========================================================================================================================
// Name-value lists
// ==========================================================================================================================

NameValueList *SQLMetadataDatastore::getNameValueList(const QString &listID)
{
    return SQLSerialiserNameValueList::getNameValueList(listID, SQLSerialiserNameValueList::Metadata, d->database);
}

QStringList SQLMetadataDatastore::getAllNameValueListIDs()
{
    return SQLSerialiserNameValueList::getAllNameValueListIDs(SQLSerialiserNameValueList::Metadata, d->database);
}

QMap<QString, QPointer<NameValueList> > SQLMetadataDatastore::getAllNameValueLists()
{
    return SQLSerialiserNameValueList::getAllNameValueLists(SQLSerialiserNameValueList::Metadata, d->database);
}

bool SQLMetadataDatastore::createNameValueList(NameValueList *list)
{
    return SQLSerialiserNameValueList::createNameValueList(list, SQLSerialiserNameValueList::Metadata, d->database);
}

bool SQLMetadataDatastore::updateNameValueList(NameValueList *list)
{
    return SQLSerialiserNameValueList::updateNameValueList(list, SQLSerialiserNameValueList::Metadata, d->database);
}

bool SQLMetadataDatastore::deleteNameValueList(const QString &listID)
{
    return SQLSerialiserNameValueList::deleteNameValueList(listID, SQLSerialiserNameValueList::Metadata, d->database);
}

// ==========================================================================================================================
// Corpus object info lists
// ==========================================================================================================================
QList<CorpusObjectInfo> SQLMetadataDatastore::getCorpusObjectInfoList(CorpusObject::Type type, const Selection &selection)
{
    return SQLSerialiserMetadata::getCorpusObjectInfoList(type, selection, d->database, d->structure, this);
}

bool SQLMetadataDatastore::saveCorpusObjectInfo(CorpusObject::Type type, const QList<CorpusObjectInfo> &list)
{
    return SQLSerialiserMetadata::saveCorpusObjectInfo(type, list, d->database, d->structure, this);
}


// ==========================================================================================================================
// Corpus
// ==========================================================================================================================
// Load metadata information
Corpus *SQLMetadataDatastore::getCorpus(const QString &corpusID)
{
    return SQLSerialiserMetadata::getCorpus(corpusID, d->database, d->structure, this);
}

QList<QPointer<CorpusCommunication> > SQLMetadataDatastore::getCommunications(const Selection &selection)
{
    return SQLSerialiserMetadata::getCommunications(selection, d->database, d->structure, this);
}

QList<QPointer<CorpusSpeaker> > SQLMetadataDatastore::getSpeakers(const Selection &selection)
{
    return SQLSerialiserMetadata::getSpeakers(selection, d->database, d->structure, this);
}

QList<QPointer<CorpusRecording> > SQLMetadataDatastore::getRecordings(const Selection &selection)
{
    return SQLSerialiserMetadata::getRecordings(selection, d->database, d->structure, this);
}

QList<QPointer<CorpusAnnotation> > SQLMetadataDatastore::getAnnotations(const Selection &selection)
{
    return SQLSerialiserMetadata::getAnnotations(selection, d->database, d->structure, this);
}

QList<QPointer<CorpusParticipation> > SQLMetadataDatastore::getParticipations(const Selection &selection)
{
    return SQLSerialiserMetadata::getParticipations(selection, d->database, d->structure, this);
}

// Save (insert or update) corpus objects
bool SQLMetadataDatastore::saveCorpus(Corpus *corpus)
{
    return SQLSerialiserMetadata::saveCorpus(corpus, d->database, d->structure, this);
}

bool SQLMetadataDatastore::saveCommunication(CorpusCommunication *communication)
{
    return SQLSerialiserMetadata::saveCommunication(communication, d->database, d->structure, this);
}

bool SQLMetadataDatastore::saveCommunications(QList<QPointer<CorpusCommunication> > &communications)
{
    bool success = true;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        success = success && SQLSerialiserMetadata::saveCommunication(com, d->database, d->structure, this);
    }
    return success;
}

bool SQLMetadataDatastore::saveSpeaker(CorpusSpeaker *speaker)
{
    return SQLSerialiserMetadata::saveSpeaker(speaker, d->database, d->structure, this);
}

bool SQLMetadataDatastore::saveSpeakers(QList<QPointer<CorpusSpeaker> > &speakers)
{
    bool success = true;
    foreach (QPointer<CorpusSpeaker> spk, speakers) {
        if (!spk) continue;
        success = success && SQLSerialiserMetadata::saveSpeaker(spk, d->database, d->structure, this);
    }
    return success;
}

bool SQLMetadataDatastore::saveRecordings(QList<QPointer<CorpusRecording> > &recordings)
{
    bool success = true;
    foreach (QPointer<CorpusRecording> rec, recordings) {
        if (!rec) continue;
        success = success && SQLSerialiserMetadata::saveRecording(rec, d->database, d->structure, this);
    }
    return success;
}

bool SQLMetadataDatastore::saveAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations)
{
    bool success = true;
    foreach (QPointer<CorpusAnnotation> annot, annotations) {
        if (!annot) continue;
        success = success && SQLSerialiserMetadata::saveAnnotation(annot, d->database, d->structure, this);
    }
    return success;
}

bool SQLMetadataDatastore::saveParticipations(QList<QPointer<CorpusParticipation> >  &participations)
{
    bool success = true;
    foreach (QPointer<CorpusParticipation> part, participations) {
        if (!part) continue;
        success = success && SQLSerialiserMetadata::saveParticipation(part, d->database, d->structure, this);
    }
    return success;
}

// Delete corpus objects
bool SQLMetadataDatastore::deleteCorpus(const QString &corpusID)
{
    return SQLSerialiserMetadata::deleteCorpus(corpusID, d->database, d->structure, this);
}

bool SQLMetadataDatastore::deleteCommunication(const QString &communicationID)
{
    return SQLSerialiserMetadata::deleteCommunication(communicationID, d->database, d->structure, this);
}

bool SQLMetadataDatastore::deleteSpeaker(const QString &speakerID)
{
    return SQLSerialiserMetadata::deleteSpeaker(speakerID, d->database, d->structure, this);
}

bool SQLMetadataDatastore::deleteRecording(const QString &recordingID)
{
    return SQLSerialiserMetadata::deleteRecording(recordingID, d->database, d->structure, this);
}

bool SQLMetadataDatastore::deleteAnnotation(const QString &annotationID)
{
    return SQLSerialiserMetadata::deleteAnnotation(annotationID, d->database, d->structure, this);
}

bool SQLMetadataDatastore::deleteParticipation(const QString &communicationID, const QString &speakerID)
{
    return SQLSerialiserMetadata::deleteParticipation(communicationID, speakerID, d->database, d->structure, this);
}

} // namespace Core
} // namespace Praaline
