#include <QString>
#include <QList>
#include <QStringList>
#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include <QFile>
#include <QTextStream>

#include "chunkannotator.h"
#include "svbridge.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

#include "pncore/interfaces/anvil/AnvilMetadataTranscript.h"

#include "InterraterAgreement.h"
#include "ProsodicBoundariesExperimentAnalysis.h"
#include "MyExperiments.h"
#include "ProsodyCourse.h"
#include "DisfluenciesExperiments.h"
#include "SpeechRateExperiments.h"
#include "TappingAnnotatorExperiment.h"
#include "MelissaExperiment.h"
#include "MacroprosodyExperiment.h"
#include "SequencerSyntax.h"
#include "SequencerDisfluencies.h"
#include "SequencerProsodicUnits.h"
#include "SequencerCombineUnits.h"
#include "BratAnnotationExporter.h"
#include "CPROMDISS.h"
#include "SilentPauseManipulator.h"

#include "pluginvaria.h"


using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Varia::PluginVariaPrivateData {
    PluginVariaPrivateData() {}
};

Praaline::Plugins::Varia::PluginVaria::PluginVaria(QObject* parent) : QObject(parent)
{
    d = new PluginVariaPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::Varia::PluginVaria::~PluginVaria()
{
    delete d;
}

bool Praaline::Plugins::Varia::PluginVaria::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::Varia::PluginVaria::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::Varia::PluginVaria::finalize() {

}

QString Praaline::Plugins::Varia::PluginVaria::pluginName() const {
    return "Varia";
}

QtilitiesCategory Praaline::Plugins::Varia::PluginVaria::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::Varia::PluginVaria::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::Varia::PluginVaria::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::Varia::PluginVaria::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::Varia::PluginVaria::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::Varia::PluginVaria::pluginDescription() const {
    return tr("Varia is a part-of-speech tagger, multi-word unit and disfluency detector for spoken language transcriptions.");
}

QString Praaline::Plugins::Varia::PluginVaria::pluginCopyright() const {
    return QString(tr("Copyright") + " 2012-2014, George Christodoulides");
}

QString Praaline::Plugins::Varia::PluginVaria::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::Varia::PluginVaria::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    return parameters;
}

void Praaline::Plugins::Varia::PluginVaria::setParameters(const QHash<QString, QVariant> &parameters)
{
    Q_UNUSED(parameters)
}

void chunk(QList<QPointer<CorpusCommunication> > communications) {
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_mwu");
                if (!tier_tokens) continue;
                IntervalTier *tier_chunks = tiers->getIntervalTierByName("chunk");
                if (!tier_chunks) {
                    tier_chunks = new IntervalTier("chunk", tiers->tMin(), tiers->tMax());
                    tiers->addTier(tier_chunks);
                }
                ChunkAnnotator chunker(tier_tokens, tier_chunks, "pos_mwu");
                chunker.annotate("d:/chunker/chunker_fr.model");
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_chunks);

                // PraatTextGrid::save(corpus->basePath() + "/" + com->ID() + ".TextGrid", tiers);
            }
            qDeleteAll(tiersAll);
        }
    }
}

QString valibelTranscription(const QList<QPointer<CorpusCommunication> > &communications)
{
    QString ret;
    // Creates a transcription
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            QString transcription;
            QList<Interval *> tokens = com->repository()->annotations()->getIntervals(AnnotationDatastore::Selection(annotationID, "", "tok_min"));
            if (tokens.isEmpty()) continue;
            QString currentSpeakerID = tokens.first()->attribute("speakerID").toString();
            transcription.append(currentSpeakerID).append("\t");
            for (int i = 0; i < tokens.count(); ++i) {
                Interval *token = tokens.at(i);
                if (token->isPauseSilent() && i == 0) continue;
                QString speakerID = token->attribute("speakerID").toString();
                if (currentSpeakerID != speakerID) {
                    QString nextSpeakerID = (i < tokens.count() - 1) ? tokens.at(i+1)->attribute("speakerID").toString() : "";
                    if (nextSpeakerID == currentSpeakerID && token->isPauseSilent()) continue;
                    transcription.append("\n").append(speakerID).append("\t");
                    currentSpeakerID = speakerID;
                }
                if (token->isPauseSilent() && token->duration().toDouble() >= 0.250)
                    transcription.append("// ");
                else if (token->isPauseSilent() && token->duration().toDouble() < 0.250)
                    transcription.append("/ ");
                else
                    transcription.append(token->text().trimmed()).append(" ");
            }
            ret.append(com->ID()).append("\n");
            ret.append(transcription).append("\n\n");
        }
    }
    return ret;
}

