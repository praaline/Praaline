#include "SQLSerialiserMetadata.h"

namespace Praaline {
namespace Core {

// static
QList<CorpusObjectInfo> SQLSerialiserMetadata::getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID,
                                                                       QSqlDatabase &db)
{
    QList<CorpusObjectInfo> list;
    return list;
}

// static
bool SQLSerialiserMetadata::loadCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                               QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                                         QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadRecordings(QList<QPointer<CorpusRecording> > &recordings,
                                           QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadAnnotations(QList<QPointer<CorpusAnnotation> > &annotations,
                                            QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveCommunication(QPointer<CorpusCommunication> com, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveSpeaker(QPointer<CorpusSpeaker> spk, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveRecording(QPointer<CorpusRecording> rec, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveAnnotation(QPointer<CorpusAnnotation> annot, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveParticipation(QPointer<CorpusParticipation> participation,
                                              QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteCommunication(const QString &communicationID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteSpeaker(const QString &speakerID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteRecording(const QString &recordingID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteAnnotation(const QString &annotationID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteParticipation(const QString &communicationID, const QString &speakerID,
                                                QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return false;
}

} // namespace Core
} // namespace Praaline
