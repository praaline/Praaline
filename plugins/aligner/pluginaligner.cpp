#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <ExtensionSystemConstants>

#include "pluginaligner.h"
#include "pncore/corpus/corpus.h"
#include "pnlib/audiosegmenter.h"
#include "phonemedatabase.h"
#include "sphinx/sphinxacousticmodeladapter.h"
#include "sphinx/sphinxfeatureextractor.h"
#include "sphinx/sphinxrecogniser.h"
#include "sphinx/sphinxsegmentation.h"
#include "phonetisers/externalphonetiser.h"
#include "easyalignbasic.h"
#include "LongSoundAligner.h"

#include "pncore/interfaces/praat/praattextgrid.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Aligner::PluginAlignerPrivateData {
    PluginAlignerPrivateData() : cmdDownsampleWaveFiles(false), cmdSplitToUtterances(false)
    {}

    bool cmdDownsampleWaveFiles;
    bool cmdSplitToUtterances;
};

Praaline::Plugins::Aligner::PluginAligner::PluginAligner(QObject* parent) : QObject(parent)
{
    d = new PluginAlignerPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::Aligner::PluginAligner::~PluginAligner()
{
    delete d;
}

bool Praaline::Plugins::Aligner::PluginAligner::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::Aligner::PluginAligner::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::Aligner::PluginAligner::finalize() {

}

QString Praaline::Plugins::Aligner::PluginAligner::pluginName() const {
    return "Aligner";
}

QtilitiesCategory Praaline::Plugins::Aligner::PluginAligner::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::Aligner::PluginAligner::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::Aligner::PluginAligner::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::Aligner::PluginAligner::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::Aligner::PluginAligner::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::Aligner::PluginAligner::pluginDescription() const {
    return tr("The Aligner plugin contains scripts to process the PFC corpus.");
}

QString Praaline::Plugins::Aligner::PluginAligner::pluginCopyright() const {
    return QString(tr("Copyright") + " 2014-2015, George Christodoulides");
}

QString Praaline::Plugins::Aligner::PluginAligner::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::Aligner::PluginAligner::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;

    parameters << PluginParameter("cmdDownsampleWaveFiles", "Create WAV files downsampled to 16kHz", QVariant::Bool, d->cmdDownsampleWaveFiles);
    parameters << PluginParameter("cmdSplitToUtterances", "Split WAV files to utterances", QVariant::Bool, d->cmdSplitToUtterances);

    return parameters;
}

void Praaline::Plugins::Aligner::PluginAligner::setParameters(QHash<QString, QVariant> parameters)
{
    if (parameters.contains("cmdDownsampleWaveFiles")) d->cmdDownsampleWaveFiles = parameters.value("cmdDownsampleWaveFiles").toBool();
    if (parameters.contains("cmdSplitToUtterances")) d->cmdDownsampleWaveFiles = parameters.value("cmdSplitToUtterances").toBool();
}

void Praaline::Plugins::Aligner::PluginAligner::addPhonetisationToTokens(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage("Adding phonetisation to tokens");
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_token = tiers->getIntervalTierByName("tok_min");
                if (!tier_token) continue;
                ExternalPhonetiser::addPhonetisationToTokens(tier_token, "", "phonetisation");
                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_token);
            }
            qDeleteAll(tiersAll);
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
        printMessage(QString("Phonetisation OK: %1").arg(com->ID()));
        QApplication::processEvents();
    }
    madeProgress(100);
    printMessage("Finished");
}

void Praaline::Plugins::Aligner::PluginAligner::createFeatureFilesFromUtterances(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        CorpusRecording *rec = com->recordings().first();
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                if (speakerID.startsWith("(")) continue;

                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;

                IntervalTier *tier_segment = tiers->getIntervalTierByName("segment");
                if (!tier_segment) continue;
                QList<Interval *> list_utterances;
                int i = 0;
                foreach (Interval *intv, tier_segment->intervals()) {
                    if (intv->isPauseSilent()) { i++; continue; }
                    Interval *utt = new Interval(intv);
                    QString spk = speakerID.replace(annotationID, "");
                    if (spk.startsWith("_")) spk = spk.remove(0, 1);
                    QString utteranceID = QString("%1_%2_%3").arg(annotationID).arg(spk).arg(i);
                    utt->setAttribute("utteranceID", utteranceID);
                    list_utterances << utt;
                    i++;
                }
                // QString wavfile = QString("G:/PFC_WAVE/16k/%1.16k.wav").arg(rec->filename().replace(".mp3", ""));
                // QString wavfile = QString("%1.16k.wav").arg(rec->filename().replace(".mp3", ""));
                //corpus->baseMediaPath() + "/" + rec->filename()
                // AudioSegmenter::segment(wavfile, "D:/Aligner_train_tests/pfc", list_utterances, "utteranceID", 16000);
//                QList<QStringList> argumentList = AudioSegmenter::script(wavfile, ".", list_utterances, "utteranceID", 16000);
//                foreach (QStringList arguments, argumentList) {
//                    QString cmd("sox ");
//                    cmd.append(arguments.join(" "));
//                    printMessage(cmd);
//                }
//                printMessage(QString("%1 %2").arg(wavfile).arg(wavfile.replace(".wav", ".mfc")));
            }
            qDeleteAll(tiersAll);
        }
        //printMessage(QString("Phonetisation OK: %1").arg(com->ID()));
    }
}

