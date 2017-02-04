#include <QDebug>
#include <QXmlStreamReader>
#include "TranscriberAnnotationGraph.h"

namespace Praaline {
namespace Core {

QString TranscriberAnnotationGraph::xmlElementName_Trans("Corpus");
QString TranscriberAnnotationGraph::xmlElementName_Topic("Topic");
QString TranscriberAnnotationGraph::xmlElementName_Speaker("Speaker");
QString TranscriberAnnotationGraph::xmlElementName_Section("Section");
QString TranscriberAnnotationGraph::xmlElementName_Turn("Turn");

// private
TranscriberAnnotationGraph::TranscriberAnnotationGraph()
{
}

// private static
TranscriberAnnotationGraph::SpeakerData *TranscriberAnnotationGraph::readSpeaker(QXmlStreamReader &xml)
{
    // Check that we're really reading a speaker
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != xmlElementName_Speaker)) {
        return 0;
    }
    SpeakerData *spk = new SpeakerData();
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("id"))           spk->ID = xmlAttributes.value("id").toString();
    if (xmlAttributes.hasAttribute("name"))         spk->name = xmlAttributes.value("name").toString();
    if (xmlAttributes.hasAttribute("check"))        spk->check = xmlAttributes.value("check").toString();
    if (xmlAttributes.hasAttribute("dialect"))      spk->dialect = xmlAttributes.value("dialect").toString();
    if (xmlAttributes.hasAttribute("accent"))       spk->accent = xmlAttributes.value("accent").toString();
    if (xmlAttributes.hasAttribute("scope"))        spk->scope = xmlAttributes.value("scope").toString();
    // qDebug() << spk->ID << spk->name << spk->check << spk->dialect << spk->accent << spk->scope;
    return spk;
}

// private static
TranscriberAnnotationGraph::TurnData *TranscriberAnnotationGraph::readTurn(QXmlStreamReader &xml)
{
    // Check that we're really reading a turn
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != xmlElementName_Turn)) {
        return 0;
    }
    TurnData *turn = new TurnData();
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("startTime"))    turn->startTime = RealTime::fromSeconds(xmlAttributes.value("startTime").toDouble());
    if (xmlAttributes.hasAttribute("endTime"))      turn->endTime = RealTime::fromSeconds(xmlAttributes.value("endTime").toDouble());
    if (xmlAttributes.hasAttribute("speaker"))      turn->speakerIDs = xmlAttributes.value("speaker").toString().split(" ");
    // qDebug() << turn->startTime.toDouble() << turn->endTime.toDouble() << turn->speakerIDs;
    RealTime tMin = turn->startTime; RealTime tMax = turn->endTime;
    QString speakerID;
    QHash<QString, QString> text;
    if (turn->speakerIDs.count() > 0) speakerID = turn->speakerIDs.at(0);
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Turn)) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            // Synchronise
            if (xml.name() == "Sync") {
                if (xml.attributes().hasAttribute("time")) {
                    RealTime sync = RealTime::fromSeconds(xml.attributes().value("time").toDouble());
                    if (text.isEmpty()) {
                        tMin = sync;
                        tMax = turn->endTime;
                    } else {
                        tMax = sync;
                    }
                }
                // Add whatever interval is waiting in the buffer
                if (!text.isEmpty()) {
                    foreach (QString spkID, text.keys()) {
                        if (!text.value(spkID).isEmpty()) {
                            Interval *intv = new Interval(tMin, tMax, text.value(spkID));
                            turn->intervals[spkID].append(intv);
                        }
                    }
                    text.clear();
                    tMin = tMax;
                }
            }
            // Change of speaker?
            if (xml.name() == "Who") {
                if (xml.attributes().hasAttribute("nb")) {
                    int i = xml.attributes().value("nb").toInt();
                    if (i >= 1 && i <= turn->speakerIDs.count()) {
                        speakerID = turn->speakerIDs.at(i - 1);
                    }
                }
            }
            if (xml.name() == "Event") {
                // Add an event right here
                QString eventDesc, eventType, eventExtent;
                xmlAttributes = xml.attributes();
                if (xmlAttributes.hasAttribute("desc"))     eventDesc = xmlAttributes.value("desc").toString();
                if (xmlAttributes.hasAttribute("type"))     eventType = xmlAttributes.value("type").toString();
                if (xmlAttributes.hasAttribute("extent"))   eventExtent = xmlAttributes.value("extent").toString();
                Point *event = new Point(tMin, eventDesc);
                event->setAttribute("type", eventType);
                event->setAttribute("extent", eventExtent);
                turn->events << event;
                // Event in the middle of a transcription?
                QString t = text.value(speakerID);
                if (!t.isEmpty()) text.insert(speakerID, t.append(" "));
            }
        }
        else if (xml.tokenType() == QXmlStreamReader::Characters) {
            QString t = text.value(speakerID);
            text.insert(speakerID, t.append(xml.text().trimmed()));
        }
        xml.readNext();
    }
    // Remaining text?
    if (!text.isEmpty()) {
        foreach (QString spkID, text.keys()) {
            if (!text.value(spkID).isEmpty()) {
                Interval *intv = new Interval(tMin, (tMax == tMin) ? turn->endTime : tMax, text.value(spkID));
                turn->intervals[spkID].append(intv);
            }
        }
    }
