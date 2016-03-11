#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <ExtensionSystemConstants>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>

#include "pluginaligner.h"
#include "pncore/corpus/corpus.h"
#include "pnlib/AudioSegmenter.h"
#include "phonemedatabase.h"
#include "pnlib/asr/sphinx/SphinxAcousticModelAdapter.h"
#include "pnlib/asr/sphinx/SphinxFeatureExtractor.h"
#include "pnlib/asr/sphinx/SphinxRecogniser.h"
#include "pnlib/asr/sphinx/SphinxSegmentation.h"
#include "pnlib/phonetiser/ExternalPhonetiser.h"
#include "easyalignbasic.h"
#include "LongSoundAligner.h"

#include "pnlib/asr/SpeechRecognitionRecipes.h"
#include "pnlib/vad/OpenSmileVAD.h"

#include "pncore/interfaces/praat/praattextgrid.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Aligner::PluginAlignerPrivateData {
    PluginAlignerPrivateData() :
        commandDownsampleWaveFiles(false), commandExtractFeatures(false), commandSplitToUtterances(false),
        commandAutomaticTranscription(false), commandLongSoundAligner(false)
    {}

    bool commandDownsampleWaveFiles;
    bool commandExtractFeatures;
    bool commandSplitToUtterances;
    bool commandAutomaticTranscription;
    bool commandLongSoundAligner;

    QString sphinxAcousticModel;
    QString sphinxLanguageModel;
    QString sphinxPronunciationDictionary;
    QString sphinxMLLRMatrix;

    QFuture<QString> future;
    QFutureWatcher<QString> watcher;
};

Praaline::Plugins::Aligner::PluginAligner::PluginAligner(QObject* parent) : QObject(parent)
{
    d = new PluginAlignerPrivateData;
    setObjectName(pluginName());
    connect(&(d->watcher), SIGNAL(resultReadyAt(int)), this, SLOT(futureResultReadyAt(int)));
    connect(&(d->watcher), SIGNAL(progressValueChanged(int)), this, SLOT(futureProgressValueChanged(int)));
    connect(&(d->watcher), SIGNAL(finished()), this, SLOT(futureFinished()));
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

    parameters << PluginParameter("commandDownsampleWaveFiles", "Create WAV files downsampled to 16kHz", QVariant::Bool, d->commandDownsampleWaveFiles);
    parameters << PluginParameter("commandExtractFeatures", "Extract MFCC feature files", QVariant::Bool, d->commandExtractFeatures);
    parameters << PluginParameter("commandSplitToUtterances", "Split WAV files to utterances", QVariant::Bool, d->commandSplitToUtterances);
    parameters << PluginParameter("commandAutomaticTranscription", "Automatic Transcription", QVariant::Bool, d->commandAutomaticTranscription);
    parameters << PluginParameter("commandLongSoundAligner", "Run Long Sound Aliger", QVariant::Bool, d->commandLongSoundAligner);

    parameters << PluginParameter("sphinxAcousticModel", "Sphinx Acoustic Model", QVariant::String, d->sphinxAcousticModel);
    parameters << PluginParameter("sphinxLanguageModel", "Sphinx Language Model", QVariant::String, d->sphinxLanguageModel);
    parameters << PluginParameter("sphinxPronunciationDictionary", "Sphinx Pronunciation Dictionary", QVariant::String, d->sphinxPronunciationDictionary);
    parameters << PluginParameter("sphinxMLLRMatrix", "Sphinx MLLR Adaptation Matrix", QVariant::String, d->sphinxMLLRMatrix);

    return parameters;
}

