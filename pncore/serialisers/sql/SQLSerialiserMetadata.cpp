#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "structure/MetadataStructure.h"
#include "datastore/CorpusRepository.h"
#include "datastore/CorpusDatastore.h"
#include "SQLSerialiserMetadata.h"

namespace Praaline {
namespace Core {

// ==============================================================================================================================
// Load metadata information
// ==============================================================================================================================

// Helper functions
// private static
void SQLSerialiserMetadata::readRecording(QSqlQuery &q, CorpusRecording *rec, MetadataStructure *structure)
{
    rec->setID(q.value("recordingID").toString());
    rec->setName(q.value("recordingName").toString());
    rec->setFilename(q.value("filename").toString());
    rec->setFormat(q.value("format").toString());
    rec->setDuration(RealTime::fromNanoseconds(q.value("duration").toLongLong()));
    rec->setChannels(q.value("channels").toInt());
    rec->setSampleRate(q.value("sampleRate").toInt());
    rec->setPrecisionBits(q.value("precisionBits").toInt());
    rec->setBitRate(q.value("bitRate").toInt());
    rec->setEncoding(q.value("encoding").toString());
    rec->setFileSize(q.value("fileSize").toLongLong());
    rec->setChecksumMD5(q.value("checksumMD5").toString());
    foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Recording)) {
        rec->setProperty(attribute->ID(), q.value(attribute->ID()));
    }
}

// private static
void SQLSerialiserMetadata::readAnnotation(QSqlQuery &q, CorpusAnnotation *annot, MetadataStructure *structure)
{
    annot->setID(q.value("annotationID").toString());
    annot->setName(q.value("annotationName").toString());
    foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Annotation)) {
        annot->setProperty(attribute->ID(), q.value(attribute->ID()));
    }
}

bool prepareSelectQuery(QSqlQuery &query, CorpusObject::Type type, const MetadataDatastore::Selection &selection)
{
    QString sql;
    if (type == CorpusObject::Type_Corpus) {
        sql = "SELECT * FROM corpus WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty()) sql.append("AND corpusID = :corpusID ");
    }
    else if (type == CorpusObject::Type_Communication) {
        sql = "SELECT * FROM communication WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty()) sql.append("AND corpusID = :corpusID ");
        if (!selection.communicationID.isEmpty()) sql.append("AND communicationID = :communicationID ");
    }
    else if (type == CorpusObject::Type_Speaker) {
        sql = "SELECT * FROM speaker WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty()) sql.append("AND corpusID = :corpusID ");
        if (!selection.speakerID.isEmpty()) sql.append("AND speakerID = :speakerID ");
    }
    else if (type == CorpusObject::Type_Participation) {
        sql = "SELECT * FROM participation WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty()) sql.append("AND corpusID = :corpusID ");
        if (!selection.communicationID.isEmpty()) sql.append("AND communicationID = :communicationID ");
        if (!selection.speakerID.isEmpty()) sql.append("AND speakerID = :speakerID ");
    }
    else if (type == CorpusObject::Type_Recording) {
        sql = "SELECT r.* FROM recording r LEFT JOIN communication c ON r.communicationID=c.communicationID WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty()) sql.append("AND corpusID = :corpusID ");
        if (!selection.communicationID.isEmpty()) sql.append("AND communicationID = :communicationID ");
    }
    else if (type == CorpusObject::Type_Annotation) {
        sql = "SELECT a.* FROM annotation a LEFT JOIN communication c ON a.communicationID=c.communicationID WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty()) sql.append("AND corpusID = :corpusID ");
        if (!selection.communicationID.isEmpty()) sql.append("AND communicationID = :communicationID ");
    }
    // Prepare query
    query.setForwardOnly(true);
    query.prepare(sql);
    // Bind parameters
    if (!selection.corpusID.isEmpty())          query.bindValue(":corpusID", selection.corpusID);
    if (!selection.communicationID.isEmpty())   query.bindValue(":communicationID", selection.communicationID);
    if (!selection.speakerID.isEmpty())         query.bindValue(":speakerID", selection.speakerID);
}

