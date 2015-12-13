#include <QDebug>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "corpus/corpus.h"
#include "corpus/corpuscommunication.h"
#include "corpus/corpusspeaker.h"
#include "corpus/corpusrecording.h"
#include "corpus/corpusannotation.h"
#include "sqlserialisercorpus.h"

MetadataStructure *SQLSerialiserCorpus::preparedMetadataStructure;
QSqlQuery SQLSerialiserCorpus::queryCommunicationSelect, SQLSerialiserCorpus::queryCommunicationInsert,
          SQLSerialiserCorpus::queryCommunicationUpdate, SQLSerialiserCorpus::queryCommunicationDelete;
QSqlQuery SQLSerialiserCorpus::queryCommunicationRecordingDelete, SQLSerialiserCorpus::queryCommunicationAnnotationDelete;
QSqlQuery SQLSerialiserCorpus::querySpeakerSelect, SQLSerialiserCorpus::querySpeakerInsert,
          SQLSerialiserCorpus::querySpeakerUpdate, SQLSerialiserCorpus::querySpeakerDelete;
QSqlQuery SQLSerialiserCorpus::queryRecordingSelect, SQLSerialiserCorpus::queryRecordingSelectAll, SQLSerialiserCorpus::queryRecordingInsert,
          SQLSerialiserCorpus::queryRecordingUpdate, SQLSerialiserCorpus::queryRecordingDelete;
QSqlQuery SQLSerialiserCorpus::queryAnnotationSelect, SQLSerialiserCorpus::queryAnnotationSelectAll, SQLSerialiserCorpus::queryAnnotationInsert,
          SQLSerialiserCorpus::queryAnnotationUpdate, SQLSerialiserCorpus::queryAnnotationDelete;
QSqlQuery SQLSerialiserCorpus::queryParticipationSelect, SQLSerialiserCorpus::queryParticipationInsert,
          SQLSerialiserCorpus::queryParticipationUpdate, SQLSerialiserCorpus::queryParticipationDelete;


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

// static
void SQLSerialiserCorpus::prepareLoadStatements(MetadataStructure *structure, QSqlDatabase &db)
{
    queryCommunicationSelect = QSqlQuery(db);
    queryCommunicationSelect.setForwardOnly(true);
    queryCommunicationSelect.prepare("SELECT * FROM communication WHERE corpusID = :corpusID");
    querySpeakerSelect = QSqlQuery(db);
    querySpeakerSelect.setForwardOnly(true);
    querySpeakerSelect.prepare("SELECT * FROM speaker WHERE corpusID = :corpusID");
    queryRecordingSelect = QSqlQuery(db);
    queryRecordingSelect.setForwardOnly(true);
    queryRecordingSelect.prepare("SELECT * FROM recording WHERE communicationID = :communicationID");
    queryRecordingSelectAll = QSqlQuery(db);
    queryRecordingSelectAll.setForwardOnly(true);
    queryRecordingSelectAll.prepare("SELECT r.* FROM recording r LEFT JOIN communication c ON r.communicationID=c.communicationID "
                                    "WHERE c.corpusID = :corpusID");
    queryAnnotationSelect = QSqlQuery(db);
    queryAnnotationSelect.setForwardOnly(true);
    queryAnnotationSelect.prepare("SELECT * FROM annotation WHERE communicationID = :communicationID");
    queryAnnotationSelectAll = QSqlQuery(db);
    queryAnnotationSelectAll.setForwardOnly(true);
    queryAnnotationSelectAll.prepare("SELECT a.* FROM annotation a LEFT JOIN communication c ON a.communicationID=c.communicationID "
                                     "WHERE c.corpusID = :corpusID");
    queryParticipationSelect = QSqlQuery(db);
    queryParticipationSelect.setForwardOnly(true);
    queryParticipationSelect.prepare("SELECT * FROM participation WHERE corpusID = :corpusID");

    preparedMetadataStructure = structure;
}