void Praaline::Plugins::Aligner::PluginAligner::align(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        CorpusRecording *rec = com->recordings().first();
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                if (speakerID.startsWith("(")) continue;

                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;

                IntervalTier *tier_segment = tiers->getIntervalTierByName("segment");
                if (!tier_segment) continue;
            }
            printMessage("Progress");
        }
        qDeleteAll(tiersAll);
    }
    //printMessage(QString("Phonetisation OK: %1").arg(com->ID()));
}

void Praaline::Plugins::Aligner::PluginAligner::createDownsampledWavFiles(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications)
{
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        CorpusRecording *rec = com->recordings().first();
//        QList<Interval *> list;
//        list << new Interval(RealTime(0, 0), rec->duration(), rec->filename().replace(".wav", ".16k"));
//        AudioSegmenter::segment(corpus->baseMediaPath() + "/" + rec->filename(), corpus->baseMediaPath(), list, QString(), 16000);
//        qDeleteAll(list);
        printMessage(rec->filename().replace(".wav", ".16k"));
    }
    //printMessage(QString("Phonetisation OK: %1").arg(com->ID()));
}

void Praaline::Plugins::Aligner::PluginAligner::autoTranscribePresegmented(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage("Automatic transcription of presegmented utterances");
    SphinxRecogniser *sphinx = new SphinxRecogniser(this);
    sphinx->setUseMLLR(true);
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        QPointer<CorpusRecording> rec = com->recordings().first();
        if (!rec) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_segment = tiers->getIntervalTierByName("auto_segments");
                if (!tier_segment) continue;
                QList<Interval *> utterances;
                QList<Interval *> segmentation;
                foreach (Interval *intv, tier_segment->intervals()) {
                    if (intv->text() != "_") utterances << intv;
                }
                sphinx->recogniseUtterances_MFC(com, rec->ID(), utterances, segmentation);
                foreach (Interval *utterance, utterances) {
                    printMessage(utterance->text());
                }
                IntervalTier *tier_auto_transcribe = new IntervalTier("auto_transcribe", RealTime(0, 0),
                                                                      rec->duration(), segmentation);
                corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_auto_transcribe);
                corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_segment);
            }
            qDeleteAll(tiersAll);

        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
        QApplication::processEvents();
    }
    delete sphinx;
    madeProgress(100);
    printMessage("Finished");
}

void Praaline::Plugins::Aligner::PluginAligner::createSegments(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_auto = tiers->getIntervalTierByName("auto_transcribe");
                if (!tier_auto) continue;
                IntervalTier *tier_segment = new IntervalTier(tier_auto, "segment");
                tier_segment->replaceText("<sil>", "");
                tier_segment->replaceText("[b]", "");
                tier_segment->replaceText("[i]", "");
                tier_segment->replaceText("(1)", "");
                tier_segment->replaceText("(2)", "");
                tier_segment->replaceText("(3)", "");
                tier_segment->replaceText("(4)", "");
                tier_segment->fillEmptyAnnotationsWith("_");
                tier_segment->mergeIdenticalAnnotations("_");
                foreach (Interval *intv, tier_segment->intervals()) {
                    if (intv->text() == "_" && intv->duration().toDouble() < 0.250)
                        intv->setText("");
                }
                tier_segment->mergeContiguousAnnotations(QStringList() << "_" << "<s>" << "</s>", " ");
                tier_segment->replaceText("<s>", "_");
                tier_segment->replaceText("</s>", "_");
                tier_segment->mergeIdenticalAnnotations("_");
                corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_segment);
            }
            printMessage(annotationID);
        }
        qDeleteAll(tiersAll);
    }
    //printMessage(QString("Phonetisation OK: %1").arg(com->ID()));
}

