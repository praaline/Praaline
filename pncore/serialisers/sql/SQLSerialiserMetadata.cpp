#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "structure/MetadataStructure.h"
#include "datastore/CorpusRepository.h"
#include "datastore/CorpusDatastore.h"
#include "SQLSerialiserMetadata.h"

namespace Praaline {
namespace Core {

// static
QList<CorpusObjectInfo> SQLSerialiserMetadata::getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID,
                                                                       QSqlDatabase &db, CorpusDatastore *datastore)
{
    QList<CorpusObjectInfo> list;


    return list;
}

// static
bool SQLSerialiserMetadata::loadCorpus(Corpus *corpus,
                                       QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadCommunications(QList<QPointer<CorpusCommunication> > &communications,
                                               QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
                                         QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadRecordings(QList<QPointer<CorpusRecording> > &recordings,
                                           QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::loadAnnotations(QList<QPointer<CorpusAnnotation> > &annotations,
                                            QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// ==============================================================================================================================
// Insert and Update Corpus Objects
// ==============================================================================================================================

// static private
QString SQLSerialiserMetadata::prepareInsertSQL(MetadataStructure *structure, CorpusObject::Type what)
{
    QString sql1, sql2;
    if (what == CorpusObject::Type_Communication) {
        sql1 = "INSERT INTO communication (communicationID, corpusID, communicationName";
        sql2 = "VALUES (:communicationID, :corpusID, :communicationName";
    }
    else if (what == CorpusObject::Type_Speaker) {
        sql1 = "INSERT INTO speaker (speakerID, corpusID, speakerName";
        sql2 = "VALUES (:speakerID, :corpusID, :speakerName";
    }
    else if (what == CorpusObject::Type_Recording) {
        sql1 = "INSERT INTO recording (recordingID, communicationID, recordingName, filename, format, duration, channels, "
               "sampleRate, precisionBits, bitRate, encoding, fileSize, checksumMD5";
        sql2 = "VALUES (:recordingID, :communicationID, :recordingName, :filename, :format, :duration, :channels, "
                ":sampleRate, :precisionBits, :bitRate, :encoding, :fileSize, :checksumMD5";
    }
    else if (what == CorpusObject::Type_Annotation) {
        sql1 = "INSERT INTO annotation (annotationID, communicationID, recordingID, annotationName";
        sql2 = "VALUES (:annotationID, :communicationID, :recordingID, :annotationName";
    }
    else if (what == CorpusObject::Type_Participation) {
        sql1 = "INSERT INTO participation (corpusID, communicationID, speakerID, role";
        sql2 = "VALUES (:corpusID, :communicationID, :speakerID, :role";
    }
    foreach (MetadataStructureSection *section, structure->sections(what)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            sql1.append(", ").append(attribute->ID());
            sql2.append(", :").append(attribute->ID());
        }
    }
    sql1.append(")");
    sql2.append(")");
    return QString("%1 %2").arg(sql1).arg(sql2);
}

// static private
QString SQLSerialiserMetadata::prepareUpdateSQL(MetadataStructure *structure, CorpusObject::Type what)
{
    QString sql1, sql2;
    if (what == CorpusObject::Type_Communication) {
        sql1 = "UPDATE communication SET corpusID = :corpusID, communicationName = :communicationName";
        sql2 = "WHERE communicationID = :communicationID";
    }
    else if (what == CorpusObject::Type_Speaker) {
        sql1 = "UPDATE speaker SET corpusID = :corpusID, speakerName = :speakerName";
        sql2 = "WHERE speakerID = :speakerID";
    }
    else if (what == CorpusObject::Type_Recording) {
        sql1 = "UPDATE recording SET communicationID = :communicationID, recordingName = :recordingName, "
               "filename = :filename, format = :format, duration = :duration, channels = :channels, sampleRate = :sampleRate, "
               "precisionBits = :precisionBits, bitRate = :bitRate, fileSize = :fileSize, checksumMD5 = :checksumMD5";
        sql2 = "WHERE recordingID = :recordingID";
    }
    else if (what == CorpusObject::Type_Annotation) {
        sql1 = "UPDATE annotation SET communicationID = :communicationID, recordingID = :recordingID, annotationName = :annotationName";
        sql2 = "WHERE annotationID = :annotationID";
    }
    else if (what == CorpusObject::Type_Participation) {
        sql1 = "UPDATE participation SET corpusID = :corpusID, communicationID = :communicationID, speakerID = :speakerID, role = :role";
        sql2 = "WHERE communicationID = :communicationID AND speakerID = :speakerID";
    }
    foreach (MetadataStructureSection *section, structure->sections(what)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            sql1.append(", ").append(attribute->ID()).append(" = :").append(attribute->ID());
        }
    }
    return QString("%1 %2").arg(sql1).arg(sql2);
}

// static private
bool SQLSerialiserMetadata::execSaveCommunication(CorpusCommunication *com, MetadataStructure *structure, QSqlDatabase &db)
{
    if (!com) return false;
    QSqlQuery q(db);
    if (com->isNew()) {
        q.prepare(prepareInsertSQL(structure, CorpusObject::Type_Communication));
    } else {
        q.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Communication));
    }
    q.bindValue(":communicationID", com->ID());
    q.bindValue(":corpusID", com->corpusID());
    q.bindValue(":communicationName", com->name());
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Communication)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            q.bindValue(QString(":%1").arg(attribute->ID()), com->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError().text(); return false; }
    return true;
}