// static
void SQLSerialiserCorpus::prepareSaveStatements(MetadataStructure *structure, QSqlDatabase &db)
{
    queryCommunicationInsert = QSqlQuery(db);
    queryCommunicationInsert.prepare(prepareInsertSQL(structure, CorpusObject::Type_Communication));
    queryCommunicationUpdate = QSqlQuery(db);
    queryCommunicationUpdate.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Communication));
    queryCommunicationDelete = QSqlQuery(db);
    queryCommunicationDelete.prepare("DELETE FROM communication WHERE corpusID = :corpusID AND communicationID = :communicationID");
    queryCommunicationRecordingDelete = QSqlQuery(db);
    queryCommunicationRecordingDelete.prepare("DELETE FROM recording WHERE communicationID = :communicationID AND recordingID = :recordingID");
    queryCommunicationAnnotationDelete = QSqlQuery(db);
    queryCommunicationAnnotationDelete.prepare("DELETE FROM annotation WHERE communicationID = :communicationID AND annotationID = :annotationID");

    querySpeakerInsert = QSqlQuery(db);
    querySpeakerInsert.prepare(prepareInsertSQL(structure, CorpusObject::Type_Speaker));
    querySpeakerUpdate = QSqlQuery(db);
    querySpeakerUpdate.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Speaker));
    querySpeakerDelete = QSqlQuery(db);
    querySpeakerDelete.prepare("DELETE FROM speaker WHERE corpusID = :corpusID AND speakerID = :speakerID");

    queryRecordingInsert = QSqlQuery(db);
    queryRecordingInsert.prepare(prepareInsertSQL(structure, CorpusObject::Type_Recording));
    queryRecordingUpdate = QSqlQuery(db);
    queryRecordingUpdate.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Recording));
    queryRecordingDelete = QSqlQuery(db);
    queryRecordingDelete.prepare("DELETE FROM recording WHERE communicationID = :communicationID AND recordingID = :recordingID");

    queryAnnotationInsert = QSqlQuery(db);
    queryAnnotationInsert.prepare(prepareInsertSQL(structure, CorpusObject::Type_Annotation));
    queryAnnotationUpdate = QSqlQuery(db);
    queryAnnotationUpdate.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Annotation));
    queryAnnotationDelete = QSqlQuery(db);
    queryAnnotationDelete.prepare("DELETE FROM annotation WHERE communicationID = :communicationID AND annotationID = :annotationID");

    queryParticipationInsert = QSqlQuery(db);
    queryParticipationInsert.prepare(prepareInsertSQL(structure, CorpusObject::Type_Participation));
    queryParticipationUpdate = QSqlQuery(db);
    queryParticipationUpdate.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Participation));
    queryParticipationDelete = QSqlQuery(db);
    queryParticipationDelete.prepare("DELETE FROM participation WHERE corpusID = :corpusID AND communicationID = :communicationID AND speakerID = :speakerID");

    preparedMetadataStructure = structure;
}