void Praaline::Plugins::Aligner::PluginAligner::process(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    addPhonetisationToTokens(corpus, communications);
    return;


    LongSoundAligner *LSA = new LongSoundAligner();
    // LSA->createRecognitionLevel(corpus, 0);
    madeProgress(0);
    int i = 0;
    QElapsedTimer timer;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        // LSA->createUtterancesFromProsogramAutosyll(corpus, com);
        com->setProperty("language_model", QString("valibel_lm/%1.lm.dmp").arg(com->ID()));
        timer.start();
        LSA->recognise(corpus, com, 0);
        double secRecognitionTime = timer.elapsed() / 1000.0;
        double secRecording = com->recordings().first()->durationSec();
        printMessage(QString("%1\tDuration:\t%2\tRecognition:\t%3\tRatio:\t%4\txRT").
                     arg(com->ID()).arg(secRecording).arg(secRecognitionTime).arg(secRecognitionTime / secRecording));
        ++i;
        madeProgress(i * 100 / communications.count());
        QApplication::processEvents();
    }
    madeProgress(100);
    delete LSA;
    return;


    if (d->cmdDownsampleWaveFiles) {
        createDownsampledWavFiles(corpus, communications);
        return;
    }
    else if (d->cmdSplitToUtterances) {
        return;
    }
    return;


    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "auto_syll" << "auto_transcribe" << "segment");
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                QString id = QString("%1/%2_%3.TextGrid").arg(corpus->basePath()).arg(annot->ID()).arg(speakerID);
//                printMessage(id);
//                PraatTextGrid::save(id, tiers);
                EasyAlignBasic *ea = new EasyAlignBasic(this);
                ea->setPreciseUtteranceBoundaries(false);
                printMessage(ea->prepareAlignmentTextgrid(com, tiers));
                ea->runEasyAlign(com);
                printMessage(ea->postAlignment(com, tiers));
                ea->runSyllabify(com);
            }
            qDeleteAll(tiersAll);
        }
    }



//    foreach (QPointer<CorpusCommunication> com, communications) {
//        if (!com) continue;
//        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
//            if (!annot) continue;
//            QString speakerID = com->ID().left(13);
//            AnnotationTier *tier = corpus->datastoreAnnotations()->getTier(annot->ID(), speakerID, "segment");
//            if (!tier) {
//                printMessage(com->ID());
//                CorpusRecording *rec = com->recordings().first();
//                IntervalTier *seg = new IntervalTier("segment", RealTime(0, 0), rec->duration());
//                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, seg);
//                delete seg;
//                QList<QPointer<CorpusCommunication> > onlythis; onlythis << com;
//                autoTranscribePresegmented(corpus, onlythis);
//                createSegments(corpus, onlythis);
//            }
//        }
//    }


//    SphinxFeatureExtractor *FE = new SphinxFeatureExtractor();
//    FE->setFeatureParametersFile("D:/SPHINX/pocketsphinx-0.8/model/hmm/french_f0/feat.params");
//    FE->createSphinxMFC(corpus, communications);
//    return;

//    AcousticModelTrainer mt;
//    // mt.createMasterLabelFile(QString("d:/aligner_train_tests/%1.mlf").arg(corpus->ID()), corpus, communications, "segment", "tok_min", "phone");
//    mt.createMasterLabelFileFromTokens(QString("d:/aligner_train_tests/%1.mlf").arg(corpus->ID()), corpus, communications, "segment", "tok_min", "phonetisation");
//    PhonemeDatabase pdb;
//    pdb.createHMMDefsBootstrap("d:/aligner_train_tests/hmmdefs");

//    createAutotranscription(corpus, communications);
//    createSegments(corpus, communications);


//    SphinxModelAdapter *adapter = new SphinxModelAdapter();
//    adapter->loadDictionary("F:/CORPUS_THESIS_EXPERIMENT/PROSO2015/adapt/french_dict.dic");
//    QStringList listUnknownWords;
//    adapter->unknownWordsInVocabFile("F:/CORPUS_THESIS_EXPERIMENT/PROSO2015/adapt/lm/qrtexts.vocab", listUnknownWords);
//    // adapter->createAdaptationFiles(corpus, communications, listUnknownWords, false);
//    foreach (QString word, listUnknownWords) {
//        printMessage(word);
//    }

    //createFeatureFilesFromUtterances(corpus, communications);

//                IntervalTier *tier_phone = new IntervalTier("phone", tier_segment->tMin(), tier_segment->tMax());
//                ForceAligner f;
//                f.alignUtterances(corpus->baseMediaPath() + "/" + rec->filename(), tier_segment, tier_token, tier_phone);
//                AnnotationTierGroup *txg = new AnnotationTierGroup;
//                txg->addTier(tier_phone);
//                txg->addTier(tier_phone_orig);
//                txg->addTier(tier_token);
//                txg->addTier(tier_segment);
//                PraatTextGrid::save("d:/aligner_tests/test.textgrid", txg);
//                delete txg;


}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Aligner;
    Q_EXPORT_PLUGIN2(PluginAligner, PluginAligner)
#endif

//QList<AlignerToken> out = Phonetiser::phonetise(tier_tokens);
//foreach (AlignerToken ptoken, out) {
//    QString s = QString("%1\t%2").arg(ptoken.orthographic).arg(ptoken.phonetisation);
//    printMessage(s);
//}

//                QString filenameRec = corpus->baseMediaPath() + "/" + rec->filename();
//                QFileInfo info(filenameRec);
//                QString filename = info.absolutePath() + QString("/%1_phone.svl").arg(speakerID);
