#ifndef SQLSERIALISERCORPUS_H
#define SQLSERIALISERCORPUS_H

#include <QSqlDatabase>
#include "sqlserialiserbase.h"
#include "structure/metadatastructure.h"
#include "corpus/corpus.h"
#include "corpus/corpuscommunication.h"
#include "corpus/corpusspeaker.h"
#include "corpus/corpusrecording.h"
#include "corpus/corpusannotation.h"
#include "corpus/corpusparticipation.h"

class SQLSerialiserCorpus : public SQLSerialiserBase
{
public:
    static bool saveCorpus(Corpus *corpus, QSqlDatabase &db);
    static bool loadCorpus(Corpus *corpus, QSqlDatabase &db);

    static bool saveCommunication(CorpusCommunication *com, MetadataStructure *structure, QSqlDatabase &db);
    static bool saveSpeaker(CorpusSpeaker *spk, MetadataStructure *structure, QSqlDatabase &db);
    static bool saveParticipation(CorpusParticipation *participation, MetadataStructure *structure, QSqlDatabase &db);

    static QList<CorpusCommunication *> getCommunications(QString corpusID, MetadataStructure *structure, QSqlDatabase &db);
    static QList<CorpusSpeaker *> getSpeakers(QString corpusID, MetadataStructure *structure, QSqlDatabase &db);
    static QList<CorpusRecording *> getRecordings(QString communicationID, MetadataStructure *structure, QSqlDatabase &db);
    static QList<CorpusAnnotation *> getAnnotations(QString communicationID, MetadataStructure *structure, QSqlDatabase &db);
    static QMultiMap<QString, CorpusRecording *> getRecordingsAll(QString corpusID, MetadataStructure *structure, QSqlDatabase &db);
    static QMultiMap<QString, CorpusAnnotation *> getAnnotationsAll(QString corpusID, MetadataStructure *structure, QSqlDatabase &db);

private:
    SQLSerialiserCorpus();

    static void prepareLoadStatements(MetadataStructure *structure, QSqlDatabase &db);
    static void prepareSaveStatements(MetadataStructure *structure, QSqlDatabase &db);

    static QList<CorpusCommunication *> getCommunications(QString corpusID);
    static QList<CorpusSpeaker *> getSpeakers(QString corpusID);
    static QList<CorpusRecording *> getRecordings(QString communicationID);
    static QList<CorpusAnnotation *> getAnnotations(QString communicationID);
    static QMultiMap<QString, CorpusRecording *> getRecordingsAll(QString corpusID);
    static QMultiMap<QString, CorpusAnnotation *> getAnnotationsAll(QString corpusID);
    static void readParticipations(Corpus *corpus);

    static bool saveCommunication(CorpusCommunication *com);
    static bool saveSpeaker(CorpusSpeaker *spk);
    static bool saveRecording(QString communicationID, CorpusRecording *rec);
    static bool saveAnnotation(QString communicationID, CorpusAnnotation *annot);
    static bool saveParticipation(CorpusParticipation *participation);
    static bool cleanUpCommunication(CorpusCommunication *com);

    static MetadataStructure *preparedMetadataStructure;
    static QSqlQuery queryCommunicationSelect, queryCommunicationInsert, queryCommunicationUpdate, queryCommunicationDelete;
    static QSqlQuery queryCommunicationRecordingDelete, queryCommunicationAnnotationDelete;
    static QSqlQuery querySpeakerSelect, querySpeakerInsert, querySpeakerUpdate, querySpeakerDelete;
    static QSqlQuery queryRecordingSelect, queryRecordingSelectAll, queryRecordingInsert, queryRecordingUpdate, queryRecordingDelete;
    static QSqlQuery queryAnnotationSelect, queryAnnotationSelectAll, queryAnnotationInsert, queryAnnotationUpdate, queryAnnotationDelete;
    static QSqlQuery queryParticipationSelect, queryParticipationInsert, queryParticipationUpdate, queryParticipationDelete;
};

#endif // SQLSERIALISERCORPUS_H
