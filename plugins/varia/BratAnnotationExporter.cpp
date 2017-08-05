#include "BratAnnotationExporter.h"

BratAnnotationExporter::BratAnnotationExporter()
{

}

QString BratAnnotationExporter::scriptEntityTypes(const QList<BratEntityType> &entityTypes)
{
    QString ret;
    ret = "collData['entity_types'] = [ ";
    foreach (BratEntityType entityType, entityTypes) {
        ret.append("{\n");
        ret.append(QString("type       : '%1',\n").arg(entityType.type));
        ret.append(QString("labels     : ['%1', '%2'],\n").arg(entityType.label).arg(entityType.labelAbbreviated));
        ret.append(QString("bgColor    : '%1',\n").arg(entityType.bgColour));
        ret.append(QString("borderColor: '%1',\n").arg(entityType.borderColour));
        ret.append("}, ");
    }
    if (ret.endsWith(", ")) ret.chop(2);
    ret.append(" ];\n");
    return ret;
}

QString BratAnnotationExporter::scriptRelationTypes(const QList<BratRelationType> &relationTypes)
{
    QString ret;
    ret = "collData['relation_types'] = [ ";
    foreach (BratRelationType relationType, relationTypes) {
        ret.append("{\n");
        ret.append(QString("type       : '%1',\n").arg(relationType.type));
        ret.append(QString("labels     : ['%1', '%2'],\n").arg(relationType.label).arg(relationType.labelAbbreviated));
        ret.append(QString("color      : '%1',\n").arg(relationType.colour));
        ret.append(QString("dashArray  : '%1',\n").arg(relationType.dashArray));
        ret.append(        "args       : [\n");
        ret.append(QString("{role: '%1', targets: ['%2']%3 },\n")
                   .arg(relationType.argumentFrom.role)
                   .arg(relationType.argumentFrom.targets.join("', '"))
                   .arg((!relationType.argumentFrom.colour.isEmpty()) ? QString(", color : '%1'").arg(relationType.argumentFrom.colour) : ""));
        ret.append(QString("{role: '%1', targets: ['%2']%3 }\n")
                   .arg(relationType.argumentTo.role)
                   .arg(relationType.argumentTo.targets.join("', '"))
                   .arg((!relationType.argumentTo.colour.isEmpty()) ? QString(", color : '%1'").arg(relationType.argumentTo.colour) : ""));
        ret.append(        "]\n");
        ret.append("}, ");
    }
    if (ret.endsWith(", ")) ret.chop(2);
    ret.append(" ];\n");
    return ret;
}

QString BratAnnotationExporter::scriptEventTypes(const QList<BratEventType> &eventTypes)
{
    QString ret;
    ret = "collData['event_types'] = [ ";
    foreach (BratEventType eventType, eventTypes) {
        ret.append("{\n");
        ret.append(QString("type       : '%1',\n").arg(eventType.type));
        ret.append(QString("labels     : ['%1', '%2'],\n").arg(eventType.label).arg(eventType.labelAbbreviated));
        ret.append(QString("bgColor    : '%1',\n").arg(eventType.bgColour));
        ret.append(QString("borderColor: '%1',\n").arg(eventType.borderColour));
        ret.append(        "arcs       : [");
        foreach (BratEventArcType arc, eventType.arcs) {
            ret.append(QString("\n{type: '%1', labels: ['%2', '%3']%4 },")
                       .arg(arc.type).arg(arc.label).arg(arc.labelAbbreviated)
                       .arg((!arc.colour.isEmpty()) ? QString(", color : '%1'").arg(arc.colour) : ""));
        }
        if (ret.endsWith(",")) ret.chop(1);
        ret.append(        "\n]\n");
        ret.append("}, ");
    }
    if (ret.endsWith(", ")) ret.chop(2);
    ret.append(" ];\n");
    return ret;
}

QString BratAnnotationExporter::scriptCollData(const BratCollData &collData)
{
    QString ret;
    ret.append("collData = {};\n\n");
    ret.append(scriptEntityTypes(collData.entityTypes)).append("\n");
    ret.append(scriptRelationTypes(collData.relationTypes)).append("\n");
    ret.append(scriptEventTypes(collData.eventTypes)).append("\n");
    return ret;
}

