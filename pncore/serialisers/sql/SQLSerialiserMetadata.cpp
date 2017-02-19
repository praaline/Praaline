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
QStringList getEffectiveAttributeIDs(MetadataStructure *structure,  CorpusObject::Type type, const QStringList &requestedAttributeIDs = QStringList())
{
    QStringList effectiveAttributeIDs;
    if (!structure) return effectiveAttributeIDs;
    // Basic attributes, based on corpus object type
    effectiveAttributeIDs << MetadataStructure::basicAttributeIDs(type);
    // Extra attributes
    if (requestedAttributeIDs.isEmpty())
        effectiveAttributeIDs << structure->attributeIDs(type);
    else {
        QStringList structureAttributeIDs = structure->attributeIDs(type);
        foreach (QString attributeID, requestedAttributeIDs) {
            if (structureAttributeIDs.contains(attributeID))
                effectiveAttributeIDs << attributeID;
        }
    }
    return effectiveAttributeIDs;
}

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
        if (!selection.corpusID.isEmpty())          sql.append("AND corpusID = :corpusID ");
    }
    else if (type == CorpusObject::Type_Communication) {
        sql = "SELECT * FROM communication WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty()) sql.append("AND corpusID = :corpusID ");
        if (!selection.communicationID.isEmpty())   sql.append("AND communicationID = :communicationID ");
    }
    else if (type == CorpusObject::Type_Speaker) {
        sql = "SELECT * FROM speaker WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty())          sql.append("AND corpusID = :corpusID ");
        if (!selection.speakerID.isEmpty())         sql.append("AND speakerID = :speakerID ");
    }
    else if (type == CorpusObject::Type_Participation) {
        sql = "SELECT * FROM participation WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty()) sql.append("AND corpusID = :corpusID ");
        if (!selection.communicationID.isEmpty())   sql.append("AND communicationID = :communicationID ");
        if (!selection.speakerID.isEmpty())         sql.append("AND speakerID = :speakerID ");
    }
    else if (type == CorpusObject::Type_Recording) {
        sql = "SELECT r.* FROM recording r LEFT JOIN communication c ON r.communicationID=c.communicationID WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty())          sql.append("AND corpusID = :corpusID ");
        if (!selection.communicationID.isEmpty())   sql.append("AND communicationID = :communicationID ");
        if (!selection.recordingID.isEmpty())       sql.append("AND recordingID = :recordingID ");
    }
    else if (type == CorpusObject::Type_Annotation) {
        sql = "SELECT a.* FROM annotation a LEFT JOIN communication c ON a.communicationID=c.communicationID WHERE 1 = 1 ";
        if (!selection.corpusID.isEmpty())          sql.append("AND corpusID = :corpusID ");
        if (!selection.communicationID.isEmpty())   sql.append("AND communicationID = :communicationID ");
        if (!selection.recordingID.isEmpty())       sql.append("AND recordingID = :recordingID ");
        if (!selection.annotationID.isEmpty())      sql.append("AND annotationID = :annotationID ");
    }
    // Prepare query
    query.setForwardOnly(true);
    query.prepare(sql);
    // Bind parameters
    if (!selection.corpusID.isEmpty())          query.bindValue(":corpusID", selection.corpusID);
    if (!selection.communicationID.isEmpty())   query.bindValue(":communicationID", selection.communicationID);
    if (!selection.speakerID.isEmpty())         query.bindValue(":speakerID", selection.speakerID);
    if (!selection.recordingID.isEmpty())       query.bindValue(":recordingID", selection.recordingID);
    if (!selection.annotationID.isEmpty())      query.bindValue(":annotationID", selection.annotationID);
    return true;
}

// static
QList<CorpusObjectInfo> SQLSerialiserMetadata::getCorpusObjectInfoList(
        CorpusObject::Type type, const MetadataDatastore::Selection &selection,
        QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    Q_UNUSED(datastore)

    QList<CorpusObjectInfo> list;
    // Create the list of attributes depending on the type of the corpus object requested
    QStringList attributeIDs = getEffectiveAttributeIDs(structure, type);
    if (attributeIDs.isEmpty()) return list; // means unsupported type, not in database
    QSqlQuery q(db);
    prepareSelectQuery(q, type, selection);
    q.exec();
    while (q.next()) {
        CorpusObjectInfo item;
        foreach (QString attributeID, attributeIDs) {
            if (q.value(attributeID).isValid()) item.setAttribute(attributeID, q.value(attributeID));
        }
        setClean(&item);
        list << item;
    }
    return list;
}

