#ifndef SQLSERIALISERMETADATA_H
#define SQLSERIALISERMETADATA_H

#include <QPointer>
#include <QSqlDatabase>
#include "structure/MetadataStructure.h"
#include "corpus/CorpusObjectInfo.h"
#include "corpus/Corpus.h"
#include "corpus/CorpusCommunication.h"
#include "corpus/CorpusSpeaker.h"
#include "corpus/CorpusRecording.h"
#include "corpus/CorpusAnnotation.h"
#include "corpus/CorpusParticipation.h"

namespace Praaline {
namespace Core {

class SQLMetadataDatastore;

class SQLSerialiserMetadata
{
public:
    // Use this function to get the basic information for each corpus object and construct an hierarchical structure
    // of (lazy-loaded) corpus objects
    // parentID: (ignored) for Corpus, corpusID for Communication and Speaker, communicationID for Recording and Annotation
    static QList<CorpusObjectInfo> getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID,
                                                           SQLMetadataDatastore *datastore);

    // These functions load metadata information in already created corpus objects
    static bool loadCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                   MetadataStructure *structure, QSqlDatabase &db, CorpusRepository *repository);
    static bool loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                             MetadataStructure *structure, QSqlDatabase &db, CorpusRepository *repository);
    static bool loadRecordings(QList<QPointer<CorpusRecording> > &recordings,
                               MetadataStructure *structure, QSqlDatabase &db, CorpusRepository *repository);
    static bool loadAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations,
                                MetadataStructure *structure, QSqlDatabase &db, CorpusRepository *repository);

    // Save means insert or update, appropriately
    static bool saveCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                   MetadataStructure *structure, QSqlDatabase &db, CorpusRepository *repository);
    static bool saveSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                             MetadataStructure *structure, QSqlDatabase &db, CorpusRepository *repository);
    static bool saveRecordings(QList<QPointer<CorpusRecording> > &recordings,
                               MetadataStructure *structure, QSqlDatabase &db, CorpusRepository *repository);
    static bool saveAnnotations(QList<QPointer<CorpusAnnotation> > &annotations,
                                MetadataStructure *structure, QSqlDatabase &db, CorpusRepository *repository);
    static bool saveParticipations(QList<QPointer<CorpusParticipation> > &participations,
                                   MetadataStructure *structure, QSqlDatabase &db, CorpusRepository *repository);

    // Delete corpus objects
    static bool deleteCommunication(const QString &communicationID, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool deleteSpeaker(const QString &speakerID, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool deleteRecording(const QString &recordingID, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool deleteAnnotation(const QString &annotationID, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool deleteParticipation(const QString &communicationID, const QString &speakerID,
                                    QPointer<MetadataStructure> structure, QSqlDatabase &db);

private:
    SQLSerialiserMetadata() {}
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERMETADATA_H
