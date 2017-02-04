#include <QString>
#include <QList>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include "MocaDBSerialiserSystem.h"
#include "MocaDBSerialiserMetadata.h"

namespace Praaline {
namespace Core {

// static
QList<CorpusObjectInfo> MocaDBSerialiserMetadata::getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID,
                                                                          QSqlDatabase &db)
{
    QList<CorpusObjectInfo> list;
    QSqlQuery q(db);
    q.setForwardOnly(true);
    if      (type == CorpusObject::Type_Corpus) {
        q.prepare("SELECT m.data_id, NULL AS parent_id, x.name, x.description, m.created_by, m.created_timestamp, m.last_update_by, m.last_update_timestamp "
                  "FROM data_corpus x "
                  "INNER JOIN master_data m ON x.data_type=m.data_type AND x.corpus_id=m.data_id ");
    }
    else if (type == CorpusObject::Type_Communication) {
        q.prepare("SELECT m.data_id, r.data_id1 AS parent_id, x.name, x.description, m.created_by, m.created_timestamp, m.last_update_by, m.last_update_timestamp "
                  "FROM data_sample x "
                  "INNER JOIN master_data m ON x.data_type=m.data_type AND x.sample_id=m.data_id "
                  "INNER JOIN rel_data_data r ON x.data_type=r.data_type2 AND x.sample_id=r.data_id2 "
                  "WHERE r.data_type1 = :parent_data_type AND r.data_id1 = :parent_id ");
        q.bindValue(":parent_data_type", MocaDBSerialiserSystem::Corpus);
        q.bindValue(":parent_id", parentID);
    }
    else if (type == CorpusObject::Type_Speaker) {
        q.prepare("SELECT m.data_id, r.data_id1 AS parent_id, x.name, x.description, m.created_by, m.created_timestamp, m.last_update_by, m.last_update_timestamp "
                  "FROM data_speaker x "
                  "INNER JOIN master_data m ON x.data_type=m.data_type AND x.speaker_id=m.data_id "
                  "INNER JOIN rel_data_data r ON x.data_type=r.data_type2 AND x.speaker_id=r.data_id2 "
                  "WHERE r.data_type1 = :parent_data_type AND r.data_id1 = :parent_id ");
        q.bindValue(":parent_data_type", MocaDBSerialiserSystem::Corpus);
        q.bindValue(":parent_id", parentID);
    }
    else if (type == CorpusObject::Type_Recording) {
        q.prepare("SELECT m.data_id, r.data_id1 AS parent_id, x.name, x.description, m.created_by, m.created_timestamp, m.last_update_by, m.last_update_timestamp "
                  "FROM data_media x "
                  "INNER JOIN master_data m ON x.data_type=m.data_type AND x.media_id=m.data_id "
                  "INNER JOIN rel_data_data r ON x.data_type=r.data_type2 AND x.media_id=r.data_id2 "
                  "WHERE r.data_type1 = :parent_data_type AND r.data_id1 = :parent_id ");
        q.bindValue(":parent_data_type", MocaDBSerialiserSystem::Communication);
        q.bindValue(":parent_id", parentID);
    }
    else if (type == CorpusObject::Type_Annotation) {
        q.prepare("SELECT m.data_id, r.data_id1 AS parent_id, x.name, x.description, m.created_by, m.created_timestamp, m.last_update_by, m.last_update_timestamp "
                  "FROM data_annotation x "
                  "INNER JOIN master_data m ON x.data_type=m.data_type AND x.annotation_id=m.data_id "
                  "INNER JOIN rel_data_data r ON x.data_type=r.data_type2 AND x.annotation_id=r.data_id2 "
                  "WHERE r.data_type1 = :parent_data_type AND r.data_id1 = :parent_id ");
        q.bindValue(":parent_data_type", MocaDBSerialiserSystem::Communication);
        q.bindValue(":parent_id", parentID);
    }
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return list; }
    while (q.next()) {
        CorpusObjectInfo item;
        item.setAttribute("",  q.value("data_id")); // etc
//        foreach (QString attributeID, attributeIDs) {
//            if (q.value(attributeID).isValid()) item.setAttribute(attributeID, q.value(attributeID));
//        }
        setClean(&item);
        list << item;
    }
    return list;
}

// static, protected
bool MocaDBSerialiserMetadata::loadCorpusObjectMetadata(CorpusObject *obj, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    if (!obj) return false;
    QSqlQuery q(db);
    q.setForwardOnly(true);
    q.prepare("SELECT * FROM metadata_values WHERE data_type=:data_type AND data_id=:data_id ");
    q.bindValue(":data_type", MocaDBSerialiserSystem::getMocaDataTypeIdForPraalineCorpusObjectType(obj->type()));
    q.bindValue(":data_id", obj->ID());
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    while (q.next()) {
        QString attributeID = q.value("metadata_def_id").toString();
        QPointer<MetadataStructureAttribute> attribute = structure->attribute(obj->type(), attributeID);
        if (attribute) {
            MocaDBSerialiserSystem::MocaValueType mocaValueType = MocaDBSerialiserSystem::getMocaValueTypeIdForPraalineDataType(attribute->datatype());
            if      (mocaValueType == MocaDBSerialiserSystem::Bool)     obj->setProperty(attributeID, q.value("value_bool").toBool());
            else if (mocaValueType == MocaDBSerialiserSystem::Text)     obj->setProperty(attributeID, q.value("value_text").toString());
            else if (mocaValueType == MocaDBSerialiserSystem::LongText) obj->setProperty(attributeID, q.value("value_longtext").toString());
            else if (mocaValueType == MocaDBSerialiserSystem::Integer)  obj->setProperty(attributeID, q.value("value_int").toInt());
            else if (mocaValueType == MocaDBSerialiserSystem::DateTime) obj->setProperty(attributeID, q.value("value_datetime").toDateTime());
            else if (mocaValueType == MocaDBSerialiserSystem::Double)   obj->setProperty(attributeID, q.value("value_double").toDouble());
            else if (mocaValueType == MocaDBSerialiserSystem::Geometry) obj->setProperty(attributeID, q.value("value_geometry"));
            else { qDebug() << "Unknown type"; }
        }
    }
    return true;
}

