#include <QFile>
#include <QXmlStreamWriter>
#include "XMLSerialiserMetadataStructure.h"

namespace Praaline {
namespace Core {

QString XMLSerialiserMetadataStructure::xmlElementName_Structure("MetadataStructure");
QString XMLSerialiserMetadataStructure::xmlElementName_Section("MetadataStructureSection");
QString XMLSerialiserMetadataStructure::xmlElementName_Attribute("MetadataStructureAttribute");

XMLSerialiserMetadataStructure::XMLSerialiserMetadataStructure()
{
}

// -------------------------------------------------------------------------------------------------------------------------

// static
bool XMLSerialiserMetadataStructure::write(MetadataStructure *structure, const QString &filename)
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
void XMLSerialiserMetadataStructure::write(MetadataStructure *structure, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Structure);
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Corpus)) {
        writeSection(CorpusObject::Type_Corpus, section, xml);
    }
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Communication)) {
        writeSection(CorpusObject::Type_Communication, section, xml);
    }
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Speaker)) {
        writeSection(CorpusObject::Type_Speaker, section, xml);
    }
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Recording)) {
        writeSection(CorpusObject::Type_Recording, section, xml);
    }
    foreach (MetadataStructureSection *section, structure->sections(CorpusObject::Type_Annotation)) {
        writeSection(CorpusObject::Type_Annotation, section, xml);
    }
    xml.writeEndElement();
}

// static
void XMLSerialiserMetadataStructure::writePartial(MetadataStructure *structure, CorpusObject::Type what, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Structure);
    foreach (MetadataStructureSection *section, structure->sections(what)) {
        writeSection(what, section, xml);
    }
    xml.writeEndElement();
}

// static
void XMLSerialiserMetadataStructure::writeSection(CorpusObject::Type what, MetadataStructureSection *section, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Section);
    switch (what) {
        case CorpusObject::Type_Corpus: xml.writeAttribute("object", "Corpus"); break;
        case CorpusObject::Type_Communication: xml.writeAttribute("object", "Communication"); break;
        case CorpusObject::Type_Speaker: xml.writeAttribute("object", "Speaker"); break;
        case CorpusObject::Type_Recording: xml.writeAttribute("object", "Recording"); break;
        case CorpusObject::Type_Annotation: xml.writeAttribute("object", "Annotation"); break;
        case CorpusObject::Type_Participation: xml.writeAttribute("object", "Participation"); break;
        default: return;
    }
    xml.writeAttribute("id", section->ID());
    xml.writeAttribute("name", section->name());
    xml.writeAttribute("description", section->description());
    foreach (MetadataStructureAttribute *attribute, section->attributes()) {
        writeAttribute(attribute, xml);
    }
    xml.writeEndElement();
}

// static
void XMLSerialiserMetadataStructure::writeAttribute(MetadataStructureAttribute *attribute, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Attribute);
    xml.writeAttribute("id", attribute->ID());
    xml.writeAttribute("name", attribute->name());
    xml.writeAttribute("description", attribute->description());
    xml.writeAttribute("datatype", attribute->datatypeString());
    xml.writeAttribute("datalength", QString::number(attribute->datatypePrecision()));
    xml.writeAttribute("indexed", (attribute->indexed()) ? "true" : "false");
    xml.writeAttribute("nameValueList", attribute->nameValueList());
    xml.writeEndElement();
}

// -------------------------------------------------------------------------------------------------------------------------

// static
MetadataStructure *XMLSerialiserMetadataStructure::read(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return new MetadataStructure();
    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        // If token is StartElement, we'll see if we can read it.
        if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == xmlElementName_Structure) {
            MetadataStructure *structure = read(xml);
            file.close();
            return structure;
        }
        xml.readNext(); // next element
    }
    // Error handling
    if(xml.hasError()) {
        file.close();
        return new MetadataStructure();
    }
    // Removes any device() or data from the reader and resets its internal state to the initial state.
    xml.clear();
    file.close();
    return new MetadataStructure();
}

