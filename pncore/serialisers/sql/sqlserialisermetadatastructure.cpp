#include <QDebug>
#include <QString>
#include <QList>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "sqlserialisermetadatastructure.h"

SQLSerialiserMetadataStructure::SQLSerialiserMetadataStructure()
{
}

// static
void SQLSerialiserMetadataStructure::initialiseMetadataStructureTables(QSqlDatabase &db)
{
    QSqlQuery q(db);
    db.transaction();
    q.prepare("DROP TABLE IF EXISTS praalineMetadataSections");
    q.exec();
    if (q.lastError().isValid()) { db.rollback(); return; }
    q.prepare("CREATE TABLE praalineMetadataSections ( "
              "    objectType char(1), "
              "    sectionID varchar(32), "
              "    name varchar(128), "
              "    description varchar(1024), "
              "    PRIMARY KEY (objectType, sectionID) )");
    q.exec();
    if (q.lastError().isValid()) { db.rollback(); return; }
    q.prepare("DROP TABLE IF EXISTS praalineMetadataAttributes");
    q.exec();
    if (q.lastError().isValid()) { db.rollback(); return; }
    q.prepare("CREATE TABLE praalineMetadataAttributes ( "
              "    objectType char(1), "
              "    attributeID varchar(32), "
              "    sectionID varchar(32), "
              "    name varchar(128), "
              "    description varchar(1024), "
              "    datatype varchar(32), "
              "    length int, "
              "    isIndexed tinyint, "
              "    nameValueList varchar(32), "
              "    PRIMARY KEY (objectType, attributeID) )");
    q.exec();
    if (q.lastError().isValid()) { db.rollback(); return; }
}

// -------------------------------------------------------------------------------------------------------------------------

bool updateDatabase(MetadataStructure *structure, CorpusObject::Type what, QSqlDatabase &db)
{
    QString objectType;
    switch (what) {
        case CorpusObject::Type_Communication : objectType = "C"; break;
        case CorpusObject::Type_Speaker : objectType = "S"; break;
        case CorpusObject::Type_Recording : objectType = "R"; break;
        case CorpusObject::Type_Annotation : objectType = "A"; break;
        case CorpusObject::Type_Participation : objectType = "P"; break;
        default: return false;
    }
    QSqlQuery q1(db), q2(db), qdel(db);
    q1.prepare("INSERT INTO praalineMetadataSections (objectType, sectionID, name, description) VALUES "
               "(:objectType, :sectionID, :name, :description)");
    q1.bindValue(":objectType", objectType);
    q2.prepare("INSERT INTO praalineMetadataAttributes (objectType, attributeID, sectionID, name, description, datatype, length, isIndexed, nameValueList) VALUES "
               "(:objectType, :attributeID, :sectionID, :name, :description, :datatype, :length, :isIndexed, :nameValueList)");
    q2.bindValue(":objectType", objectType);
    //
    qdel.prepare("DELETE FROM praalineMetadataSections WHERE objectType = :objectType");
    qdel.bindValue(":objectType", objectType);
    qdel.exec();
    if (qdel.lastError().isValid()) { qDebug() << qdel.lastError(); return false; }
    qdel.prepare("DELETE FROM praalineMetadataAttributes WHERE objectType = :objectType");
    qdel.bindValue(":objectType", objectType);
    qdel.exec();
    if (qdel.lastError().isValid()) { qDebug() << qdel.lastError(); return false; }
    foreach (MetadataStructureSection *section, structure->sections(what)) {
        q1.bindValue(":sectionID", section->ID());
        q1.bindValue(":name", section->name());
        q1.bindValue(":description", section->description());
        q1.exec();
        if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return false; }
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            q2.bindValue(":attributeID", attribute->ID());
            q2.bindValue(":sectionID", section->ID());
            q2.bindValue(":name", attribute->name());
            q2.bindValue(":description", attribute->description());
            q2.bindValue(":datatype", attribute->datatype());
            q2.bindValue(":length", attribute->datalength());
            q2.bindValue(":isIndexed", (attribute->indexed()) ? 1 : 0);
            q2.bindValue(":nameValueList", attribute->nameValueList());
            q2.exec();
            if (q2.lastError().isValid()) { qDebug() << q2.lastError(); return false; }
        }
    }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::write(MetadataStructure *structure, QSqlDatabase &db)
{
    db.transaction();
    if (!updateDatabase(structure, CorpusObject::Type_Communication, db)) { db.rollback(); return false; }
    if (!updateDatabase(structure, CorpusObject::Type_Speaker, db)) { db.rollback(); return false; }
    if (!updateDatabase(structure, CorpusObject::Type_Recording, db)) { db.rollback(); return false; }
    if (!updateDatabase(structure, CorpusObject::Type_Annotation, db)) { db.rollback(); return false; }
    if (!updateDatabase(structure, CorpusObject::Type_Participation, db)) { db.rollback(); return false; }
    db.commit();
    return true;
}

