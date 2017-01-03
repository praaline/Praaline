#ifndef ABSTRACTMETADATADATASTORE_H
#define ABSTRACTMETADATADATASTORE_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include "DatastoreInfo.h"
#include "structure/MetadataStructure.h"

namespace Praaline {
namespace Core {

class Corpus;

class PRAALINE_CORE_SHARED_EXPORT MetadataDatastore : public QObject
{
    Q_OBJECT
public:
    MetadataDatastore(QObject *parent = 0) : QObject(parent) {}
    virtual bool createDatastore(const DatastoreInfo &info) = 0;
    virtual bool openDatastore(const DatastoreInfo &info) = 0;
    virtual bool closeDatastore() = 0;

    virtual bool loadMetadataStructure() = 0;
    virtual bool saveMetadataStructure() = 0;
    virtual bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute) = 0;
    virtual bool renameMetadataAttribute(CorpusObject::Type type, QString attributeID, QString newAttributeID) = 0;
    virtual bool deleteMetadataAttribute(CorpusObject::Type type, QString attributeID) = 0;

    virtual bool loadCorpus(Corpus *corpus) = 0;
    virtual bool saveCorpus(Corpus *corpus) = 0;

    virtual QString lastError() const { return m_lastError; }
    virtual void clearError() { m_lastError.clear(); }

protected:
    QString m_lastError;
};

} // namespace Core
} // namespace Praaline

#endif // ABSTRACTMETADATADATASTORE_H
