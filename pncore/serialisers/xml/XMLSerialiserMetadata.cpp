#include <QString>
#include <QList>
#include <QDate>
#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "XMLSerialiserBase.h"
#include "corpus/Corpus.h"
#include "corpus/CorpusCommunication.h"
#include "corpus/CorpusSpeaker.h"
#include "corpus/CorpusRecording.h"
#include "corpus/CorpusAnnotation.h"
#include "XMLSerialiserMetadataStructure.h"
#include "XMLSerialiserAnnotationStructure.h"
#include "XMLSerialiserMetadata.h"

namespace Praaline {
namespace Core {

QString XMLSerialiserCorpus::xmlElementName_Corpus("Corpus");
QString XMLSerialiserCorpus::xmlElementName_Communication("Communication");
QString XMLSerialiserCorpus::xmlElementName_Speaker("Speaker");
QString XMLSerialiserCorpus::xmlElementName_Recording("Recording");
QString XMLSerialiserCorpus::xmlElementName_Annotation("Annotation");
QString XMLSerialiserCorpus::xmlElementName_Participation("Participation");

XMLSerialiserCorpus::XMLSerialiserCorpus()
{
}

// ========================================================================================================================================

// static
void XMLSerialiserCorpus::writeCommunication(CorpusCommunication *com, MetadataStructure *mstructure, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Communication);
    xml.writeAttribute("id", com->ID());
    xml.writeAttribute("name", com->name());
    foreach (MetadataStructureAttribute *attribute, mstructure->attributes(CorpusObject::Type_Communication)) {
        xml.writeAttribute(attribute->ID(), com->property(attribute->ID()).toString());
    }
    foreach (CorpusRecording *rec, com->recordings()) {
        writeRecording(rec, mstructure, xml);
    }
    foreach (CorpusAnnotation *annot, com->annotations()) {
        writeAnnotation(annot, mstructure, xml);
    }
    xml.writeEndElement(); // Communication
}

// static
void XMLSerialiserCorpus::writeSpeaker(CorpusSpeaker *spk, MetadataStructure *mstructure, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Speaker);
    xml.writeAttribute("id", spk->ID());
    xml.writeAttribute("name", spk->name());
    foreach (MetadataStructureAttribute *attribute, mstructure->attributes(CorpusObject::Type_Speaker)) {
        xml.writeAttribute(attribute->ID(), spk->property(attribute->ID()).toString());
    }
    xml.writeEndElement(); // Speaker
}

// static
void XMLSerialiserCorpus::writeRecording(CorpusRecording *rec, MetadataStructure *mstructure, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Recording);
    xml.writeAttribute("id", rec->ID());
    xml.writeAttribute("name", rec->name());
    xml.writeAttribute("filename", rec->filename());
    xml.writeAttribute("format", rec->format());
    xml.writeAttribute("duration", QString::number(rec->duration().toNanoseconds()));
    xml.writeAttribute("channels", QString::number(rec->channels()));
    xml.writeAttribute("sampleRate", QString::number(rec->sampleRate()));
    xml.writeAttribute("bitRate", QString::number(rec->bitRate()));
    xml.writeAttribute("encoding", rec->encoding());
    xml.writeAttribute("fileSize", QString::number(rec->fileSize()));
    xml.writeAttribute("checksumMD5", rec->checksumMD5());
    foreach (MetadataStructureAttribute *attribute, mstructure->attributes(CorpusObject::Type_Recording)) {
        xml.writeAttribute(attribute->ID(), rec->property(attribute->ID()).toString());
    }
    xml.writeEndElement(); // Recording
}

// static
void XMLSerialiserCorpus::writeAnnotation(CorpusAnnotation *annot, MetadataStructure *mstructure, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Annotation);
    xml.writeAttribute("id", annot->ID());
    xml.writeAttribute("name", annot->name());
    xml.writeAttribute("filename", annot->filename());
    xml.writeAttribute("format", annot->format());
    xml.writeStartElement("languages");
    foreach (QString language, annot->languages())
        xml.writeTextElement("language", language);
    xml.writeEndElement(); // languages
    foreach (MetadataStructureAttribute *attribute, mstructure->attributes(CorpusObject::Type_Annotation)) {
        xml.writeAttribute(attribute->ID(), annot->property(attribute->ID()).toString());
    }
    xml.writeEndElement(); // Annotation
}

