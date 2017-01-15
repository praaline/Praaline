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

namespace Praaline {
namespace Core {

class CorpusRepositoryDefinition;
class MetadataDatastore;
class AnnotationDatastore;
class FileDatastore;
class MetadataStructure;
class AnnotationStructure;
class NameValueList;

class PRAALINE_CORE_SHARED_EXPORT CorpusRepository : public QObject
{
    Q_OBJECT
public:
    // Corpus repository
    // ==========================================================================================================================
    static CorpusRepository *create(const CorpusRepositoryDefinition &definition, QString &errorMessages, QObject *parent = 0);
    static CorpusRepository *open(const CorpusRepositoryDefinition &definition, QString &errorMessages, QObject *parent = 0);
    void save();
    void close();

    // Datastores
    // ==========================================================================================================================
    QPointer<AnnotationDatastore> annotations() const;
    QPointer<MetadataDatastore> metadata() const;
    QPointer<FileDatastore> files() const;

    // Metadata and Annotation structures
    // ==========================================================================================================================
    QPointer<MetadataStructure> metadataStructure() const { return m_metadataStructure; }
    QPointer<AnnotationStructure> annotationStructure() const { return m_annotationStructure; }   
    void importMetadataStructure(MetadataStructure *otherStructure);
    void importAnnotationStructure(AnnotationStructure *otherStructure);

signals:

public slots:

protected:
    QPointer<MetadataStructure> m_metadataStructure;
    QPointer<AnnotationStructure> m_annotationStructure;
    QPointer<MetadataDatastore> m_datastoreMetadata;
    QPointer<AnnotationDatastore> m_datastoreAnnotations;
    QPointer<FileDatastore> m_datastoreFiles;

private:
    CorpusRepository(const CorpusRepositoryDefinition &definition, QObject *parent = 0);
    ~CorpusRepository();
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSREPOSITORY_H
