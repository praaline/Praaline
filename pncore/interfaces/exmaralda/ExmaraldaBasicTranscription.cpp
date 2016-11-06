#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include "base/RealTime.h"
#include "annotation/AnnotationTierGroup.h"
#include "annotation/AnnotationTier.h"
#include "annotation/IntervalTier.h"
#include "annotation/PointTier.h"

#include "ExmaraldaBasicTranscription.h"

namespace Praaline {
namespace Core {

bool ExmaraldaBasicTranscription::readMetadata(QXmlStreamReader &xml)
{
    // Check that we're really reading a corpus annotation
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "head") {
        return false;
    }
    xml.readNext(); // next element
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "head")) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "meta-information") {
                readMetaInformation(xml);
            }
            else if (xml.name() == "speakertable") {
                readSpeakerTable(xml);
            }
        }
        xml.readNext(); // next element
    }
    return true;
}

bool ExmaraldaBasicTranscription::writeMetadata(QXmlStreamWriter &xml)
{
    xml.writeStartElement("meta-information");
    writeMetaInformation(xml);
    xml.writeEndElement(); // meta-information
    xml.writeStartElement("speakertable");
    writeSpeakerTable(xml);
    xml.writeEndElement(); // speakertable
    return true;
}

bool ExmaraldaBasicTranscription::readMetaInformation(QXmlStreamReader &xml)
{
    // Check that we're really reading a corpus annotation
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "meta-information") {
        return false;
    }
    xml.readNext(); // next element
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "meta-information")) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "project-name") { m_projectName = xml.readElementText(); }
            else if (xml.name() == "transcription-name") { m_transcriptionName = xml.readElementText(); }
            else if (xml.name() == "transcription-convention") { m_transcriptionConvention = xml.readElementText(); }
            else if (xml.name() == "comment") { m_comment = xml.readElementText(); }
            else if (xml.name() == "referenced-file") {
                if (xml.attributes().hasAttribute("url")) {
                    m_referencedFiles.append(xml.attributes().value("url").toString());
                }
            }
            else if (xml.name() == "ud-meta-information") {
                xml.readNext(); // next element
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "ud-meta-information")) {
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "ud-information") {
                        if (xml.attributes().hasAttribute("attribute-name")) {
                            QString attributeName = xml.attributes().value("attribute-name").toString();
                            m_udMetaInfo.insert(attributeName, xml.readElementText());
                        }
                    }
                    xml.readNext(); // next element
                }
            }
        }
        xml.readNext(); // next element
    }
    return true;
}

bool ExmaraldaBasicTranscription::writeMetaInformation(QXmlStreamWriter &xml)
{
    xml.writeTextElement("project-name", m_projectName);
    xml.writeTextElement("transcription-name", m_transcriptionName);
    foreach (QString referencedFile, m_referencedFiles) {
        xml.writeStartElement("referenced-file");
        xml.writeAttribute("url", referencedFile);
        xml.writeEndElement();
    }
    xml.writeStartElement("ud-meta-information");
    foreach(QString key, m_udMetaInfo.keys()) {
        xml.writeStartElement("ud-information");
        xml.writeAttribute("attribute-name", key);
        xml.writeCharacters(m_udMetaInfo.value(key));
        xml.writeEndElement();
    }
    xml.writeEndElement(); //ud-meta-information
    xml.writeTextElement("comment", m_comment);
    xml.writeTextElement("transcription-convention", m_transcriptionConvention);
    return true;
}

bool ExmaraldaBasicTranscription::readSpeakerTable(QXmlStreamReader &xml)
{
    // Check that we're really reading a corpus annotation
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "speakertable") {
        return false;
    }
    xml.readNext(); // next element
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "speakertable")) {
        if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "speaker") {
            SpeakerInfo s = readSpeaker(xml);
            m_speakerTable.append(s);
        }
        xml.readNext(); // next element
    }
    return true;
}

