#include <QDebug>
#include <QSqlError>
#include "corpus/CorpusObject.h"
#include "corpus/CorpusObjectInfo.h"
#include "structure/MetadataStructure.h"
#include "structure/MetadataStructureAttribute.h"
#include "datastore/DatastoreInfo.h"
#include "datastore/CorpusRepository.h"
#include "SQLSerialiserCorpus.h"
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
        d->lastError = d->database.lastError().text();
        qDebug() << d->lastError;
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
        d->lastError = d->database.lastError().text();
        qDebug() << d->lastError;
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
    QList<CorpusObjectInfo> ret;
    return ret;
}

// ==========================================================================================================================
// Corpus
// ==========================================================================================================================
// Load metadata information in already created corpus objects
bool SQLMetadataDatastore::loadCorpus(Corpus *corpus)
{
    return SQLSerialiserCorpus::loadCorpus(corpus, d->database);
}

bool SQLMetadataDatastore::loadCommunications(QList<QPointer<CorpusCommunication> > &communications)
{
    return false;
}

bool SQLMetadataDatastore::loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers)
{
    return false;
}

bool SQLMetadataDatastore::loadRecordings(QList<QPointer<CorpusRecording> > &recordings)
{
    return false;
}

bool SQLMetadataDatastore::loadAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations)
{
    return false;
}

// Save (insert or update) corpus objects
bool SQLMetadataDatastore::saveCorpus(Corpus *corpus)
{
    return SQLSerialiserCorpus::saveCorpus(corpus, d->database);
}

bool SQLMetadataDatastore::saveCommunications(QList<QPointer<CorpusCommunication> > &communications)
{
    return false;
}

bool SQLMetadataDatastore::saveSpeakers(QList<QPointer<CorpusSpeaker> > &speakers)
{
    return false;
}

bool SQLMetadataDatastore::saveRecordings(QList<QPointer<CorpusRecording> > &recordings)
{
    return false;
}

bool SQLMetadataDatastore::saveAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations)
{
    return false;
}

bool SQLMetadataDatastore::saveParticipations(QList<QPointer<CorpusParticipation> >  &participations)
{
    return false;
}

// Delete corpus objects
bool SQLMetadataDatastore::deleteCorpus(const QString &corpusID)
{
    return false;
}

bool SQLMetadataDatastore::deleteCommunication(const QString &communicationID)
{
    return false;
}

bool SQLMetadataDatastore::deleteSpeaker(const QString &speakerID)
{
    return false;
}

bool SQLMetadataDatastore::deleteRecording(const QString &recordingID)
{
    return false;
}

bool SQLMetadataDatastore::deleteAnnotation(const QString &annotationID)
{
    return false;
}

bool SQLMetadataDatastore::deleteParticipation(const QString &communicationID, const QString &speakerID)
{
    return false;
}

} // namespace Core
} // namespace Praaline