bool SQLSerialiserCorpus::saveCommunication(CorpusCommunication *com)
{
    QSqlQuery &q = queryCommunicationUpdate;
    if (com->isNew()) q = queryCommunicationInsert;
    q.bindValue(":communicationID", com->ID());
    q.bindValue(":corpusID", com->corpusID());
    q.bindValue(":communicationName", com->name());
    foreach (MetadataStructureSection *section, preparedMetadataStructure->sections(CorpusObject::Type_Communication)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
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

bool SQLSerialiserCorpus::saveSpeaker(CorpusSpeaker *spk)
{
    QSqlQuery &q = querySpeakerUpdate;
    if (spk->isNew()) q = querySpeakerInsert;
    q.bindValue(":speakerID", spk->ID());
    q.bindValue(":corpusID", spk->corpusID());
    q.bindValue(":speakerName", spk->name());
    foreach (MetadataStructureSection *section, preparedMetadataStructure->sections(CorpusObject::Type_Speaker)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
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

bool SQLSerialiserCorpus::saveRecording(QString communicationID, CorpusRecording *rec)
{
    QSqlQuery &q = queryRecordingUpdate;
    if (rec->isNew()) q = queryRecordingInsert;
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
    foreach (MetadataStructureSection *section, preparedMetadataStructure->sections(CorpusObject::Type_Recording)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
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

bool SQLSerialiserCorpus::saveAnnotation(QString communicationID, CorpusAnnotation *annot)
{
    QSqlQuery &q = queryAnnotationUpdate;
    if (annot->isNew()) q = queryAnnotationInsert;
    q.bindValue(":annotationID", annot->ID());
    q.bindValue(":communicationID", communicationID);
    q.bindValue(":annotationName", annot->name());
    foreach (MetadataStructureSection *section, preparedMetadataStructure->sections(CorpusObject::Type_Annotation)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
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

bool SQLSerialiserCorpus::saveParticipation(CorpusParticipation *participation)
{
    QSqlQuery &q = queryParticipationUpdate;
    if (participation->isNew()) q = queryParticipationInsert;
    q.bindValue(":corpusID", participation->corpusID());
    q.bindValue(":communicationID", participation->communicationID());
    q.bindValue(":speakerID", participation->speakerID());
    q.bindValue(":role", participation->role());
    foreach (MetadataStructureSection *section, preparedMetadataStructure->sections(CorpusObject::Type_Participation)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            q.bindValue(QString(":%1").arg(attribute->ID()), participation->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) return false;
    return true;
}

bool SQLSerialiserCorpus::cleanUpCommunication(CorpusCommunication *com)
{
    // deleted recordings
    queryCommunicationRecordingDelete.bindValue(":communicationID", com->ID());
    foreach (QString recordingID, com->deletedRecordingIDs) {
        queryCommunicationRecordingDelete.bindValue(":recordingID", recordingID);
        queryCommunicationRecordingDelete.exec();
    }
    com->deletedRecordingIDs.clear();
    // deleted annotations
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
    prepareSaveStatements(structure, db);
    cleanUpCommunication(com);
    db.transaction();
    if (com->isDirty())
        if (!saveCommunication(com)) { db.rollback(); return false; }
    foreach (CorpusRecording *rec, com->recordings()) {
        if (!rec->isDirty()) continue;
        if (!saveRecording(com->ID(), rec)) { db.rollback(); return false; }
    }
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot->isDirty()) continue;
        if (!saveAnnotation(com->ID(), annot)) { db.rollback(); return false; }
    }
    db.commit();
    foreach (CorpusRecording *rec, com->recordings()) {
        rec->setDirty(false); rec->setNew(false);
    }
    foreach (CorpusAnnotation *annot, com->annotations()) {
        annot->setDirty(false); annot->setNew(false);
    }
    com->setDirty(false);
    com->setNew(false);
    return true;
}

// static
bool SQLSerialiserCorpus::saveSpeaker(CorpusSpeaker *spk, MetadataStructure *structure, QSqlDatabase &db)
{
    if (!spk) return false;
    if (!structure) return false;
    prepareSaveStatements(structure, db);
    db.transaction();
    if (!saveSpeaker(spk)) {
        db.rollback();
        return false;
    }
    db.commit();
    spk->setDirty(false);
    spk->setNew(false);
    return true;
}

// static
bool SQLSerialiserCorpus::saveParticipation(CorpusParticipation *participation, MetadataStructure *structure, QSqlDatabase &db)
{
    if (!participation) return false;
    if (!structure) return false;
    prepareSaveStatements(structure, db);
    db.transaction();
    if (!saveParticipation(participation)) {
        db.rollback();
        return false;
    }
    db.commit();
    participation->setDirty(false);
    participation->setNew(false);
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
    rec->setDirty(false);
    rec->setNew(false);
}

// static private
QList<CorpusRecording *> SQLSerialiserCorpus::getRecordings(QString communicationID)
{
    QList<CorpusRecording *> list;
    QSqlQuery &q = queryRecordingSelect;
    q.bindValue(":communicationID", communicationID);
    q.exec();
    while (q.next()) {
        CorpusRecording *rec = new CorpusRecording();
        readRecording(q, rec, preparedMetadataStructure);
        list << rec;
    }
    return list;
}

// static
QList<CorpusRecording *> SQLSerialiserCorpus::getRecordings(QString communicationID, MetadataStructure *structure, QSqlDatabase &db)
{
    prepareLoadStatements(structure, db);
    return getRecordings(communicationID);
}

// static private
QMultiMap<QString, CorpusRecording *> SQLSerialiserCorpus::getRecordingsAll(QString corpusID)
{
    QMultiMap<QString, CorpusRecording *> recordings;
    QSqlQuery &q = queryRecordingSelectAll;
    q.bindValue(":corpusID", corpusID);
    q.exec();
    while (q.next()) {
        CorpusRecording *rec = new CorpusRecording();
        QString communicationID = q.value("communicationID").toString();
        readRecording(q, rec, preparedMetadataStructure);
        recordings.insert(communicationID, rec);
    }
    return recordings;
}

// static public
QMultiMap<QString, CorpusRecording *> SQLSerialiserCorpus::getRecordingsAll(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    prepareLoadStatements(structure, db);
    return getRecordingsAll(corpusID);
}

void readAnnotation(QSqlQuery &q, CorpusAnnotation *annot, MetadataStructure *structure)
{
    annot->setID(q.value("annotationID").toString());
    annot->setName(q.value("annotationName").toString());
    foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Annotation)) {
        annot->setProperty(attribute->ID(), q.value(attribute->ID()));
    }
    annot->setDirty(false);
    annot->setNew(false);
}

// static private
QList<CorpusAnnotation *> SQLSerialiserCorpus::getAnnotations(QString communicationID)
{
    QList<CorpusAnnotation *> list;
    QSqlQuery &q = queryAnnotationSelect;
    q.bindValue(":communicationID", communicationID);
    q.exec();
    while (q.next()) {
        CorpusAnnotation *annot = new CorpusAnnotation();
        readAnnotation(q, annot, preparedMetadataStructure);
        list << annot;
    }
    return list;
}

// static
QList<CorpusAnnotation *> SQLSerialiserCorpus::getAnnotations(QString communicationID, MetadataStructure *structure, QSqlDatabase &db)
{
    prepareLoadStatements(structure, db);
    return getAnnotations(communicationID);
}

// static private
QMultiMap<QString, CorpusAnnotation *> SQLSerialiserCorpus::getAnnotationsAll(QString corpusID)
{
    QMultiMap<QString, CorpusAnnotation *> annotations;
    QSqlQuery &q = queryAnnotationSelectAll;
    q.bindValue(":corpusID", corpusID);
    q.exec();
    while (q.next()) {
        CorpusAnnotation *annot = new CorpusAnnotation();
        QString communicationID = q.value("communicationID").toString();
        readAnnotation(q, annot, preparedMetadataStructure);
        annotations.insert(communicationID, annot);
    }
    return annotations;
}

// static public
QMultiMap<QString, CorpusAnnotation *> SQLSerialiserCorpus::getAnnotationsAll(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    prepareLoadStatements(structure, db);
    return getAnnotationsAll(corpusID);
}

// static private
QList<CorpusCommunication *> SQLSerialiserCorpus::getCommunications(QString corpusID)
{
    QList<CorpusCommunication *> list;
    QMap<QString, CorpusRecording *> recordings = getRecordingsAll(corpusID);
    QMap<QString, CorpusAnnotation *> annotations = getAnnotationsAll(corpusID);

    QSqlQuery &q = queryCommunicationSelect;
    q.bindValue(":corpusID", corpusID);
    q.exec();
    while (q.next()) {
        CorpusCommunication *com = new CorpusCommunication();
        com->setCorpusID(corpusID);
        QString communicationID = q.value("communicationID").toString();
        com->setID(communicationID);
        com->setName(q.value("communicationName").toString());
        foreach (MetadataStructureAttribute *attribute, preparedMetadataStructure->attributes(CorpusObject::Type_Communication)) {
            com->setProperty(attribute->ID(), q.value(attribute->ID()));
        }
        foreach (CorpusRecording *rec, recordings.values(communicationID))
            com->addRecording(rec);
        foreach (CorpusAnnotation *annot, annotations.values(communicationID))
            com->addAnnotation(annot);
        com->setDirty(false);
        com->setNew(false);
        list << com;
    }
    return list;
}

// static public
QList<CorpusCommunication *> SQLSerialiserCorpus::getCommunications(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    prepareLoadStatements(structure, db);
    return getCommunications(corpusID);
}

// static
QList<CorpusSpeaker *> SQLSerialiserCorpus::getSpeakers(QString corpusID)
{
    QList<CorpusSpeaker *> list;
    QSqlQuery &q = querySpeakerSelect;
    q.bindValue(":corpusID", corpusID);
    q.exec();
    while (q.next()) {
        CorpusSpeaker *spk = new CorpusSpeaker();
        spk->setCorpusID(corpusID);
        spk->setID(q.value("speakerID").toString());
        spk->setName(q.value("speakerName").toString());
        foreach (MetadataStructureAttribute *attribute, preparedMetadataStructure->attributes(CorpusObject::Type_Speaker)) {
            spk->setProperty(attribute->ID(), q.value(attribute->ID()));
        }
        spk->setDirty(false);
        spk->setNew(false);
        list << spk;
    }
    return list;
}

// static
QList<CorpusSpeaker *> SQLSerialiserCorpus::getSpeakers(QString corpusID, MetadataStructure *structure, QSqlDatabase &db)
{
    prepareLoadStatements(structure, db);
    return getSpeakers(corpusID);
}

// static private
void SQLSerialiserCorpus::readParticipations(Corpus *corpus)
{
    QSqlQuery &q = queryParticipationSelect;
    q.bindValue(":corpusID", corpus->ID());
    q.exec();
    while (q.next()) {
        QString communicationID = q.value("communicationID").toString();
        QString speakerID = q.value("speakerID").toString();
        QString role = q.value("role").toString();
        CorpusParticipation *participation = corpus->addParticipation(communicationID, speakerID, role);
        if (participation) {
            participation->setCorpusID(corpus->ID());
            foreach (MetadataStructureAttribute *attribute, preparedMetadataStructure->attributes(CorpusObject::Type_Participation)) {
                participation->setProperty(attribute->ID(), q.value(attribute->ID()));
            }
            participation->setDirty(false);
            participation->setNew(false);
        }
    }
}

// ----------------------------------------------------------------------------------------------------------------------------------------
// static
bool SQLSerialiserCorpus::saveCorpus(Corpus *corpus, QSqlDatabase &db)
{
    if (!corpus) return false;
    MetadataStructure *structure = corpus->metadataStructure();
    if (!structure) return false;
    // Prepare statements
    prepareSaveStatements(structure, db);

    db.transaction();

    // Clean up
    // Communication
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
    // Speaker
    querySpeakerDelete.bindValue(":corpusID", corpus->ID());
    foreach (QString speakerID, corpus->deletedSpeakerIDs) {
        querySpeakerDelete.bindValue(":speakerID", speakerID);
        querySpeakerDelete.exec();
    }
    corpus->deletedSpeakerIDs.clear();
    // Participation
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
        cleanUpCommunication(com);
        bool result = saveCommunication(com);
        if (result) {
            com->setNew(false); com->setDirty(false);
            foreach (CorpusRecording *rec, com->recordings()) {
                if (saveRecording(com->ID(), rec)) {
                    rec->setNew(false); rec->setDirty(false);
                }
            }
            foreach (CorpusAnnotation *annot, com->annotations()) {
                if (saveAnnotation(com->ID(), annot)) {
                    annot->setNew(false); annot->setDirty(false);
                }
            }
        }
    }
    foreach (CorpusSpeaker *spk, corpus->speakers()) {
        if (saveSpeaker(spk)) {
            spk->setNew(false); spk->setDirty(false);
        }
    }
    foreach (CorpusParticipation *participation, corpus->participations()) {
        if (saveParticipation(participation)) {
            participation->setNew(false); participation->setDirty(false);
        }
    }
    db.commit();

    return true;
}

// static
bool SQLSerialiserCorpus::loadCorpus(Corpus *corpus, QSqlDatabase &db)
{
    if (!corpus) return false;
    MetadataStructure *structure = corpus->metadataStructure();
    if (!structure) return false;
    // Prepare statements
    prepareLoadStatements(structure, db);

    // Read
    foreach (CorpusCommunication *com, getCommunications(corpus->ID()))
        corpus->addCommunication(com);
    foreach (CorpusSpeaker *spk, getSpeakers(corpus->ID()))
        corpus->addSpeaker(spk);
    readParticipations(corpus);
    corpus->setNew(false);
    corpus->setDirty(false);
    return true;
}