void expeHesitation(const QList<QPointer<CorpusCommunication> > &communications)
{
    if (communications.isEmpty()) return;
    QPointer<CorpusRepository> repository = communications.first()->repository();
    QString corpusID = communications.first()->corpusID();

    TappingAnnotatorExperiment texp;
    QString path = "/home/george/Dropbox/2017_Perception_of_hesitation_expe/result_raw_files";
    QDirIterator iterator(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        QString filename = iterator.next();
        texp.readResultsFile(repository, corpusID, filename);
        // printMessage(filename);
    }
}

#include "XMLTranscription.h"
void importJohannaFiles(const QList<QPointer<CorpusCommunication> > &communications)
{
    QString path = "/home/george/Dropbox/Annotation allemand/";
    if (communications.isEmpty()) return;
    QPointer<CorpusRepository> repository = communications.first()->repository();
    QPointer<Corpus> corpus = communications.first()->corpus();
    foreach (QPointer<CorpusCommunication> com, communications) {
        XMLTranscription xml;
        xml.load(path + com->ID() + ".xml");
        QHash<QString, QList<Interval *> > allIntervals;
        foreach (QString speakerID, xml.speakerIDs()) {
            if (!corpus->hasSpeaker(speakerID)) {
                CorpusSpeaker *spk = new CorpusSpeaker(speakerID);
                spk->setName(speakerID);
                corpus->addSpeaker(spk);
                corpus->save();
            }
            allIntervals.insert(speakerID, QList<Interval *>());
        }
        int i = 0;
        foreach (XMLTranscription::ParagraphInfo para, xml.paragraphs) {
            foreach (XMLTranscription::TurnInfo turn, para.turns) {
                allIntervals[turn.speakerID] << new Interval(RealTime::fromSeconds(i), RealTime::fromSeconds(i + 1),
                                                             turn.transcription);
                i++;
            }
        }
        foreach (QString speakerID, xml.speakerIDs()) {
            IntervalTier *tier_transcription = new IntervalTier("transcription", allIntervals[speakerID]);
            repository->annotations()->saveTier(com->ID(), speakerID, tier_transcription);

        }
    }

}

void cuttingStimuli(const QList<QPointer<CorpusCommunication> > &communications)
{
    QString path = "/home/george/Dropbox/Annotation allemand/";
    if (communications.isEmpty()) return;
    QPointer<CorpusRepository> repository = communications.first()->repository();
    QPointer<Corpus> corpus = communications.first()->corpus();
    foreach (QPointer<CorpusCommunication> com, communications) {

    }

}

void expeProsodicBoundariesExperts(const QList<QPointer<CorpusCommunication> > &communications)
{
    if (communications.isEmpty()) return;
    QPointer<CorpusRepository> repository = communications.first()->repository();
    QString corpusID = communications.first()->corpusID();

    TappingAnnotatorExperiment texp;
    QString path = "/home/george/Dropbox/2015-10 SP8 - Prosodic boundaries perception experiment/RESULTS_EXPERTS";
    QDirIterator iterator(path, QStringList() << "*.xml", QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        QString filename = iterator.next();
        texp.readResultsFile(repository, corpusID, filename);
        // printMessage(filename);
    }
}

void expeEmilie(const QList<QPointer<CorpusCommunication> > &communications)
{
    if (communications.isEmpty()) return;
    QPointer<Corpus> corpus = communications.first()->corpus();

    QString path = "/home/george/Dropbox/2017 Experience perceptive disfluence Emilie/Annotation2";
    QDir dirinfo(path);
    QFileInfoList list;
    list << dirinfo.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
    dirinfo.setNameFilters(QStringList() << "conv-*.txt");
    list << dirinfo.entryInfoList();

    foreach (QFileInfo info, list) {
        if (!info.isDir()) {
            DisfluenciesExperiments::resultsReadTapping("2", info.canonicalFilePath(), corpus);
            qDebug() << info.baseName();
            // printMessage(QString("Read data from %1").arg(info.baseName()));
        }
    }
    corpus->save();
}



