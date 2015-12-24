#include <QObject>
#include <QMetaEnum>
#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>
#include <QMultiHash>
#include <QPair>
#include <QDir>
#include "corpusobject.h"
#include "corpus.h"
#include "serialisers/datastorefactory.h"

Corpus::Corpus(const CorpusDefinition &definition, QObject *parent) :
    CorpusObject(definition.corpusID, parent)
{
    m_name = definition.corpusName;
    m_basePath = definition.basePath;
    m_baseMediaPath = definition.baseMediaPath;
    m_annotationStructure = new AnnotationStructure(this);
    m_metadataStructure = new MetadataStructure(this);
    m_datastoreAnnotations = DatastoreFactory::getAnnotationDatastore(definition.datastoreAnnotations, m_annotationStructure);
    m_datastoreMetadata = DatastoreFactory::getMetadataDatastore(definition.datastoreMetadata, m_metadataStructure);
}

Corpus::~Corpus()
{
    qDeleteAll(m_communications);
    qDeleteAll(m_speakers);
    if (m_annotationStructure) delete m_annotationStructure;
    if (m_metadataStructure) delete m_metadataStructure;
    if (m_datastoreAnnotations) delete m_datastoreAnnotations;
    if (m_datastoreMetadata) delete m_datastoreMetadata;
}

// ----------------------------------------------------------------------------------------------------------------
// CorpusObject implementation
// ----------------------------------------------------------------------------------------------------------------

void Corpus::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        setDirty(true);
    }
}

void Corpus::setCorpusID(const QString &corpusID)
{
    if (m_ID != corpusID) {
        m_ID = corpusID;
        m_corpusID = corpusID;
        foreach (QPointer<CorpusCommunication> com, m_communications) if (com) com->setCorpusID(corpusID);
        foreach (QPointer<CorpusSpeaker> spk, m_speakers) if (spk) spk->setCorpusID(corpusID);
        foreach (QPointer<CorpusParticipation> part, m_participationsByCommunication.values()) if (part) part->setCorpusID(corpusID);
        setDirty(true);
    }
}

QString Corpus::getRelativeToBasePath(const QString &absoluteFilePath) const
{
    if (m_basePath.isEmpty())
        return absoluteFilePath;
    return QDir(m_basePath).relativeFilePath(absoluteFilePath);
}

QString Corpus::getRelativeToBaseMediaPath (const QString &absoluteFilePath) const
{
    if (m_baseMediaPath.isEmpty())
        return absoluteFilePath;
    return QDir(m_baseMediaPath).relativeFilePath(absoluteFilePath);
}

void Corpus::setBaseMediaPath(const QString &path)
{
    m_baseMediaPath = QDir(path).absolutePath();
}

// ----------------------------------------------------------------------------------------------------------------
// CORPUS
// ----------------------------------------------------------------------------------------------------------------
// static
Corpus *Corpus::create(const CorpusDefinition &definition, QString &errorMessages, QObject *parent)
{
    errorMessages.clear();
    Corpus *corpus = new Corpus(definition, parent);
    corpus->metadataStructure()->addSection(CorpusObject::Type_Corpus, new MetadataStructureSection("corpus", "Corpus", "(Default section)"));
    corpus->metadataStructure()->addSection(CorpusObject::Type_Communication, new MetadataStructureSection("communication", "Communication", "(Default section)"));
    corpus->metadataStructure()->addSection(CorpusObject::Type_Recording, new MetadataStructureSection("recording", "Recording", "(Default section)"));
    corpus->metadataStructure()->addSection(CorpusObject::Type_Annotation, new MetadataStructureSection("annotation", "Annotation", "(Default section)"));
    corpus->metadataStructure()->addSection(CorpusObject::Type_Speaker, new MetadataStructureSection("speaker", "Speaker", "(Default section)"));
    corpus->metadataStructure()->addSection(CorpusObject::Type_Participation, new MetadataStructureSection("participation", "Participation", "(Default section)"));
    if (corpus->datastoreAnnotations()) {
        if (!corpus->datastoreAnnotations()->createDatastore(definition.datastoreAnnotations)) {
            errorMessages.append("Error creating annotation datastore: ").append(corpus->datastoreAnnotations()->lastError()).append("\n");
            corpus->datastoreAnnotations()->clearError();
        }
    }
    if (corpus->datastoreMetadata()) {
        if (!corpus->datastoreMetadata()->createDatastore(definition.datastoreMetadata)) {
            errorMessages.append("Error creating metadata datastore: ").append(corpus->datastoreMetadata()->lastError()).append("\n");
            corpus->datastoreMetadata()->clearError();
        }
    }
    return corpus;
}