// static
QList<CorpusObjectInfo> SQLSerialiserMetadata::getCorpusObjectInfoList(
        CorpusObject::Type type, const MetadataDatastore::Selection &selection,
        QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    QList<CorpusObjectInfo> list;
    QSqlQuery q(db);
    prepareSelectQuery(q, type, selection);
    q.exec();
    while (q.next()) {

    }
    return list;
}

// static
Corpus *SQLSerialiserMetadata::getCorpus(
        const QString &corpusID, QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    if (!structure) return Q_NULLPTR;
    Corpus *corpus = new Corpus();
    // Read communications and speakers
    foreach (CorpusCommunication *com, getCommunications(MetadataDatastore::Selection(corpusID, "", ""), db, structure, datastore))
        corpus->addCommunication(com);
    foreach (CorpusSpeaker *spk, getSpeakers(MetadataDatastore::Selection(corpusID, "", ""), db, structure, datastore))
        corpus->addSpeaker(spk);
    // Read participations
    QSqlQuery q(db);
    prepareSelectQuery(q, CorpusObject::Type_Participation, MetadataDatastore::Selection(corpusID, "", ""));
    q.exec();
    while (q.next()) {
        QString communicationID = q.value("communicationID").toString();
        QString speakerID = q.value("speakerID").toString();
        QString role = q.value("role").toString();
        CorpusParticipation *participation = corpus->addParticipation(communicationID, speakerID, role);
        if (participation) {
            participation->setCorpusID(corpus->ID());
            foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Participation)) {
                participation->setProperty(attribute->ID(), q.value(attribute->ID()));
            }
            setClean(participation);
            datastore->setRepository(participation);
        }
    }
    setClean(corpus);
    datastore->setRepository(corpus);
    return corpus;
}

// static
QList<QPointer<CorpusCommunication> > SQLSerialiserMetadata::getCommunications(
        const MetadataDatastore::Selection &selection, QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    QList<QPointer<CorpusCommunication> > communications;
    QMap<QString, QPointer<CorpusRecording> > recordings = getRecordingsByCommunication(selection, db, structure, datastore);
    QMap<QString, QPointer<CorpusAnnotation> > annotations = getAnnotationsByCommunication(selection, db, structure, datastore);

    QSqlQuery q(db);
    prepareSelectQuery(q, CorpusObject::Type_Communication, selection);
    q.exec();
    while (q.next()) {
        CorpusCommunication *com = new CorpusCommunication();
        com->setCorpusID(q.value("corpusID").toString());
        QString communicationID = q.value("communicationID").toString();
        com->setID(communicationID);
        com->setName(q.value("communicationName").toString());
        foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Communication)) {
            com->setProperty(attribute->ID(), q.value(attribute->ID()));
        }
        foreach (CorpusRecording *rec, recordings.values(communicationID))
            com->addRecording(rec);
        foreach (CorpusAnnotation *annot, annotations.values(communicationID))
            com->addAnnotation(annot);
        setClean(com);
        datastore->setRepository(com);
        communications << com;
    }
    return communications;
}

// static
QList<QPointer<CorpusSpeaker> > SQLSerialiserMetadata::getSpeakers(
        const MetadataDatastore::Selection &selection, QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    QList<QPointer<CorpusSpeaker> > speakers;
    QSqlQuery q(db);
    prepareSelectQuery(q, CorpusObject::Type_Speaker, selection);
    q.exec();
    while (q.next()) {
        CorpusSpeaker *spk = new CorpusSpeaker();
        spk->setCorpusID(q.value("corpusID").toString());
        spk->setID(q.value("speakerID").toString());
        spk->setName(q.value("speakerName").toString());
        foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Speaker)) {
            spk->setProperty(attribute->ID(), q.value(attribute->ID()));
        }
        setClean(spk);
        datastore->setRepository(spk);
        speakers << spk;
    }
    return speakers;
}

// static
QList<QPointer<CorpusRecording> > SQLSerialiserMetadata::getRecordings(
        const MetadataDatastore::Selection &selection, QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    QList<QPointer<CorpusRecording> > recordings;
    QSqlQuery q(db);
    prepareSelectQuery(q, CorpusObject::Type_Recording, selection);
    q.exec();
    while (q.next()) {
        QPointer<CorpusRecording> rec = new CorpusRecording();
        readRecording(q, rec, structure);
        setClean(rec);
        datastore->setRepository(rec);
        recordings << rec;
    }
    return recordings;
}

// static
QList<QPointer<CorpusAnnotation> > SQLSerialiserMetadata::getAnnotations(
        const MetadataDatastore::Selection &selection, QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    QList<QPointer<CorpusAnnotation> > annotations;
    QSqlQuery q(db);
    prepareSelectQuery(q, CorpusObject::Type_Annotation, selection);
    q.exec();
    while (q.next()) {
        QPointer<CorpusAnnotation>  annot = new CorpusAnnotation();
        readAnnotation(q, annot, structure);
        setClean(annot);
        datastore->setRepository(annot);
        annotations << annot;
    }
    return annotations;
}

// static
QMultiMap<QString, QPointer<CorpusRecording> > SQLSerialiserMetadata::getRecordingsByCommunication(
        const MetadataDatastore::Selection &selection, QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    QMultiMap<QString, QPointer<CorpusRecording> > recordings;
    QSqlQuery q(db);
    prepareSelectQuery(q, CorpusObject::Type_Recording, selection);
    q.exec();
    while (q.next()) {
        QPointer<CorpusRecording> rec = new CorpusRecording();
        QString communicationID = q.value("communicationID").toString();
        readRecording(q, rec, structure);
        setClean(rec);
        datastore->setRepository(rec);
        recordings.insert(communicationID, rec);
    }
    return recordings;
}

// static
QMultiMap<QString, QPointer<CorpusAnnotation> > SQLSerialiserMetadata::getAnnotationsByCommunication(
        const MetadataDatastore::Selection &selection, QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    QMultiMap<QString, QPointer<CorpusAnnotation> > annotations;
    QSqlQuery q(db);
    prepareSelectQuery(q, CorpusObject::Type_Annotation, selection);
    q.exec();
    while (q.next()) {
        QPointer<CorpusAnnotation> annot = new CorpusAnnotation();
        QString communicationID = q.value("communicationID").toString();
        readAnnotation(q, annot, structure);
        setClean(annot);
        datastore->setRepository(annot);
        annotations.insert(communicationID, annot);
    }
    return annotations;
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
    if (!corpus) return false;
    if (!structure) return false;
    // Start transaction
    db.transaction();
    // Clean up statements
    QSqlQuery queryCommunicationDelete(db);
    queryCommunicationDelete.prepare("DELETE FROM communication WHERE corpusID = :corpusID AND communicationID = :communicationID");
    QSqlQuery queryRecordingDelete(db);
    queryRecordingDelete.prepare("DELETE FROM recording WHERE communicationID = :communicationID AND recordingID = :recordingID");
    QSqlQuery queryAnnotationDelete(db);
    queryAnnotationDelete.prepare("DELETE FROM annotation WHERE communicationID = :communicationID AND annotationID = :annotationID");
    QSqlQuery querySpeakerDelete(db);
    querySpeakerDelete.prepare("DELETE FROM speaker WHERE corpusID = :corpusID AND speakerID = :speakerID");
    QSqlQuery queryParticipationDelete(db);
    queryParticipationDelete.prepare("DELETE FROM participation WHERE corpusID = :corpusID AND communicationID = :communicationID AND speakerID = :speakerID");
    // Clean up Communications (including recordings and annotations)
    queryCommunicationDelete.bindValue(":corpusID", corpus->ID());
    foreach (QString communicationID, corpus->deletedCommunicationIDs) {
        queryCommunicationDelete.bindValue(":communicationID", communicationID);
        queryCommunicationDelete.exec();
        queryRecordingDelete.bindValue(":communicationID", communicationID);
        queryRecordingDelete.exec();
        queryAnnotationDelete.bindValue(":communicationID", communicationID);
        queryAnnotationDelete.exec();
    }
    corpus->deletedCommunicationIDs.clear();
    // Clean up Speakers
    querySpeakerDelete.bindValue(":corpusID", corpus->ID());
    foreach (QString speakerID, corpus->deletedSpeakerIDs) {
        querySpeakerDelete.bindValue(":speakerID", speakerID);
        querySpeakerDelete.exec();
    }
    corpus->deletedSpeakerIDs.clear();
    // Clean up Participations
    queryParticipationDelete.bindValue(":corpusID", corpus->ID());
    QPair<QString, QString> participationID;
    foreach (participationID, corpus->deletedParticipationIDs) {
        queryParticipationDelete.bindValue(":communicationID", participationID.first);
        queryParticipationDelete.bindValue(":speakerID", participationID.second);
        queryParticipationDelete.exec();
    }
    corpus->deletedParticipationIDs.clear();
    // Update
    foreach (CorpusCommunication *com, corpus->communications()) {
        execCleanUpCommunication(com, db);
        bool result = execSaveCommunication(com, structure, db);
        if (result) {
            setClean(com);
            foreach (CorpusRecording *rec, com->recordings()) {
                if (execSaveRecording(com->ID(), rec, structure, db)) {
                    setClean(rec);
                }
            }
            foreach (CorpusAnnotation *annot, com->annotations()) {
                if (execSaveAnnotation(com->ID(), annot, structure, db)) {
                    setClean(annot);
                }
            }
        }
    }
    foreach (CorpusSpeaker *spk, corpus->speakers()) {
        if (execSaveSpeaker(spk, structure, db)) {
            setClean(spk);
        }
    }
    foreach (CorpusParticipation *participation, corpus->participations()) {
        if (execSaveParticipation(participation, structure, db)) {
            setClean(participation);
        }
    }
    db.commit();

    return true;
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
    if (!annotation) return false;
    if (!structure) return false;
    db.transaction();
    if (!execSaveAnnotation(annotation->communicationID(), annotation, structure, db)) {
        db.rollback();
        return false;
    }
    db.commit();
    setClean(annotation);
    datastore->setRepository(annotation);
    return true;
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
    Q_UNUSED(structure)
    Q_UNUSED(datastore)
    return false;
}