// static
bool SQLSerialiserMetadataStructure::writePartial(MetadataStructure *structure, CorpusObject::Type what, QSqlDatabase &db)
{
    db.transaction();
    if (!updateDatabase(structure, what, db)) { db.rollback(); return false; }
    db.commit();
    return true;
}

// -------------------------------------------------------------------------------------------------------------------------

bool readFromDatabase(MetadataStructure *structure, QSqlDatabase &db, QString objectTypeFilter)
{
    if (!structure) return false;
    QSqlQuery q1(db), q2(db);
    q1.setForwardOnly(true);
    if (objectTypeFilter.isEmpty())
        q1.prepare("SELECT objectType, sectionID, name, description FROM praalineMetadataSections");
    else
        q1.prepare(QString("SELECT objectType, sectionID, name, description FROM praalineMetadataSections WHERE objectType = '%1'").arg(objectTypeFilter));
    q2.setForwardOnly(true);
    q2.prepare("SELECT attributeID, name, description, datatype, length, isIndexed, nameValueList FROM praalineMetadataAttributes WHERE sectionID = :sectionID");
    //
    q1.exec();
    while (q1.next()) {
        QString objectType = q1.value(0).toString();
        CorpusObject::Type object = CorpusObject::Type_Undefined;
        if (objectType == "C") object = CorpusObject::Type_Communication;
        else if (objectType == "S") object = CorpusObject::Type_Speaker;
        else if (objectType == "R") object = CorpusObject::Type_Recording;
        else if (objectType == "A") object = CorpusObject::Type_Annotation;
        else if (objectType == "P") object = CorpusObject::Type_Participation;
        MetadataStructureSection *section = new MetadataStructureSection(q1.value(1).toString(),
                                                                         q1.value(2).toString(),
                                                                         q1.value(3).toString());
        q2.bindValue(":sectionID", section->ID());
        q2.exec();
        while (q2.next()) {
            MetadataStructureAttribute *attribute = new MetadataStructureAttribute();
            attribute->setID(q2.value(0).toString());
            attribute->setName(q2.value(1).toString());
            attribute->setDescription(q2.value(2).toString());
            attribute->setDatatype(q2.value(3).toString());
            attribute->setDatalength(q2.value(4).toInt());
            if (q2.value(5).toInt() > 0) attribute->setIndexed(true); else attribute->setIndexed(false);
            attribute->setNameValueList(q2.value(6).toString());
            attribute->setParent(section);
            section->addAttribute(attribute);
        }
        section->setParent(structure);
        structure->addSection(object, section);
    }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::read(MetadataStructure *structure, QSqlDatabase &db)
{
    if (!structure) return false;
    structure->clearAll();
    return readFromDatabase(structure, db, "");
}

// static
bool SQLSerialiserMetadataStructure::readPartial(MetadataStructure *structure, CorpusObject::Type what, QSqlDatabase &db)
{
    if (!structure) return false;
    structure->clear(what);
    QString objectTypeFilter;
    switch (what) {
        case CorpusObject::Type_Communication : objectTypeFilter = "C"; break;
        case CorpusObject::Type_Speaker : objectTypeFilter = "S"; break;
        case CorpusObject::Type_Recording : objectTypeFilter = "R"; break;
        case CorpusObject::Type_Annotation : objectTypeFilter = "A"; break;
        case CorpusObject::Type_Participation : objectTypeFilter = "P"; break;
        default: objectTypeFilter = "";
    }
    return readFromDatabase(structure, db, objectTypeFilter);
}