//    foreach (QString sp, turn->intervals.keys()) {
//        foreach (Interval *intv, turn->intervals.value(sp)) {
//            qDebug() << sp << "\t" << intv->tMin().toDouble() << "\t" << intv->tMax().toDouble() << "\t" << intv->text();
//        }
//    }
//    foreach (Point *p, turn->events) {
//        qDebug() << p->time().toDouble() << p->text() << p->attribute("type").toString();
//    }

    return turn;
}

// private static
TranscriberAnnotationGraph::SectionData *TranscriberAnnotationGraph::readSection(QXmlStreamReader &xml)
{
    // Check that we're really reading a section
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() != xmlElementName_Section)) {
        return 0;
    }
    SectionData *section = new SectionData();
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("type"))         section->type = xmlAttributes.value("type").toString();
    if (xmlAttributes.hasAttribute("startTime"))    section->startTime = RealTime::fromSeconds(xmlAttributes.value("startTime").toDouble());
    if (xmlAttributes.hasAttribute("endTime"))      section->endTime = RealTime::fromSeconds(xmlAttributes.value("endTime").toDouble());
    if (xmlAttributes.hasAttribute("topic"))        section->topicID = xmlAttributes.value("topic").toString();
    // qDebug() << section->type << section->startTime.toDouble() << section->endTime.toDouble() << section->topicID;
    // Process turns
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Section)) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == xmlElementName_Turn) {
                TurnData *turn = readTurn(xml);
                if (turn) section->turns << turn;
            }
        }
        xml.readNext();
    }
    return section;
}

// private static
TranscriberAnnotationGraph::TransData *TranscriberAnnotationGraph::readTrans(QXmlStreamReader &xml)
{
    // Check that we're really reading a corpus annotation specification
    if ((xml.tokenType() != QXmlStreamReader::StartElement) || (xml.name() == xmlElementName_Trans)) {
        return 0;
    }
    // Create the new Transcriber transcription
    TransData *trans = new TransData();
    while (!(xml.atEnd()) && !(xml.hasError()) &&
           !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == xmlElementName_Trans)) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "Topics") {
                xml.readNext();
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Topics")) {
                    if (xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == xmlElementName_Topic) {
                            QString topicID, topicDesc;
                            if (xml.attributes().hasAttribute("id"))   topicID = xml.attributes().value("id").toString();
                            if (xml.attributes().hasAttribute("desc")) topicDesc = xml.attributes().value("desc").toString();
                            trans->topics.insert(topicID, topicDesc);
                            // qDebug() << "Topic" << topicID << topicDesc;
                        }
                    }
                    xml.readNext();
                }
            }
            else if (xml.name() == "Speakers") {
                xml.readNext();
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Speakers")) {
                    if (xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == xmlElementName_Speaker) {
                            SpeakerData *speaker = readSpeaker(xml);
                            if (speaker) trans->speakers << speaker;
                        }
                    }
                    xml.readNext();
                }
            }
            else if (xml.name() == "Episode") {
                xml.readNext();
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Episode")) {
                    if (xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == xmlElementName_Section) {
                            SectionData *section = readSection(xml);
                            if (section) trans->sections << section;
                        }
                    }
                    xml.readNext();
                }
            }
        }
        xml.readNext();
    }
    return trans;
}

// ====================================================================================================================
// PUBLIC


// static
bool TranscriberAnnotationGraph::load(const QString &filename, QList<QPointer<CorpusSpeaker> > &speakers,
                                      QMap<QString, QPointer<AnnotationTierGroup> > &tiersAll)
{
    TransData *trans = 0;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        // If token is StartElement, we'll see if we can read it.
        if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "Trans") {
            trans = readTrans(xml);
            file.close();
        }
        xml.readNext(); // next element
    }
    // Removes any device() or data from the reader and resets its internal state to the initial state.
    xml.clear();
    file.close();
    // Convert Transcriber annotation graph to Praaline structures
    if (!trans) return false;
    foreach (SpeakerData *spk, trans->speakers) {
        CorpusSpeaker *speaker = new CorpusSpeaker(spk->name);
        speaker->setName(spk->name);
        speaker->setProperty("check", spk->check);
        speaker->setProperty("dialect", spk->dialect);
        speaker->setProperty("accent", spk->accent);
        speaker->setProperty("scope", spk->scope);
        speakers << speaker;
        // Tiers for this speaker
        AnnotationTierGroup *tiers = new AnnotationTierGroup();
        // Create transcription for this speaker
        QList<Interval *> listTranscriptionIntervals;
        foreach (SectionData *section, trans->sections) {
            foreach (TurnData *turn, section->turns) {
                listTranscriptionIntervals << turn->intervals.value(spk->ID);
            }
        }
        IntervalTier *tierTranscription = new IntervalTier("transcription", listTranscriptionIntervals);
        tiers->addTier(tierTranscription);
        tiersAll.insert(spk->name, tiers);
    }
    return true;
}

// static
bool TranscriberAnnotationGraph::save(const QString &filename, QList<QPointer<CorpusSpeaker> > &speakers,
                                      QMap<QString, QPointer<AnnotationTierGroup> > &tiersAll)
{
    Q_UNUSED(filename)
    Q_UNUSED(speakers)
    Q_UNUSED(tiersAll)
    return true;
}

} // namespace Core
} // namespace Praaline
