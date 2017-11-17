// Includes from Praaline Core
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

// Other Sphinx modules and OpenSmile VAD
#include "SphinxConfiguration.h"
#include "SphinxFeatureExtractor.h"
#include "featextract/OpenSmileVAD.h"
#include "SphinxOfflineRecogniser.h"

#include "SphinxAutoTranscriber.h"

namespace Praaline {
namespace ASR {

struct SphinxAutoTranscriberData {
    SphinxAutoTranscriberData() :
        state(SphinxAutoTranscriber::StateInitial),
        recogniser(0)
    {}

    SphinxAutoTranscriber::State state;
    SphinxOfflineRecogniser *recogniser;

    QString filePathRecording;
    QString basePath;
};

SphinxAutoTranscriber::SphinxAutoTranscriber(QObject *parent) :
    QObject(parent), d(new SphinxAutoTranscriberData)
{
    d->recogniser = new SphinxOfflineRecogniser(this);

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

QString SphinxAutoTranscriber::basePath() const
{
    return d->basePath;
}

void SphinxAutoTranscriber::setBasePath(const QString &basePath)
{
    d->basePath = basePath;
}

SphinxConfiguration SphinxAutoTranscriber::config() const
{
    return d->recogniser->config();
}

bool SphinxAutoTranscriber::initialize(const SphinxConfiguration &config)
{
    return d->recogniser->initialize(config);
}

// ====================================================================================================================

bool SphinxAutoTranscriber::stepExtractFeaturesFile()
{

    return true;
}

bool SphinxAutoTranscriber::stepVoiceActivityDetection()
{

    return true;
}

bool SphinxAutoTranscriber::stepAutoTranscribe()
{
    return true;
}



} // namespace ASR
} // namespace Praaline
