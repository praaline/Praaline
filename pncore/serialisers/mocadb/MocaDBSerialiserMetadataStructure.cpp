#include <QString>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "structure/MetadataStructure.h"
#include "MocaDBSerialiserSystem.h"
#include "MocaDBSerialiserMetadataStructure.h"

namespace Praaline {
namespace Core {

// static
bool MocaDBSerialiserMetadataStructure::initialiseMetadataStructureTables(QSqlDatabase &db)
{
    Q_UNUSED(db)
    // None of our business.
    return true;
}

// static
bool MocaDBSerialiserMetadataStructure::loadMetadataStructure(QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    if (!structure) return false;
    QSqlQuery q1(db);
    q1.setForwardOnly(true);
    q1.prepare("SELECT * FROM metadata_def");
    q1.exec();
    if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return false; }
    // There are no sections in a Moca3 system. Use default sections
    structure->clearAll();
    MetadataStructureSection *sectionCorpus = new MetadataStructureSection("corpus", "Corpus", "Corpus (Moca3)", 0, structure);
    MetadataStructureSection *sectionCommunication = new MetadataStructureSection("communication", "Communication", "Communication (Moca3)", 0, structure);
    MetadataStructureSection *sectionSpeaker = new MetadataStructureSection("speaker", "Speaker", "Speaker (Moca3)", 0, structure);
    MetadataStructureSection *sectionRecording = new MetadataStructureSection("recording", "Recording", "Recording (Moca3)", 0, structure);
    MetadataStructureSection *sectionAnnotation = new MetadataStructureSection("annotation", "Annotation", "Annotation (Moca3)", 0, structure);
    MetadataStructureSection *sectionParticipation = new MetadataStructureSection("participation", "Participation", "Participation (Moca3)", 0, structure);
    structure->addSection(CorpusObject::Type_Corpus, sectionCorpus);
    structure->addSection(CorpusObject::Type_Communication, sectionCommunication);
    structure->addSection(CorpusObject::Type_Speaker, sectionSpeaker);
    structure->addSection(CorpusObject::Type_Recording, sectionRecording);
    structure->addSection(CorpusObject::Type_Annotation, sectionAnnotation);
    structure->addSection(CorpusObject::Type_Participation, sectionParticipation);
    while (q1.next()) {
        MetadataStructureSection *section = 0;
        CorpusObject::Type type = MocaDBSerialiserSystem::getPraalineCorpusObjectTypeForMocaDataTypeId(q1.value("data_type_id").toInt());
        if      (type == CorpusObject::Type_Corpus)         section = sectionCorpus;
        else if (type == CorpusObject::Type_Communication)  section = sectionCommunication;
        else if (type == CorpusObject::Type_Speaker)        section = sectionSpeaker;
        else if (type == CorpusObject::Type_Recording)      section = sectionRecording;
        else if (type == CorpusObject::Type_Annotation)     section = sectionAnnotation;
        if (section) {
            MetadataStructureAttribute *attribute = new MetadataStructureAttribute();
            attribute->setID(q1.value("metadata_def_id").toString());
            attribute->setName(q1.value("name").toString());
            attribute->setDescription(q1.value("description").toString());
            attribute->setDatatype(MocaDBSerialiserSystem::getPraalineDataTypeForMocaValueTypeId(
                                       q1.value("value_type_id").toInt()));
            attribute->setDefaultValue(q1.value("default_value"));
            attribute->setMandatory(q1.value("mandatory").toBool());
            attribute->setItemOrder(q1.value("order_metadata").toInt());
            // Extra properties of the Moca3 system
            attribute->setProperty("form_type_id", q1.value("form_type_id"));
            attribute->setProperty("system", q1.value("system"));
            attribute->setProperty("share_opt_meta_id", q1.value("share_opt_meta_id"));
            // Add attribute to section
            attribute->setParent(section);
            section->addAttribute(attribute);
        }
    }
    return true;
}

// static
bool MocaDBSerialiserMetadataStructure::createMetadataAttribute(CorpusObject::Type type,
                                                                QPointer<MetadataStructureAttribute> newAttribute, QSqlDatabase &db)
{
    QSqlQuery q(db);
    q.prepare("INSERT INTO metadata_def (form_type_id, data_type_id, value_type_id, name, description, default_value, mandatory, system, order_metadata, share_opts_meta_id) "
              "VALUES (:form_type_id, :data_type_id, :value_type_id, :name, :description, :default_value, :mandatory, :system, :order_metadata, :share_opts_meta_id) ");
    q.bindValue(":form_type_id", newAttribute->property("form_type_id"));
    q.bindValue(":data_type_id", MocaDBSerialiserSystem::getMocaDataTypeIdForPraalineCorpusObjectType(type));
    q.bindValue(":value_type_id", MocaDBSerialiserSystem::getMocaValueTypeIdForPraalineDataType(newAttribute->datatype()));
    q.bindValue(":name", newAttribute->name());
    q.bindValue(":description", newAttribute->description());
    q.bindValue(":default_value", newAttribute->defaultValue());
    q.bindValue(":mandatory", newAttribute->mandatory());
    q.bindValue(":system", newAttribute->property("system"));
    q.bindValue(":order_metadata", newAttribute->itemOrder());
    q.bindValue(":share_opts_meta_id", newAttribute->property("share_opts_meta_id"));
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    newAttribute->setID(q.lastInsertId().toString());
    return true;
}

// static
bool MocaDBSerialiserMetadataStructure::updateMetadataAttribute(CorpusObject::Type type,
                                                                QPointer<MetadataStructureAttribute> updatedAttribute, QSqlDatabase &db)
{
    Q_UNUSED(type)
    QSqlQuery q(db);
    q.prepare("UPDATE metadata_def SET form_type_id = :form_type_id, name = :name, description = :description, default_value = :default_value, "
              "mandatory = :mandatory, system = :system, order_metadata = :order_metadata, share_opts_meta_id = :share_opts_meta_id "
              "WHERE metadata_def_id = :metadata_def_id");
    q.bindValue(":form_type_id", updatedAttribute->property("form_type_id"));
    q.bindValue(":name", updatedAttribute->name());
    q.bindValue(":description", updatedAttribute->description());
    q.bindValue(":default_value", updatedAttribute->defaultValue());
    q.bindValue(":mandatory", updatedAttribute->mandatory());
    q.bindValue(":system", updatedAttribute->property(":system"));
    q.bindValue(":order_metadata", updatedAttribute->itemOrder());
    q.bindValue(":share_opts_meta_id", updatedAttribute->property("share_opts_meta_id"));
    q.bindValue(":metadata_def_id", updatedAttribute->ID());
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool MocaDBSerialiserMetadataStructure::renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID,
                                                                const QString &newAttributeID, QSqlDatabase &db)
{
    Q_UNUSED(type)
    Q_UNUSED(attributeID)
    Q_UNUSED(newAttributeID)
    Q_UNUSED(db)
    // IDs are automatically generated in Moca3, therefore this operation is meaningless.
    return false;
}

// static
bool MocaDBSerialiserMetadataStructure::retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID,
                                                                const DataType &newDataType, QSqlDatabase &db)
{
    Q_UNUSED(type)
    Q_UNUSED(attributeID)
    Q_UNUSED(newDataType)
    Q_UNUSED(db)


    return false;
}

// static
bool MocaDBSerialiserMetadataStructure::deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID, QSqlDatabase &db)
{
    Q_UNUSED(type)
    QSqlQuery q(db);
    q.prepare("DELETE FROM metadata_def WHERE metadata_def_id = :metadata_def_id");
    q.bindValue(":metadata_def_id", attributeID);
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

} // namespace Core
} // namespace Praaline
