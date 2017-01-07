#include <QDebug>
#include <QSqlError>
#include "SQLSerialiserCorpus.h"
#include "SQLSerialiserMetadataStructure.h"
#include "SQLSerialiserNameValueList.h"
#include "SQLMetadataDatastore.h"

namespace Praaline {
namespace Core {

SQLMetadataDatastore::SQLMetadataDatastore(QPointer<MetadataStructure> structure, QObject *parent) :
    MetadataDatastore(parent), m_structure(structure)
{
}

SQLMetadataDatastore::~SQLMetadataDatastore()
{
    closeDatastore();
}

// ==========================================================================================================================
// Datastore
// ==========================================================================================================================
bool SQLMetadataDatastore::createDatastore(const DatastoreInfo &info)
{
    if (m_database.isOpen()) m_database.close();
    m_database = QSqlDatabase::addDatabase(info.driver, info.datasource + "_metadata");
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
    SQLSerialiserMetadataStructure::initialiseMetadataStructureSchema(m_database);
    SQLSerialiserMetadataStructure::createMetadataSchema(m_structure, m_database);
    SQLSerialiserNameValueList::initialiseNameValueListSchema(m_database);
    return true;
}

bool SQLMetadataDatastore::openDatastore(const DatastoreInfo &info)
{
    if (m_database.isOpen()) m_database.close();
    m_database = QSqlDatabase::addDatabase(info.driver, info.datasource + "_metadata");
    m_database.setHostName(info.hostname);
    m_database.setDatabaseName(info.datasource);
    m_database.setUserName(info.username);
    m_database.setPassword(info.password);
    if (!m_database.open()) {
        m_lastError = m_database.lastError().text();
        qDebug() << m_lastError;
        return false;
    }
    // Upgrade database if needed
    SQLSerialiserMetadataStructure::upgradeMetadataStructureSchema(m_database);
    SQLSerialiserNameValueList::upgradeNameValueListSchema(m_database);
    return true;
}

bool SQLMetadataDatastore::closeDatastore()
{
    if (m_database.isOpen())
        m_database.close();
    return true;
}

// ==========================================================================================================================
// Metadata Structure
// ==========================================================================================================================
bool SQLMetadataDatastore::createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute)
{
    return SQLSerialiserMetadataStructure::createMetadataAttribute(type, newAttribute, m_database);
}

bool SQLMetadataDatastore::renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID)
{
    return SQLSerialiserMetadataStructure::renameMetadataAttribute(type, attributeID, newAttributeID, m_database);
}

bool SQLMetadataDatastore::deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID)
{
    return SQLSerialiserMetadataStructure::deleteMetadataAttribute(type, attributeID, m_database);
}

bool SQLMetadataDatastore::retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const DataType &newDatatype)
{
    // return SQLSerialiserMetadataStructure::retypeMetadataAttribute(type, attributeID, newDatatype, m_database);
    return false;
}

bool SQLMetadataDatastore::loadMetadataStructure()
{
    return SQLSerialiserMetadataStructure::loadMetadataStructure(m_structure, m_database);
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
    return SQLSerialiserNameValueList::getNameValueList(listID, SQLSerialiserNameValueList::Metadata, m_database);
}

QStringList SQLMetadataDatastore::getAllNameValueListIDs()
{
    return SQLSerialiserNameValueList::getAllNameValueListIDs(SQLSerialiserNameValueList::Metadata, m_database);
}

QMap<QString, QPointer<NameValueList> > SQLMetadataDatastore::getAllNameValueLists()
{
    return SQLSerialiserNameValueList::getAllNameValueLists(SQLSerialiserNameValueList::Metadata, m_database);
}

bool SQLMetadataDatastore::createNameValueList(NameValueList *list)
{
    return SQLSerialiserNameValueList::createNameValueList(list, SQLSerialiserNameValueList::Metadata, m_database);
}

bool SQLMetadataDatastore::updateNameValueList(NameValueList *list)
{
    return SQLSerialiserNameValueList::updateNameValueList(list, SQLSerialiserNameValueList::Metadata, m_database);
}

bool SQLMetadataDatastore::deleteNameValueList(const QString &listID)
{
    return SQLSerialiserNameValueList::deleteNameValueList(listID, SQLSerialiserNameValueList::Metadata, m_database);
}

// ==========================================================================================================================
// Corpus
// ==========================================================================================================================

bool SQLMetadataDatastore::loadCorpus(Corpus *corpus)
{
    return SQLSerialiserCorpus::loadCorpus(corpus, m_database);
}

bool SQLMetadataDatastore::saveCorpus(Corpus *corpus)
{
    return SQLSerialiserCorpus::saveCorpus(corpus, m_database);
}

} // namespace Core
} // namespace Praaline
