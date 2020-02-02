#include <QFileInfo>
#include <QTemporaryDir>

#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
using namespace Praaline::Core;

#include "PraalineMedia/AudioSegmenter.h"
using namespace Praaline::Media;

// #include "pnlib/featextract/OpenSmileVAD.h"

// Other Sphinx modules and OpenSmile VAD
#include "PraalineASR/Sphinx/SphinxConfiguration.h"
#include "PraalineASR/Sphinx/SphinxFeatureExtractor.h"
#include "PraalineASR/Sphinx/SphinxOfflineRecogniser.h"
#include "PraalineASR/Sphinx/SphinxAutoTranscriber.h"

namespace Praaline {
namespace ASR {

struct SphinxAutoTranscriberData {
    SphinxAutoTranscriberData() :
        state(SphinxAutoTranscriber::StateInitial),
        extractor(0), recogniser(0),
        keepDownsampledFile(false)
    {}

    SphinxAutoTranscriber::State state;
    SphinxFeatureExtractor *extractor;
    SphinxOfflineRecogniser *recogniser;

    QString filePathRecording;
    QString filePathDownsampled;
    QString filePathFeaturesFile;
    QString workingDirectory;
    bool keepDownsampledFile;

    QTemporaryDir tempDirectory;
    QList<QPair<double, double> > vadResults;
    QList<Interval *> utterances;
    QHash<long long, QList<Interval *> > tokens;
};

SphinxAutoTranscriber::SphinxAutoTranscriber(QObject *parent) :
    QObject(parent), d(new SphinxAutoTranscriberData)
{
    d->extractor = new SphinxFeatureExtractor(this);
    d->recogniser = new SphinxOfflineRecogniser(this);
    connect(d->recogniser, SIGNAL(error(QString)), this, SLOT(recogniserError(QString)));
    connect(d->recogniser, SIGNAL(info(QString)), this, SLOT(recogniserInfo(QString)));
}

SphinxAutoTranscriber::~SphinxAutoTranscriber()
{
    delete d;
}

SphinxAutoTranscriber::State SphinxAutoTranscriber::currentState() const
{
    return d->state;
}

QString SphinxAutoTranscriber::filePathRecording() const
{
    return d->filePathRecording;
}

void SphinxAutoTranscriber::setFilePathRecording(const QString &filepath)
{
    if (d->filePathRecording != filepath) {
        d->state = StateInitial;
        d->filePathRecording = filepath;
    }
}

QString SphinxAutoTranscriber::workingDirectory() const
{
    return d->workingDirectory;
}

void SphinxAutoTranscriber::setWorkingDirectory(const QString &workingDirectory)
{
    d->workingDirectory = workingDirectory;
}

bool SphinxAutoTranscriber::keepDownsampledFile() const
{
    return d->keepDownsampledFile;
}

void SphinxAutoTranscriber::setKeepDownsampledFile(bool keep)
{
    d->keepDownsampledFile = keep;
}

SphinxConfiguration SphinxAutoTranscriber::configuration() const
{
    return d->recogniser->config();
}

bool SphinxAutoTranscriber::setConfiguration(const SphinxConfiguration &config)
{
    return d->recogniser->initialize(config);
}

QList<QPair<double, double> > &SphinxAutoTranscriber::vadResults()
{
    return d->vadResults;
}

QList<Interval *> SphinxAutoTranscriber::utterances()
{
    return d->utterances;
}

QList<Interval *> SphinxAutoTranscriber::tokens()
{
    QList<Interval *> tokens;
    foreach (long long t, d->tokens.keys()) {
        tokens << d->tokens.value(t);
    }
    return tokens;
}

void SphinxAutoTranscriber::recogniserError(QString message)
{
    printMessage(message);
}

void SphinxAutoTranscriber::recogniserInfo(QString message)
{
    printMessage(message);
}

// ====================================================================================================================

bool SphinxAutoTranscriber::stepExtractFeaturesFile()
{
    // Decide on paths and filenames
    if (!QFile::exists(d->filePathRecording)) {
        printMessage(QString("Error: File <i>%1</i> does not exist.").arg(d->filePathRecording));
        return false;
    }
    QFileInfo fi(d->filePathRecording);
    if (d->workingDirectory.isEmpty()) {
        if (!d->tempDirectory.isValid()) {
            printMessage(QString("Error: Could not create temporary working directory. %1")
                         .arg(d->tempDirectory.errorString()));
            return false;
        }
        d->filePathDownsampled = d->tempDirectory.path() + "/" + fi.baseName() + ".16k.wav";
        d->filePathFeaturesFile = d->tempDirectory.path() + "/" + fi.baseName() + ".mfc";
    }
    else {
        if (d->workingDirectory.endsWith("/") || d->workingDirectory.endsWith("\\"))
            d->workingDirectory.chop(1);
        d->filePathDownsampled = d->workingDirectory + "/" + fi.baseName() + ".16k.wav";
        d->filePathFeaturesFile = d->workingDirectory + "/" + fi.baseName() + ".mfc";
    }
    // Create downsampled file
    AudioSegmenter::resample(d->filePathRecording, d->filePathDownsampled, 16000, false, 1);
    if (!QFile::exists(d->filePathDownsampled)) {
        printMessage(QString("Error: Could not create down-sampled wave file %1")
                     .arg(d->filePathDownsampled));
        return false;
    }
    // Extract features file
    d->extractor->setFeatureParametersFile(d->recogniser->config().directoryAcousticModel() + "/feat.params");
    d->extractor->batchCreateSphinxMFC(QStringList() << d->filePathDownsampled);
    QString filenameFEOut = QString(d->filePathDownsampled).replace(".wav", ".mfc");
    QString filenameSimple = QString(filenameFEOut).replace(".16k.mfc", ".mfc");
    QFile::rename(filenameFEOut, filenameSimple);
    // Check for success
    if (!QFile::exists(d->filePathFeaturesFile)) {
        printMessage(QString("Error: Could not create extracted features file %1")
                     .arg(d->filePathFeaturesFile));
        return false;
    }
    printMessage("Succesfully extracted features for speech recognition.");
    d->state = StateFeaturesFileExtracted;
    return true;
}

bool SphinxAutoTranscriber::stepVoiceActivityDetection()
{
    if (!QFile::exists(d->filePathRecording)) {
        printMessage(QString("Error: File <i>%1</i> does not exist.").arg(d->filePathRecording));
        return false;
    }
    // Run OpenSmile VAD
//    d->vadResults.clear();
//    if (!OpenSmileVAD::runVAD(d->filePathRecording, d->vadResults)) {
//        printMessage(QString("Error: Unable to run OpenSmile voice activity detection."));
//        return false;
//    }
//    // Process VAD results to find utterance boundaries
//    qDeleteAll(d->utterances);
//    d->utterances = OpenSmileVAD::splitToUtterancesWithoutPauses(
//                d->vadResults, RealTime::fromMilliseconds(300), RealTime::fromMilliseconds(300), "");
//    printMessage("Succesfully performed voice activity detection.");
    return true;
}

bool SphinxAutoTranscriber::stepAutoTranscribe()
{
    if (!QFile::exists(d->filePathFeaturesFile)) {
        printMessage(QString("Error: Extracted features file <i>%1</i> does not exist. Please extract features.")
                     .arg(d->filePathFeaturesFile));
        return false;
    }
    if (d->utterances.isEmpty()) {
        printMessage(QString("The utterance list is empty. Please run Voice Activity Detection."));
        return false;
    }
    d->recogniser->openFeatureFile(d->filePathFeaturesFile);
    foreach (Interval *utt, d->utterances) {
        qint64 frameStart = utt->tMin().toNanoseconds() / 10000000L;
        qint64 frameEnd = utt->tMax().toNanoseconds() / 10000000L;
        if (d->recogniser->decode(frameStart, frameEnd)) {
            utt->setText(d->recogniser->getUtteranceText());
            d->tokens.insert(utt->tMin().toNanoseconds(), d->recogniser->getSegmentation());
        }
        printMessage(QString("%1\t%2\t%3\t%4\t%5")
                     .arg(utt->tMin().toDouble()).arg(utt->tMax().toDouble())
                     .arg(frameStart).arg(frameEnd)
                     .arg(utt->text().replace("<", "{").replace(">", "}")));
    }
    d->recogniser->closeFeatureFile();
    return true;
}



} // namespace ASR
} // namespace Praaline
