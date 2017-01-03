#ifndef CORPUS_H
#define CORPUS_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QMultiMap>
#include "CorpusObject.h"
#include "CorpusCommunication.h"
#include "CorpusSpeaker.h"
#include "CorpusParticipation.h"
#include "structure/AnnotationStructure.h"
#include "structure/MetadataStructure.h"
#include "serialisers/AnnotationDatastore.h"
#include "serialisers/DatastoreFactory.h"
#include "serialisers/CorpusDefinition.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT Corpus : public CorpusObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
public:
    Corpus(const CorpusDefinition &definition, QObject *parent = 0);
    ~Corpus();

    // ----------------------------------------------------------------------------------------------------------------
    // CorpusObject implementation
    // ----------------------------------------------------------------------------------------------------------------
    CorpusObject::Type type() const { return CorpusObject::Type_Corpus; }

    QString name() const { return m_name; }
    void setName(const QString &name);

    QString basePath() const { return m_basePath; }
    QString getRelativeToBasePath (const QString &absoluteFilePath) const;

    QString baseMediaPath() const { return m_baseMediaPath; }
    void setBaseMediaPath(const QString &path);
    QString getRelativeToBaseMediaPath (const QString &absoluteFilePath) const;

    // Override corpusID = ID
    QString corpusID() const { return m_ID; }
    void setCorpusID(const QString &corpusID);

    // ----------------------------------------------------------------------------------------------------------------
    // CORPUS
    // ----------------------------------------------------------------------------------------------------------------
    static Corpus *create(const CorpusDefinition &definition, QString &errorMessages, QObject *parent = 0);
    static Corpus *open(const CorpusDefinition &definition, QString &errorMessages, QObject *parent = 0);
    void save();
    void saveAs(const CorpusDefinition &definition);
    void close();
    void clear();
    void importCorpus(Corpus *corpusSource, const QString &prefix);

    // Datastore
    QPointer<AnnotationDatastore> datastoreAnnotations() const;
    QPointer<MetadataDatastore> datastoreMetadata() const;

    // ----------------------------------------------------------------------------------------------------------------
    // STRUCTURE
    // ----------------------------------------------------------------------------------------------------------------
    QPointer<MetadataStructure> metadataStructure() const { return m_metadataStructure; }
    QPointer<AnnotationStructure> annotationStructure() const { return m_annotationStructure; }
    // Structural changes that have an impact on corpus objects and the data stores
    // Metadata
    bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute);
    bool renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID);
    bool deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID);
    void importMetadataStructure(MetadataStructure *otherStructure);
    // Annotations
    bool createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel);
    bool renameAnnotationLevel(const QString &levelID, const QString &newLevelID);
    bool deleteAnnotationLevel(const QString &levelID);
    bool createAnnotationAttribute(const QString &levelID, QPointer<AnnotationStructureAttribute> newAttribute);
    bool renameAnnotationAttribute(const QString &levelID, const QString &attributeID, const QString &newAttributeID);
    bool deleteAnnotationAttribute(const QString &levelID, const QString &attributeID);
    bool retypeAnnotationAttribute(const QString &levelID, const QString &attributeID, const DataType &newDatatype);
    void importAnnotationStructure(AnnotationStructure *otherStructure);

    // ----------------------------------------------------------------------------------------------------------------
    // COMMUNICATIONS
    // ----------------------------------------------------------------------------------------------------------------
    QPointer<CorpusCommunication> communication(const QString &communicationID) const;
    int communicationsCount() const;
    bool hasCommunications() const;
    bool hasCommunication(const QString &communicationID) const;
    QStringList communicationIDs() const;
    const QMap<QString, QPointer<CorpusCommunication> > &communications() const;
    void addCommunication(CorpusCommunication *communication);
    void removeCommunication(const QString &communicationID);
    // ----------------------------------------------------------------------------------------------------------------
    // SPEAKERS
    // ----------------------------------------------------------------------------------------------------------------
    QPointer<CorpusSpeaker> speaker(const QString &speakerID) const;
    int speakersCount() const;
    bool hasSpeakers() const;
    bool hasSpeaker(const QString &speakerID) const;
    QStringList speakerIDs() const;
    const QMap<QString, QPointer<CorpusSpeaker> > &speakers() const;
    void addSpeaker(CorpusSpeaker *speaker);
    void removeSpeaker(const QString &speakerID);
    // ----------------------------------------------------------------------------------------------------------------
    // SPEAKER PARTICIPATION IN COMMUNICATIONS
    // ----------------------------------------------------------------------------------------------------------------
    QPointer<CorpusParticipation> participation(const QString &communicationID, const QString &speakerID);
    bool hasParticipation(const QString &communicationID, const QString &speakerID);
    QList<QPointer<CorpusParticipation> > participations();
    QList<QPointer<CorpusParticipation> > participationsForCommunication(const QString &communicationID);
    QList<QPointer<CorpusParticipation> > participationsForSpeaker(const QString &speakerID);
    QPointer<CorpusParticipation> addParticipation(const QString &communicationID, const QString &speakerID, const QString &role = QString());
    void removeParticipation(const QString &communicationID, const QString &speakerID);

    // ----------------------------------------------------------------------------------------------------------------
    // Cross recordings x annotations
    // ----------------------------------------------------------------------------------------------------------------
    QStringList recordingIDs() const;
    QStringList annotationIDs() const;
    QList<QPointer<CorpusCommunication> > communicationsList() const;
    QList<QPointer<CorpusSpeaker> > speakersList() const;
    QList<QPointer<CorpusRecording> > recordingsList() const;
    QList<QPointer<CorpusAnnotation> > annotationsList() const;
    QMap<QString, QPair<QPointer<CorpusRecording>, QPointer<CorpusAnnotation> > > getRecordings_x_Annotations() const;
    QMap<QString, QPair<QPointer<CorpusAnnotation>, QPointer<CorpusRecording> > > getAnnotations_x_Recordings() const;
    QList<QPair<QString, QString> > getCommunicationsAnnotationsIDs() const;
    QList<QPair<QString, QString> > getCommunicationsRecordingsIDs() const;

    // Database helpers
    QList<QString> deletedCommunicationIDs;
    QList<QString> deletedSpeakerIDs;
    QList<QPair<QString, QString> > deletedParticipationIDs;

signals:
    void corpusChanged();
    void corpusCommunicationAdded(QPointer<Praaline::Core::CorpusCommunication> communication);
    void corpusCommunicationDeleted(QString ID);
    void corpusSpeakerAdded(QPointer<Praaline::Core::CorpusSpeaker> speaker);
    void corpusSpeakerDeleted(QString ID);

public slots:

protected:
    QString m_name;
    QString m_basePath;
    QString m_baseMediaPath;
    QMap<QString, QPointer<CorpusCommunication> > m_communications;
    QMap<QString, QPointer<CorpusSpeaker> > m_speakers;
    QMultiMap<QString, QPointer<CorpusParticipation> > m_participationsByCommunication;
    QMultiMap<QString, QPointer<CorpusParticipation> > m_participationsBySpeaker;
    QPointer<AnnotationStructure> m_annotationStructure;
    QPointer<MetadataStructure> m_metadataStructure;
    QPointer<AnnotationDatastore> m_datastoreAnnotations;
    QPointer<MetadataDatastore> m_datastoreMetadata;

private slots:
    void communicationChangedID(const QString &oldID, const QString &newID);
    void speakerChangedID(const QString &oldID, const QString &newID);

private:

};

} // namespace Core
} // namespace Praaline

#endif // CORPUS_H
