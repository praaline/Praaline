#ifndef SQLSERIALISERMETADATA_H
#define SQLSERIALISERMETADATA_H

#include <QPointer>
#include <QSqlDatabase>
#include "corpus/CorpusObjectInfo.h"
#include "corpus/Corpus.h"
#include "corpus/CorpusCommunication.h"
#include "corpus/CorpusSpeaker.h"
#include "corpus/CorpusRecording.h"
#include "corpus/CorpusAnnotation.h"
#include "corpus/CorpusParticipation.h"

namespace Praaline {
namespace Core {

class MetadataStructure;
class CorpusRepository;

class SQLSerialiserMetadata
{
public:
    // Use this function to get the basic information for each corpus object and construct an hierarchical structure
    // of (lazy-loaded) corpus objects
    // parentID: (ignored) for Corpus, corpusID for Communication and Speaker, communicationID for Recording and Annotation
    static QList<CorpusObjectInfo> getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID,
                                                           QSqlDatabase &db, CorpusRepository *repository);

    // These functions load metadata information in already created corpus objects
    static bool loadCorpus(Corpus *corpus,
                           QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool loadCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                   QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                             QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool loadRecordings(QList<QPointer<CorpusRecording> > &recordings,
                               QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool loadAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations,
                                QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);

    // Save means insert or update, appropriately
    static bool saveCorpus(Corpus *corpus,
                           QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool saveCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                   QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool saveSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                             QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool saveRecordings(QList<QPointer<CorpusRecording> > &recordings,
                               QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool saveAnnotations(QList<QPointer<CorpusAnnotation> > &annotations,
                                QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool saveParticipations(QList<QPointer<CorpusParticipation> > &participations,
                                   QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);

    // Delete corpus objects
    static bool deleteCorpus(const QString &corpusID,
                             QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool deleteCommunication(const QString &communicationID,
                                    QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool deleteSpeaker(const QString &speakerID,
                              QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool deleteRecording(const QString &recordingID,
                                QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool deleteAnnotation(const QString &annotationID,
                                 QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);
    static bool deleteParticipation(const QString &communicationID, const QString &speakerID,
                                    QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository);

private:
    SQLSerialiserMetadata() {}
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERMETADATA_H