// static
void XMLSerialiserCorpus::writeParticipation(CorpusParticipation *participation, MetadataStructure *mstructure, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_Participation);
    xml.writeAttribute("communicationID", participation->communicationID());
    xml.writeAttribute("speakerID", participation->speakerID());
    xml.writeAttribute("role", participation->role());
    foreach (MetadataStructureAttribute *attribute, mstructure->attributes(CorpusObject::Type_Participation)) {
        xml.writeAttribute(attribute->ID(), participation->property(attribute->ID()).toString());
    }
    xml.writeEndElement(); // Participation
}

// ====================================================================================================================

// static
CorpusCommunication *XMLSerialiserCorpus::readCommunication(MetadataStructure *mstructure, QXmlStreamReader &xml)
{
    CorpusCommunication *com = new CorpusCommunication();
    // Check that we're really reading a communication
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == xmlElementName_Communication) {
        return 0;
    }
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("id"))           com->setID(xmlAttributes.value("id").toString());
    if (xmlAttributes.hasAttribute("name"))         com->setName(xmlAttributes.value("name").toString());
    readAttributes(com, mstructure, CorpusObject::Type_Communication, xml);
    // Read recordings and annotations. The order of the elements is not fixed. We'll continue the loop until
    // we hit an EndElement named Communication.
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Communication)) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == xmlElementName_Recording) {
                CorpusRecording *rec = readRecording(mstructure, xml);
                if (rec) com->addRecording(rec);
            }
            else if (xml.name() == xmlElementName_Annotation) {
                CorpusAnnotation *annot = readAnnotation(mstructure, xml);
                if (annot) com->addAnnotation(annot);
            }
        }
        xml.readNext(); // next element
    }
    return com;
}

// static
CorpusSpeaker *XMLSerialiserCorpus::readSpeaker(MetadataStructure *mstructure, QXmlStreamReader &xml)
{
    CorpusSpeaker *spk = new CorpusSpeaker();
    // Check that we're really reading a speaker
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == xmlElementName_Speaker) {
        return 0;
    }
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("id"))           spk->setID(xmlAttributes.value("id").toString());
    if (xmlAttributes.hasAttribute("name"))         spk->setName(xmlAttributes.value("name").toString());
    readAttributes(spk, mstructure, CorpusObject::Type_Speaker, xml);
    return spk;
}

// static
CorpusRecording *XMLSerialiserCorpus::readRecording(MetadataStructure *mstructure, QXmlStreamReader &xml)
{
    CorpusRecording *rec = new CorpusRecording();
    // Check that we're really reading a recording
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == xmlElementName_Recording) {
        return 0;
    }
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("id"))           rec->setID(xmlAttributes.value("id").toString());
    if (xmlAttributes.hasAttribute("name"))         rec->setName(xmlAttributes.value("name").toString());
    if (xmlAttributes.hasAttribute("filename"))     rec->setFilename(xmlAttributes.value("filename").toString());
    if (xmlAttributes.hasAttribute("format"))       rec->setFormat(xmlAttributes.value("format").toString());
    if (xmlAttributes.hasAttribute("duration"))     rec->setDuration(RealTime::fromNanoseconds((xmlAttributes.value("duration").toLongLong())));
    if (xmlAttributes.hasAttribute("channels"))     rec->setChannels(xmlAttributes.value("channels").toInt());
    if (xmlAttributes.hasAttribute("sampleRate"))   rec->setSampleRate(xmlAttributes.value("sampleRate").toInt());
    if (xmlAttributes.hasAttribute("bitRate"))      rec->setBitRate(xmlAttributes.value("bitRate").toInt());
    if (xmlAttributes.hasAttribute("encoding"))     rec->setEncoding(xmlAttributes.value("encoding").toString());
    if (xmlAttributes.hasAttribute("fileSize"))     rec->setFileSize(xmlAttributes.value("fileSize").toLongLong());
    if (xmlAttributes.hasAttribute("checksumMD5"))  rec->setChecksumMD5(xmlAttributes.value("checksumMD5").toString());
    readAttributes(rec, mstructure, CorpusObject::Type_Recording, xml);
    return rec;
}

// static
CorpusAnnotation *XMLSerialiserCorpus::readAnnotation(MetadataStructure *mstructure, QXmlStreamReader &xml)
{
    CorpusAnnotation *annot = new CorpusAnnotation();
    // Check that we're really reading an annotation
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == xmlElementName_Annotation) {
        return 0;
    }
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("id"))           annot->setID(xmlAttributes.value("id").toString());
    if (xmlAttributes.hasAttribute("name"))         annot->setName(xmlAttributes.value("name").toString());
    if (xmlAttributes.hasAttribute("filename"))     annot->setFilename(xmlAttributes.value("filename").toString());
    if (xmlAttributes.hasAttribute("format"))       annot->setFormat(xmlAttributes.value("format").toString());
    readAttributes(annot, mstructure, CorpusObject::Type_Annotation, xml);
    // Languages
    xml.readNext(); // next element
    // The order of the elements is not fixed: loop until we hit an EndElement.
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "languages")) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "language"){
                annot->addLanguage(xml.readElementText());
            }
        }
        xml.readNext(); // next element
    }
    return annot;
}

