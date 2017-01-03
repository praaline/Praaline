#include <QString>
#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "XMLSerialiserAnnotationStructure.h"

namespace Praaline {
namespace Core {

QString XMLSerialiserAnnotationStructure::xmlElementName_Structure("AnnotationStructure");
QString XMLSerialiserAnnotationStructure::xmlElementName_Level("AnnotationStructureLevel");
QString XMLSerialiserAnnotationStructure::xmlElementName_Attribute("AnnotationStructureAttribute");

XMLSerialiserAnnotationStructure::XMLSerialiserAnnotationStructure()
{
}

// -------------------------------------------------------------------------------------------------------------------------

// static
bool XMLSerialiserAnnotationStructure::write(AnnotationStructure *structure, const QString &filename)
{
    if (!structure) return false;
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    write(structure, xml);
    xml.writeEndDocument();
    file.close();
    return true;
}

// static
void XMLSerialiserAnnotationStructure::write(AnnotationStructure *structure, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Structure);
    foreach (AnnotationStructureLevel *level, structure->levels()) {
        writeLevel(level, xml);
    }
    xml.writeEndElement();
}

QString convertLevelTypeToXML(AnnotationStructureLevel::LevelType levelType)
{
    QString ret;
    switch(levelType) {
    case AnnotationStructureLevel::IndependentPointsLevel:      ret = "independentpoints";    break;
    case AnnotationStructureLevel::IndependentIntervalsLevel:   ret = "independentintervals"; break;
    case AnnotationStructureLevel::GroupingLevel:               ret = "grouping";             break;
    case AnnotationStructureLevel::SequencesLevel:              ret = "sequences";            break;
    case AnnotationStructureLevel::TreeLevel:                   ret = "tree";                 break;
    case AnnotationStructureLevel::RelationsLevel:              ret = "relations";            break;
    default: ret = "independentintervals";
    }
    return ret;
}

AnnotationStructureLevel::LevelType convertXMLToLevelType(QString xml)
{
    if (xml == "independentpoints")     return AnnotationStructureLevel::IndependentPointsLevel;
    if (xml == "independentintervals")  return AnnotationStructureLevel::IndependentIntervalsLevel;
    if (xml == "grouping")              return AnnotationStructureLevel::GroupingLevel;
    if (xml == "sequences")             return AnnotationStructureLevel::SequencesLevel;
    if (xml == "tree")                  return AnnotationStructureLevel::TreeLevel;
    if (xml == "relations")             return AnnotationStructureLevel::RelationsLevel;
    return AnnotationStructureLevel::IndependentIntervalsLevel;
}

// static
void XMLSerialiserAnnotationStructure::writeLevel(AnnotationStructureLevel *level, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Level);
    xml.writeAttribute("id", level->ID());
    xml.writeAttribute("levelType", convertLevelTypeToXML(level->levelType()));
    xml.writeAttribute("parentLevelID", level->parentLevelID());
    xml.writeAttribute("name", level->name());
    xml.writeAttribute("description", level->description());
    xml.writeAttribute("datatype", level->datatypeString());
    xml.writeAttribute("datalength", QString::number(level->datatypePrecision()));
    xml.writeAttribute("indexed", (level->indexed()) ? "true" : "false");
    xml.writeAttribute("nameValueList", level->nameValueList());
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        writeAttribute(attribute, xml);
    }
    xml.writeEndElement();
}

// static
void XMLSerialiserAnnotationStructure::writeAttribute(AnnotationStructureAttribute *attribute, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Attribute);
    xml.writeAttribute("id", attribute->ID());
    xml.writeAttribute("name", attribute->name());
    xml.writeAttribute("description", attribute->description());
    xml.writeAttribute("datatype", attribute->datatypeString());
    xml.writeAttribute("datalength", QString::number(attribute->datatypePrecision()));
    xml.writeAttribute("indexed", (attribute->indexed()) ? "true" : "false");
    xml.writeAttribute("nameValueList", attribute->nameValueList());
    xml.writeAttribute("statLevelOfMeasurement", attribute->statLevelOfMeasurement());
    xml.writeEndElement();
}

// -------------------------------------------------------------------------------------------------------------------------

// static
AnnotationStructure *XMLSerialiserAnnotationStructure::read(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return new AnnotationStructure();
    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        // If token is StartElement, we'll see if we can read it.
        if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == xmlElementName_Structure) {
            AnnotationStructure *structure = read(xml);
            file.close();
            return structure;
        }
        xml.readNext(); // next element
    }
    // Error handling
    if(xml.hasError()) {
        file.close();
        return new AnnotationStructure();
    }
    // Removes any device() or data from the reader and resets its internal state to the initial state.
    xml.clear();
    file.close();
    return new AnnotationStructure();
}

