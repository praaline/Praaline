#ifndef ABSTRACTMETADATADATASTORE_H
#define ABSTRACTMETADATADATASTORE_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include "DatastoreInfo.h"
#include "NameValueListDatastore.h"
#include "base/DataType.h"
#include "structure/MetadataStructure.h"
#include "structure/NameValueList.h"

namespace Praaline {
namespace Core {

class Corpus;

class PRAALINE_CORE_SHARED_EXPORT MetadataDatastore : public QObject, public NameValueListDatastore
{
    Q_OBJECT
public:
    MetadataDatastore(QObject *parent = 0) : QObject(parent) {}
    virtual ~MetadataDatastore() {}

    // ==========================================================================================================================
    // Datastore
    // ==========================================================================================================================
    virtual bool createDatastore(const DatastoreInfo &info) = 0;
    virtual bool openDatastore(const DatastoreInfo &info) = 0;
    virtual bool closeDatastore() = 0;

    // ==========================================================================================================================
    // Metadata Structure
    // ==========================================================================================================================
    virtual bool loadMetadataStructure() = 0;
    virtual bool saveMetadataStructure() = 0;
    virtual bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute) = 0;
    virtual bool renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID) = 0;
    virtual bool deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID) = 0;
    virtual bool retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const DataType &newDatatype) = 0;

    // ==========================================================================================================================
    // Name-value lists
    // ==========================================================================================================================
    virtual NameValueList *getNameValueList(const QString &listID) = 0;
    virtual QStringList getAllNameValueListIDs() = 0;
    virtual QMap<QString, QPointer<NameValueList> > getAllNameValueLists() = 0;
    virtual bool createNameValueList(NameValueList *list) = 0;
    virtual bool updateNameValueList(NameValueList *list) = 0;
    virtual bool deleteNameValueList(const QString &listID) = 0;

    // ==========================================================================================================================
    // Corpus
    // ==========================================================================================================================
    virtual bool loadCorpus(Corpus *corpus) = 0;
    virtual bool saveCorpus(Corpus *corpus) = 0;

    // ==========================================================================================================================
    // Error handling
    // ==========================================================================================================================
    virtual QString lastError() const { return m_lastError; }
    virtual void clearError() { m_lastError.clear(); }

protected:
    QString m_lastError;
};

} // namespace Core
} // namespace Praaline

#endif // ABSTRACTMETADATADATASTORE_H