// static
Corpus *Corpus::open(const CorpusDefinition &definition, QString &errorMessages, QObject *parent)
{
    errorMessages.clear();
    QPointer<Corpus> corpus = new Corpus(definition, parent);
    if (corpus->datastoreAnnotations()) {
        if (!corpus->datastoreAnnotations()->openDatastore(definition.datastoreAnnotations)) {
            errorMessages.append("Error opening annotation datastore: ").append(corpus->datastoreAnnotations()->lastError()).append("\n");
            corpus->datastoreAnnotations()->clearError();
            return 0;
        }
        if (!corpus->datastoreAnnotations()->loadAnnotationStructure()) {
            errorMessages.append("Error reading annotation structure: ").append(corpus->datastoreAnnotations()->lastError()).append("\n");
            corpus->datastoreAnnotations()->clearError();
            return 0;
        }
    }
    if (corpus->datastoreMetadata()) {
        if (!corpus->datastoreMetadata()->openDatastore(definition.datastoreMetadata)) {
            errorMessages.append("Error opening metadata datastore: ").append(corpus->datastoreMetadata()->lastError()).append("\n");
            corpus->datastoreMetadata()->clearError();
            return 0;
        }
        if (!corpus->datastoreMetadata()->loadMetadataStructure()) {
            errorMessages.append("Error reading metadata structure: ").append(corpus->datastoreMetadata()->lastError()).append("\n");
            corpus->datastoreMetadata()->clearError();
            return 0;
        }
        if (!corpus->datastoreMetadata()->loadCorpus(corpus)) {
            errorMessages.append("Error loading corpus: ").append(corpus->datastoreMetadata()->lastError()).append("\n");
            corpus->datastoreMetadata()->clearError();
            return 0;
        }
    }
    return corpus;
}

void Corpus::save()
{
    m_datastoreMetadata->saveMetadataStructure();
    m_datastoreMetadata->saveCorpus(this);
    m_datastoreAnnotations->saveAnnotationStructure();
}

void Corpus::saveAs(const CorpusDefinition &definition)
{
    Q_UNUSED(definition)
}

void Corpus::close()
{
    m_datastoreMetadata->closeDatastore();
    m_datastoreAnnotations->closeDatastore();
}

void Corpus::clear()
{
    m_name = "";
    m_basePath = "";
    qDeleteAll(m_communications);
    m_communications.clear();
    qDeleteAll(m_speakers);
    m_speakers.clear();
    m_metadataStructure.clear();
    m_annotationStructure.clear();
}

// Datastore

QPointer<AbstractAnnotationDatastore> Corpus::datastoreAnnotations() const
{
    return m_datastoreAnnotations;
}

QPointer<AbstractMetadataDatastore> Corpus::datastoreMetadata() const
{
    return m_datastoreMetadata;
}

// ==========================================================================================================
// STRUCTURE MANAGEMENT
// ==========================================================================================================

bool Corpus::createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute)
{
    if (!m_datastoreMetadata) return false;
    return m_datastoreMetadata->createMetadataAttribute(type, newAttribute);
}

bool Corpus::renameMetadataAttribute(CorpusObject::Type type, QString attributeID, QString newAttributeID)
{
    if (!m_datastoreMetadata) return false;
    return m_datastoreMetadata->renameMetadataAttribute(type, attributeID, newAttributeID);
}