QString BratAnnotationExporter::scriptDocData(const BratDocData &docData)
{
    QString ret;
    ret.append("docData = {};\n\n");
    ret.append(QString("docData['text'] = \"%1\"\n").arg(docData.text));
    // Entities
    if (!docData.entities.isEmpty()) {
        ret.append("docData['entities'] = [");
        foreach (BratEntity entity, docData.entities) {
            ret.append(QString("\n    ['%1', '%2', [[%3, %4]]],").arg(entity.ID).arg(entity.type).arg(entity.start).arg(entity.end));
        }
        if (ret.endsWith(",")) ret.chop(1);
        ret.append("\n];\n");
    }
    // Relations
    if (!docData.relations.isEmpty()) {
    ret.append("docData['relations'] = [");
        foreach (BratRelation relation, docData.relations) {
            ret.append(QString("\n    ['%1', '%2', [['%3', '%4'], ['%5', '%6']]],")
                       .arg(relation.ID).arg(relation.type)
                       .arg(relation.argnameFrom).arg(relation.targetFrom)
                       .arg(relation.argnameTo).arg(relation.targetTo));
        }
        if (ret.endsWith(",")) ret.chop(1);
        ret.append("\n];\n");
    }
    // Triggers
    if (!docData.triggers.isEmpty()) {
        ret.append("docData['triggers'] = [");
        foreach (BratEntity trigger, docData.triggers) {
            ret.append(QString("\n    ['%1', '%2', [[%3, %4]]],").arg(trigger.ID).arg(trigger.type).arg(trigger.start).arg(trigger.end));
        }
        if (ret.endsWith(",")) ret.chop(1);
        ret.append("\n];\n");
    }
    // Events
    if (!docData.events.isEmpty()) {
        ret.append("docData['events'] = [");
        foreach (BratEvent event, docData.events) {
            ret.append(QString("\n    ['%1', '%2', [").arg(event.ID).arg(event.triggerID));
            foreach (BratEventArc arc, event.arcs) {
                ret.append(QString("['%1', '%2'], ").arg(arc.arcType).arg(arc.arcID));
            }
            if (ret.endsWith(", ")) ret.chop(2);
            ret.append("]],");
        }
        if (ret.endsWith(",")) ret.chop(1);
        ret.append("\n];\n");
    }
    return ret;
}


QString BratAnnotationExporter::test()
{
    BratCollData collData;
    collData.entityTypes << BratEntityType("Person", "Person", "Per", "#7fa2ff", "darken");
    collData.entityTypes << BratEntityType("Test", "Test", "T", "#7fbaee", "darken");
    collData.relationTypes << BratRelationType("Anaphora", "Anaphora", "Ana", "purple", "3,3",
                                               BratRelationArgumentType("Anaphor", QStringList() << "Person", ""),
                                               BratRelationArgumentType("Entity", QStringList() << "Person", ""));
    collData.eventTypes << BratEventType("Assassination", "Assassination", "Assas", "lightgreen", "darken",
                                         QList<BratEventArcType>() <<
                                         BratEventArcType("Victim", "Victim", "Vict") <<
                                         BratEventArcType("Perpetrator", "Perpetrator", "Perp", "green"));
    BratDocData docData;
    docData.text = "Ed O'Kelley was the man who shot the man who shot Jesse James.";
    docData.entities << BratEntity("T1", "Person", 0, 11)
                     << BratEntity("T8", "Test", 0, 23)
                     << BratEntity("T2", "Person", 20, 23)
                     << BratEntity("T3", "Person", 37, 40)
                     << BratEntity("T4", "Person", 50, 61);
    docData.relations << BratRelation("R1", "Anaphora", "Anaphor", "T2", "Entity", "T1");
    docData.triggers << BratEntity("T5", "Assassination", 45, 49)
                     << BratEntity("T6", "Assassination", 28, 32);
    docData.events << BratEvent("E1", "T5", QList<BratEventArc>() << BratEventArc("Perpetrator", "T3") << BratEventArc("Victim", "T4"))
                   << BratEvent("E2", "T6", QList<BratEventArc>() << BratEventArc("Perpetrator", "T2") << BratEventArc("Victim", "T3"));

    return scriptCollData(collData) + "\n" + scriptDocData(docData);

}
