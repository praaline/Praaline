#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include <QFile>
#include <QTextStream>
#include <QStandardPaths>

#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Datastore/FileDatastore.h"
#include "PraalineCore/Structure/MetadataStructure.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"

#include "AnalyserTemporalItem.h"
#include "PluginWebSimpleCMS.h"


using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMSPrivateData {
    PluginWebSimpleCMSPrivateData() :
    includeSyllables(false) {}

    QString pathXML;
    bool includeSyllables;
    QString levelSyllables;
    QString levelTokens;
    QStringList filledPauseTokens;
};

Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::PluginWebSimpleCMS(QObject* parent) : QObject(parent)
{
    d = new PluginWebSimpleCMSPrivateData;
    setObjectName("Web interface to SimpleCMS");
}

Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::~PluginWebSimpleCMS()
{
    delete d;
}

bool Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    // Default values for plug-in parameters
    d->pathXML = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).at(0);
    d->includeSyllables = false;
    d->levelSyllables = "";
    d->levelTokens = "tok_min";
    d->filledPauseTokens << "euh" << "euhm" << "hum";
    return true;
}

bool Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::finalize() {

}

QString Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginName() const {
    return "Web interface to SimpleCMS";
}

QtilitiesCategory Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginVersionInformation() const {
    VersionInformation version_info(2, 0, 0);
    return version_info;
}

QString Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginDescription() const {
    return tr("The WebSimpleCMS plugin generates XML files to feed websites based on SimpleCMS.");
}

QString Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginCopyright() const {
    return QString(tr("Copyright") + " 2014-2021, George Christodoulides");
}

QString Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter>  parameters;
    parameters << PluginParameter("pathXML", "Folder to save XML files to", QVariant::String, d->pathXML);
    parameters << PluginParameter("includeSyllables", "Include syllable information", QVariant::Bool, d->includeSyllables);
    parameters << PluginParameter("levelSyllables", "Annotation level for syllables", QVariant::String, d->levelSyllables);
    parameters << PluginParameter("levelTokens", "Annotation level for tokens", QVariant::String, d->levelTokens);
    parameters << PluginParameter("filledPauseTokens", "List of tokens considered filled pauses", QVariant::String, d->filledPauseTokens.join(","));
    return parameters;
}

void Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("pathXML")) d->pathXML = parameters.value("pathXML").toString();
    if (parameters.contains("includeSyllables")) d->includeSyllables = parameters.value("includeSyllables").toBool();
    if (parameters.contains("levelSyllables")) d->levelSyllables = parameters.value("levelSyllables").toString().trimmed();
    if (parameters.contains("levelTokens")) d->levelTokens = parameters.value("levelTokens").toString().trimmed();
    if (parameters.contains("filledPauseTokens")) d->filledPauseTokens = parameters.value("filledPauseTokens").toString().split(",");
}


//
// HELPER FREE FUNCTIONS
//

QString filterNonPrintable(QString text) {
    QString ret = text.remove("\x1F");
    return ret;
}

void writeAttributeToXML(QXmlStreamWriter &xml, MetadataStructureAttribute *attribute, QVariant value)
{
    if (attribute->datatype().base() == DataType::Boolean)
        xml.writeAttribute(attribute->ID(), value.toBool() ? "true" : "false");
    else if (attribute->datatype().base() == DataType::Integer)
        xml.writeAttribute(attribute->ID(), QString::number(value.toInt()));
    else if (attribute->datatype().base() == DataType::DateTime)
        xml.writeAttribute(attribute->ID(), value.toDate().toString("yyyy-MM-dd"));
    else
        xml.writeAttribute(attribute->ID(), value.toString());
}

//
// XML FILE OUTPUT
//