bool Corpus::deleteMetadataAttribute(CorpusObject::Type type, QString attributeID)
{
    if (!m_datastoreMetadata) return false;
    return m_datastoreMetadata->deleteMetadataAttribute(type, attributeID);
}

// Annotations
bool Corpus::createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel)
{
    if (!m_datastoreAnnotations) return false;
    return m_datastoreAnnotations->createAnnotationLevel(newLevel);
}

bool Corpus::renameAnnotationLevel(QString levelID, QString newLevelID)
{
    if (!m_datastoreAnnotations) return false;
    return m_datastoreAnnotations->renameAnnotationLevel(levelID, newLevelID);
}

bool Corpus::deleteAnnotationLevel(QString levelID)
{
    if (!m_datastoreAnnotations) return false;
    return m_datastoreAnnotations->deleteAnnotationLevel(levelID);
}

bool Corpus::createAnnotationAttribute(QString levelID, QPointer<AnnotationStructureAttribute> newAttribute)
{
    if (!m_datastoreAnnotations) return false;
    return m_datastoreAnnotations->createAnnotationAttribute(levelID, newAttribute);
}

bool Corpus::renameAnnotationAttribute(QString levelID, QString attributeID, QString newAttributeID)
{
    if (!m_datastoreAnnotations) return false;
    return m_datastoreAnnotations->renameAnnotationAttribute(levelID, attributeID, newAttributeID);
}

bool Corpus::deleteAnnotationAttribute(QString levelID, QString attributeID)
{
    if (!m_datastoreAnnotations) return false;
    return m_datastoreAnnotations->deleteAnnotationAttribute(levelID, attributeID);
}

bool Corpus::retypeAnnotationAttribute(QString levelID, QString attributeID, QString newDatatype, int newDatalength)
{
    if (!m_datastoreAnnotations) return false;
    return m_datastoreAnnotations->retypeAnnotationAttribute(levelID, attributeID, newDatatype, newDatalength);
}

void Corpus::importMetadataStructure(MetadataStructure *otherStructure)
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

void Corpus::importAnnotationStructure(AnnotationStructure *otherStructure)
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
                                                   otherLevel->datatype(), otherLevel->datalength(), otherLevel->indexed(), otherLevel->nameValueList());
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

// ==========================================================================================================
// ITEM MANAGEMENT
// ==========================================================================================================

// ==========================================================================================================
// Communications
// ==========================================================================================================

QPointer<CorpusCommunication> Corpus::communication(const QString &communicationID) const
{
    return m_communications.value(communicationID, 0);
}

int Corpus::communicationsCount() const
{
    return m_communications.count();
}

bool Corpus::hasCommunications() const
{
    return !m_communications.isEmpty();
}

bool Corpus::hasCommunication(const QString &communicationID) const
{
    return m_communications.contains(communicationID);
}

QStringList Corpus::communicationIDs() const
{
    return m_communications.keys();
}

const QMap<QString, QPointer<CorpusCommunication> > &Corpus::communications() const
{
    return m_communications;
}

void Corpus::addCommunication(CorpusCommunication *communication)
{
    if (!communication) return;
    communication->setParent(this);
    communication->setCorpusID(this->ID());
    m_communications.insert(communication->ID(), communication);
    // connect(communication, SIGNAL(changedID(QString,QString)), this, SLOT(communicationChangedID(QString,QString)));
    setDirty(true);
    emit corpusCommunicationAdded(communication);
}

void Corpus::removeCommunication(const QString &communicationID)
{
    if (!m_communications.contains(communicationID)) return;
    QPointer<CorpusCommunication> communication = m_communications.value(communicationID);
    // remove related participations before removing speaker
    QList<QPointer<CorpusParticipation> > participationsToDelete;
    participationsToDelete = m_participationsByCommunication.values(communicationID);
    foreach (QPointer<CorpusParticipation> participation, participationsToDelete)
        removeParticipation(participation->communicationID(), participation->speakerID());
    // remove communication
    m_communications.remove(communicationID);
    delete communication;
    setDirty(true);
    deletedCommunicationIDs << communicationID;
    emit corpusCommunicationDeleted(communicationID);
}

