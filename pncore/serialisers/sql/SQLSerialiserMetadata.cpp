#include <QSqlDatabase>
#include "structure/MetadataStructure.h"
#include "datastore/CorpusRepository.h"
#include "SQLSerialiserMetadata.h"

namespace Praaline {
namespace Core {

// static
QList<CorpusObjectInfo> SQLSerialiserMetadata::getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID,
                                                                       QSqlDatabase &db, CorpusRepository *repository)
{
    QList<CorpusObjectInfo> list;
    return list;
}

// static
bool SQLSerialiserMetadata::loadCorpus(Corpus *corpus,
                                       QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                               QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                                         QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadRecordings(QList<QPointer<CorpusRecording> > &recordings,
                                           QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadAnnotations(QList<QPointer<CorpusAnnotation> > &annotations,
                                            QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveCorpus(Corpus *corpus,
                                       QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                               QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                                         QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveRecordings(QList<QPointer<CorpusRecording> > &recordings,
                                           QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveAnnotations(QList<QPointer<CorpusAnnotation> > &annotations,
                                            QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveParticipations(QList<QPointer<CorpusParticipation> > &participations,
                                               QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteCorpus(const QString &corpusID,
                                         QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteCommunication(const QString &communicationID,
                                                QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteSpeaker(const QString &speakerID,
                                          QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteRecording(const QString &recordingID,
                                            QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteAnnotation(const QString &annotationID,
                                             QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteParticipation(const QString &communicationID, const QString &speakerID,
                                                QSqlDatabase &db, MetadataStructure *structure, CorpusRepository *repository)
{
    return false;
}

} // namespace Core
} // namespace Praaline
