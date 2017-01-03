#ifndef XMLMETADATADATASTORE_H
#define XMLMETADATADATASTORE_H

#include <QObject>
#include "serialisers/MetadataDatastore.h"

namespace Praaline {
namespace Core {

class XMLMetadataDatastore : public QObject
{
    Q_OBJECT
public:
    explicit XMLMetadataDatastore(QPointer<MetadataStructure> structure, QObject *parent = 0);
    ~XMLMetadataDatastore();

    bool createDatastore(const DatastoreInfo &info);
    bool openDatastore(const DatastoreInfo &info);
    bool closeDatastore();

    bool loadAnnotationStructure();
    bool saveAnnotationStructure();
    bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute);
    bool renameMetadataAttribute(CorpusObject::Type type, QString attributeID, QString newAttributeID);
    bool deleteMetadataAttribute(CorpusObject::Type type, QString attributeID);

    bool loadCorpus(Corpus *corpus);
    bool saveCorpus(Corpus *corpus);

private:
    QString m_filename;
    QPointer<MetadataStructure> m_structure;
};

} // namespace Core
} // namespace Praaline

#endif // XMLMETADATADATASTORE_H
