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

class SQLSerialiserMetadata
{
public:
    // Use this function to get the basic information for each corpus object and construct an hierarchical structure
    // of (lazy-loaded) corpus objects
    // parentID: (ignored) for Corpus, corpusID for Communication and Speaker, communicationID for Recording and Annotation
    static QList<CorpusObjectInfo> getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID, QSqlDatabase &db);

    // These functions load metadata information in already created corpus objects
    static bool loadCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                   QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                             QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool loadRecordings(QList<QPointer<CorpusRecording> > &recordings,
                               QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool loadAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations,
                                QPointer<MetadataStructure> structure, QSqlDatabase &db);

    // Save means insert or update, appropriately
    static bool saveCommunication(QPointer<CorpusCommunication> com, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool saveSpeaker(QPointer<CorpusSpeaker> spk, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool saveRecording(QPointer<CorpusRecording> rec, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool saveAnnotation(QPointer<CorpusAnnotation> annot, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool saveParticipation(QPointer<CorpusParticipation> participation, QPointer<MetadataStructure> structure, QSqlDatabase &db);

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