// static
bool MocaDBSerialiserMetadata::loadCommunication(QPointer<CorpusCommunication> communication, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return loadCorpusObjectMetadata(communication, structure, db);
}

// static
bool MocaDBSerialiserMetadata::loadSpeaker(QPointer<CorpusSpeaker> speaker, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return loadCorpusObjectMetadata(speaker, structure, db);
}

// static
bool MocaDBSerialiserMetadata::loadRecording(QPointer<CorpusRecording> recording, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return loadCorpusObjectMetadata(recording, structure, db);
}

// static
bool MocaDBSerialiserMetadata::loadAnnotation(QPointer<CorpusAnnotation> annotation, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return loadCorpusObjectMetadata(annotation, structure, db);
}

// static
bool MocaDBSerialiserMetadata::loadParticipation(QPointer<CorpusParticipation> participation, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    return loadCorpusObjectMetadata(participation, structure, db);
}

//// static
//bool MocaDBSerialiserMetadata::loadCommunications(QList<QPointer<CorpusCommunication> > &communications,
//                                                  QPointer<MetadataStructure> structure, QSqlDatabase &db)
//{
//    return false;
//}

//// static
//bool MocaDBSerialiserMetadata::loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers,
//                                            QPointer<MetadataStructure> structure, QSqlDatabase &db)
//{
//    return false;
//}

//// static
//bool MocaDBSerialiserMetadata::loadRecordings(QList<QPointer<CorpusRecording> > &recordings,
//                                              QPointer<MetadataStructure> structure, QSqlDatabase &db)
//{
//    return false;
//}

//// static
//bool MocaDBSerialiserMetadata::loadAnnotations(QList<QPointer<CorpusAnnotation> > &annotations,
//                                               QPointer<MetadataStructure> structure, QSqlDatabase &db)
//{
//    return false;
//}

// static
bool MocaDBSerialiserMetadata::saveCorpusObject(CorpusObject *obj, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(structure)
    if (!obj) return false;
    QString baseTable = MocaDBSerialiserSystem::getMocaBaseTableForPraalineCorpusObjectType(obj->type());
    if (baseTable.isEmpty()) return false;
    QSqlQuery q(db);
    q.prepare("SELECT data_id, last_update_by, last_update_timestamp "
              "FROM master_data WHERE data_type=:data_type AND data_id=:data_id ");
    q.bindValue(":data_type", MocaDBSerialiserSystem::getMocaDataTypeIdForPraalineCorpusObjectType(obj->type()));
    q.bindValue(":data_id", obj->ID());
    q.exec();
    bool exists = false;
    while (q.next()) {
        exists = true;
        QDateTime lastUpdateInDb = q.value("last_update_timestamp").toDateTime();
        QString lastUpdateBy = q.value("last_update_by").toString();
        // Check if there is a newer update in the database => refuse update (optimistic concurrency check)
    }
    db.transaction();
    if (exists) {
        q.prepare(QString("UPDATE %1 SET name = :name, description = :description WHERE data_id = :data_id ").arg(baseTable));

    }
    db.commit();
    return true;
}

// static
bool MocaDBSerialiserMetadata::saveCommunication(QPointer<CorpusCommunication> com, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(com)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

// static
bool MocaDBSerialiserMetadata::saveSpeaker(QPointer<CorpusSpeaker> spk, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(spk)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

// static
bool MocaDBSerialiserMetadata::saveRecording(QPointer<CorpusRecording> rec, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(rec)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

// static
bool MocaDBSerialiserMetadata::saveAnnotation(QPointer<CorpusAnnotation> annot, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(annot)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

// static
bool MocaDBSerialiserMetadata::saveParticipation(QPointer<CorpusParticipation> participation,
                                                 QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(participation)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

// static
bool MocaDBSerialiserMetadata::deleteCommunication(const QString &communicationID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(communicationID)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

// static
bool MocaDBSerialiserMetadata::deleteSpeaker(const QString &speakerID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(speakerID)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

// static
bool MocaDBSerialiserMetadata::deleteRecording(const QString &recordingID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(recordingID)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

// static
bool MocaDBSerialiserMetadata::deleteAnnotation(const QString &annotationID, QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(annotationID)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

// static
bool MocaDBSerialiserMetadata::deleteParticipation(const QString &communicationID, const QString &speakerID,
                                                   QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    Q_UNUSED(communicationID)
    Q_UNUSED(speakerID)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

} // namespace Core
} // namespace Praaline