ExmaraldaBasicTranscription::SpeakerInfo ExmaraldaBasicTranscription::readSpeaker(QXmlStreamReader &xml)
{
    SpeakerInfo s;
    // Check that we're really reading a speaker
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "speaker") {
        return s;
    }
    if (xml.attributes().hasAttribute("id")) {
        s.id = xml.attributes().value("id").toString();
    }
    xml.readNext(); // next element
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "speaker")) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "abbreviation") { s.abbreviation = xml.readElementText(); }
            else if (xml.name() == "languages-used") { s.languagesUsed = xml.readElementText(); }
            else if (xml.name() == "l1") { s.L1 = xml.readElementText(); }
            else if (xml.name() == "l2") { s.L2 = xml.readElementText(); }
            else if (xml.name() == "comment") { s.comment = xml.readElementText(); }
            else if (xml.name() == "sex") {
                if (xml.attributes().hasAttribute("value")) { s.sex =  xml.attributes().value("value").toString(); }
            }
            else if (xml.name() == "ud-speaker-information") {
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "ud-speaker-information")) {
                    if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "ud-information") {
                        if (xml.attributes().hasAttribute("attribute-name")) {
                            QString attributeName = xml.attributes().value("attribute-name").toString();
                            s.udMetaInfo.insert(attributeName, xml.readElementText());
                        }
                    }
                    xml.readNext(); // next element
                }
            }
        }
        xml.readNext(); // next element
    }
    return s;
}

bool ExmaraldaBasicTranscription::writeSpeakerTable(QXmlStreamWriter &xml)
{
    foreach(SpeakerInfo speaker, m_speakerTable) {
        xml.writeStartElement("speaker");
        xml.writeAttribute("id", speaker.id);
        xml.writeTextElement("abbreviation", speaker.abbreviation);
        xml.writeStartElement("sex");
        xml.writeAttribute("value", speaker.sex);
        xml.writeEndElement();
        xml.writeTextElement("languages-used", speaker.languagesUsed);
        xml.writeTextElement("l1", speaker.L1);
        xml.writeTextElement("l2", speaker.L2);
        xml.writeStartElement("ud-speaker-information");
        foreach(QString key, speaker.udMetaInfo.keys()) {
            xml.writeStartElement("ud-information");
            xml.writeAttribute("attribute-name", key);
            xml.writeCharacters(speaker.udMetaInfo.value(key));
            xml.writeEndElement();
        }
        xml.writeEndElement(); // ud-speaker-information
        xml.writeTextElement("comment", speaker.comment);
        xml.writeEndElement(); // speaker
    }
    return true;
}

bool ExmaraldaBasicTranscription::readCommonTimeline(QXmlStreamReader &xml)
{
    // Check that we're really reading a common timeline
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "common-timeline") {
        return false;
    }
    xml.readNext(); // next element
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "common-timeline")) {
        if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "tli") {
            TimelineInfo tli = readTimelineInfo(xml);
            m_commonTL.append(tli);
        }
        xml.readNext(); // next element
    }
    return true;
}

ExmaraldaBasicTranscription::TimelineInfo ExmaraldaBasicTranscription::readTimelineInfo(QXmlStreamReader &xml)
{
    TimelineInfo tli;
    // Check that we're really reading a timeline info
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "tli") {
        return tli;
    }
    if (xml.attributes().hasAttribute("id")) {
        tli.id = xml.attributes().value("id").toString();
    }
    if (xml.attributes().hasAttribute("time")) {
        tli.t = RealTime::fromSeconds(xml.attributes().value("time").toString().toDouble());
    }
    if (xml.attributes().hasAttribute("type")) {
        tli.type =  xml.attributes().value("type").toString();
    }
    return tli;
}

bool ExmaraldaBasicTranscription::writeCommonTimeline(QXmlStreamWriter &xml)
{
    xml.writeStartElement("common-timeline");
    foreach (TimelineInfo tli, m_commonTL) {
        xml.writeStartElement("tli");
        xml.writeAttribute("id", tli.id);
        xml.writeAttribute("time", QString::number(tli.t.toDouble(), 'f', 14));
        if (!tli.type.isEmpty())
           xml.writeAttribute("type", tli.type);
        xml.writeEndElement(); // tli
    }
    xml.writeEndElement(); // common-timeline
    return true;
}

