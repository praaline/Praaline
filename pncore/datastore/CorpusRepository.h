#ifndef CORPUSREPOSITORY_H
#define CORPUSREPOSITORY_H

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
#include <QString>
#include <QPointer>
#include "pncore/corpus/CorpusObjectInfo.h"

namespace Praaline {
namespace Core {

class CorpusRepositoryDefinition;
class MetadataDatastore;
class AnnotationDatastore;
class FileDatastore;
class MetadataStructure;
class AnnotationStructure;

struct CorpusRepositoryData;

class PRAALINE_CORE_SHARED_EXPORT CorpusRepository : public QObject
{
    Q_OBJECT
public:
    // Corpus repository
    // ==========================================================================================================================
    static CorpusRepository *create(const CorpusRepositoryDefinition &definition, QString &errorMessages, QObject *parent = 0);
    static CorpusRepository *open(const CorpusRepositoryDefinition &definition, QString &errorMessages, QObject *parent = 0);
    void close();

    QString ID() const;
    void setID(const QString &ID);
    QString description() const;
    void setDescription(const QString &description);

    CorpusRepositoryDefinition definition() const;

    // Datastores
    // ==========================================================================================================================
    QPointer<AnnotationDatastore> annotations() const;
    QPointer<MetadataDatastore> metadata() const;
    QPointer<FileDatastore> files() const;
    QList<CorpusObjectInfo> listCorporaInfo() const;
    QStringList listCorporaIDs() const;

    // Metadata and Annotation structures
    // ==========================================================================================================================
    QPointer<MetadataStructure> metadataStructure() const;
    QPointer<AnnotationStructure> annotationStructure() const;
    void importMetadataStructure(MetadataStructure *otherStructure);
    void importAnnotationStructure(AnnotationStructure *otherStructure);

    // Error handling and logging
    // ==========================================================================================================================
    QString lastError() const;
    void setLastError(const QString &errorMessage);
    void clearLastError();
    void sendLogMessage(const QString &category, const QString &message);

signals:
    void changedID(const QString &oldID, const QString &newID);
    void logMessage(const QString &category, const QString &message);

public slots:

private:
    CorpusRepository(const CorpusRepositoryDefinition &definition, QObject *parent = 0);
    ~CorpusRepository();

    CorpusRepositoryData *d;
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSREPOSITORY_H
