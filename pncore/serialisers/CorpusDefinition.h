#ifndef CORPUSDEFINITION_H
#define CORPUSDEFINITION_H

#include "pncore_global.h"
#include <QObject>
#include "DatastoreInfo.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT CorpusDefinition
{
public:
    explicit CorpusDefinition();
    ~CorpusDefinition();

    QString filenameDefinition;
    QString corpusID;
    QString corpusName;
    DatastoreInfo datastoreMetadata;
    DatastoreInfo datastoreAnnotations;
    QString basePath;
    QString baseMediaPath;

    bool save(const QString &filename);
    bool load(const QString &filename);

signals:

public slots:
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSDEFINITION_H