void preprocess_zeinab_transcriptions(const QList<QPointer<CorpusCommunication> > &communications)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_transcription = tiers->getIntervalTierByName("transcription");
                foreach (Interval *intv, tier_transcription->intervals()) {
                    if (intv->text().trimmed() == "-") intv->setText("_");
                }
                int i = tier_transcription->count() - 1;
                while (i >= 2) {
                    if (tier_transcription->interval(i - 2)->text().trimmed().endsWith("/") && tier_transcription->interval(i)->text().trimmed().startsWith("+") &&
                        tier_transcription->interval(i - 1)->isPauseSilent() && tier_transcription->interval(i - 1)->duration().toDouble() < 0.400)
                    {
                        QString s = tier_transcription->interval(i - 2)->text().trimmed();
                        s.chop(1);
                        tier_transcription->interval(i - 2)->setText(s);
                        tier_transcription->interval(i - 1)->setText("");
                        tier_transcription->interval(i)->setText(tier_transcription->interval(i)->text().remove(0, 1));
                        tier_transcription->merge(i - 2, i, "");
                    }
                    else if (tier_transcription->interval(i - 1)->isPauseSilent() && tier_transcription->interval(i - 1)->duration().toDouble() < 0.180) {
                        tier_transcription->interval(i-1)->setText("");
                        tier_transcription->merge(i - 2, i, " ");
                    }
                    --i;
                }
                tier_transcription->replace("", "  ", " ");
                tier_transcription->replace("", "_", "");
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_transcription);
            }
            qDeleteAll(tiersAll);
            // printMessage(com->ID());
        }
    }
}

void merge_pauses(const QList<QPointer<CorpusCommunication> > &communications)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_transcription = tiers->getIntervalTierByName("transcription");
                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
                IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");

                tier_transcription->fillEmptyWith("", "_");
                tier_transcription->mergeIdenticalAnnotations("_");

                tier_tok_min->fillEmptyWith("", "_");
                tier_tok_min->mergeIdenticalAnnotations("_");

                tier_tok_mwu->fillEmptyWith("", "_");
                tier_tok_mwu->mergeIdenticalAnnotations("_");

                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_transcription);
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_mwu);
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_min);
            }
            qDeleteAll(tiersAll);
            // printMessage(com->ID());
        }
    }
}

#include "PhonetiserExternal.h"
#include "BratSyntaxAndDisfluencies.h"