bool outputXML(CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot,
               const QString &filename, AnalyserTemporalItem &temporalStatistics, bool includeSyllables = false)
{
    // Checks
    if (!com || !rec || !annot) return false;
    if (!(com->repository())) return false;

    // Get annotation tiers
    SpeakerAnnotationTierGroupMap tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID());

    // Begin XML file
    QFile fileXML(filename);
    if (!fileXML.open(QFile::WriteOnly | QFile::Truncate)) return false;
    QTextStream out(&fileXML);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);

    QXmlStreamWriter xml(out.device());
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("praaline_to_simple_cms");
    xml.writeAttribute("version", "2.0");

    // ********************************************************************************************
    // Metadata
    // ********************************************************************************************
    // ********************************************************************************************
    // XML structure for metadata should be as follows
    // <communications>
    //     <communication>
    //         <recordings>  <recording />  </recordings>
    //         <annotations> <annotation /> </annotations>
    //     </communication>
    // </communications>
    // <speakers>
    //     <speaker />
    // </speakers>
    // <participations>
    //     <participation />
    // </participations>
    // <relations>
    //     <speaker_relations relationID="proximity">
    //         <speaker_relation />
    //     </speaker_relations>
    // </relations>
    // ********************************************************************************************


    xml.writeStartElement("metadata");

    // Communications: this script outputs only one communication, but there could have been more
    xml.writeStartElement("communications");
    // Communication
    xml.writeStartElement("communication");
    xml.writeAttribute("id", com->ID());
    xml.writeAttribute("name", com->name());
    foreach (MetadataStructureSection *section, com->repository()->metadataStructure()->sections(CorpusObject::Type_Communication)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            writeAttributeToXML(xml, attribute, com->property(attribute->ID()));
        }
    }
    // Recordings section
    xml.writeStartElement("recordings");
    // Recording
    xml.writeStartElement("recording");
    xml.writeAttribute("id", rec->ID());
    xml.writeAttribute("name", rec->name());
    xml.writeAttribute("checksumMD5", rec->property("checksumMD5").toString());
    xml.writeAttribute("sound_url", com->ID() + ".wav");
    xml.writeAttribute("duration", QString::number(rec->duration().toDouble()));
    foreach (MetadataStructureSection *section, com->repository()->metadataStructure()->sections(CorpusObject::Type_Recording)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            writeAttributeToXML(xml, attribute, rec->property(attribute->ID()));
        }
    }
    xml.writeEndElement(); // Recording
    // End Recordings section
    xml.writeEndElement(); // Recordings
    // Annotations section
    xml.writeStartElement("annotations");
    // Annotation
    xml.writeStartElement("annotation");
    xml.writeAttribute("id", annot->ID());
    xml.writeAttribute("name", annot->name());
    xml.writeAttribute("textgrid_url", com->ID() + ".textgrid");
    foreach (MetadataStructureSection *section, com->repository()->metadataStructure()->sections(CorpusObject::Type_Annotation)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            writeAttributeToXML(xml, attribute, annot->property(attribute->ID()));
        }
    }
    xml.writeEndElement(); // Annotation
    // End Annotations section
    xml.writeEndElement(); // Annotations
    // End Communication
    xml.writeEndElement(); // Communication
    // End Communications section
    xml.writeEndElement(); // Communications

    // Speakers Section: this script outputs the data for all speakers participating in this Communication
    xml.writeStartElement("speakers");
    foreach (QString speakerID, tiersAll.keys()) {
        CorpusSpeaker *spk = com->corpus()->speaker(speakerID);
        if (!spk) {
            xml.writeStartElement("speaker");
            xml.writeAttribute("id", speakerID);
            xml.writeAttribute("name", speakerID);
            xml.writeEndElement(); // speaker
            continue;
        }
        xml.writeStartElement("speaker");
        xml.writeAttribute("id", spk->ID());
        xml.writeAttribute("name", spk->name());
        foreach (MetadataStructureSection *section, com->repository()->metadataStructure()->sections(CorpusObject::Type_Speaker)) {
            foreach (MetadataStructureAttribute *attribute, section->attributes()) {
                writeAttributeToXML(xml, attribute, spk->property(attribute->ID()));
            }
        }
        xml.writeEndElement(); // speaker
    }
    xml.writeEndElement(); // Speakers section

    // Participations Section: this script outputs the data for all speakers participating in this Communication
    xml.writeStartElement("participations");
    foreach (QString speakerID, tiersAll.keys()) {
        CorpusParticipation *participation = com->corpus()->participation(com->ID(), speakerID);
        if (!participation) {
            xml.writeStartElement("participation");
            xml.writeAttribute("communicationID", com->ID());
            xml.writeAttribute("speakerID", speakerID);
            xml.writeEndElement(); // participation
            continue;
        }
        xml.writeStartElement("participation");
        xml.writeAttribute("communicationID", com->ID());
        xml.writeAttribute("speakerID", speakerID);
        xml.writeAttribute("role", participation->role());
        foreach (MetadataStructureSection *section, com->repository()->metadataStructure()->sections(CorpusObject::Type_Participation)) {
            foreach (MetadataStructureAttribute *attribute, section->attributes()) {
                writeAttributeToXML(xml, attribute, participation->property(attribute->ID()));
            }
        }
        xml.writeEndElement(); // participation
    }
    xml.writeEndElement(); // Participations section

    // Speaker relations
    xml.writeStartElement("relations");
    xml.writeStartElement("speaker_relations");
    xml.writeAttribute("relationID", "proximity");
    // for each
    xml.writeStartElement("speaker_relation");
    xml.writeAttribute("communicationID", com->ID());
    xml.writeAttribute("speakerID_A", "");
    xml.writeAttribute("speakerID_B", "");
    xml.writeAttribute("value", "");
    xml.writeAttribute("notes", "");
    xml.writeEndElement(); // speaker_relation
    // end for each
    xml.writeEndElement(); // speaker relations: proximity
    xml.writeEndElement(); // relations

    // End of metadata
    xml.writeEndElement();

    // ********************************************************************************************
    // Statistics
    // ********************************************************************************************

    xml.writeStartElement("statistics");
    xml.writeStartElement("temporal_statistics");
    xml.writeStartElement("communications");
    xml.writeStartElement("communication");
    xml.writeAttribute("communicationID", com->ID());
    foreach (QString measureID, temporalStatistics.measureIDsForCommunication()) {
        xml.writeAttribute(measureID, QString::number(temporalStatistics.measureCom(measureID)));
    }
    xml.writeEndElement(); // communication
    xml.writeEndElement(); // communications
    xml.writeStartElement("speakers");
    foreach (QString speakerID, temporalStatistics.speakerIDs()) {
        xml.writeStartElement("speaker");
        xml.writeAttribute("speakerID", speakerID);
        foreach (QString measureID, temporalStatistics.measureIDsForSpeaker()) {
            xml.writeAttribute(measureID, QString::number(temporalStatistics.measureSpk(speakerID, measureID)));
        }
        xml.writeEndElement(); // speaker
    }
    xml.writeEndElement(); // speakers
    xml.writeEndElement(); // temporal_statistics
    xml.writeEndElement(); // statistics

    // ********************************************************************************************
    // Transcription and word count
    // ********************************************************************************************
    QMap<RealTime, Interval *> listAllTokMin;
    foreach (QString speakerID, tiersAll.keys()) {
        IntervalTier *tier_tok_min = tiersAll.value(speakerID)->getIntervalTierByName("tok_min");
        if (!tier_tok_min) continue;
        foreach (Interval *intv, tier_tok_min->intervals()) {
            listAllTokMin.insert(intv->tMin(), intv);
        }
    }
    int count = 0;
    QString text;
    foreach (Interval *tok_min, listAllTokMin.values()) {
        text.append(tok_min->text()).append(" ");
        if ((!tok_min->isPauseSilent()) && (tok_min->text() != "#")) {
            count++;
        }
    }
    xml.writeStartElement("transcription");
    xml.writeAttribute("text", filterNonPrintable(text));
    xml.writeAttribute("word_count", QString::number(count));
    xml.writeEndElement(); // transcription


    // ********************************************************************************************
    // Annotation tables
    // ********************************************************************************************

    xml.writeStartElement("annotation");

    QMultiMap<RealTime, Interval *> listAllSegments;
    foreach (QString speakerID, tiersAll.keys()) {
        IntervalTier *tier_segment = tiersAll.value(speakerID)->getIntervalTierByName("segment");
        if (!tier_segment) continue;
        int interval_nr = 0;
        foreach (Interval *intv, tier_segment->intervals()) {
            intv->setAttribute("speakerID", speakerID);
            intv->setAttribute("interval_nr", interval_nr);
            listAllSegments.insert(intv->tMin(), intv);
            interval_nr++;
        }
    }

    QHash<QString, IntervalTier *> tok_mwu_tiers;
    QHash<QString, IntervalTier *> tok_min_tiers;
    QHash<QString, IntervalTier *> syll_tiers;
    foreach (QString speakerID, tiersAll.keys()) {
        tok_mwu_tiers.insert(speakerID, tiersAll.value(speakerID)->getIntervalTierByName("tok_mwu"));
        tok_min_tiers.insert(speakerID, tiersAll.value(speakerID)->getIntervalTierByName("tok_min"));
        syll_tiers.insert(speakerID, tiersAll.value(speakerID)->getIntervalTierByName("syll"));
    }

    // Write sound segments
    xml.writeStartElement("table_soundsegment");
    int soundsegment_id = 0;
    foreach (Interval *soundsegment, listAllSegments.values()) {
        QString speakerID = soundsegment->attribute("speakerID").toString();
        IntervalTier *tier_tokmwu = tok_mwu_tiers.value(speakerID);
        if (!tier_tokmwu) continue;
        QList<Interval *> tok_mwus = tier_tokmwu->getIntervalsContainedIn(soundsegment);
        //
        xml.writeStartElement("soundsegment");
        xml.writeAttribute("id", QString::number(soundsegment_id));
        xml.writeAttribute("speaker_id", speakerID);
        xml.writeAttribute("name", QString("%1_%2").arg(rec->ID()).arg(soundsegment_id));
        xml.writeAttribute("interval_nr", QString::number(soundsegment->attribute("interval_nr").toInt()));
        xml.writeAttribute("duration", QString::number(soundsegment->duration().toDouble()));
        xml.writeAttribute("word_count", QString::number(tok_mwus.count()));
        xml.writeAttribute("tmin", QString::number(soundsegment->tMin().toDouble()));
        xml.writeAttribute("tmax", QString::number(soundsegment->tMax().toDouble()));
        xml.writeAttribute("text", filterNonPrintable(soundsegment->text()));
        xml.writeAttribute("type", soundsegment->isPauseSilent() ? "PAUSE" : "TEXT");
        xml.writeEndElement();
        soundsegment_id++;
    }
    xml.writeEndElement(); // table_soundsegment

    xml.writeStartElement("table_tok_mwu");
    soundsegment_id = 0;
    int tokmwu_id = 0;
    foreach (Interval *soundsegment, listAllSegments.values()) {
        QString speakerID = soundsegment->attribute("speakerID").toString();
        IntervalTier *tier_tokmwu = tok_mwu_tiers.value(speakerID);
        if (!tier_tokmwu) continue;
        QPair<int, int> tokmwu_indexes = tier_tokmwu->getIntervalIndexesContainedIn(soundsegment);
        if (tokmwu_indexes.first < 0 || tokmwu_indexes.second >= tier_tokmwu->count()) continue;
        //
        for (int interval_nr = tokmwu_indexes.first; interval_nr <= tokmwu_indexes.second; ++interval_nr) {
            Interval *tokmwu = tier_tokmwu->interval(interval_nr);
            xml.writeStartElement("tokmwu");
            xml.writeAttribute("id", QString::number(tokmwu_id));
            xml.writeAttribute("speaker_id", speakerID);
            xml.writeAttribute("soundsegment_id", QString::number(soundsegment_id));
            xml.writeAttribute("interval_nr", QString::number(interval_nr));
            xml.writeAttribute("tmin", QString::number(tokmwu->tMin().toDouble()));
            xml.writeAttribute("tmax", QString::number(tokmwu->tMax().toDouble()));
            xml.writeAttribute("text", filterNonPrintable(tokmwu->text()));
            xml.writeAttribute("pos_mwu", tokmwu->attribute("pos_mwu").toString());
            xml.writeAttribute("pos_ext_mwu", tokmwu->attribute("pos_ext_mwu").toString());
            xml.writeAttribute("discourse", tokmwu->attribute("discourse").toString());
            xml.writeEndElement();
            tokmwu_id++;
        }
        soundsegment_id++;
    }
    xml.writeEndElement(); // table_tok_mwu

    xml.writeStartElement("table_tok_min");
    soundsegment_id = 0;
    int tokmin_id = 0;
    foreach (Interval *soundsegment, listAllSegments.values()) {
        QString speakerID = soundsegment->attribute("speakerID").toString();
        IntervalTier *tier_tokmin = tok_min_tiers.value(speakerID);
        if (!tier_tokmin) continue;
        QPair<int, int> tokmin_indexes =  tier_tokmin->getIntervalIndexesContainedIn(soundsegment);
        if (tokmin_indexes.first < 0 || tokmin_indexes.second >= tier_tokmin->count()) continue;
        //
        for (int interval_nr = tokmin_indexes.first; interval_nr <= tokmin_indexes.second; ++interval_nr) {
            Interval *tokmin = tier_tokmin->interval(interval_nr);
            xml.writeStartElement("tokmin");
            xml.writeAttribute("id", QString::number(tokmin_id));
            xml.writeAttribute("speaker_id", speakerID);
            xml.writeAttribute("soundsegment_id", QString::number(soundsegment_id));
            xml.writeAttribute("interval_nr", QString::number(interval_nr));
            xml.writeAttribute("tmin", QString::number(tokmin->tMin().toDouble()));
            xml.writeAttribute("tmax", QString::number(tokmin->tMax().toDouble()));
            xml.writeAttribute("text", filterNonPrintable(tokmin->text()));
            xml.writeAttribute("pos_min", tokmin->attribute("pos_min").toString());
            xml.writeAttribute("pos_ext_min", tokmin->attribute("pos_ext_min").toString());
            xml.writeAttribute("disfluency", tokmin->attribute("disfluency").toString());
            xml.writeAttribute("lemma", filterNonPrintable(tokmin->attribute("lemma_min").toString()));
            xml.writeEndElement();
            tokmin_id++;
        }
        soundsegment_id++;
    }
    xml.writeEndElement(); // table_tok_min

    if (includeSyllables) {
        xml.writeStartElement("table_syll");
        soundsegment_id = 0;
        int syll_id = 0;
        foreach (Interval *soundsegment, listAllSegments.values()) {
            QString speakerID = soundsegment->attribute("speakerID").toString();
            IntervalTier *tier_syll = syll_tiers.value(speakerID);
            if (!tier_syll) continue;
            QPair<int, int> syll_indexes =  tier_syll->getIntervalIndexesContainedIn(soundsegment);
            if (syll_indexes.first < 0 || syll_indexes.second >= tier_syll->count()) continue;
            //
            for (int interval_nr = syll_indexes.first; interval_nr <= syll_indexes.second; ++interval_nr) {
                Interval *syll = tier_syll->interval(interval_nr);
                xml.writeStartElement("syll");
                xml.writeAttribute("id", QString::number(syll_id));
                xml.writeAttribute("speaker_id", speakerID);
                xml.writeAttribute("soundsegment_id", QString::number(soundsegment_id));
                xml.writeAttribute("interval_nr", QString::number(interval_nr));
                xml.writeAttribute("tmin", QString::number(syll->tMin().toDouble()));
                xml.writeAttribute("tmax", QString::number(syll->tMax().toDouble()));
                xml.writeAttribute("text", filterNonPrintable(syll->text()));
                xml.writeAttribute("delivery", syll->attribute("delivery").toString());
                xml.writeAttribute("promgrad", QString::number(syll->attribute("prom").toInt()));
                xml.writeAttribute("syll_if", syll->attribute("syll_if").toString());
                xml.writeEndElement();
                syll_id++;
            }
            soundsegment_id++;
        }
        xml.writeEndElement(); // table_syll
    }

    xml.writeEndElement(); // annotation
    xml.writeEndDocument();
    fileXML.close();

    return true;
}

void Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::process(const QList<CorpusCommunication *> &communications)
{
    if (d->pathXML.isEmpty())
        d->pathXML = QDir::homePath();
    int countDone = 0;
    emit madeProgress(0);
    emit printMessage("XML file creator for Simple CMS v. 2.0");
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        emit printMessage(QString("Running Temporal Statistical Analysis for %1").arg(com->ID()));
        AnalyserTemporalItem temporalStatistics;
        temporalStatistics.setLevelIDTokens(d->levelTokens);
        if (d->levelSyllables.isEmpty())
            temporalStatistics.setLevelIDSyllables(d->levelTokens);
        else
            temporalStatistics.setLevelIDSyllables(d->levelSyllables);
        temporalStatistics.setFilledPauseTokens(d->filledPauseTokens);
        temporalStatistics.analyse(com);
        emit printMessage(QString("Creating XML file for %1").arg(com->ID()));
        foreach (CorpusRecording *rec, com->recordings()) {
            if (!rec) continue;
            foreach (CorpusAnnotation *annot, com->annotations()) {
                if (!annot) continue;
                outputXML(com, rec, annot, d->pathXML + "/" + annot->ID() + ".xml", temporalStatistics, d->includeSyllables);
            }
        }
        countDone++;
        emit madeProgress(countDone * 100 / communications.count());
    }
    emit madeProgress(100);
    emit printMessage("Finished creating XML files for SimpleCMS.");
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::WebSimpleCMS;
    Q_EXPORT_PLUGIN2(PluginWebSimpleCMS, PluginWebSimpleCMS)
#endif