// static
AnnotationStructure *XMLSerialiserAnnotationStructure::read(QXmlStreamReader &xml)
{
    // Create the new corpus annotation structure specification
    AnnotationStructure *structure = new AnnotationStructure();
    // Check that we're really reading a corpus annotation specification
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == xmlElementName_Structure) {
        return structure; // empty
    }
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Structure)) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            // parse each item:
            if (xml.name() == xmlElementName_Level) {
                AnnotationStructureLevel *level = readLevel(xml);
                if (level) {
                    level->setParent(structure);
                    structure->addLevel(level);
                }
            }
        }
        xml.readNext(); // next element
    }
    return structure;
}

// static
AnnotationStructureLevel *XMLSerialiserAnnotationStructure::readLevel(QXmlStreamReader &xml)
{
    // Create the new corpus annotation level specification
    AnnotationStructureLevel *level = new AnnotationStructureLevel();
    // Check that we're really reading a corpus annotation level specification
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == xmlElementName_Level) {
        return 0;
    }
    // Read the corpus item's attributes
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("id")) {
        level->setID(xmlAttributes.value("id").toString());
    }
    if (xmlAttributes.hasAttribute("levelType")) {
        level->setLevelType(convertXMLToLevelType(xmlAttributes.value("levelType").toString()));
    }
    if (xmlAttributes.hasAttribute("parentLevelID")) {
        level->setParentLevelID(xmlAttributes.value("parentLevelID").toString());
    }
    if (xmlAttributes.hasAttribute("name")) {
        level->setName(xmlAttributes.value("name").toString());
    }
    if (xmlAttributes.hasAttribute("description")) {
        level->setDescription(xmlAttributes.value("description").toString());
    }
    if (xmlAttributes.hasAttribute("datatype")) {
        level->setDatatype(DataType(xmlAttributes.value("datatype").toString()));
    }
    if (xmlAttributes.hasAttribute("datalength")) {
        level->setDatatype(DataType(level->datatype().base(), xmlAttributes.value("datalength").toInt()));
    }
    if (xmlAttributes.hasAttribute("indexed")) {
        if (xmlAttributes.value("indexed").toString() == "true") level->setIndexed(true); else level->setIndexed(false);
    }
    if (xmlAttributes.hasAttribute("nameValueList")) {
        level->setNameValueList(xmlAttributes.value("nameValueList").toString());
    }
    xml.readNext(); // next element
    // The order of the elements is not fixed: loop until we hit an EndElement.
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Level)) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == xmlElementName_Attribute) {
                AnnotationStructureAttribute *attribute = readAttribute(xml);
                if (attribute) {
                    attribute->setParent(level);
                    level->addAttribute(attribute);
                }
            }
        }
        xml.readNext(); // next element
    }
    return level;
}

// static
AnnotationStructureAttribute *XMLSerialiserAnnotationStructure::readAttribute(QXmlStreamReader &xml)
{
    // Create the new corpus annotation level specification
    AnnotationStructureAttribute *attribute = new AnnotationStructureAttribute();
    // Check that we're really reading a corpus annotation level specification
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == xmlElementName_Attribute) {
        return 0;
    }
    // Read the corpus item's attributes
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("id")) {
        attribute->setID(xmlAttributes.value("id").toString());
    }
    if (xmlAttributes.hasAttribute("name")) {
        attribute->setName(xmlAttributes.value("name").toString());
    }
    if (xmlAttributes.hasAttribute("description")) {
        attribute->setDescription(xmlAttributes.value("description").toString());
    }
    if (xmlAttributes.hasAttribute("datatype")) {
        attribute->setDatatype(DataType(xmlAttributes.value("datatype").toString()));
    }
    if (xmlAttributes.hasAttribute("datalength")) {
        attribute->setDatatype(DataType(attribute->datatype().base(), xmlAttributes.value("datalength").toInt()));
    }
    if (xmlAttributes.hasAttribute("indexed")) {
        if (xmlAttributes.value("indexed").toString() == "true") attribute->setIndexed(true); else attribute->setIndexed(false);
    }
    if (xmlAttributes.hasAttribute("nameValueList")) {
        attribute->setNameValueList(xmlAttributes.value("nameValueList").toString());
    }
    if (xmlAttributes.hasAttribute("statLevelOfMeasurement")) {
        attribute->setStatLevelOfMeasurement(xmlAttributes.value("statLevelOfMeasurement").toString());
    }
    xml.readNext(); // next element
    // The order of the elements is not fixed: loop until we hit an EndElement.
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Attribute)) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
//            if (xml.name() == "Description"){
//                attribute->readPropertiesFromXML(xml);
//            }
        }
        xml.readNext(); // next element
    }
    return attribute;
}

} // namespace Core
} // namespace Praaline