// static
Corpus *SQLSerialiserMetadata::getCorpus(
        const QString &corpusID, QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    if (!structure) return Q_NULLPTR;
    Corpus *corpus = new Corpus(corpusID);
    QSqlQuery q(db);
    prepareSelectQuery(q, CorpusObject::Type_Corpus, MetadataDatastore::Selection(corpusID, "", ""));
    q.exec();
    while (q.next()) {
        corpus->setName(q.value("corpusName").toString());
        corpus->setDescription(q.value("description").toString());
        foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Corpus)) {
            corpus->setProperty(attribute->ID(), q.value(attribute->ID()));
        }
    }
    // Read communications and speakers
    foreach (CorpusCommunication *com, getCommunications(MetadataDatastore::Selection(corpusID, "", ""), db, structure, datastore))
        corpus->addCommunication(com);
    foreach (CorpusSpeaker *spk, getSpeakers(MetadataDatastore::Selection(corpusID, "", ""), db, structure, datastore))
        corpus->addSpeaker(spk);
    // Read participations
    QSqlQuery qpart(db);
    prepareSelectQuery(qpart, CorpusObject::Type_Participation, MetadataDatastore::Selection(corpusID, "", ""));
    qpart.exec();
    while (qpart.next()) {
        QString communicationID = qpart.value("communicationID").toString();
        QString speakerID = qpart.value("speakerID").toString();
        QString role = qpart.value("role").toString();
        CorpusParticipation *participation = corpus->addParticipation(communicationID, speakerID, role);
        if (participation) {
            participation->setCorpusID(corpus->ID());
            foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Participation)) {
                participation->setProperty(attribute->ID(), qpart.value(attribute->ID()));
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
QList<QPointer<CorpusParticipation> > SQLSerialiserMetadata::getParticipations(
        const MetadataDatastore::Selection &selection, QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    QList<QPointer<CorpusParticipation> > participations;
    QSqlQuery q(db);
    prepareSelectQuery(q, CorpusObject::Type_Participation, selection);
    q.exec();
    while (q.next()) {
        QString corpusID =  q.value("corpusID").toString();
        QString communicationID = q.value("communicationID").toString();
        QString speakerID = q.value("speakerID").toString();
        QList<QPointer<CorpusCommunication> > communications = getCommunications(MetadataDatastore::Selection(corpusID, communicationID, ""), db, structure, datastore);
        QList<QPointer<CorpusSpeaker> > speakers = getSpeakers(MetadataDatastore::Selection(corpusID, "", speakerID), db, structure, datastore);
        if ((communications.count() != 1) || (speakers.count() != 1)) {
            qDeleteAll(communications);
            qDeleteAll(speakers);
            continue;
        }
        CorpusParticipation *participation = new CorpusParticipation(communications.first(), speakers.first());
        participation->setRole(q.value("role").toString());
        foreach (MetadataStructureAttribute *attribute, structure->attributes(CorpusObject::Type_Participation)) {
            participation->setProperty(attribute->ID(), q.value(attribute->ID()));
        }
        setClean(participation);
        datastore->setRepository(participation);
        participations << participation;
    }
    return participations;
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
QString SQLSerialiserMetadata::prepareInsertSQL(MetadataStructure *structure, CorpusObject::Type what, QStringList requestedAttributeIDs)
{   
    QString tableName;
    if      (what == CorpusObject::Type_Corpus)         tableName = "corpus";
    else if (what == CorpusObject::Type_Communication)  tableName = "communication";
    else if (what == CorpusObject::Type_Speaker)        tableName = "speaker";
    else if (what == CorpusObject::Type_Recording)      tableName = "recording";
    else if (what == CorpusObject::Type_Annotation)     tableName = "annotation";
    else if (what == CorpusObject::Type_Participation)  tableName = "participation";
    QString sql1 = QString("INSERT INTO %1 (").arg(tableName);
    QString sql2 = "VALUES (";
    QStringList attributeIDs = getEffectiveAttributeIDs(structure, what, requestedAttributeIDs);
    foreach (QString attributeID, attributeIDs) {
        if (attributeID != attributeIDs.first()) sql1.append(", ");
        sql1.append(attributeID);
        if (attributeID != attributeIDs.first()) sql2.append(", ");
        sql2.append(":").append(attributeID);
    }
    sql1.append(")");
    sql2.append(")");
    qDebug() << sql1 << " " << sql2;
    return QString("%1 %2").arg(sql1).arg(sql2);
}

// static private
QString SQLSerialiserMetadata::prepareUpdateSQL(MetadataStructure *structure, CorpusObject::Type what, QStringList requestedAttributeIDs)
{
    QString sql1, sql2;
    if (what == CorpusObject::Type_Corpus) {
        sql1 = "UPDATE corpus SET ";
        sql2 = "WHERE corpusID = :corpusID";
    }
    else if (what == CorpusObject::Type_Communication) {
        sql1 = "UPDATE communication SET ";
        sql2 = "WHERE communicationID = :communicationID";
    }
    else if (what == CorpusObject::Type_Speaker) {
        sql1 = "UPDATE speaker SET ";
        sql2 = "WHERE speakerID = :speakerID";
    }
    else if (what == CorpusObject::Type_Recording) {
        sql1 = "UPDATE recording SET ";
        sql2 = "WHERE recordingID = :recordingID";
    }
    else if (what == CorpusObject::Type_Annotation) {
        sql1 = "UPDATE annotation SET ";
        sql2 = "WHERE annotationID = :annotationID";
    }
    else if (what == CorpusObject::Type_Participation) {
        sql1 = "UPDATE participation SET ";
        sql2 = "WHERE communicationID = :communicationID AND speakerID = :speakerID";
    }
    QStringList attributeIDs = getEffectiveAttributeIDs(structure, what, requestedAttributeIDs);
    foreach (QString attributeID, attributeIDs) {
        if (attributeID != attributeIDs.first()) sql1.append(", ");
        sql1.append(attributeID).append(" = :").append(attributeID);
    }
    qDebug() << sql1 << " " << sql2;
    return QString("%1 %2").arg(sql1).arg(sql2);
}

// static
bool SQLSerialiserMetadata::saveCorpusObjectInfo(
        CorpusObject::Type type, const QList<CorpusObjectInfo> &list,
        QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    Q_UNUSED(datastore)
    if (!structure) return false;
    // Derive the list of attributes that will be requested from the database
    QStringList effectiveAttributeIDs = getEffectiveAttributeIDs(structure, type);
    QSqlQuery queryInsert(db), queryUpdate(db);
    queryInsert.prepare(prepareInsertSQL(structure, type));
    queryUpdate.prepare(prepareUpdateSQL(structure, type));
    bool success = true;
    foreach (CorpusObjectInfo item, list) {
        if (item.isClean()) continue;
        QSqlQuery &query(queryUpdate);
        if (item.isNew()) query = queryInsert;
        // Bind parameters
        foreach (QString attributeID, effectiveAttributeIDs) {
            query.bindValue(QString(":%1").arg(attributeID), item.attribute(attributeID));
        }
        if (query.exec()) {
            setClean(&item);
        } else {
            qDebug() << "ERROR: saveCorpusObjectInfo: " << query.lastError();
            success = false;
        }
    }
    return success;
}

// static private
bool SQLSerialiserMetadata::execSaveCorpus(Corpus *corpus, MetadataStructure *structure, QSqlDatabase &db)
{
    if (!corpus) return false;
    QSqlQuery q(db);
    if (corpus->isNew()) {
        q.prepare(prepareInsertSQL(structure, CorpusObject::Type_Corpus));
    } else {
        q.prepare("SELECT corpusID FROM corpus WHERE corpusID = :corpusID");
        q.bindValue(":corpusID", corpus->ID());
        q.exec();
        bool exists = false;
        if (q.next()) exists = true;
        if (exists) q.prepare(prepareUpdateSQL(structure, CorpusObject::Type_Corpus));
        else q.prepare(prepareInsertSQL(structure, CorpusObject::Type_Corpus));
    }
    q.bindValue(":corpusID", corpus->ID());
    q.bindValue(":corpusName", corpus->name());
    q.bindValue(":description", corpus->description());
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Corpus)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            q.bindValue(QString(":%1").arg(attribute->ID()), corpus->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) { qDebug() << "ERROR: execSaveCorpus: ID:" << corpus->ID() << " error:" << q.lastError().text(); return false; }
    return true;
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
    if (q.lastError().isValid()) { qDebug() << "ERROR: execSaveCommunication: ID:" << com->ID() << " error:" << q.lastError().text(); return false; }
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
    if (q.lastError().isValid()) { qDebug() << "ERROR: execSaveSpeaker: ID:" << spk->ID() << " error:" << q.lastError().text(); return false; }
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
    if (q.lastError().isValid()) { qDebug() << "ERROR: execSaveRecording: ID:" << rec->ID() << " error:" << q.lastError().text(); return false; }
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
    q.bindValue(":recordingID", annot->recordingID());
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Annotation)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            q.bindValue(QString(":%1").arg(attribute->ID()), annot->property(attribute->ID()));
        }
    }
    q.exec();
    if (q.lastError().isValid()) { qDebug() << "ERROR: execSaveAnnotation: ID:" << annot->ID() << " error:" << q.lastError().text(); return false; }
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
    if (q.lastError().isValid()) { qDebug() << "ERROR: execSaveParticipation: ID:" << participation->ID() << " error:" << q.lastError().text(); return false; }
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
        if (!queryCommunicationRecordingDelete.exec()) {
            qDebug() << "ERROR: queryCommunicationRecordingDelete: ID: " << recordingID << " error:" << queryCommunicationRecordingDelete.lastError().text();
        }
    }
    com->deletedRecordingIDs.clear();
    // deleted annotations
    QSqlQuery queryCommunicationAnnotationDelete(db);
    queryCommunicationAnnotationDelete.prepare("DELETE FROM annotation WHERE communicationID = :communicationID AND annotationID = :annotationID");
    queryCommunicationAnnotationDelete.bindValue(":communicationID", com->ID());
    foreach (QString annotationID, com->deletedAnnotationIDs) {
        queryCommunicationAnnotationDelete.bindValue(":annotationID", annotationID);
        if (queryCommunicationAnnotationDelete.exec()) {
            qDebug() << "ERROR: queryCommunicationAnnotationDelete: ID: " << annotationID << " error:" << queryCommunicationAnnotationDelete.lastError().text();
        }
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
    QSqlQuery queryCommunicationRecordingsDelete(db);
    queryCommunicationRecordingsDelete.prepare("DELETE FROM recording WHERE communicationID = :communicationID");
    QSqlQuery queryCommunicationAnnotationsDelete(db);
    queryCommunicationAnnotationsDelete.prepare("DELETE FROM annotation WHERE communicationID = :communicationID");
    QSqlQuery queryCommunicationParticipationsDelete(db);
    queryCommunicationParticipationsDelete.prepare("DELETE FROM participation WHERE corpusID = :corpusID AND communicationID = :communicationID");
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
    queryCommunicationParticipationsDelete.bindValue(":corpusID", corpus->ID());
    foreach (QString communicationID, corpus->deletedCommunicationIDs) {
        queryCommunicationRecordingsDelete.bindValue(":communicationID", communicationID);
        queryCommunicationRecordingsDelete.exec();
        queryCommunicationAnnotationsDelete.bindValue(":communicationID", communicationID);
        queryCommunicationAnnotationsDelete.exec();
        queryCommunicationParticipationsDelete.bindValue(":communicationID", communicationID);
        queryCommunicationParticipationsDelete.exec();
        queryCommunicationDelete.bindValue(":communicationID", communicationID);
        queryCommunicationDelete.exec();
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
    // Corpus metadata
    execSaveCorpus(corpus, structure, db);
    // Update
    foreach (CorpusCommunication *com, corpus->communications()) {
        execCleanUpCommunication(com, db);
        bool result = execSaveCommunication(com, structure, db);
        if (result) {
            setClean(com);
            datastore->setRepository(com);
            foreach (CorpusRecording *rec, com->recordings()) {
                if (execSaveRecording(com->ID(), rec, structure, db)) {
                    setClean(rec);
                    datastore->setRepository(rec);
                }
            }
            foreach (CorpusAnnotation *annot, com->annotations()) {
                if (execSaveAnnotation(com->ID(), annot, structure, db)) {
                    setClean(annot);
                    datastore->setRepository(annot);
                }
            }
        }
    }
    foreach (CorpusSpeaker *spk, corpus->speakers()) {
        if (execSaveSpeaker(spk, structure, db)) {
            setClean(spk);
            datastore->setRepository(spk);
        }
    }
    foreach (CorpusParticipation *participation, corpus->participations()) {
        if (execSaveParticipation(participation, structure, db)) {
            setClean(participation);
            datastore->setRepository(participation);
        }
    }
    setClean(corpus);
    datastore->setRepository(corpus);
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
    QSqlQuery q(db);
    db.transaction();
    q.prepare("SELECT r.recordingID FROM recording r INNER JOIN communication c ON r.communicationID=c.communicationID WHERE c.corpusID=:corpusID");
    q.bindValue(":corpusID", corpusID);
    if (!q.exec()) { db.rollback(); qDebug() << "delete recordings: " << q.lastError().text(); return false; }
    while (q.next()) {
        deleteRecording(q.value("recordingID").toString(), db, structure, datastore);
    }
    q.prepare("SELECT a.annotationID FROM annotation a INNER JOIN communication c ON a.communicationID=c.communicationID WHERE c.corpusID=:corpusID");
    q.bindValue(":corpusID", corpusID);
    if (!q.exec()) { db.rollback(); qDebug() << "delete annotations: " << q.lastError().text(); return false; }
    while (q.next()) {
        deleteAnnotation(q.value("annotationID").toString(), db, structure, datastore);
    }
    q.prepare("DELETE FROM participation WHERE corpusID=:corpusID");
    q.bindValue(":corpusID", corpusID);
    if (!q.exec()) { db.rollback(); qDebug() << "delete participations: " << q.lastError().text(); return false; }
    q.prepare("DELETE FROM communication WHERE corpusID=:corpusID");
    q.bindValue(":corpusID", corpusID);
    if (!q.exec()) { db.rollback(); qDebug() << "delete communications: " << q.lastError().text(); return false; }
    q.prepare("DELETE FROM speaker WHERE corpusID=:corpusID");
    q.bindValue(":corpusID", corpusID);
    if (!q.exec()) { db.rollback(); qDebug() << "delete speakers: " << q.lastError().text(); return false; }
    q.prepare("DELETE FROM corpus WHERE corpusID=:corpusID");
    q.bindValue(":corpusID", corpusID);
    if (!q.exec()) { db.rollback(); qDebug() << "delete corpus: " << q.lastError().text(); return false; }
    db.commit();
    return true;
}

// static
bool SQLSerialiserMetadata::deleteCommunication(const QString &communicationID,
                                                QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    Q_UNUSED(structure)
    Q_UNUSED(datastore)
    QSqlQuery q(db);
    db.transaction();
    q.prepare("DELETE FROM recording WHERE communicationID = :communicationID");
    q.bindValue(":communicationID", communicationID);
    if (!q.exec()) { db.rollback(); qDebug() << q.lastError().text(); return false; }
    q.prepare("DELETE FROM annotation WHERE communicationID = :communicationID");
    q.bindValue(":communicationID", communicationID);
    if (!q.exec()) { db.rollback(); qDebug() << q.lastError().text(); return false; }
    q.prepare("DELETE FROM participation WHERE communicationID = :communicationID");
    q.bindValue(":communicationID", communicationID);
    if (!q.exec()) { db.rollback(); qDebug() << q.lastError().text(); return false; }
    q.prepare("DELETE FROM communication WHERE communicationID = :communicationID");
    q.bindValue(":communicationID", communicationID);
    if (!q.exec()) { db.rollback(); qDebug() << q.lastError().text(); return false; }
    db.commit();
    return true;
}

// static
bool SQLSerialiserMetadata::deleteSpeaker(const QString &speakerID,
                                          QSqlDatabase &db, MetadataStructure *structure, CorpusDatastore *datastore)
{
    Q_UNUSED(structure)
    Q_UNUSED(datastore)
    QSqlQuery q(db);
    db.transaction();
    q.prepare("DELETE FROM participation WHERE speakerID = :speakerID");
    q.bindValue(":speakerID", speakerID);
    if (!q.exec()) { db.rollback(); qDebug() << q.lastError().text(); return false; }
    q.prepare("DELETE FROM speaker WHERE speakerID = :speakerID");
    q.bindValue(":speakerID", speakerID);
    if (!q.exec()) { db.rollback(); qDebug() << q.lastError().text(); return false; }
    db.commit();
    return true;
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
