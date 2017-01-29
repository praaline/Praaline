#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamAttribute>
#include <QFileInfo>
#include <QScopedPointer>

#include "pncore/corpus/CorpusObject.h"
#include "pncore/corpus/CorpusSpeaker.h"
#include "pncore/annotation/PointTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"

using namespace Praaline::Core;

#include "SpeechRateExperiments.h"

SpeechRateExperiments::SpeechRateExperiments()
{
}

bool correctFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return false;
    QTextStream stream(&file);
    QFile fileOut(QString(filename).replace(".txt", ".xml"));
    if (!fileOut.open( QIODevice::WriteOnly | QIODevice::Text )) return false;
    QTextStream out(&fileOut);

    out << "<?xml version=\"1.0\"?>\n";
    bool inWMtest = false;
    do {
        QString line = stream.readLine();
        if (line.startsWith("<tapper")) {
            out << QString(line).replace("<tapper", "<gamepadannotator") << "\n";
        }
        else if (line.startsWith("<wmresponse") && !inWMtest) {
            out << "<workingmemory>\n";
            out << line;
            inWMtest = true;
        }
        else if (inWMtest && !line.startsWith("<wmresponse")) {
            out << "</workingmemory>\n";
            out << line;
            inWMtest = false;
        }
        else if (line.startsWith("</tapping timer")) {
            out << QString(line).replace("/tapping", "event type=\"NEXT\"").replace(">", " />") << "\n";
            out << "</tapping>\n";
        }
        else if (line.startsWith("</joystick timer")) {
            out << QString(line).replace("/joystick", "event type=\"NEXT\"").replace(">", " />") << "\n";
            out << "</joystick>\n";
        }
        else if (line.startsWith("<joypoll")) {
            QString corr = line;
            corr = corr.replace("axis0=", "axis0=\"");
            corr = corr.replace(" axis1=", "\" axis1=\"");
            corr = corr.replace(" axis2=", "\" axis2=\"");
            corr = corr.replace(" axis3=", "\" axis3=\"");
            for (int i = 0; i < 16; i++) {
                corr = corr.replace(QString(" button%1=").arg(i), QString("\" button%1=\"").arg(i));
            }
            corr = corr.replace("  />", "\" />");
            out << corr << "\n";
        }
        else if (line.startsWith("<question id=1>")) {
            out << "<question id=\"1\">\n";
        }
        else if (line.startsWith("<question id=2>")) {
            out << "<question id=\"2\">\n";
        }
        else {
            out << line << "\n";
        }
    } while (!stream.atEnd());
    out << "</gamepadannotator>\n";
    return true;
}

bool readWorkingMemory(QXmlStreamReader &xml)
{
    return false;
}

PointTier *readTapping(QXmlStreamReader &xml)
{
    xml.readNext();
    QList<Point *> points;
    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "tapping")
            break;
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "event") {
            QString eventType; long long timer(0); int keycode; long mediaPos(0);
            QXmlStreamAttributes xmlAttributes = xml.attributes();
            if (xmlAttributes.hasAttribute("type"))     eventType = xmlAttributes.value("type").toString();
            if (xmlAttributes.hasAttribute("timer"))    timer = xmlAttributes.value("timer").toLongLong();
            if (xmlAttributes.hasAttribute("keycode"))  keycode = xmlAttributes.value("keycode").toInt();
            if (xmlAttributes.hasAttribute("mediaPos")) mediaPos = xmlAttributes.value("mediaPos").toLong();
            if (eventType == "KEYPRS" && keycode == 57) {
                Point *p = new Point(RealTime::fromMilliseconds(mediaPos), "x");
                p->setAttribute("timer", timer);
                points << p;
            }
        }
        xml.readNext(); // next element
    }
    return new PointTier("tapping", points);
}

PointTier *readJoystick(QXmlStreamReader &xml)
{
    xml.readNext();
    QList<Point *> points;
    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "joystick")
            break;
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "joypoll") {
            long long timer(0); long mediaPos(0);
            QXmlStreamAttributes xmlAttributes = xml.attributes();
            if (xmlAttributes.hasAttribute("timer"))    timer = xmlAttributes.value("timer").toLongLong();
            if (xmlAttributes.hasAttribute("mediaPos")) mediaPos = xmlAttributes.value("mediaPos").toLong();
            Point *p = new Point(RealTime::fromMilliseconds(mediaPos), "");
            for (int i = 0; i < 4; ++i) {
                QString id = QString("axis%1").arg(i);
                int value = xmlAttributes.value(id).toInt();
                if (xmlAttributes.hasAttribute(id)) p->setAttribute(id, value);
            }
            p->setAttribute("timer", timer);
            points << p;
        }
        xml.readNext(); // next element
    }
    return new PointTier("joystick", points);
}

bool readQuestionnaire(QXmlStreamReader &xml, CorpusSpeaker *participant)
{
    if (!participant) return false;
    xml.readNext();
    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "questionnaire")
            break;
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "question") {
            int questionNo(0);
            QXmlStreamAttributes xmlAttributes = xml.attributes();
            if (xmlAttributes.hasAttribute("id"))    questionNo = xmlAttributes.value("id").toInt();
            QString response = xml.readElementText();
            participant->setProperty(QString("question_%1").arg(questionNo), response);
        }
        xml.readNext(); // next element
    }
    if (xml.hasError()) return false;
    return true;
}

void readAttributesToCorpusObject(QXmlStreamReader &xml, CorpusObject *obj, QStringList attributes)
{
    if (!obj) return;
    foreach (QString attributeID, attributes) {
        QXmlStreamAttributes xmlAttributes = xml.attributes();
        if (xmlAttributes.hasAttribute(attributeID))
            obj->setProperty(attributeID, xmlAttributes.value(attributeID).toString());
    }
}

