#ifndef SQLMETADATADATASTORE_H
#define SQLMETADATADATASTORE_H

#include <QObject>
#include <QSqlDatabase>
#include "serialisers/abstractmetadatadatastore.h"

class SQLMetadataDatastore : public AbstractMetadataDatastore
{
    Q_OBJECT
public:
    SQLMetadataDatastore(QPointer<MetadataStructure> structure, QObject *parent = 0);
    ~SQLMetadataDatastore();

    bool createDatastore(const DatastoreInfo &info);
    bool openDatastore(const DatastoreInfo &info);
    bool closeDatastore();

    bool loadMetadataStructure();
    bool saveMetadataStructure();
    bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute);
    bool renameMetadataAttribute(CorpusObject::Type type, QString attributeID, QString newAttributeID);
    bool deleteMetadataAttribute(CorpusObject::Type type, QString attributeID);

    bool loadCorpus(Corpus *corpus);
    bool saveCorpus(Corpus *corpus);

private:
    QSqlDatabase m_database;
    QPointer<MetadataStructure> m_structure;
};

#endif // SQLMETADATADATASTORE_H