// static private
bool SQLSerialiserMetadata::execSaveSpeaker(CorpusSpeaker *spk, MetadataStructure *structure, QSqlDatabase &db)
{
    QSqlQuery q(db);
    if (spk->isNew()) {
        q.prepare(prepareInsertSQL(structure, CorpusObject::Type_Speaker));
    } else {
        q.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Speaker));
    }
    q.bindValue(":speakerID", spk->ID());
    q.bindValue(":corpusID", spk->corpusID());
    q.bindValue(":speakerName", spk->name());
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Speaker)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            q.bindValue(QString(":%1").arg(attribute->ID()), spk->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError().text(); return false; }
    return true;
}

// static private
bool SQLSerialiserMetadata::execSaveRecording(QString communicationID, CorpusRecording *rec, MetadataStructure *structure, QSqlDatabase &db)
{
    QSqlQuery q(db);
    if (rec->isNew()) {
        q.prepare(prepareInsertSQL(structure, CorpusObject::Type_Recording));
    } else {
        q.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Recording));
    }
    q.bindValue(":recordingID", rec->ID());
    q.bindValue(":communicationID", communicationID);
    q.bindValue(":recordingName", rec->name());
    q.bindValue(":filename", rec->filename());
    q.bindValue(":format", rec->format());
    q.bindValue(":duration", rec->duration().toNanoseconds());
    q.bindValue(":channels", rec->channels());
    q.bindValue(":sampleRate", rec->sampleRate());
    q.bindValue(":precisionBits", rec->precisionBits());
    q.bindValue(":bitRate", rec->bitRate());
    q.bindValue(":encoding", rec->encoding());
    q.bindValue(":fileSize", rec->fileSize());
    q.bindValue(":checksumMD5", rec->checksumMD5());
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Recording)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            q.bindValue(QString(":%1").arg(attribute->ID()), rec->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError().text(); return false; }
    return true;
}

// static private
bool SQLSerialiserMetadata::execSaveAnnotation(QString communicationID, CorpusAnnotation *annot, MetadataStructure *structure, QSqlDatabase &db)
{
    QSqlQuery q(db);
    if (annot->isNew()) {
        q.prepare(prepareInsertSQL(structure, CorpusObject::Type_Annotation));
    } else {
        q.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Annotation));
    }
    q.bindValue(":annotationID", annot->ID());
    q.bindValue(":communicationID", communicationID);
    q.bindValue(":annotationName", annot->name());
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Annotation)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            q.bindValue(QString(":%1").arg(attribute->ID()), annot->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError().text(); return false; }
    return true;
}

// static private
bool SQLSerialiserMetadata::execSaveParticipation(CorpusParticipation *participation, MetadataStructure *structure, QSqlDatabase &db)
{
    QSqlQuery q(db);
    if (participation->isNew()) {
        q.prepare(prepareInsertSQL(structure, CorpusObject::Type_Participation));
    } else {
        q.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Participation));
    }
    q.bindValue(":corpusID", participation->corpusID());
    q.bindValue(":communicationID", participation->communicationID());
    q.bindValue(":speakerID", participation->speakerID());
    q.bindValue(":role", participation->role());
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Participation)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            q.bindValue(QString(":%1").arg(attribute->ID()), participation->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError().text(); return false; }
    return true;
}

