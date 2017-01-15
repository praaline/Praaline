#include <QDebug>
#include <QSqlDatabase>
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
    d->database.setDatabaseName(info.datasource);
    d->database.setUserName(info.username);
    d->database.setPassword(info.password);
    if (!d->database.open()) {
        d->repository->setLastError(d->database.lastError().text());
        qDebug() << d->database.lastError().text();
        return false;
    }
    // Initialise database
    SQLSerialiserMetadataStructure::initialiseMetadataStructureSchema(d->database);
    SQLSerialiserMetadataStructure::createMetadataSchema(d->structure, d->database);
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
    SQLSerialiserMetadataStructure::upgradeMetadataStructureSchema(d->database);
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
    // return SQLSerialiserMetadataStructure::retypeMetadataAttribute(type, attributeID, newDatatype, d->database);
    return false;
}

bool SQLMetadataDatastore::loadMetadataStructure()
{
    return SQLSerialiserMetadataStructure::loadMetadataStructure(d->structure, d->database);
}

bool SQLMetadataDatastore::saveMetadataStructure()
{
    return true;
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
QList<CorpusObjectInfo> SQLMetadataDatastore::getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID)
{
    return SQLSerialiserMetadata::getCorpusObjectInfoList(type, parentID, d->database, d->repository);
}

// ==========================================================================================================================
// Corpus
// ==========================================================================================================================
// Load metadata information in already created corpus objects
bool SQLMetadataDatastore::loadCorpus(Corpus *corpus)
{
    return SQLSerialiserMetadata::loadCorpus(corpus, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::loadCommunications(QList<QPointer<CorpusCommunication> > &communications)
{
    return SQLSerialiserMetadata::loadCommunications(communications, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers)
{
    return SQLSerialiserMetadata::loadSpeakers(speakers, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::loadRecordings(QList<QPointer<CorpusRecording> > &recordings)
{
    return SQLSerialiserMetadata::loadRecordings(recordings, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::loadAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations)
{
    return SQLSerialiserMetadata::loadAnnotations(annotations, d->database, d->structure, d->repository);
}

// Save (insert or update) corpus objects
bool SQLMetadataDatastore::saveCorpus(Corpus *corpus)
{
    return SQLSerialiserMetadata::saveCorpus(corpus, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::saveCommunications(QList<QPointer<CorpusCommunication> > &communications)
{
    return SQLSerialiserMetadata::saveCommunications(communications, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::saveSpeakers(QList<QPointer<CorpusSpeaker> > &speakers)
{
    return SQLSerialiserMetadata::saveSpeakers(speakers, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::saveRecordings(QList<QPointer<CorpusRecording> > &recordings)
{
    return SQLSerialiserMetadata::saveRecordings(recordings, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::saveAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations)
{
    return SQLSerialiserMetadata::saveAnnotations(annotations, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::saveParticipations(QList<QPointer<CorpusParticipation> >  &participations)
{
    return SQLSerialiserMetadata::saveParticipations(participations, d->database, d->structure, d->repository);
}

// Delete corpus objects
bool SQLMetadataDatastore::deleteCorpus(const QString &corpusID)
{
    return SQLSerialiserMetadata::deleteCorpus(corpusID, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::deleteCommunication(const QString &communicationID)
{
    return SQLSerialiserMetadata::deleteCommunication(communicationID, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::deleteSpeaker(const QString &speakerID)
{
    return SQLSerialiserMetadata::deleteSpeaker(speakerID, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::deleteRecording(const QString &recordingID)
{
    return SQLSerialiserMetadata::deleteRecording(recordingID, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::deleteAnnotation(const QString &annotationID)
{
    return SQLSerialiserMetadata::deleteAnnotation(annotationID, d->database, d->structure, d->repository);
}

bool SQLMetadataDatastore::deleteParticipation(const QString &communicationID, const QString &speakerID)
{
    return SQLSerialiserMetadata::deleteParticipation(communicationID, speakerID, d->database, d->structure, d->repository);
}

} // namespace Core
} // namespace Praaline
