#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamAttribute>
#include <QFileInfo>
#include <QScopedPointer>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusObject.h"
#include "pncore/corpus/CorpusSpeaker.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"

using namespace Praaline::Core;

#include "TappingAnnotatorExperiment.h"

TappingAnnotatorExperiment::TappingAnnotatorExperiment()
{

}

IntervalTier *TappingAnnotatorExperiment::readTappingWithHold(QXmlStreamReader &xml)
{
    xml.readNext();
    QList<Interval *> intervals;
    RealTime start, end;
    bool pressed(false);
    while (!xml.atEnd() && !xml.hasError()) {
        if (xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "tapping")
            break;
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "event") {
            QString eventType; long long timer(0); int keycode; long mediaPos(0); bool isAutoRepeat;
            QXmlStreamAttributes xmlAttributes = xml.attributes();
            if (xmlAttributes.hasAttribute("type"))     eventType = xmlAttributes.value("type").toString();
            if (xmlAttributes.hasAttribute("timer"))    timer = xmlAttributes.value("timer").toLongLong();
            if (xmlAttributes.hasAttribute("keycode"))  keycode = xmlAttributes.value("keycode").toInt();
            if (xmlAttributes.hasAttribute("mediaPos")) mediaPos = xmlAttributes.value("mediaPos").toLong();
            if (xmlAttributes.hasAttribute("isAutoRepeat")) {
                isAutoRepeat = (xmlAttributes.value("isAutoRepeat").toInt() == 0) ? false : true;
            }
            bool keyOK = (keycode == 57) || (keycode == 0) || (keycode == 1);
            RealTime t = RealTime::fromMilliseconds(mediaPos);
            if      (!isAutoRepeat && eventType == "KEYPRS" && (keyOK)) {
                if (!pressed) {
                    pressed = true;
                    start = t;
                }
            }
            else if (!isAutoRepeat && eventType == "KEYREL" && (keyOK)) {
                if (pressed) {
                    pressed = false;
                    end = t;
                    Interval *intv = new Interval(start, end, "x");
                    intervals << intv;
                }
            }
        }
        xml.readNext(); // next element
    }
    return new IntervalTier("tapping", intervals);
}

bool TappingAnnotatorExperiment::readQuestionnaire(QXmlStreamReader &xml, CorpusSpeaker *participant)
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

void TappingAnnotatorExperiment::readAttributesToCorpusObject(QXmlStreamReader &xml, CorpusObject *obj, QStringList attributes)
{
    if (!obj) return;
    foreach (QString attributeID, attributes) {
        QXmlStreamAttributes xmlAttributes = xml.attributes();
        if (xmlAttributes.hasAttribute(attributeID))
            obj->setProperty(attributeID, xmlAttributes.value(attributeID).toString());
    }
}

bool correctFileTapping(const QString &filename)
{
    QFile file(filename);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return false;
    QTextStream stream(&file);
    QFile fileOut(QString(filename).replace(".txt", ".xml"));
    if (!fileOut.open( QIODevice::WriteOnly | QIODevice::Text )) return false;
    QTextStream out(&fileOut);

    do {
        QString line = stream.readLine();
        if (line.startsWith("<gamepadannotator")) {
            out << QString(line).replace(" />", ">") << "\n";
        }
        else {
            out << line << "\n";
        }
    } while (!stream.atEnd());
    return true;
}

bool TappingAnnotatorExperiment::readResultsFile(Praaline::Core::CorpusRepository *repository, const QString &corpusID, const QString &filename)
{    
    QScopedPointer<Corpus> corpus(repository->metadata()->getCorpus(corpusID));
    if (!corpus) return false;

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

    QHash<QString, QSharedPointer<IntervalTier> > tiersTapping;

    QFileInfo info(filename);
    QString participantID = QString(info.fileName()).replace("results_", "").replace(".xml", "").replace(".txt", "");
    QScopedPointer<CorpusSpeaker> participant;
    if (corpus->hasSpeaker(participantID))
        participant.reset(corpus->speaker(participantID));
    else {
        participant.reset(new CorpusSpeaker(participantID, repository));
        corpus->addSpeaker(participant.data());
    }
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
            // EXPE Hesitation: attributes << "age" << "sex" << "email" << "L1" << "hearingprob" << "musicaltraining" << "annotexperience";
            attributes << "age" << "sex" << "email" << "L1" << "hearingprob" << "musicaltraining" << "annotexperience" <<
                          "annot_accentuation" << "annot_prominence" << "annot_intonation" << "annot_prosodicunits" <<
                          "annot_background";
            readAttributesToCorpusObject(xml, participant.data(), attributes);
        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "intonation" && state == InExperiment) {

        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "workingmemory" && state == InExperiment) {
            // readWorkingMemory(xml);
        }
        else if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "tapping" && state == InExperiment) {
            QXmlStreamAttributes xmlAttributes = xml.attributes();
            QFileInfo infoStimulus(xmlAttributes.value("file").toString());
            QString stimulusID = QString(infoStimulus.fileName()).replace(".wav", "");
            QSharedPointer<IntervalTier> tapping(readTappingWithHold(xml));
            tiersTapping.insert(stimulusID, tapping);
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
    participant->save();
    foreach (QString stimulusID, tiersTapping.keys()) {
        QSharedPointer<IntervalTier> tier = tiersTapping.value(stimulusID);
        tier->setName("tapping");
        repository->annotations()->saveTier(stimulusID, participantID, tier.data());
    }

    return true;
}
