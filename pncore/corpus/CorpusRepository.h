#ifndef CORPUSREPOSITORY_H
#define CORPUSREPOSITORY_H

#include <QObject>
#include <QPointer>
#include "structure/NameValueList.h"
#include "serialisers/DatastoreFactory.h"
#include "serialisers/MetadataDatastore.h"
#include "serialisers/AnnotationDatastore.h"

namespace Praaline {
namespace Core {

class CorpusRepository : public QObject
{
    Q_OBJECT
public:
    explicit CorpusRepository(QObject *parent = 0);

//    // Name-value lists
//    QStringList nameValueListIDs(NameValueList::) const;
//    QPointer<NameValueList> nameValueList(const QString &listID) const;
//    QPointer<NameValueList> createNameValueList(const QString &listID, const QString &name, const DataType &dataType);
//    QPointer<NameValueList> createNameValueList(QPointer<MetadataStructureAttribute> attribute);
//    bool deleteNameValueList(const QString &listID);

signals:

public slots:

protected:
//    QPointer<MetadataDatastore> m_datastoreMetadata;
//    QPointer<AnnotationDatastore> m_datastoreAnnotations;


};

} // namespace Core
} // namespace Praaline

#endif // CORPUSREPOSITORY_H