void Praaline::Plugins::Varia::PluginVaria::process(const QList<QPointer<CorpusCommunication> > &communications)
{

//    BratAnnotationExporter b;
//    QString m = b.test();
//    if (!m.isEmpty()) printMessage(m);

    //merge_pauses(communications);

//    foreach (QPointer<CorpusCommunication> com, communications) {
//        if (!com) continue;
//        QString m;

//         SequencerSyntax s;
//         m = s.checkGroupingAnnotation(com);
//         m = s.createSequencesFromGroupingAnnotation(com);
        // if (!m.isEmpty()) printMessage(m);
        // m = SequencerDisfluencies::getAllDistinctSequences(com);
        // SequencerDisfluencies s;
        // m = s.checkAnnotation(com);
        // m = MelissaExperiment::splitResponses(com);
        // m = MelissaExperiment::exportForAlignment(com);
        // m = MelissaExperiment::preprocessAlignment(com);
        // m = MelissaExperiment::processPausesInsertedByAligner(com);
        // m = MelissaExperiment::reimportAlignment(com);
        // SequencerProsodicUnits p;
        // p.setAttributeBoundary("promise_boundary");
        // m = p.createSequencesFromProsodicBoundaries(com, "B3");
        // SequencerCombineUnits c;
        // c.setSequencesLevelA("prosodic_units");
        // c.setSequencesLevelB("syntactic_units");
        // c.setSequencesLevelCombined("bdu");
        // m = c.createSequences(com);
        // BratSyntaxAndDisfluencies exporter;
        // exporter.setSentenceTier("response");
        // m = exporter.getHTML(com);
        // m = MelissaExperiment::reactionTimes(com);
        // PhonetiserExternal p;
        // m = p.importFromPhonetiser(com, true);
        // m = CPROMDISS::movePointAnnotationToInterval(com);

        // ICIQ3
        // SilentPauseManipulator s;
        // m = s.process(com);
        // if (!m.isEmpty()) printMessage(m);

        // MelissaExperiment::exportForEA(com);
        // MelissaExperiment::importPhonetisation(com);
        // printMessage(MelissaExperiment::exportSyntacticAnnotation(com));
//    }


//    return;

    // NASSIMA EXPERIMENT - boundaries and pauses
     if (communications.isEmpty()) return;
     QPointer<Corpus> corpus = communications.first()->corpus();
     if (!corpus) return;
    // calculate delta RT and adjust taps
    // ProsodicBoundariesExperimentAnalysis::analysisCalculateDeltaRT(corpus);
    // calculate adjusted tapping tiers
    // ProsodicBoundariesExperimentAnalysis::analysisCreateAdjustedTappingTier(corpus, "tapping_boundaries");
    // ProsodicBoundariesExperimentAnalysis::analysisCreateAdjustedTappingTier(corpus, "tapping_pauses");
    // Calculate moving average (smoothed curve) and Attribute tapping peaks to syllables
    // ProsodicBoundariesExperimentAnalysis::analysisCalculateSmoothedTappingModel(corpus, 0, "tapping_boundaries");
    // ProsodicBoundariesExperimentAnalysis::analysisAttributeTappingToSyllablesLocalMaxima(corpus, "tok_min", "boundary", "tapping_boundaries");
    // ProsodicBoundariesExperimentAnalysis::analysisCalculateSmoothedTappingModel(corpus, 0, "tapping_pauses");
    // ProsodicBoundariesExperimentAnalysis::analysisAttributeTappingToSyllablesLocalMaxima(corpus, "tok_min", "pause", "tapping_pauses");
    // Calculate delay and dispersion after attribution
    // ProsodicBoundariesExperimentAnalysis::calculateDelayAndDispersion(corpus, "boundary");
    // ProsodicBoundariesExperimentAnalysis::calculateDelayAndDispersion(corpus, "pause");
    // Extract tables for statistics
    QString path = "/home/george/Dropbox/MIS_Phradico/Experiences/01 Analyse commune frontieres/";
    // ProsodicBoundariesExperimentAnalysis::statExtractFeaturesForModelling(path + "titeuf_naive_boundary_features.txt", corpus, "boundary", false);
    // ProsodicBoundariesExperimentAnalysis::statExtractFeaturesForModelling(path + "titeuf_naive_pause_features.txt", corpus, "pause", false);

    // z-score normalisation
    // MacroprosodyExperiment::calculateZScoreForJoystickDataPerSampleAndParticipant(communications, "joystick_speechrate");
    // MacroprosodyExperiment::calculateZScoreForJoystickDataPerSampleAndParticipant(communications, "joystick_pitchmovement");
    // MacroprosodyExperiment::createCombinedJoystickData(communications, "joystick_speechrate");
    // MacroprosodyExperiment::createCombinedJoystickData(communications, "joystick_pitchmovement");


//    expeEmilie(communications);
//    DisfluenciesExperiments::analysisCreateAdjustedTappingTier(communications);
//    return;

//    if (communications.isEmpty()) return;
//    QPointer<Corpus> corpus = communications.first()->corpus();
//    if (!corpus) return;
//    QString path = "/home/george/Dropbox/MIS_Phradico/Experiences/01b_perception_FP_experts/ANALYSES";
//    // read files
//    expeProsodicBoundariesExperts(communications);
//    // calculate delta RT and adjust taps
//    ProsodicBoundariesExperimentAnalysis::analysisCalculateDeltaRT(corpus);
//    ProsodicBoundariesExperimentAnalysis::analysisCreateAdjustedTappingTier(corpus);
//    // calculate smoothed curve
//    ProsodicBoundariesExperimentAnalysis::analysisCalculateSmoothedTappingModel(corpus);
//    // Attribute tapping peaks to syllables
//    ProsodicBoundariesExperimentAnalysis::analysisAttributeTappingToSyllablesLocalMaxima(corpus, "tok_min", "boundaryExpert");
//    ProsodicBoundariesExperimentAnalysis::calculateDelayAndDispersion(corpus, "boundaryExpert");
//    QString path = "/home/george/Dropbox/MIS_Phradico/Experiences/01 Analyse commune frontieres/";
    // ProsodicBoundariesExperimentAnalysis::statExtractFeaturesForModelling(path + "locas_naive_boundary_features.txt", corpus, "boundary", false);
    // ProsodicBoundariesExperimentAnalysis::statExtractFeaturesForModelling(path + "locas_expert_boundary_features.txt", corpus, "boundaryExpert", false);
    // Correspondance - Divergence
//    ProsodicBoundariesExperimentAnalysis::statCorrespondanceNSandMS(path + "locas_naive_correspondance.txt", path + "locas_naive_correspondance_bookmarks.xml",
//                                                                    corpus, "boundary");
//    ProsodicBoundariesExperimentAnalysis::statCorrespondanceNSandMS(path + "locas_expert_correspondance.txt", path + "locas_expert_correspondance_bookmarks.xml",
//                                                                    corpus, "boundaryExpert");
//    ProsodicBoundariesExperimentAnalysis::statCorrespondanceInternal(path + "locas_groups_correspondance.txt", path + "locas_groups_correspondance_bookmarks.xml",
//                                                                     corpus, "boundaryNaive", "boundaryExpert");
//    ProsodicBoundariesExperimentAnalysis::statInterAnnotatorAgreement(path + "locas_naive_cohenkappa.txt", path + "locas_naive_fleisskappa.txt",
//                                                                      corpus, "boundaryNaive", "tapping_naive");
//    ProsodicBoundariesExperimentAnalysis::statInterAnnotatorAgreement(path + "locas_expert_cohenkappa.txt", path + "locas_expert_fleisskappa.txt",
//                                                                      corpus, "boundaryExpert", "tapping_expert");
    // Titeuf
//    ProsodicBoundariesExperimentAnalysis::statCorrespondanceInternal(path + "titeuf_boundary_vs_pause_correspondance.txt", path + "titeuf_boundary_vs_pause_correspondance_bookmarks.xml",
//                                                                     corpus, "boundary", "pause");
//    ProsodicBoundariesExperimentAnalysis::statInterAnnotatorAgreement(path + "titeuf_naive_boundary_kappacohen.txt", path + "titeuf_naive_boundary_kappafleiss.txt",
//                                                                      corpus, "boundary", "tapping_boundaries");
//    ProsodicBoundariesExperimentAnalysis::statInterAnnotatorAgreement(path + "titeuf_naive_pause_kappacohen.txt", path + "titeuf_naive_pause_kappafleiss.txt",
//                                                                      corpus, "pause", "tapping_pauses");

    // cuttingStimuli(communications);




    // printMessage(valibelTranscription(communications));
    // prepareClassifierFiles(communications);
//    importPhonTranscriptionsIvana(communications);

//    if (communications.isEmpty()) return;
//    SpeechRateExperiments sr;
//    QString path = "/home/george/Dropbox/MIS_Phradico/Experiences/02_perception-macroprosodie/Results raw files/";

//    for (int i = 13; i <= 40; ++i) {
//        QString filename = QString("results_P%1.txt").arg(i);
//        sr.readResultsFile(communications.first()->repository(), path + filename);
//        printMessage(filename);
//    }
//    return;

//    // DisfluenciesExperiments::analysisCalculateDeltaRT(communications);
//    DisfluenciesExperiments::analysisCreateAdjustedTappingTier(communications);
//    return;

//    int countDone = 0;
//    madeProgress(0);
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        if (!com) continue;
//        // MyExperiments::createTextgridsFromAutosyll(corpus, com);
//        // MyExperiments::updateTranscriptionMode(corpus, com);
//        countDone++;
//        madeProgress(countDone * 100 / communications.count());
//    }

//    int countDone = 0;
//    madeProgress(0);
//    printMessage(QString("Inter-rater agreement"));
//    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        if (!com) continue;
//        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
//            if (!annot) continue;
//            QString annotationID = annot->ID();
//            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
//            foreach (QString speakerID, tiersAll.keys()) {
//                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
//                if (!tiers) continue;

//                IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
//                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");

//                InterraterAgreement agreement;
//                agreement.addGroup("0", QStringList() << "0");
//                agreement.addGroup("P", QStringList() << "P" << "L");
//                QList<Interval *> syllables = tier_syll->intervals();
//                foreach (Interval * syll, syllables) {
//                    if (syll->attribute("promise_pos").toString().isEmpty()) syll->setAttribute("promise_pos", "0");
//                    if (syll->attribute("delivery2").toString().isEmpty()) syll->setAttribute("delivery2", "0");
//                }
//                double k = agreement.getCohenKappa("0", "P", syllables, "promise_pos", "delivery2");
//                printMessage(QString("%1\t%2").arg(com->ID()).arg(k));
//            }
//            qDeleteAll(tiersAll);
//        }
//        countDone++;
//        madeProgress(countDone * 100 / communications.count());
//    }
//    return;

//    exportMultiTierTextgrids(corpus, communications);
//    return;

//    QString path = "/home/george/Dropbox/2015-10 SP8 - Prosodic boundaries perception experiment/RESULTS/Result files raw 22-02-2016";
//    QDir dirinfo(path);
//    QFileInfoList list;
//    list << dirinfo.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
//    dirinfo.setNameFilters(QStringList() << "*.csv");
//    list << dirinfo.entryInfoList();
//    foreach (QFileInfo info, list) {
//        if (!info.isDir()) {
//            QString subjectID = info.baseName().replace("expe1-subject-", "").replace("expe2-subject-", "")
//                    .replace("expe3-subject-", "").replace("expe4-subject-", "").replace(".csv", "");
//            if (subjectID.length() == 1) subjectID = "S20" + subjectID;
//            else subjectID = "S2" + subjectID;
//            QString expeType;
//            if (info.baseName().contains("expe1")) expeType = "1";
//            else if (info.baseName().contains("expe2")) expeType = "2";
//            else if (info.baseName().contains("expe3")) expeType = "3";
//            else if (info.baseName().contains("expe4")) expeType = "4";
//            // PBExpe::resultsReadParticipantInfo(subjectID, expeType, info.canonicalFilePath(), corpus);
//            // PBExpe::resultsReadTapping(subjectID, info.canonicalFilePath(), corpus);
//            printMessage(QString("Read data from %1").arg(info.baseName()));
//        }
//    }
//    corpus->save();
//    return;

//    PBExpe::actualStimuliFromCorpus(corpus, communications);

//    PBExpe::analysisCalculateDeltaRT(corpus);
//    PBExpe::analysisCreateAdjustedTappingTier(corpus);

//    PBExpe::analysisCalculateSmoothedTappingModel(corpus);

//    PBExpe::createProsodicUnits(corpus);
//    QStringList transcripts = PBExpe::printTranscriptionInProsodicUnits(corpus);
//    foreach (QString t, transcripts) printMessage(t);

//    PBExpe::analysisAttributeTappingToSyllablesLocalMaxima(corpus, "tok_min", "boundary");
//    PBExpe::analysisCalculateAverageDelay(corpus, "boundary2");
//    PBExpe::analysisCalculateCoverage(corpus, "boundary2");
//    PBExpe::analysisCheckBoundaryRightAfterPause(corpus);

//    PBExpe::analysisStabilisation(corpus, 10, 100);
//    PBExpe::analysisStabilisation(corpus, 20, 100);
//    PBExpe::analysisStabilisation(corpus, 30, 100);
//    PBExpe::analysisStabilisation(corpus, 15, 100);
//    PBExpe::analysisStabilisation(corpus, 25, 100);


//    PBExpe::statExtractFeaturesForModelling(corpus, true, false, "boundary");
//    PBExpe::statInterAnnotatorAgreement(corpus);
//    PBExpe::statCorrespondanceNSandMS(corpus, "boundary");


//    QString path = "C:/Users/George/Downloads/Rhap_meta";
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        bool result = AnvilMetadataTranscript::load(QString("%1/%2-meta.xml").arg(path).arg(com->ID()), corpus);
//        printMessage(QString("%1 %2").arg(com->ID()).arg((result) ? "OK" : "Error"));
//    }



//    analyseBoundaries(corpus, communications);
//    return;
//    chunk(corpus, communications);
//    return;

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Varia;
    Q_EXPORT_PLUGIN2(PluginVaria, PluginVaria)
#endif


//IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
//AnnotationTierGroup *txg = new AnnotationTierGroup();
//PraatTextGrid::load(QString("D:/corpora/locasf/%1.textgrid").arg(speakerID), txg);
//if (!txg) printMessage("cannot open textgrid");
//PointTier *tier_boundary = txg->getPointTierByName("boundary");
//foreach (Point *point, tier_boundary->points()) {
//    Interval *syll = tier_syll->intervalAtTime(point->timePoint() - RealTime(0, 100));
//    if (!syll) { printMessage(QString("no syll at %1").arg(point->timePoint().toDouble())); continue; }
//    syll->setAttribute("boundary", point->text());
//}
//corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_syll);


//IntervalTier *tier_phone = tiers->getIntervalTierByName("phone");
//if (!tier_phone) continue;
//QString filenameRec = corpus->baseMediaPath() + "/" + rec->filename();
//QFileInfo info(filenameRec);
//QString filename = info.absolutePath() + QString("/%1_phone.svl").arg(speakerID);
//SVBridge::saveSVTimeInstantsLayer(filename, rec->sampleRate(), tier_phone);


//QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
//foreach (QPointer<CorpusCommunication> com, communications) {
//    if (!com) continue;
//    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
//        if (!annot) continue;
//        QString annotationID = annot->ID();
//        tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
//        foreach (QStringprepareStimuliCorpus speakerID, tiersAll.keys()) {
//            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
//            if (!tiers) continue;

//            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
//            QStringList subjectsA, subjectsB, subjects;
//            subjectsA << "AH" << "CdlC" << "GB" << "LF" << "MB" << "NH";
//            subjectsB << "AEL" << "AP" << "JLV" << "ML" << "SL" << "ZT";
//            subjects << subjectsA << subjectsB;
//            foreach (QString code, QStringList() << "P" << "Z") {
//                foreach (Interval *syll, tier_syll->intervals()) {
//                    int score = 0, scoreA = 0, scoreB = 0;
//                    foreach (QString subject, subjectsA) {
//                        if (syll->attribute(QString("prom_%1").arg(subject)).toString().toUpper() == code) scoreA++;
//                    }
//                    foreach (QString subject, subjectsB) {
//                        if (syll->attribute(QString("prom_%1").arg(subject)).toString().toUpper() == code) scoreB++;
//                    }
//                    score = scoreA + scoreB;
//                    syll->setAttribute(QString("%1_scoreA").arg(code), scoreA);
//                    syll->setAttribute(QString("%1_scoreB").arg(code), scoreB);
//                    syll->setAttribute(QString("%1_score").arg(code), score);
//                }
//            }
//            corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_syll);
//        }
//        qDeleteAll(tiersAll);
//    }
//}

//QString path = "D:/DROPBOX/LCLIG2210/2014-2015 Projet/Zeinab/";
//AnnotationTierGroup *txg = new AnnotationTierGroup();
//PraatTextGrid::load(path + "Tier_ortho-phono-mot-syllabe-zeinab_traore.textgrid", txg);
//IntervalTier *phones = txg->getIntervalTierByName("phones");
//if (!phones) {
//    printMessage("Tier not found phones");
//    return;
//}
//foreach(AnnotationTier *t, txg->tiers()) {
//    if (t->tierType() != AnnotationTier::TierType_Intervals) continue;
//    IntervalTier *tier = (IntervalTier *)t;
//    tier->fixBoundariesBasedOnTier(phones, RealTime(0, 10000000));
//    printMessage(QString("aligned tier %1").arg(tier->name()));
//}
//PraatTextGrid::save(path + "zeinab_participant01b_corr.textgrid", txg);