// static
void XMLSerialiserCorpus::readParticipation(Corpus *corpus, MetadataStructure *mstructure, QXmlStreamReader &xml)
{
    // Check that we're really reading a participation
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == xmlElementName_Participation) {
        return;
    }
    QString communicationID, speakerID, role;
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("communicationID"))  communicationID = xmlAttributes.value("communicationID").toString();
    if (xmlAttributes.hasAttribute("speakerID"))        speakerID = xmlAttributes.value("speakerID").toString();
    if (xmlAttributes.hasAttribute("role"))             role = xmlAttributes.value("role").toString();
    QPointer<CorpusParticipation> participation = corpus->addParticipation(communicationID, speakerID, role);
    if (!participation) return;
    readAttributes(participation, mstructure, CorpusObject::Type_Participation, xml);
}

// static
void XMLSerialiserCorpus::readAttributes(CorpusObject *obj, MetadataStructure *mstructure, CorpusObject::Type what, QXmlStreamReader &xml)
{
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    foreach (QPointer<MetadataStructureAttribute> attribute, mstructure->attributes(what)) {
        if (xmlAttributes.hasAttribute(attribute->ID())) {
            if (attribute->datatype().base() == DataType::Integer)
                obj->setProperty(attribute->ID(),
                                 xmlAttributes.value(attribute->ID()).toInt());
            else if (attribute->datatype().base() == DataType::Double)
                obj->setProperty(attribute->ID(),
                                 xmlAttributes.value(attribute->ID()).toDouble());
            else if (attribute->datatype().base() == DataType::DateTime)
                obj->setProperty(attribute->ID(),
                                 QDateTime::fromString(xmlAttributes.value(attribute->ID()).toString(), Qt::ISODate));
            else
                obj->setProperty(attribute->ID(),
                                 xmlAttributes.value(attribute->ID()).toString());
        }
    }
}

// ========================================================================================================================================

// static
bool XMLSerialiserCorpus::saveCorpus(Corpus *corpus, QXmlStreamWriter &xml)
{
    Q_UNUSED(corpus)
    Q_UNUSED(xml)
//    xml.writeStartElement("CorpusData");
//    xml.writeStartElement("Communications");
//    foreach (CorpusCommunication *com, corpus->communications()) {
//        writeCommunication(com, corpus->metadataStructure(), xml);
//    }
//    xml.writeEndElement(); // Communications
//    xml.writeStartElement("Speakers");
//    foreach (CorpusSpeaker *spk, corpus->speakers()) {
//        writeSpeaker(spk, corpus->metadataStructure(), xml);
//    }
//    xml.writeEndElement(); // Speakers
//    xml.writeStartElement("Participations");
//    foreach (CorpusParticipation *participation, corpus->participations()) {
//        writeParticipation(participation, corpus->metadataStructure(), xml);
//    }
//    xml.writeEndElement(); // Participations
//    xml.writeEndElement(); // CorpusData
//    return true;
    return false;
}

// static
bool XMLSerialiserCorpus::loadCorpus(Corpus *corpus, QXmlStreamReader &xml)
{
    Q_UNUSED(corpus)
    Q_UNUSED(xml)
//    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Corpus)) {
//        if (xml.tokenType() == QXmlStreamReader::StartElement) {
//            if (xml.name() == "CorpusData") {
//                xml.readNext();
//                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "CorpusData")) {
//                    if (xml.tokenType() == QXmlStreamReader::StartElement) {
//                        if (xml.name() == xmlElementName_Communication) {
//                            CorpusCommunication *com = readCommunication(corpus->metadataStructure(), xml);
//                            if (com) corpus->addCommunication(com);
//                        }
//                        else if (xml.name() == xmlElementName_Speaker) {
//                            CorpusSpeaker *spk = readSpeaker(corpus->metadataStructure(), xml);
//                            if (spk) corpus->addSpeaker(spk);
//                        }
//                        else if (xml.name() == xmlElementName_Participation) {
//                            readParticipation(corpus, corpus->metadataStructure(), xml);
//                        }
//                    }
//                    xml.readNext();
//                }
//            }
//        }
//        xml.readNext(); // next element
//    }
//    return true
    return false;
}

} // namespace Core
} // namespace Praaline