void Corpus::communicationChangedID(const QString &oldID, const QString &newID)
{
    if (oldID == newID) return;
    QPointer<CorpusCommunication> com = m_communications.value(oldID);
    if (!com) return;
    m_communications.remove(oldID);
    m_communications.insert(newID, com);
    // participations
    foreach (QPointer<CorpusParticipation> participation, m_participationsByCommunication.values(oldID)) {
        m_participationsByCommunication.insert(newID, participation);
    }
    m_participationsByCommunication.remove(oldID);
}

// ==========================================================================================================
// Speakers
// ==========================================================================================================

QPointer<CorpusSpeaker> Corpus::speaker(const QString &speakerID) const
{
    return m_speakers.value(speakerID, 0);
}

int Corpus::speakersCount() const
{
    return m_speakers.count();
}

bool Corpus::hasSpeakers() const
{
    return !m_speakers.isEmpty();
}

bool Corpus::hasSpeaker(const QString &speakerID) const
{
    return m_speakers.contains(speakerID);
}

QStringList Corpus::speakerIDs() const
{
    return m_speakers.keys();
}

const QMap<QString, QPointer<CorpusSpeaker> > &Corpus::speakers() const
{
    return m_speakers;
}

void Corpus::addSpeaker(CorpusSpeaker *speaker)
{
    if (!speaker) return;
    speaker->setParent(this);
    speaker->setCorpusID(this->ID());
    m_speakers.insert(speaker->ID(), speaker);
    connect(speaker, SIGNAL(changedID(QString,QString)), this, SLOT(speakerChangedID(QString,QString)));
    setDirty(true);
    emit corpusSpeakerAdded(speaker);
}

void Corpus::removeSpeaker(const QString &speakerID)
{
    QPointer<CorpusSpeaker> speaker = m_speakers.value(speakerID, 0);
    if (!speaker) return;
    // remove related participations before removing speaker
    QList<QPointer<CorpusParticipation> > participationsToDelete;
    participationsToDelete = m_participationsBySpeaker.values(speakerID);
    foreach (QPointer<CorpusParticipation> participation, participationsToDelete) {
        if (!participation) continue;
        removeParticipation(participation->communicationID(), participation->speakerID());
    }
    // remove speaker
    m_speakers.remove(speakerID);
    delete speaker;
    setDirty(true);
    deletedSpeakerIDs << speakerID;
    emit corpusSpeakerDeleted(speakerID);
}

void Corpus::speakerChangedID(const QString &oldID, const QString &newID)
{
    if (oldID == newID) return;
    QPointer<CorpusSpeaker> spk = m_speakers.value(oldID);
    if (!spk) return;
    m_speakers.remove(oldID);
    m_speakers.insert(newID, spk);
    // participations
    foreach (QPointer<CorpusParticipation> participation, m_participationsBySpeaker.values(oldID)) {
        m_participationsBySpeaker.insert(newID, participation);
    }
    m_participationsBySpeaker.remove(oldID);
}

// ==========================================================================================================
// Participation of Speakers in Communications
// ==========================================================================================================
QPointer<CorpusParticipation> Corpus::participation(const QString &communicationID, const QString &speakerID)
{
    foreach (QPointer<CorpusParticipation> participation, m_participationsByCommunication.values(communicationID)) {
        if (participation && participation->speakerID() == speakerID)
            return participation;
    }
    return 0;
}

bool Corpus::hasParticipation(const QString &communicationID, const QString &speakerID)
{
    foreach (QPointer<CorpusParticipation> participation, m_participationsByCommunication.values(communicationID)) {
        if (participation && participation->speakerID() == speakerID)
            return true;
    }
    return false;
}

QList<QPointer<CorpusParticipation> > Corpus::participations()
{
    return m_participationsByCommunication.values();
}