bool SpeechRateExperiments::readResultsFile(CorpusRepository *repository, const QString &filename)
{
    // if (!correctFile(filename)) return false;

    QFile file(QString(filename).replace(".txt", ".xml"));
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return false;
    QXmlStreamReader xml(&file);

    enum XmlState {
        Initial,
        InGamepadFile,
        InExperiment
    };
    XmlState state = Initial;

    QString experimentDatetime;
    QString sysinfoOS;
    int sysinfoTimer(0);
    int sysinfoTimerMonotonic(0);

    bool inPauses(false);
    bool inSpeechRate(false);
    QHash<QString, QSharedPointer<PointTier> > tiersTappingBoundaries;
    QHash<QString, QSharedPointer<PointTier> > tiersTappingPauses;
    QHash<QString, QSharedPointer<PointTier> > tiersJoystickSpeechRate;
    QHash<QString, QSharedPointer<PointTier> > tiersJoystickPitchMovement;

    QFileInfo info(filename);
    QString participantID = QString(info.fileName()).replace("results_", "").replace(".xml", "").replace(".txt", "");
    QScopedPointer<CorpusSpeaker> participant(new CorpusSpeaker(participantID));
    participant->setName(participantID);

    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "gamepadannotator" && state == Initial) {
            state = InGamepadFile;
            QXmlStreamAttributes xmlAttributes = xml.attributes();
            if (xmlAttributes.hasAttribute("datetime")) experimentDatetime = xmlAttributes.value("datetime").toString();
        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "sysinfo" && state == InGamepadFile) {
            QXmlStreamAttributes xmlAttributes = xml.attributes();
            if (xmlAttributes.hasAttribute("type")) sysinfoOS = xmlAttributes.value("type").toString();
            if (xmlAttributes.hasAttribute("timer")) sysinfoTimer = xmlAttributes.value("timer").toInt();
            if (xmlAttributes.hasAttribute("timermonotonic")) sysinfoTimerMonotonic = xmlAttributes.value("timermonotonic").toInt();
        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "experiment" && state >= InGamepadFile) {
            state = InExperiment;
        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "identification" && state == InExperiment) {
            QStringList attributes;
            attributes << "age=" << "sex" << "email" << "L1" << "hearingprob" << "musicaltraining" << "annotexperience";
            readAttributesToCorpusObject(xml, participant.data(), attributes);
        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "workingmemory" && state == InExperiment) {
            readWorkingMemory(xml);
        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "tapping" && state == InExperiment) {
            QXmlStreamAttributes xmlAttributes = xml.attributes();
            QFileInfo infoStimulus(xmlAttributes.value("file").toString());
            QString stimulusID = QString(infoStimulus.fileName()).replace(".wav", "").replace("_norm", "");
            if      (stimulusID == "hollande")  inPauses = true;
            else if (stimulusID == "recteur")   inPauses = false;
            QSharedPointer<PointTier> tapping(readTapping(xml));
            if (inPauses)
                tiersTappingPauses.insert(stimulusID, tapping);
            else
                tiersTappingBoundaries.insert(stimulusID, tapping);
        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "joystick" && state == InExperiment) {
            QXmlStreamAttributes xmlAttributes = xml.attributes();
            QFileInfo infoStimulus(xmlAttributes.value("file").toString());
            QString stimulusID = QString(infoStimulus.fileName()).replace(".wav", "").replace("_norm", "");
            if      (stimulusID == "credit")     inSpeechRate = true;
            else if (stimulusID == "grenouille") inSpeechRate = false;
            QSharedPointer<PointTier> tapping(readJoystick(xml));
            if (inSpeechRate)
                tiersJoystickSpeechRate.insert(stimulusID, tapping);
            else
                tiersJoystickPitchMovement.insert(stimulusID, tapping);
        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "questionnaire" && state == InExperiment) {
            readQuestionnaire(xml, participant.data());
        }
        xml.readNext(); // next element
    }
    // Error handling
    if (xml.hasError()) {
        qDebug() << xml.errorString();
        file.close();
        return false;
    }
    xml.clear();
    file.close();
    // Complete participant data
    participant->setProperty("isExperimentalSubject", true);
    participant->setProperty("experimentDatetime", experimentDatetime);
    participant->setProperty("sysinfoOS", sysinfoOS);
    participant->setProperty("sysinfoTimer", sysinfoTimer);
    participant->setProperty("sysinfoTimerMonotonic", sysinfoTimerMonotonic);
    // Save data in the corpus
    repository->metadata()->saveSpeaker(participant.data());
    foreach (QString stimulusID, tiersTappingBoundaries.keys()) {
        QSharedPointer<PointTier> tier = tiersTappingBoundaries.value(stimulusID);
        tier->setName("tapping_boundaries");
        repository->annotations()->saveTier(stimulusID, participantID, tier.data());
    }
    foreach (QString stimulusID, tiersTappingPauses.keys()) {
        QSharedPointer<PointTier> tier = tiersTappingPauses.value(stimulusID);
        tier->setName("tapping_pauses");
        repository->annotations()->saveTier(stimulusID, participantID, tier.data());
    }
    foreach (QString stimulusID, tiersJoystickSpeechRate.keys()) {
        QSharedPointer<PointTier> tier = tiersJoystickSpeechRate.value(stimulusID);
        tier->setName("joystick_speechrate");
        repository->annotations()->saveTier(stimulusID, participantID, tier.data());
    }
    foreach (QString stimulusID, tiersJoystickPitchMovement.keys()) {
        QSharedPointer<PointTier> tier = tiersJoystickPitchMovement.value(stimulusID);
        tier->setName("joystick_pitchmovement");
        repository->annotations()->saveTier(stimulusID, participantID, tier.data());
    }
    return true;
}