void Praaline::Plugins::Aligner::PluginAligner::setParameters(QHash<QString, QVariant> parameters)
{
    if (parameters.contains("commandDownsampleWaveFiles")) d->commandDownsampleWaveFiles = parameters.value("commandDownsampleWaveFiles").toBool();
    if (parameters.contains("commandExtractFeatures")) d->commandExtractFeatures = parameters.value("commandExtractFeatures").toBool();
    if (parameters.contains("commandSplitToUtterances")) d->commandSplitToUtterances = parameters.value("commandSplitToUtterances").toBool();
    if (parameters.contains("commandAutomaticTranscription")) d->commandAutomaticTranscription = parameters.value("commandAutomaticTranscription").toBool();
    if (parameters.contains("commandLongSoundAligner")) d->commandLongSoundAligner = parameters.value("commandLongSoundAligner").toBool();

    if (parameters.contains("sphinxAcousticModel")) d->sphinxAcousticModel = parameters.value("sphinxAcousticModel").toString();
    if (parameters.contains("sphinxLanguageModel")) d->sphinxLanguageModel = parameters.value("sphinxLanguageModel").toString();
    if (parameters.contains("sphinxPronunciationDictionary")) d->sphinxPronunciationDictionary = parameters.value("sphinxPronunciationDictionary").toString();
    if (parameters.contains("sphinxMLLRMatrix")) d->sphinxMLLRMatrix = parameters.value("sphinxMLLRMatrix").toString();
}

void Praaline::Plugins::Aligner::PluginAligner::createUtterancesFromProsogramAutosyll(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    printMessage("Creating auto_utterance from Prosogram auto_syll and auto_syll_nucl");
    QPointer<LongSoundAligner> LSA = new LongSoundAligner();
    madeProgress(0);
    int countDone = 0;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        LSA->createUtterancesFromProsogramAutosyll(corpus, com);
        ++countDone;
        madeProgress(countDone * 100 / communications.count());
        QApplication::processEvents();
    }
    madeProgress(100);
}


// ================================================================================================
// Asynchronous execution
// ================================================================================================
void Praaline::Plugins::Aligner::PluginAligner::futureResultReadyAt(int index)
{
    QString result = d->watcher.resultAt(index);
    qDebug() << d->watcher.progressValue() << result;
    emit printMessage(result);
    emit madeProgress(d->watcher.progressValue() * 100 / d->watcher.progressMaximum());
}

void Praaline::Plugins::Aligner::PluginAligner::futureProgressValueChanged(int progressValue)
{
    qDebug() << progressValue;
    emit madeProgress(progressValue * 100 / d->watcher.progressMaximum());
}

void Praaline::Plugins::Aligner::PluginAligner::futureFinished()
{
    emit madeProgress(100);
    emit printMessage("Finished");
    qDebug() << "Finished";
}
// ================================================================================================

struct LSAStep
{
    LSAStep(QPointer<Corpus> corpus) : m_corpus(corpus) { }
    typedef QString result_type;

    QString operator() (const QPointer<CorpusCommunication> &com)
    {
        QPointer<LongSoundAligner> LSA = new LongSoundAligner();
        QElapsedTimer timer;
        if (!com) return QString("%1\tEmpty").arg(com->ID());
        if (!com->hasRecordings()) {
            com->setProperty("LSA_status", "NoRecordings");
            return QString("%1\tNo Recordings").arg(com->ID());
        }
        //com->setProperty("language_model", QString("valibel_lm/%1.lm.dmp").arg(com->ID()));
        timer.start();
        LSA->recognise(m_corpus, com, 0);
        double secRecognitionTime = timer.elapsed() / 1000.0;
        double secRecording = com->recordings().first()->durationSec();
        return QString("%1\tDuration:\t%2\tRecognition:\t%3\tRatio:\t%4\txRT").
                arg(com->ID()).arg(secRecording).arg(secRecognitionTime).
                arg(secRecognitionTime / ((secRecording > 300.0) ? 300.0 : secRecording));
        // For testing: return QString("%1 %2 %3").arg(com->ID()).arg(com->recordingsCount()).arg(timer.elapsed());
    }

    QPointer<Corpus> m_corpus;
};

struct DownsampleWaveFileStep
{
    DownsampleWaveFileStep(QPointer<Corpus> corpus) : m_corpus(corpus) { }
    typedef QString result_type;