// static
MetadataStructure *XMLSerialiserMetadataStructure::read(QXmlStreamReader &xml)
{
    // Create the new corpus metadata structure specification
    MetadataStructure *structure = new MetadataStructure();
    // Check that we're really reading a corpus metadata specification
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != xmlElementName_Structure) {
        return structure; // empty
    }
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Structure)) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            // parse each item:
            if (xml.name() == xmlElementName_Section) {
                if (xml.attributes().hasAttribute("object")) {
                    QString objectType = xml.attributes().value("object").toString();
                    CorpusObject::Type object = CorpusObject::Type_Undefined;
                    if (objectType == "Corpus") object = CorpusObject::Type_Corpus;
                    else if (objectType == "Communication") object = CorpusObject::Type_Communication;
                    else if (objectType == "Speaker") object = CorpusObject::Type_Speaker;
                    else if (objectType == "Recording") object = CorpusObject::Type_Recording;
                    else if (objectType == "Annotation") object = CorpusObject::Type_Annotation;
                    else if (objectType == "Participation") object = CorpusObject::Type_Participation;
                    MetadataStructureSection *section = readSection(xml);
                    if (section) {
                        section->setParent(structure);
                        structure->addSection(object, section);
                    }
                }
            }
        }
        xml.readNext(); // next element
    }
    return structure;
}

// static
MetadataStructure *XMLSerialiserMetadataStructure::readPartial(CorpusObject::Type what, QXmlStreamReader &xml)
{
    // Create the new corpus metadata structure specification
    MetadataStructure *structure = new MetadataStructure();
    // Check that we're really reading a corpus metadata specification
    if (xml.tokenType() != QXmlStreamReader::StartElement || xml.name() != xmlElementName_Structure) {
        return structure; // empty
    }
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Structure)) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            // parse each item:
            if (xml.name() == xmlElementName_Section) {
                if (xml.attributes().hasAttribute("object")) {
                    QString objectType = xml.attributes().value("object").toString();
                    CorpusObject::Type object = CorpusObject::Type_Undefined;
                    if (objectType == "Corpus") object = CorpusObject::Type_Corpus;
                    else if (objectType == "Communication") object = CorpusObject::Type_Communication;
                    else if (objectType == "Speaker") object = CorpusObject::Type_Speaker;
                    else if (objectType == "Recording") object = CorpusObject::Type_Recording;
                    else if (objectType == "Annotation") object = CorpusObject::Type_Annotation;
                    else if (objectType == "Participation") object = CorpusObject::Type_Participation;
                    // PARTIAL READ: if the section is about a different type of object, ignore
                    if (object == what) {
                        MetadataStructureSection *section = readSection(xml);
                        if (section) {
                            section->setParent(structure);
                            structure->addSection(object, section);
                        }
                    }
                }
            }
        }
        xml.readNext(); // next element
    }
    return structure;
}

// static
MetadataStructureSection *XMLSerialiserMetadataStructure::readSection(QXmlStreamReader &xml)
{
    // Create the new corpus metadata section specification
    MetadataStructureSection *section = new MetadataStructureSection();
    // Check that we're really reading a corpus metadata section specification
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == xmlElementName_Section) {
        return 0;
    }
    // Read the corpus item's attributes
    QXmlStreamAttributes attributes = xml.attributes();
    if (attributes.hasAttribute("id")) {
        section->setID(attributes.value("id").toString());
    }
    if (attributes.hasAttribute("name")) {
        section->setName(attributes.value("name").toString());
    }
    if (attributes.hasAttribute("description")) {
        section->setDescription(attributes.value("description").toString());
    }
    xml.readNext(); // next element
    // The order of the elements is not fixed: loop until we hit an EndElement.
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Section)) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == xmlElementName_Attribute) {
                MetadataStructureAttribute *attribute = readAttribute(xml);
                if (attribute) {
                    attribute->setParent(section);
                    section->addAttribute(attribute);
                }
            }
        }
        xml.readNext(); // next element
    }
    return section;
}

// static
MetadataStructureAttribute *XMLSerialiserMetadataStructure::readAttribute(QXmlStreamReader &xml)
{
    // Create the new corpus metadata section specification
    MetadataStructureAttribute *attribute = new MetadataStructureAttribute();
    // Check that we're really reading a corpus metadata section specification
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
    xml.readNext(); // next element
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Attribute)) {
        xml.readNext(); // next element
    }
    return attribute;
}

} // namespace Core
} // namespace Praaline
