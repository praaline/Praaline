#ifndef CORPUSREPOSITORYDEFINITION_H
#define CORPUSREPOSITORYDEFINITION_H

/*
    Praaline - Core module - Datastores
    Copyright (c) 2011-2017 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "pncore_global.h"
#include <QObject>
#include "DatastoreInfo.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT CorpusRepositoryDefinition
{
public:
    explicit CorpusRepositoryDefinition();
    ~CorpusRepositoryDefinition();

    QString filenameDefinition;
    QString repositoryID;
    QString repositoryName;
    DatastoreInfo infoDatastoreMetadata;
    DatastoreInfo infoDatastoreAnnotations;
    QString basePath;
    QString basePathMedia;

    bool save(const QString &filename);
    bool load(const QString &filename);

signals:

public slots:
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSREPOSITORYDEFINITION_H
