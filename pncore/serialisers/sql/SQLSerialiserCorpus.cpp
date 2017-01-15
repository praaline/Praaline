#include <QDebug>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "corpus/Corpus.h"
#include "corpus/CorpusCommunication.h"
#include "corpus/CorpusSpeaker.h"
#include "corpus/CorpusRecording.h"
#include "corpus/CorpusAnnotation.h"
#include "SQLSerialiserCorpus.h"

namespace Praaline {
namespace Core {

SQLSerialiserCorpus::SQLSerialiserCorpus()
{
}

QString prepareInsertSQL(MetadataStructure *structure, CorpusObject::Type what)
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

QString prepareUpdateSQL(MetadataStructure *structure, CorpusObject::Type what)
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


bool SQLSerialiserCorpus::execSaveCommunication(CorpusCommunication *com, MetadataStructure *structure, QSqlDatabase &db)
{
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
            if (attribute->datatype().base() == DataType::Boolean)
                q.bindValue(QString(":%1").arg(attribute->ID()), com->property(attribute->ID()).toBool());
            else
                q.bindValue(QString(":%1").arg(attribute->ID()), com->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) {
        qDebug() << q.lastError().text();
        return false;
    }
    return true;
}

bool SQLSerialiserCorpus::execSaveSpeaker(CorpusSpeaker *spk, MetadataStructure *structure, QSqlDatabase &db)
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
            if (attribute->datatype().base() == DataType::Boolean)
                q.bindValue(QString(":%1").arg(attribute->ID()), spk->property(attribute->ID()).toBool());
            else
                q.bindValue(QString(":%1").arg(attribute->ID()), spk->property(attribute->ID()));
        }
    }

    q.exec();
    if (q.lastError().isValid()) {
        qDebug() << q.lastError().text();
        return false;
    }
    return true;
}

bool SQLSerialiserCorpus::execSaveRecording(QString communicationID, CorpusRecording *rec, MetadataStructure *structure, QSqlDatabase &db)
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
            if (attribute->datatype().base() == DataType::Boolean)
                q.bindValue(QString(":%1").arg(attribute->ID()), rec->property(attribute->ID()).toBool());
            else
                q.bindValue(QString(":%1").arg(attribute->ID()), rec->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) {
        qDebug() << q.lastError().text();
        return false;
    }
    return true;
}

bool SQLSerialiserCorpus::execSaveAnnotation(QString communicationID, CorpusAnnotation *annot, MetadataStructure *structure, QSqlDatabase &db)
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
            if (attribute->datatype().base() == DataType::Boolean)
                q.bindValue(QString(":%1").arg(attribute->ID()), annot->property(attribute->ID()).toBool());
            else
                q.bindValue(QString(":%1").arg(attribute->ID()), annot->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) {
        qDebug() << q.lastError().text();
        return false;
    }
    return true;
}

bool SQLSerialiserCorpus::execSaveParticipation(CorpusParticipation *participation, MetadataStructure *structure, QSqlDatabase &db)
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
            if (attribute->datatype().base() == DataType::Boolean)
                q.bindValue(QString(":%1").arg(attribute->ID()), participation->property(attribute->ID()).toBool());
            else
                q.bindValue(QString(":%1").arg(attribute->ID()), participation->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) return false;
    return true;
}

bool SQLSerialiserCorpus::execCleanUpCommunication(CorpusCommunication *com, QSqlDatabase &db)
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

// -------------------------------------------------------------------------------------------------------------------------
// INDIVIDUAL SAVE (public static)
// -------------------------------------------------------------------------------------------------------------------------

// static
bool SQLSerialiserCorpus::saveCommunication(CorpusCommunication *com, MetadataStructure *structure, QSqlDatabase &db)
{
    if (!com) return false;
    if (!structure) return false;
    execCleanUpCommunication(com, db);
    db.transaction();
    if (com->isDirty())
        if (!execSaveCommunication(com, structure, db)) { db.rollback(); return false; }
    foreach (CorpusRecording *rec, com->recordings()) {
        if (!rec->isDirty()) continue;
        if (!execSaveRecording(com->ID(), rec, structure, db)) { db.rollback(); return false; }
    }
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot->isDirty()) continue;
        if (!execSaveAnnotation(com->ID(), annot, structure, db)) { db.rollback(); return false; }
    }
    db.commit();
    foreach (CorpusRecording *rec, com->recordings()) {
         setClean(rec);
    }
    foreach (CorpusAnnotation *annot, com->annotations()) {
        setClean(annot);
    }
    setClean(com);
    return true;
}

// static
bool SQLSerialiserCorpus::saveSpeaker(CorpusSpeaker *spk, MetadataStructure *structure, QSqlDatabase &db)
{
    if (!spk) return false;
    if (!structure) return false;
    db.transaction();
    if (!execSaveSpeaker(spk, structure, db)) {
        db.rollback();
        return false;
    }
    db.commit();
    setClean(spk);
    return true;
}

// static
bool SQLSerialiserCorpus::saveParticipation(CorpusParticipation *participation, MetadataStructure *structure, QSqlDatabase &db)
{
    if (!participation) return false;
    if (!structure) return false;
    db.transaction();
    if (!saveParticipation(participation, structure, db)) {
        db.rollback();
        return false;
    }
    db.commit();
    setClean(participation);
    return true;
}

// -------------------------------------------------------------------------------------------------------------------------
// SELECTS
// -------------------------------------------------------------------------------------------------------------------------

void readRecording(QSqlQuery &q, CorpusRecording *rec, MetadataStructure *structure)
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

// static private
QList<CorpusRecording *> SQLSerialiserCorpus::execGetRecordings(QString communicationID, MetadataStructure *structure, QSqlDatabase &db)
{
    QList<CorpusRecording *> list;
    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare("SELECT * FROM recording WHERE communicationID = :communicationID");
    q.bindValue(":communicationID", communicationID);
    q.exec();
    while (q.next()) {
        CorpusRecording *rec = new CorpusRecording();
        readRecording(q, rec, structure);
        setClean(rec);
        list << rec;
    }
    return list;
}

// static
QList<CorpusRecording *> SQLSerialiserCorpus::getRecordings(QString communicationID, MetadataStructure *structure, QSqlDatabase &db)
{
    return execGetRecordings(communicationID, structure, db);
}

// static private
QMultiMap<QString, CorpusRecording *> SQLSerialiserCorpus::execGetRecordingsAll(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    QMultiMap<QString, CorpusRecording *> recordings;
    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare("SELECT r.* FROM recording r LEFT JOIN communication c ON r.communicationID=c.communicationID "
              "WHERE c.corpusID = :corpusID");
    q.bindValue(":corpusID", corpusID);
    q.exec();
    while (q.next()) {
        CorpusRecording *rec = new CorpusRecording();
        QString communicationID = q.value("communicationID").toString();
        readRecording(q, rec, structure);
        setClean(rec);
        recordings.insert(communicationID, rec);
    }
    return recordings;
}

// static public
QMultiMap<QString, CorpusRecording *> SQLSerialiserCorpus::getRecordingsAll(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    return execGetRecordingsAll(corpusID, structure, db);
}

void readAnnotation(QSqlQuery &q, CorpusAnnotation *annot, MetadataStructure *structure)
{
    annot->setID(q.value("annotationID").toString());
    annot->setName(q.value("annotationName").toString());
    foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Annotation)) {
        annot->setProperty(attribute->ID(), q.value(attribute->ID()));
    }
}

// static private
QList<CorpusAnnotation *> SQLSerialiserCorpus::execGetAnnotations(QString communicationID, MetadataStructure *structure, QSqlDatabase &db)
{
    QList<CorpusAnnotation *> list;
    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare("SELECT * FROM annotation WHERE communicationID = :communicationID");
    q.bindValue(":communicationID", communicationID);
    q.exec();
    while (q.next()) {
        CorpusAnnotation *annot = new CorpusAnnotation();
        readAnnotation(q, annot, structure);
        setClean(annot);
        list << annot;
    }
    return list;
}

