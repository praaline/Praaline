#ifndef CORPUSDEFINITION_H
#define CORPUSDEFINITION_H

#include "pncore_global.h"
#include <QObject>
#include "datastoreinfo.h"

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

#endif // CORPUSDEFINITION_H
