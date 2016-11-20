#include "MocaDBSerialiserMetadata.h"

namespace Praaline {
namespace Core {

// static
QList<CorpusObjectInfo> MocaDBSerialiserMetadata::getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID,
                                                                          QSqlDatabase &db)
{
    QList<CorpusObjectInfo> list;
    return list;
}

// static
bool MocaDBSerialiserMetadata::loadCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                                  QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                                            QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::loadRecordings(QList<QPointer<CorpusRecording> > &recordings,
                                              QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::loadAnnotations(QList<QPointer<CorpusAnnotation> > &annotations,
                                               QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::saveCommunication(QPointer<CorpusCommunication> com, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::saveSpeaker(QPointer<CorpusSpeaker> spk, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::saveRecording(QPointer<CorpusRecording> rec, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::saveAnnotation(QPointer<CorpusAnnotation> annot, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::saveParticipation(QPointer<CorpusParticipation> participation,
                                                 QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::deleteCommunication(const QString &communicationID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::deleteSpeaker(const QString &speakerID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::deleteRecording(const QString &recordingID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::deleteAnnotation(const QString &annotationID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadata::deleteParticipation(const QString &communicationID, const QString &speakerID,
                                                   QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

} // namespace Core
} // namespace Praaline
