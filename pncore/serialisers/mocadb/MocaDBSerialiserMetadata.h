#ifndef MOCADBSERIALISERMETADATA_H
#define MOCADBSERIALISERMETADATA_H

#include <QPointer>
#include <QSqlDatabase>
#include "base/IDatastore.h"
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

class MocaDBSerialiserMetadata : public IDatastore
{
public:
    // Use this function to get the basic information for each corpus object and construct an hierarchical structure
    // of (lazy-loaded) corpus objects
    // parentID: (ignored) for Corpus, corpusID for Communication and Speaker, communicationID for Recording and Annotation
    static QList<CorpusObjectInfo> getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID, QSqlDatabase &db);

    // These functions load metadata information in already created corpus objects
    static bool loadCommunication(QPointer<CorpusCommunication> communication, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool loadSpeaker(QPointer<CorpusSpeaker> speaker, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool loadRecording(QPointer<CorpusRecording> recording, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool loadAnnotation(QPointer<CorpusAnnotation> annotation, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool loadParticipation(QPointer<CorpusParticipation> participation, QPointer<MetadataStructure> structure, QSqlDatabase &db);

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

protected:
    static bool loadCorpusObjectMetadata(CorpusObject *obj, QPointer<MetadataStructure> structure, QSqlDatabase &db);
    static bool saveCorpusObject(CorpusObject *obj, QPointer<MetadataStructure> structure, QSqlDatabase &db);

private:
    MocaDBSerialiserMetadata() {}
};

} // namespace Core
} // namespace Praaline

#endif // MOCADBSERIALISERMETADATA_H