QList<QPointer<CorpusParticipation> > Corpus::participationsForCommunication(const QString &communicationID)
{
    return m_participationsByCommunication.values(communicationID);
}

QList<QPointer<CorpusParticipation> > Corpus::participationsForSpeaker(const QString &speakerID)
{
    return m_participationsBySpeaker.values(speakerID);
}

QPointer<CorpusParticipation> Corpus::addParticipation(const QString &communicationID, const QString &speakerID, const QString &role)
{
    QPointer<CorpusCommunication> com = this->communication(communicationID);
    QPointer<CorpusSpeaker> spk = this->speaker(speakerID);
    if (!com || !spk) return 0;
    CorpusParticipation *participation = new CorpusParticipation(com, spk, role, this);
    participation->setCorpusID(this->ID());
    m_participationsByCommunication.insert(communicationID, participation);
    m_participationsBySpeaker.insert(speakerID, participation);
    return participation;
}

void Corpus::removeParticipation(const QString &communicationID, const QString &speakerID)
{
    QList<QPointer<CorpusParticipation> > participationsToDelete;
    foreach (QPointer<CorpusParticipation> participation, m_participationsByCommunication.values(communicationID)) {
        if (participation && participation->speakerID() == speakerID) {
            m_participationsByCommunication.remove(communicationID, participation);
            participationsToDelete << participation;
        }
    }
    foreach (QPointer<CorpusParticipation> participation, m_participationsBySpeaker.values(speakerID)) {
        if (participation && participation->communicationID() == communicationID) {
            m_participationsBySpeaker.remove(speakerID, participation);
        }
    }
    foreach (QPointer<CorpusParticipation> participation, participationsToDelete) {
        if (participation) delete participation;
    }
    deletedParticipationIDs << QPair<QString, QString>(communicationID, speakerID);
}

// ==========================================================================================================
// Other
// ==========================================================================================================

QStringList Corpus::recordingIDs() const
{
    QStringList list;
    foreach(QPointer<CorpusCommunication> com, m_communications) {
        foreach(QPointer<CorpusRecording> rec, com->recordings())
            if (rec) list << rec->ID();
    }
    return list;
}

QStringList Corpus::annotationIDs() const
{
    QStringList list;
    foreach(QPointer<CorpusCommunication> com, m_communications) {
        foreach(QPointer<CorpusAnnotation> annot, com->annotations())
            if (annot) list << annot->ID();
    }
    return list;
}

QList<QPointer<CorpusCommunication> > Corpus::communicationsList() const
{
    return m_communications.values();
}

QList<QPointer<CorpusSpeaker> > Corpus::speakersList() const
{
    return m_speakers.values();
}

QList<QPointer<CorpusRecording> > Corpus::recordingsList() const
{
    QList<QPointer<CorpusRecording> > list;
    foreach(QPointer<CorpusCommunication> com, m_communications) {
        foreach(QPointer<CorpusRecording> rec, com->recordings())
            if (rec) list << rec;
    }
    return list;
}

QList<QPointer<CorpusAnnotation> > Corpus::annotationsList() const
{
    QList<QPointer<CorpusAnnotation> > list;
    foreach(QPointer<CorpusCommunication> com, m_communications) {
        foreach(QPointer<CorpusAnnotation> annot, com->annotations())
            if (annot) list << annot;
    }
    return list;
}

QMap<QString, QPair<QPointer<CorpusRecording>, QPointer<CorpusAnnotation> > > Corpus::getRecordings_x_Annotations() const
{
    QMap<QString, QPair<QPointer<CorpusRecording>, QPointer<CorpusAnnotation> > > list;
    foreach(QPointer<CorpusCommunication> com, m_communications) {
        if (!com) continue;
        foreach(QPointer<CorpusRecording> rec, com->recordings()) {
            foreach(QPointer<CorpusAnnotation> annot, com->annotations()) {
                if (!rec || !annot) continue;
                QPair<QPointer<CorpusRecording>, QPointer<CorpusAnnotation> > item(rec, annot);
                list.insert(rec->ID(), item);
            }
        }
    }
    return list;
}