    QString operator() (const QPointer<CorpusCommunication> &com)
    {
        if (!com) return QString("%1\tis empty.").arg(com->ID());
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            SpeechRecognitionRecipes::downsampleWaveFile(m_corpus, rec);
        }
        return QString("%1\tdownsampled %2 recordings.").arg(com->ID()).arg(com->recordingsCount());
    }
    QPointer<Corpus> m_corpus;
};

struct SphinxFeatureExtractionStep
{
    SphinxFeatureExtractionStep(QPointer<Corpus> corpus, SpeechRecognitionRecipes::Configuration config)
        : m_corpus(corpus), m_config(config) { }
    typedef QString result_type;

    QString operator() (const QPointer<CorpusCommunication> &com)
    {
        if (!com) return QString("%1\tis empty.").arg(com->ID());
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            SpeechRecognitionRecipes::createSphinxFeatureFile(m_corpus, rec, m_config);
        }
        return QString("%1\textracted feature files for %2 recordings.").arg(com->ID()).arg(com->recordingsCount());
    }
    QPointer<Corpus> m_corpus;
    SpeechRecognitionRecipes::Configuration m_config;
};

struct OpenSmileVADSegmentationStep
{
    OpenSmileVADSegmentationStep(QPointer<Corpus> corpus) : m_corpus(corpus) { }
    typedef QString result_type;

    QString operator() (const QPointer<CorpusCommunication> &com)
    {
        static QMutex mutex;
        if (!com) return QString("%1\tis empty.").arg(com->ID());
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            IntervalTier *tier_utterances =
                    OpenSmileVAD::splitToUtterances(rec, RealTime::fromSeconds(0.250), RealTime::fromSeconds(0.250), "_", "");
            if (!tier_utterances) continue;
            tier_utterances->setName("auto_segment");
            // Save
            mutex.lock();
            m_corpus->datastoreAnnotations()->saveTier(rec->ID(), "", tier_utterances);
            mutex.unlock();
        }
        return QString("%1\tsegmented %2 recording(s) using OpenSMILE VAD.").arg(com->ID()).arg(com->recordingsCount());
    }
    QPointer<Corpus> m_corpus;
};

struct SphinxAutomaticTranscriptionStep
{
    SphinxAutomaticTranscriptionStep(
            QPointer<Corpus> corpus, const QString &sphinxAcousticModel, const QString &sphinxLanguageModel,
            const QString &sphinxPronunciationDictionary, const QString &sphinxMLLRMatrix)
        : m_corpus(corpus), m_sphinxAcousticModel(sphinxAcousticModel), m_sphinxLanguageModel(sphinxLanguageModel),
          m_sphinxPronunciationDictionary(sphinxPronunciationDictionary), m_sphinxMLLRMatrix(sphinxMLLRMatrix) { }
    typedef QString result_type;

    QString operator() (const QPointer<CorpusCommunication> &com)
    {
        if (!com) return QString("%1\tis empty.").arg(com->ID());
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            bool result = SpeechRecognitionRecipes::transcribeUtterancesWithSphinx(
                        m_corpus, com, rec, rec->ID(), "auto_segment", "transcription",
                        m_sphinxAcousticModel, m_sphinxLanguageModel,
                        m_sphinxPronunciationDictionary, m_sphinxMLLRMatrix);
            if (result) {
                SpeechRecognitionRecipes::updateSegmentationFromTranscription(
                            m_corpus, com, "auto_segment", "transcription");
            }
        }
        return QString("%1\tautomatically transcribed %2 recording(s) using Sphinx.").arg(com->ID()).arg(com->recordingsCount());
    }
    QPointer<Corpus> m_corpus;
    QString m_sphinxAcousticModel;
    QString m_sphinxLanguageModel;
    QString m_sphinxPronunciationDictionary;
    QString m_sphinxMLLRMatrix;
};