// static
bool SQLSerialiserMetadata::deleteCommunication(const QString &communicationID,
                                                QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    Q_UNUSED(structure)
    Q_UNUSED(datastore)
    QSqlQuery q(db);
    q.prepare("DELETE FROM communication WHERE communicationID = :communicationID");
    q.bindValue(":communicationID", communicationID);
    return q.exec();
}

// static
bool SQLSerialiserMetadata::deleteSpeaker(const QString &speakerID,
                                          QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    Q_UNUSED(structure)
    Q_UNUSED(datastore)
    QSqlQuery q(db);
    q.prepare("DELETE FROM speaker WHERE speakerID = :speakerID");
    q.bindValue(":speakerID", speakerID);
    return q.exec();
}

// static
bool SQLSerialiserMetadata::deleteRecording(const QString &recordingID,
                                            QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    Q_UNUSED(structure)
    Q_UNUSED(datastore)
    QSqlQuery q(db);
    q.prepare("DELETE FROM recording WHERE recordingID = :recordingID");
    q.bindValue(":recordingID", recordingID);
    return q.exec();
}

// static
bool SQLSerialiserMetadata::deleteAnnotation(const QString &annotationID,
                                             QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    Q_UNUSED(structure)
    Q_UNUSED(datastore)
    QSqlQuery q(db);
    q.prepare("DELETE FROM annotation WHERE AND annotationID = :annotationID");
    q.bindValue(":annotationID", annotationID);
    return q.exec();
}

// static
bool SQLSerialiserMetadata::deleteParticipation(const QString &communicationID, const QString &speakerID,
                                                QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    Q_UNUSED(structure)
    Q_UNUSED(datastore)
    QSqlQuery q(db);
    q.prepare("DELETE FROM participation WHERE AND communicationID = :communicationID AND speakerID = :speakerID");
    q.bindValue(":communicationID", communicationID);
    q.bindValue(":speakerID", speakerID);
    return q.exec();
}

} // namespace Core
} // namespace Praaline