QMap<QString, QPair<QPointer<CorpusAnnotation>, QPointer<CorpusRecording> > > Corpus::getAnnotations_x_Recordings() const
{
    QMap<QString, QPair<QPointer<CorpusAnnotation>, QPointer<CorpusRecording> > > list;
    foreach(QPointer<CorpusCommunication> com, m_communications) {
        if (!com) continue;
        foreach(QPointer<CorpusAnnotation> annot, com->annotations()) {
            foreach(QPointer<CorpusRecording> rec, com->recordings()) {
                if (!rec || !annot) continue;
                QPair<QPointer<CorpusAnnotation>, QPointer<CorpusRecording> > item(annot, rec);
                list.insert(annot->ID(), item);
            }
        }
    }
    return list;
}

QList<QPair<QString, QString> > Corpus::getCommunicationsAnnotationsIDs() const
{
    QList<QPair<QString, QString> > list;
    foreach(QPointer<CorpusCommunication> com, m_communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            list << QPair<QString, QString>(com->ID(), annot->ID());
        }
    }
    return list;
}

QList<QPair<QString, QString> > Corpus::getCommunicationsRecordingsIDs() const
{
    QList<QPair<QString, QString> > list;
    foreach(QPointer<CorpusCommunication> com, m_communications) {
        if (!com) continue;
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            if (!rec) continue;
            list << QPair<QString, QString>(com->ID(), rec->ID());
        }
    }
    return list;
}

// ====================================================================================================================
// Import corpus from another corpus

void Corpus::importCorpus(Corpus *corpusSource, const QString &prefix)
{
    if (!corpusSource) return;

    // Import and merge metadata and annotation structure
    importMetadataStructure(corpusSource->metadataStructure());
    importAnnotationStructure(corpusSource->annotationStructure());
    save();

    // Copy over corpus items
    foreach (QPointer<CorpusSpeaker> spk, corpusSource->speakers()) {
        CorpusSpeaker *destSpk = new CorpusSpeaker(spk);
        destSpk->setID(prefix + spk->ID());
        this->addSpeaker(destSpk);
    }
    foreach (QPointer<CorpusCommunication> com, corpusSource->communications()) {
        if (!com) continue;
        CorpusCommunication *destCom = new CorpusCommunication(com);
        destCom->setID(prefix + com->ID());
        this->addCommunication(destCom);
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            if (!rec) continue;
            CorpusRecording *destRec = new CorpusRecording(rec);
            destRec->setID(prefix + rec->ID());
            destCom->addRecording(destRec);
            // copy over media file
            QFileInfo infoDest(baseMediaPath() + "/" + destRec->filename());
            infoDest.absoluteDir().mkpath(infoDest.absolutePath());
            QFile::copy(corpusSource->baseMediaPath() + "/" + rec->filename(),
                        baseMediaPath() + "/" + destRec->filename());
        }
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            CorpusAnnotation *destAnnot = new CorpusAnnotation(annot);
            destAnnot->setID(prefix + annot->ID());
            destCom->addAnnotation(destAnnot);
            QMap<QString, QPointer<AnnotationTierGroup> > data =
                    corpusSource->datastoreAnnotations()->getTiersAllSpeakers(annot->ID());
            datastoreAnnotations()->saveTiersAllSpeakers(destAnnot->ID(), data);
        }
        foreach (QPointer<CorpusParticipation> participation, corpusSource->participationsForCommunication(com->ID())) {
            if (!participation) continue;
            QPointer<CorpusSpeaker> destSpk = speaker(prefix + participation->speakerID());
            if (!destSpk) continue;
            CorpusParticipation *destParticipcation = addParticipation(destCom->ID(), destSpk->ID(), participation->role());
            if (destParticipcation) destParticipcation->copyProperties(participation);
        }
    }
}
