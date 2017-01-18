#ifndef SQLSERIALISERMETADATA_H
#define SQLSERIALISERMETADATA_H

#include <QPointer>
#include <QSqlDatabase>
#include "base/IDatastore.h"
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
class CorpusDatastore;

class SQLSerialiserMetadata : public IDatastore
{
public:
    // Use this function to get the basic information for each corpus object and construct an hierarchical structure
    // of (lazy-loaded) corpus objects
    // parentID: (ignored) for Corpus, corpusID for Communication and Speaker, communicationID for Recording and Annotation
    static QList<CorpusObjectInfo> getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID,
                                                           QSqlDatabase &db, CorpusDatastore *datastore);

    // These functions load metadata information in already created corpus objects
    static bool loadCorpus(Corpus *corpus,
                           QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool loadCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                   QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                             QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool loadRecordings(QList<QPointer<CorpusRecording> > &recordings,
                               QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool loadAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations,
                                QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);

    // Save means insert or update, appropriately
    static bool saveCorpus(Corpus *corpus,
                           QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool saveCommunication(CorpusCommunication *communication,
                                  QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool saveSpeaker(CorpusSpeaker *speaker,
                            QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool saveRecording(CorpusRecording *recording,
                              QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool saveAnnotation(CorpusAnnotation *annotation,
                               QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool saveParticipation(CorpusParticipation *participation,
                                  QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);

    // Delete corpus objects
    static bool deleteCorpus(const QString &corpusID,
                             QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool deleteCommunication(const QString &communicationID,
                                    QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool deleteSpeaker(const QString &speakerID,
                              QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool deleteRecording(const QString &recordingID,
                                QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool deleteAnnotation(const QString &annotationID,
                                 QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);
    static bool deleteParticipation(const QString &communicationID, const QString &speakerID,
                                    QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore);

private:
    SQLSerialiserMetadata() {}

    static QString prepareInsertSQL(MetadataStructure *structure, CorpusObject::Type what);
    static QString prepareUpdateSQL(MetadataStructure *structure, CorpusObject::Type what);
    static bool execSaveCommunication(CorpusCommunication *com, MetadataStructure *structure, QSqlDatabase &db);
    static bool execSaveSpeaker(CorpusSpeaker *spk, MetadataStructure *structure, QSqlDatabase &db);
    static bool execSaveRecording(QString communicationID, CorpusRecording *rec, MetadataStructure *structure, QSqlDatabase &db);
    static bool execSaveAnnotation(QString communicationID, CorpusAnnotation *annot, MetadataStructure *structure, QSqlDatabase &db);
    static bool execSaveParticipation(CorpusParticipation *participation, MetadataStructure *structure, QSqlDatabase &db);
    static bool execCleanUpCommunication(CorpusCommunication *com, QSqlDatabase &db);
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERMETADATA_H
