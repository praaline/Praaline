#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QMetaEnum>
#include <QRegularExpression>

#include "structure/MetadataStructure.h"
#include "structure/AnnotationStructure.h"
#include "structure/NameValueList.h"
#include "CorpusRepositoryDefinition.h"
#include "DatastoreFactory.h"
#include "MetadataDatastore.h"
#include "AnnotationDatastore.h"
#include "FileDatastore.h"
#include "CorpusRepository.h"

namespace Praaline {
namespace Core {

CorpusRepository::CorpusRepository(const CorpusRepositoryDefinition &definition, QObject *parent) :
    QObject(parent)
{
    m_annotationStructure = new AnnotationStructure(this);
    m_metadataStructure = new MetadataStructure(this);
    m_datastoreAnnotations = DatastoreFactory::getAnnotationDatastore(definition.infoDatastoreAnnotations, m_annotationStructure, this);
    m_datastoreMetadata = DatastoreFactory::getMetadataDatastore(definition.infoDatastoreMetadata, m_metadataStructure, this);
    m_datastoreFiles = new FileDatastore(this);
}

CorpusRepository::~CorpusRepository()
{
    if (m_datastoreMetadata)    delete m_datastoreMetadata;
    if (m_datastoreAnnotations) delete m_datastoreAnnotations;
    if (m_datastoreFiles)       delete m_datastoreFiles;
    if (m_metadataStructure)    delete m_metadataStructure;
    if (m_annotationStructure)  delete m_annotationStructure;
}

// static
CorpusRepository *CorpusRepository::create(const CorpusRepositoryDefinition &definition, QString &errorMessages, QObject *parent)
{
    errorMessages.clear();
    CorpusRepository *repository = new CorpusRepository(definition, parent);
    repository->metadataStructure()->addSection(CorpusObject::Type_Corpus, new MetadataStructureSection("corpus", "Corpus", "(Default section)"));
    repository->metadataStructure()->addSection(CorpusObject::Type_Communication, new MetadataStructureSection("communication", "Communication", "(Default section)"));
    repository->metadataStructure()->addSection(CorpusObject::Type_Recording, new MetadataStructureSection("recording", "Recording", "(Default section)"));
    repository->metadataStructure()->addSection(CorpusObject::Type_Annotation, new MetadataStructureSection("annotation", "Annotation", "(Default section)"));
    repository->metadataStructure()->addSection(CorpusObject::Type_Speaker, new MetadataStructureSection("speaker", "Speaker", "(Default section)"));
    repository->metadataStructure()->addSection(CorpusObject::Type_Participation, new MetadataStructureSection("participation", "Participation", "(Default section)"));
    if (repository->annotations()) {
        if (!repository->annotations()->createDatastore(definition.infoDatastoreAnnotations)) {
            errorMessages.append("Error creating annotation datastore: ").append(repository->annotations()->lastError()).append("\n");
            repository->annotations()->clearError();
        }
    }
    if (repository->metadata()) {
        if (!repository->metadata()->createDatastore(definition.infoDatastoreMetadata)) {
            errorMessages.append("Error creating metadata datastore: ").append(repository->metadata()->lastError()).append("\n");
            repository->metadata()->clearError();
        }
    }
    if (repository->files()) {
        repository->files()->setBasePath(definition.basePathMedia);
    }
    return repository;
}

// static
CorpusRepository *CorpusRepository::open(const CorpusRepositoryDefinition &definition, QString &errorMessages, QObject *parent)
{
    errorMessages.clear();
    QPointer<CorpusRepository> repository = new CorpusRepository(definition, parent);
    if (repository->annotations()) {
        if (!repository->annotations()->openDatastore(definition.infoDatastoreAnnotations)) {
            errorMessages.append("Error opening annotation datastore: ").append(repository->annotations()->lastError()).append("\n");
            repository->annotations()->clearError();
            return Q_NULLPTR;
        }
        if (!repository->annotations()->loadAnnotationStructure()) {
            errorMessages.append("Error reading annotation structure: ").append(repository->annotations()->lastError()).append("\n");
            repository->annotations()->clearError();
            return Q_NULLPTR;
        }
    }
    if (repository->metadata()) {
        if (!repository->metadata()->openDatastore(definition.infoDatastoreMetadata)) {
            errorMessages.append("Error opening metadata datastore: ").append(repository->metadata()->lastError()).append("\n");
            repository->metadata()->clearError();
            return Q_NULLPTR;
        }
        if (!repository->metadata()->loadMetadataStructure()) {
            errorMessages.append("Error reading metadata structure: ").append(repository->metadata()->lastError()).append("\n");
            repository->metadata()->clearError();
            return Q_NULLPTR;
        }
    }
    if (repository->files()) {
        repository->files()->setBasePath(definition.basePathMedia);
    }
    return repository;
}

void CorpusRepository::save()
{
    m_datastoreMetadata->saveMetadataStructure();
    m_datastoreAnnotations->saveAnnotationStructure();
}

void CorpusRepository::close()
{
    m_datastoreMetadata->closeDatastore();
    m_datastoreAnnotations->closeDatastore();
}

// ==============================================================================================================================
// Datastores
// ==============================================================================================================================

QPointer<AnnotationDatastore> CorpusRepository::annotations() const
{
    return m_datastoreAnnotations;
}

QPointer<MetadataDatastore> CorpusRepository::metadata() const
{
    return m_datastoreMetadata;
}

QPointer<FileDatastore> CorpusRepository::files() const
{
    return m_datastoreFiles;
}

// ==============================================================================================================================
// Importing metadata and annotation structure from other sources
// ==============================================================================================================================

void CorpusRepository::importMetadataStructure(MetadataStructure *otherStructure)
{
    if (!m_datastoreMetadata) return;
    if (!m_metadataStructure) return;

    const QMetaObject &mo = CorpusObject::staticMetaObject;
    int index = mo.indexOfEnumerator("Type");
    QMetaEnum metaEnum = mo.enumerator(index);
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        CorpusObject::Type type = static_cast<CorpusObject::Type>(metaEnum.value(i));
        MetadataStructureSection *mySection;
        foreach (MetadataStructureSection *otherSection, otherStructure->sections(type)) {
            mySection = m_metadataStructure->section(type, otherSection->ID());
            if (mySection) {
                // Copy over name and description, only if this structure does not already define them
                if (mySection->name().isEmpty())
                    mySection->setName(otherSection->name());
                if (mySection->description().isEmpty())
                    mySection->setDescription(otherSection->description());
                // Have section in common, check attributes.
                foreach (MetadataStructureAttribute *otherAttribute, otherSection->attributes()) {
                    MetadataStructureAttribute *myAttribute = mySection->attribute(otherAttribute->ID());
                    if (!myAttribute) {
                        myAttribute = new MetadataStructureAttribute(otherAttribute);
                        if (m_datastoreMetadata->createMetadataAttribute(type, myAttribute))
                            mySection->addAttribute(myAttribute);
                        else delete myAttribute;
                    }
                }
            }
            else {
                // Copy Section from other
                mySection = new MetadataStructureSection(otherSection->ID(), otherSection->name(), otherSection->description());
                m_metadataStructure->addSection(type, mySection);
                foreach (MetadataStructureAttribute *otherAttribute, otherSection->attributes()) {
                    MetadataStructureAttribute *myAttribute = new MetadataStructureAttribute(otherAttribute);
                    if (m_datastoreMetadata->createMetadataAttribute(type, myAttribute))
                        mySection->addAttribute(myAttribute);
                    else delete myAttribute;
                }
            }
        }
    }
    m_datastoreMetadata->saveMetadataStructure();
}

void CorpusRepository::importAnnotationStructure(AnnotationStructure *otherStructure)
{
    if (!m_datastoreAnnotations) return;
    if (!m_annotationStructure) return;
    AnnotationStructureLevel *myLevel;
    foreach (AnnotationStructureLevel *otherLevel, otherStructure->levels()) {
        myLevel = m_annotationStructure->level(otherLevel->ID());
        if (myLevel) {
            // Copy over name and description, only if this structure does not already define them
            if (myLevel->name().isEmpty())
                myLevel->setName(otherLevel->name());
            if (myLevel->description().isEmpty())
                myLevel->setDescription(otherLevel->description());
            // Have level in common, check attributes.
            foreach (AnnotationStructureAttribute *otherAttribute, otherLevel->attributes()) {
                AnnotationStructureAttribute *myAttribute = myLevel->attribute(otherAttribute->ID());
                if (!myAttribute) {
                    myAttribute = new AnnotationStructureAttribute(otherAttribute);
                    if (m_datastoreAnnotations->createAnnotationAttribute(myLevel->ID(), myAttribute))
                        myLevel->addAttribute(myAttribute);
                    else delete myAttribute;
                }
            }
        }
        else {
            // Copy level from other
            myLevel = new AnnotationStructureLevel(otherLevel->ID(), otherLevel->levelType(), otherLevel->name(),
                                                   otherLevel->description(), otherLevel->parentLevelID(),
                                                   otherLevel->datatype(), otherLevel->itemOrder(),
                                                   otherLevel->indexed(), otherLevel->nameValueList());
            if (m_datastoreAnnotations->createAnnotationLevel(myLevel)) {
                m_annotationStructure->addLevel(myLevel);
                foreach (AnnotationStructureAttribute *otherAttribute, otherLevel->attributes()) {
                    AnnotationStructureAttribute *myAttribute = new AnnotationStructureAttribute(otherAttribute);
                    if (m_datastoreAnnotations->createAnnotationAttribute(myLevel->ID(), myAttribute))
                        myLevel->addAttribute(myAttribute);
                    else delete myAttribute;
                }
            }
            else delete myLevel;
        }
    }
    m_datastoreAnnotations->saveAnnotationStructure();
}

} // namespace Core
} // namespace Praaline