// static
QList<CorpusAnnotation *> SQLSerialiserCorpus::getAnnotations(QString communicationID, MetadataStructure *structure, QSqlDatabase &db)
{
    return execGetAnnotations(communicationID, structure, db);
}

// static private
QMultiMap<QString, CorpusAnnotation *> SQLSerialiserCorpus::execGetAnnotationsAll(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    QMultiMap<QString, CorpusAnnotation *> annotations;
    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare("SELECT a.* FROM annotation a LEFT JOIN communication c ON a.communicationID=c.communicationID "
              "WHERE c.corpusID = :corpusID");
    q.bindValue(":corpusID", corpusID);
    q.exec();
    while (q.next()) {
        CorpusAnnotation *annot = new CorpusAnnotation();
        QString communicationID = q.value("communicationID").toString();
        readAnnotation(q, annot, structure);
        setClean(annot);
        annotations.insert(communicationID, annot);
    }
    return annotations;
}

// static public
QMultiMap<QString, CorpusAnnotation *> SQLSerialiserCorpus::getAnnotationsAll(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    return execGetAnnotationsAll(corpusID, structure, db);
}

// static private
QList<CorpusCommunication *> SQLSerialiserCorpus::execGetCommunications(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    QList<CorpusCommunication *> list;
    QMap<QString, CorpusRecording *> recordings = getRecordingsAll(corpusID, structure, db);
    QMap<QString, CorpusAnnotation *> annotations = getAnnotationsAll(corpusID, structure, db);

    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare("SELECT * FROM communication WHERE corpusID = :corpusID");
    q.bindValue(":corpusID", corpusID);
    q.exec();
    while (q.next()) {
        CorpusCommunication *com = new CorpusCommunication();
        com->setCorpusID(corpusID);
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
        list << com;
    }
    return list;
}

// static public
QList<CorpusCommunication *> SQLSerialiserCorpus::getCommunications(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    return execGetCommunications(corpusID, structure, db);
}

// static
QList<CorpusSpeaker *> SQLSerialiserCorpus::execGetSpeakers(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    QList<CorpusSpeaker *> list;
    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare("SELECT * FROM speaker WHERE corpusID = :corpusID");
    q.bindValue(":corpusID", corpusID);
    q.exec();
    while (q.next()) {
        CorpusSpeaker *spk = new CorpusSpeaker();
        spk->setCorpusID(corpusID);
        spk->setID(q.value("speakerID").toString());
        spk->setName(q.value("speakerName").toString());
        foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Speaker)) {
            spk->setProperty(attribute->ID(), q.value(attribute->ID()));
        }
        setClean(spk);
        list << spk;
    }
    return list;
}

// static
QList<CorpusSpeaker *> SQLSerialiserCorpus::getSpeakers(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    return execGetSpeakers(corpusID, structure, db);
}

// static private
void SQLSerialiserCorpus::execGetParticipations(Corpus *corpus, MetadataStructure *structure, QSqlDatabase &db)
{
    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare("SELECT * FROM participation WHERE corpusID = :corpusID");
    q.bindValue(":corpusID", corpus->ID());
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
        }
    }
}

// ----------------------------------------------------------------------------------------------------------------------------------------
// static
bool SQLSerialiserCorpus::saveCorpus(Corpus *corpus, QSqlDatabase &db)
{
    if (!corpus) return false;
    MetadataStructure *structure = 0; // corpus->metadataStructure();
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
bool SQLSerialiserCorpus::loadCorpus(Corpus *corpus, QSqlDatabase &db)
{
    if (!corpus) return false;
    MetadataStructure *structure = 0; // corpus->metadataStructure();
    if (!structure) return false;
    // Read
    foreach (CorpusCommunication *com, getCommunications(corpus->ID(), structure, db))
        corpus->addCommunication(com);
    foreach (CorpusSpeaker *spk, getSpeakers(corpus->ID(), structure, db))
        corpus->addSpeaker(spk);
    execGetParticipations(corpus, structure, db);
    setClean(corpus);
    return true;
}

} // namespace Core
} // namespace Praaline