// static private
bool SQLSerialiserMetadata::execCleanUpCommunication(CorpusCommunication *com, QSqlDatabase &db)
{
    // deleted recordings
    QSqlQuery queryCommunicationRecordingDelete(db);
    queryCommunicationRecordingDelete.prepare("DELETE FROM recording WHERE communicationID = :communicationID AND recordingID = :recordingID");
    queryCommunicationRecordingDelete.bindValue(":communicationID", com->ID());
    foreach (QString recordingID, com->deletedRecordingIDs) {
        queryCommunicationRecordingDelete.bindValue(":recordingID", recordingID);
        queryCommunicationRecordingDelete.exec();
    }
    com->deletedRecordingIDs.clear();
    // deleted annotations
    QSqlQuery queryCommunicationAnnotationDelete(db);
    queryCommunicationAnnotationDelete.prepare("DELETE FROM annotation WHERE communicationID = :communicationID AND annotationID = :annotationID");
    queryCommunicationAnnotationDelete.bindValue(":communicationID", com->ID());
    foreach (QString annotationID, com->deletedAnnotationIDs) {
        queryCommunicationAnnotationDelete.bindValue(":annotationID", annotationID);
        queryCommunicationAnnotationDelete.exec();
    }
    com->deletedAnnotationIDs.clear();
    return true;
}

// ==============================================================================================================================

// static
bool SQLSerialiserMetadata::saveCorpus(Corpus *corpus,
                                       QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveCommunication(CorpusCommunication *communication,
                                              QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    if (!communication) return false;
    if (!structure) return false;
    execCleanUpCommunication(communication, db);
    db.transaction();
    if (communication->isDirty())
        if (!execSaveCommunication(communication, structure, db)) { db.rollback(); return false; }
    foreach (CorpusRecording *rec, communication->recordings()) {
        if (!rec->isDirty()) continue;
        if (!execSaveRecording(communication->ID(), rec, structure, db)) { db.rollback(); return false; }
    }
    foreach (CorpusAnnotation *annot, communication->annotations()) {
        if (!annot->isDirty()) continue;
        if (!execSaveAnnotation(communication->ID(), annot, structure, db)) { db.rollback(); return false; }
    }
    db.commit();
    foreach (CorpusRecording *rec, communication->recordings()) {
        setClean(rec);
        datastore->setRepository(rec);
    }
    foreach (CorpusAnnotation *annot, communication->annotations()) {
        setClean(annot);
        datastore->setRepository(annot);
    }
    setClean(communication);
    datastore->setRepository(communication);
    return true;
}

// static
bool SQLSerialiserMetadata::saveSpeaker(CorpusSpeaker *speaker,
                                        QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    if (!speaker) return false;
    if (!structure) return false;
    db.transaction();
    if (!execSaveSpeaker(speaker, structure, db)) {
        db.rollback();
        return false;
    }
    db.commit();
    setClean(speaker);
    datastore->setRepository(speaker);
    return true;
}

// static
bool SQLSerialiserMetadata::saveRecording(CorpusRecording *recording,
                                          QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    if (!recording) return false;
    if (!structure) return false;
    db.transaction();
    if (!execSaveRecording(recording->communicationID(), recording, structure, db)) {
        db.rollback();
        return false;
    }
    db.commit();
    setClean(recording);
    datastore->setRepository(recording);
    return true;
}

// static
bool SQLSerialiserMetadata::saveAnnotation(CorpusAnnotation *annotation,
                                           QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::saveParticipation(CorpusParticipation *participation,
                                              QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    if (!participation) return false;
    if (!structure) return false;
    db.transaction();
    if (!execSaveParticipation(participation, structure, db)) {
        db.rollback();
        return false;
    }
    db.commit();
    setClean(participation);
    datastore->setRepository(participation);
    return true;
}

// ==============================================================================================================================
// Delete corpus objects
// ==============================================================================================================================

// static
bool SQLSerialiserMetadata::deleteCorpus(const QString &corpusID,
                                         QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteCommunication(const QString &communicationID,
                                                QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteSpeaker(const QString &speakerID,
                                          QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteRecording(const QString &recordingID,
                                            QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteAnnotation(const QString &annotationID,
                                             QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

// static
bool SQLSerialiserMetadata::deleteParticipation(const QString &communicationID, const QString &speakerID,
                                                QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    return false;
}

} // namespace Core
} // namespace Praaline
