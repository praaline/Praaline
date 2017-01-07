#ifndef SQLMETADATADATASTORE_H
#define SQLMETADATADATASTORE_H

#include <QObject>
#include <QSqlDatabase>
#include "serialisers/MetadataDatastore.h"

namespace Praaline {
namespace Core {

class SQLMetadataDatastore : public MetadataDatastore
{
    Q_OBJECT
public:
    SQLMetadataDatastore(QPointer<MetadataStructure> structure, QObject *parent = 0);
    ~SQLMetadataDatastore();

    // ==========================================================================================================================
    // Datastore
    // ==========================================================================================================================
    bool createDatastore(const DatastoreInfo &info) override;
    bool openDatastore(const DatastoreInfo &info) override;
    bool closeDatastore() override;

    // ==========================================================================================================================
    // Metadata Structure
    // ==========================================================================================================================
    bool loadMetadataStructure() override;
    bool saveMetadataStructure() override;
    bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute) override;
    bool renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID) override;
    bool deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID) override;
    bool retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const DataType &newDatatype) override;

    // ==========================================================================================================================
    // Name-value lists
    // ==========================================================================================================================
    NameValueList *getNameValueList(const QString &listID) override;
    QStringList getAllNameValueListIDs() override;
    QMap<QString, QPointer<NameValueList> > getAllNameValueLists() override;
    bool createNameValueList(NameValueList *list) override;
    bool updateNameValueList(NameValueList *list) override;
    bool deleteNameValueList(const QString &listID) override;

    // ==========================================================================================================================
    // Corpus
    // ==========================================================================================================================
    bool loadCorpus(Corpus *corpus) override;
    bool saveCorpus(Corpus *corpus) override;

private:
    QSqlDatabase m_database;
    QPointer<MetadataStructure> m_structure;
};

} // namespace Core
} // namespace Praaline

#endif // SQLMETADATADATASTORE_H
