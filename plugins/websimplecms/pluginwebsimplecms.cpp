#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include <QFile>
#include <QTextStream>

#include "pluginwebsimplecms.h"
#include "pncore/corpus/Corpus.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMSPrivateData {
    PluginWebSimpleCMSPrivateData() {}
    QString pathXML;
    bool includeSyllables;
};

Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::PluginWebSimpleCMS(QObject* parent) : QObject(parent)
{
    d = new PluginWebSimpleCMSPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::~PluginWebSimpleCMS()
{
    delete d;
}

bool Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

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
    VersionInformation version_info(1, 0, 0);
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
    return QString(tr("Copyright") + " 2014-2015, George Christodoulides");
}

QString Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter>  parameters;
    parameters << PluginParameter("pathXML", "Folder to save XML files to", QVariant::String, d->pathXML);
    parameters << PluginParameter("includeSyllables", "Include syllable information", QVariant::Bool, d->includeSyllables);
    return parameters;
}

void Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::setParameters(QHash<QString, QVariant> parameters)
{
    if (parameters.contains("pathXML")) d->pathXML = parameters.value("pathXML").toString();
    if (parameters.contains("includeSyllables")) d->includeSyllables = parameters.value("includeSyllables").toBool();
}

QString filterNonPrintable(QString text) {
    QString ret = text.remove("\x1F");
    return ret;
}

bool outputXML(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot,
               const QString &filename, bool includeSyllables = false)
{
    // Checks
    if (!corpus || !com || !rec || !annot) return false;
    if (!(corpus->datastoreAnnotations())) return false;

    // Begin XML file
    QFile fileXML(filename);
    if (!fileXML.open(QFile::WriteOnly | QFile::Truncate)) return false;
    QTextStream out(&fileXML);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);

    QXmlStreamWriter xml(out.device());
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("annotation_data");
    xml.writeAttribute("version", "1.0");

    // Get annotation tiers
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID());

    // recording data
    xml.writeStartElement("sound");
    xml.writeAttribute("id", rec->ID());
    xml.writeAttribute("name", rec->name());
    xml.writeAttribute("checksumMD5", rec->property("checksumMD5").toString());
    xml.writeAttribute("sound_url", com->ID() + ".wav");
    xml.writeAttribute("textgrid_url", com->ID() + ".textgrid");
    xml.writeAttribute("duration", QString::number(rec->duration().toDouble()));
    foreach (MetadataStructureSection *section, corpus->metadataStructure()->sections(CorpusObject::Type_Communication)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            if (attribute->datatype().base() == DataType::DateTime)
                xml.writeAttribute(attribute->ID(), com->property(attribute->ID()).toDate().toString("yyyy-MM-dd"));
            else if (attribute->datatype().base() == DataType::Integer)
                xml.writeAttribute(attribute->ID(), QString::number(com->property(attribute->ID()).toInt()));
            else
                xml.writeAttribute(attribute->ID(), com->property(attribute->ID()).toString());
        }
    }
    foreach (MetadataStructureSection *section, corpus->metadataStructure()->sections(CorpusObject::Type_Recording)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            if (attribute->datatype().base() == DataType::DateTime)
                xml.writeAttribute(attribute->ID(), rec->property(attribute->ID()).toDate().toString("yyyy-MM-dd"));
            else if (attribute->datatype().base() == DataType::Integer)
                xml.writeAttribute(attribute->ID(), QString::number(rec->property(attribute->ID()).toInt()));
            else
                xml.writeAttribute(attribute->ID(), rec->property(attribute->ID()).toString());
        }
    }

    // --- text and word count
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
        count++;
    }
    xml.writeAttribute("text", filterNonPrintable(text));
    xml.writeAttribute("word_count", QString::number(count));
    xml.writeEndElement(); // recording data

    // speakers data
    xml.writeStartElement("speakers");
    foreach (QString speakerID, tiersAll.keys()) {
        CorpusSpeaker *spk = corpus->speaker(speakerID);
        if (!spk) {
            xml.writeStartElement("speaker");
            xml.writeAttribute("id", speakerID);
            xml.writeAttribute("name", speakerID);
            xml.writeEndElement();
            continue;
        }
        xml.writeStartElement("speaker");
        xml.writeAttribute("id", spk->ID());
        xml.writeAttribute("name", spk->name());
        foreach (MetadataStructureSection *section, corpus->metadataStructure()->sections(CorpusObject::Type_Speaker)) {
            foreach (MetadataStructureAttribute *attribute, section->attributes()) {
                if (attribute->datatype().base() == DataType::DateTime)
                    xml.writeAttribute(attribute->ID(), spk->property(attribute->ID()).toDate().toString("yyyy-MM-dd"));
                else if (attribute->datatype().base() == DataType::Integer)
                    xml.writeAttribute(attribute->ID(), QString::number(spk->property(attribute->ID()).toInt()));
                else
                    xml.writeAttribute(attribute->ID(), spk->property(attribute->ID()).toString());
            }
        }
        xml.writeEndElement(); // speaker
        delete spk;
    }
    xml.writeEndElement(); // speakers

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

    xml.writeEndElement(); // annotation_data
    xml.writeEndDocument();
    fileXML.close();

    return true;
}

void Praaline::Plugins::WebSimpleCMS::PluginWebSimpleCMS::process(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    if (d->pathXML.isEmpty()) d->pathXML = corpus->basePath();
    int countDone = 0;
    madeProgress(0);
    printMessage("XML file creator for Simple CMS v. 1.0");
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        printMessage(QString("Creating XML file for %1").arg(com->ID()));
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            if (!rec) continue;
            foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
                if (!annot) continue;
                outputXML(corpus, com, rec, annot, d->pathXML + "/" + annot->ID() + ".xml", d->includeSyllables);
            }
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
    madeProgress(100);
    printMessage("Finished creating XML files for SimpleCMS.");
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::WebSimpleCMS;
    Q_EXPORT_PLUGIN2(PluginWebSimpleCMS, PluginWebSimpleCMS)
#endif
