#include <QDebug>
#include <QString>
#include <QFile>
#include <QXmlStreamReader>
#include "XMLTranscription.h"

XMLTranscription::XMLTranscription()
{
}

bool XMLTranscription::load(const QString &filename)
{
    bool result(false);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        // If token is StartElement, we'll see if we can read it.
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "text") {
            result = readText(xml);
            file.close();
        }
        xml.readNext(); // next element
    }
    // Removes any device() or data from the reader and resets its internal state to the initial state.
    xml.clear();
    file.close();
    return result;
}

bool XMLTranscription::readText(QXmlStreamReader &xml)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != "text")) return false;
    // Attributes
    if (xml.attributes().hasAttribute("topic"))         topic = xml.attributes().value("topic").toString();
    if (xml.attributes().hasAttribute("source_file"))   source_file = xml.attributes().value("source_file").toString();
    // Child elements
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "text") && !xml.atEnd()) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "paragraph") {
                ParagraphInfo paragraph;
                if (readParagraph(xml, paragraph)) paragraphs << paragraph;
            }
        }
        xml.readNext();
    }
    return true;
}

bool XMLTranscription::readParagraph(QXmlStreamReader &xml, XMLTranscription::ParagraphInfo &paragraph)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != "paragraph")) return false;
    // Attributes
    QString durationString;
    if (xml.attributes().hasAttribute("duration"))  durationString = xml.attributes().value("duration").toString();
    QString start = durationString.section("] [", 0, 0).remove("[").remove("]").remove(" ");
    QString end   = durationString.section("] [", 1, 1).remove("[").remove("]").remove(" ");
    paragraph.tMin = RealTime::fromSeconds(start.toDouble());
    paragraph.tMax = RealTime::fromSeconds(end.toDouble());
    if (xml.attributes().hasAttribute("topic"))     paragraph.topic = xml.attributes().value("topic").toString();
    // Child elements
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "paragraph") && !xml.atEnd()) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "turn") {
                TurnInfo turn;
                if (readTurn(xml, turn)) paragraph.turns << turn;
            }
        }
        xml.readNext();
    }
    return true;
}

bool XMLTranscription::readTurn(QXmlStreamReader &xml, TurnInfo &turn)
{
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != "turn")) return false;
    // Attributes
    if (xml.attributes().hasAttribute("speaker"))     turn.speakerID = xml.attributes().value("speaker").toString();
    // Text
    QString text = xml.readElementText(QXmlStreamReader::IncludeChildElements);
    // qDebug() << turn.speakerID << " " << text;
    turn.transcription = text.trimmed().replace("  ", " ");
    return true;
}

QStringList XMLTranscription::speakerIDs()
{
    QStringList speakerIDs;
    foreach (ParagraphInfo paragraph, paragraphs) {
        foreach (TurnInfo turn, paragraph.turns) {
            if (!speakerIDs.contains(turn.speakerID)) speakerIDs << turn.speakerID;
        }
    }
    return speakerIDs;
}