void Praaline::Plugins::Aligner::PluginAligner::process(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    madeProgress(0);
    printMessage("Starting");
    QElapsedTimer timer;
    timer.start();

    // asynchronous execution
    if (d->commandDownsampleWaveFiles) {
        d->future = QtConcurrent::mapped(communications, DownsampleWaveFileStep(corpus));
        d->watcher.setFuture(d->future);
        while (d->watcher.isRunning()) QApplication::processEvents();
    }
    if (d->commandExtractFeatures) {
        SpeechRecognitionRecipes::Configuration config;
        QString sphinxPath = QCoreApplication::applicationDirPath() + "/plugins/aligner/sphinx/";
        config.sphinxHMModelPath = sphinxPath + "model/hmm/french_f0";
        // BATCH MODE (but UI unresponsive) SpeechRecognitionRecipes::batchCreateSphinxFeatureFiles(corpus, communications, config);
        d->future = QtConcurrent::mapped(communications, SphinxFeatureExtractionStep(corpus, config));
        d->watcher.setFuture(d->future);
        while (d->watcher.isRunning()) QApplication::processEvents();
    }
    if (d->commandSplitToUtterances) {
        d->future = QtConcurrent::mapped(communications, OpenSmileVADSegmentationStep(corpus));
        d->watcher.setFuture(d->future);
        while (d->watcher.isRunning()) QApplication::processEvents();
    }
    if (d->commandAutomaticTranscription) {
        d->future = QtConcurrent::mapped(communications, SphinxAutomaticTranscriptionStep(
                                             corpus, d->sphinxAcousticModel, d->sphinxLanguageModel,
                                             d->sphinxPronunciationDictionary, d->sphinxMLLRMatrix));
        d->watcher.setFuture(d->future);
        while (d->watcher.isRunning()) QApplication::processEvents();
    }
    if (d->commandLongSoundAligner) {
        QPointer<LongSoundAligner> LSA = new LongSoundAligner();
        LSA->createRecognitionLevel(corpus, 0);
        d->future = QtConcurrent::mapped(communications, LSAStep(corpus));
        d->watcher.setFuture(d->future);
        while (d->watcher.isRunning()) QApplication::processEvents();
    }
    printMessage(QString("Time: %1").arg(timer.elapsed() / 1000.0));
    madeProgress(100);
    return;

//    checks(corpus, communications);
//    return;

//    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        if (!com) continue;
//        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
//            if (!annot) continue;
//            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "auto_syll" << "auto_transcribe" << "segment");
//            foreach (QString speakerID, tiersAll.keys()) {
//                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
//                if (!tiers) continue;
//                QString id = QString("%1/%2_%3.TextGrid").arg(corpus->basePath()).arg(annot->ID()).arg(speakerID);
//                printMessage(id);
//                PraatTextGrid::save(id, tiers);
//                EasyAlignBasic *ea = new EasyAlignBasic(this);
//                ea->setPreciseUtteranceBoundaries(false);
//                printMessage(ea->prepareAlignmentTextgrid(com, tiers));
//                ea->runEasyAlign(com);
//                printMessage(ea->postAlignment(com, tiers));
//                ea->runSyllabify(com);
//            }
//            qDeleteAll(tiersAll);
//        }
//    }

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

void Praaline::Plugins::Aligner::PluginAligner::addPhonetisationToTokens(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage("Adding phonetisation to tokens");
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (com->hasRecordings()) continue;
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

void Praaline::Plugins::Aligner::PluginAligner::checks(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage("Checking...");
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
                if (!tier_tok_min) continue;
                IntervalTier *tier_segment = tiers->getIntervalTierByName("segment");
                if (!tier_segment) continue;
                foreach (Interval *segment, tier_segment->intervals()) {
                    QString seg, tok;
                    QList<Interval *> tokens = tier_tok_min->getIntervalsContainedIn(segment);
                    foreach (Interval *token, tokens) tok.append(token->text());
                    tok = tok.remove(" ").remove("_");
                    seg = segment->text().remove(" ").remove("|-").remove("-|").remove("/");
                    if (tok != seg) {
                        printMessage(QString("%1\t%2\t%3").arg(annotationID).arg(seg).arg(tok));
                    }
                }
                // corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_token);
            }
            qDeleteAll(tiersAll);
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
        QApplication::processEvents();
    }
    madeProgress(100);
    printMessage("Finished");
}