bool ExmaraldaBasicTranscription::readTier(QXmlStreamReader &xml)
{
    TierInfo tier;
    QString tierID;
    // Check that we're really reading a tier
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == "tier") {
        return false;
    }
    // Attributes
    if (xml.attributes().hasAttribute("id")) {
        tierID = xml.attributes().value("id").toString();
    }
    else
        return false;
    if (xml.attributes().hasAttribute("speaker")) {
        tier.speaker = xml.attributes().value("speaker").toString();
    }
    if (xml.attributes().hasAttribute("category")) {
        tier.category = xml.attributes().value("category").toString();
    }
    if (xml.attributes().hasAttribute("type")) {
        tier.type = xml.attributes().value("type").toString();
    }
    if (xml.attributes().hasAttribute("display-name")) {
        tier.displayName = xml.attributes().value("display-name").toString();
    }
    xml.readNext(); // next element
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "tier")) {
        if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "event") {
            EventInfo event;
            if (xml.attributes().hasAttribute("start")) {
                event.start = xml.attributes().value("start").toString();
            } else continue;
            if (xml.attributes().hasAttribute("end")) {
                event.end = xml.attributes().value("end").toString();
            } else continue;
            event.text = xml.readElementText();
            tier.events.append(event);
        }
        xml.readNext(); // next element
    }
    m_tiers.insert(tierID, tier);
    return true;
}

bool ExmaraldaBasicTranscription::writeTier(QString id, QXmlStreamWriter &xml)
{
    xml.writeStartElement("tier");
    xml.writeAttribute("id", id);
    if (!m_tiers[id].speaker.isEmpty())
        xml.writeAttribute("speaker", m_tiers[id].speaker);
    xml.writeAttribute("category", m_tiers[id].category);
    xml.writeAttribute("type", m_tiers[id].type);
    xml.writeAttribute("display-name", m_tiers[id].displayName);
    foreach (EventInfo event, m_tiers[id].events) {
        xml.writeStartElement("event");
        xml.writeAttribute("start", event.start);
        xml.writeAttribute("end", event.end);
        xml.writeCharacters(event.text);
        xml.writeEndElement(); // event
    }
    xml.writeEndElement(); // tier
    return true;
}

// static
bool ExmaraldaBasicTranscription::load(const QString &filename, ExmaraldaBasicTranscription &transcription)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // emit file_error(msg);
        return false;
    }
    // Reat the XML corpus definition file
    QXmlStreamReader xml(&file);
    // Parsing Exmaralda Partitur transcription file
    while (!xml.atEnd() && !xml.hasError()) {
        // Read next element
        QXmlStreamReader::TokenType token = xml.readNext();
        // If token is just StartDocument, we'll go to next.
        if (token == QXmlStreamReader::StartDocument) continue;
        // If token is StartElement, we'll see if we can read it.
        if(token == QXmlStreamReader::StartElement) {
            if (xml.name() == "basic-transcription")
                continue;
            else if (xml.name() == "head") {
                transcription.readMetadata(xml);
            }
            else if (xml.name() == "common-timeline") {
                transcription.readCommonTimeline(xml);
            }
            else if (xml.name() == "tier") {
                transcription.readTier(xml);
            }
        }
    }
    // Error handling
    if(xml.hasError()) {
        // emit an error signal
    }
    // Removes any device() or data from the reader and resets its internal state to the initial state.
    xml.clear();
    file.close();
    return true;
}

// static
bool ExmaraldaBasicTranscription::save(const QString &filename, ExmaraldaBasicTranscription &transcription)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // emit file_error(msg);
        return false;
    }
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeComment("EXMARaLDA Transcription File created with Praaline www.praaline.org");
    xml.writeStartElement("basic-transcription");
    xml.writeStartElement("head");
    transcription.writeMetadata(xml); // writes meta-information & speakertable
    xml.writeEndElement(); // head
    xml.writeStartElement("basic-body");
    transcription.writeCommonTimeline(xml);
    // tiers
    QList<QString> tierIDs = transcription.tiers().keys();
    qSort(tierIDs);
    foreach (QString tierID, tierIDs) {
        transcription.writeTier(tierID, xml);
    }
    xml.writeEndElement(); // basic-body
    xml.writeEndElement();  // basic-transcription
    xml.writeEndDocument();
    file.close();
    return true;
}

} // namespace Core
} // namespace Praaline

