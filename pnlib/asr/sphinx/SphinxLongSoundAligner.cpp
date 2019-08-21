// Includes from Praaline Core
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

// Includes for diff functionality
#include "pncore/diff/dtl/dtl.h"
#include "pncore/diff/DiffIntervals.h"

// Includes for other Sphinx modules
#include "SphinxConfiguration.h"
#include "SphinxOfflineRecogniser.h"
#include "SphinxSegmentation.h"
#include "SphinxLongSoundAligner.h"

namespace Praaline {
namespace ASR {

struct SphinxLongSoundAlignerData {
    SphinxLongSoundAlignerData() :
        state(SphinxLongSoundAligner::StateInitial)
    {}

    SphinxLongSoundAligner::State state;

    SphinxOfflineRecogniser *recogniser;

    QPointer<Corpus> corpus;
    CorpusCommunication *communication;
    CorpusRecording *recording;
    CorpusAnnotation *annotation;

    QList<Interval *> tokens_transcription;
    QList<Interval *> tokens_recognised;

    dtl::Ses<Interval *>::sesElemVec sesSequence;
};

SphinxLongSoundAligner::SphinxLongSoundAligner(QObject *parent) :
    QObject(parent), d(new SphinxLongSoundAlignerData())
{
    d->recogniser = new SphinxOfflineRecogniser(this);
}

SphinxLongSoundAligner::~SphinxLongSoundAligner()
{
    // d->recogniser is deleted as a child QObject of this
    qDeleteAll(d->tokens_transcription);
    qDeleteAll(d->tokens_recognised);
    delete d;
}

SphinxLongSoundAligner::State SphinxLongSoundAligner::currentState() const
{
    return d->state;
}

SphinxConfiguration SphinxLongSoundAligner::config() const
{
    return d->recogniser->config();
}

bool SphinxLongSoundAligner::initialize(const SphinxConfiguration &config)
{
    return d->recogniser->initialize(config);
}

QString SphinxLongSoundAligner::tableTokensRecognised() const
{
    QString s;
    foreach (Interval *intv, d->tokens_recognised) {
        s = s.append(QString("%1\t%2\t%3\n")
                     .arg(intv->tMin().toDouble()).arg(intv->tMax().toDouble()).arg(intv->text()));
    }
    return s;
}

QList<Interval *> SphinxLongSoundAligner::tokensTranscription() const
{
    return d->tokens_transcription;
}

QList<Interval *> SphinxLongSoundAligner::tokensRecognised() const
{
    return d->tokens_recognised;
}

dtl::Ses<Interval *>::sesElemVec SphinxLongSoundAligner::diffSequence() const
{
    return d->sesSequence;
}

bool SphinxLongSoundAligner::stepExtractFeaturesFile()
{

    return true;
}

bool SphinxLongSoundAligner::stepVoiceActivityDetection()
{

    return true;
}

bool SphinxLongSoundAligner::stepCreateContrainedLanguageModel()
{
    return true;
}

bool SphinxLongSoundAligner::stepFirstPassRecogniser()
{

    return true;
}

bool SphinxLongSoundAligner::stepDiffTranscriptionWithRecogniser()
{
    if (!d->annotation) return false;
    QString path = QDir::homePath() + "/Praaline/test_lsa/";
    d->tokens_recognised = SphinxSegmentation::readContinuousFile(path + "output.txt");

    d->tokens_transcription = d->annotation->repository()->annotations()->getIntervals(
                AnnotationDatastore::Selection(d->annotation->ID(), "", "tok_min"));

    if ((d->tokens_recognised.count() == 0) || (d->tokens_transcription.count() == 0))
        return false;

    d->sesSequence = DiffIntervals::intervalDiff(
                d->tokens_transcription, d->tokens_recognised, false).getSes().getSequence();

    return true;
}

bool SphinxLongSoundAligner::stepFindAnchors()
{
    if (d->sesSequence.size() == 0) return false;

    size_t i = 0;
    while (i < d->sesSequence.size()) {
        dtl::Ses<Interval *>::sesElem elem = d->sesSequence[i];
        Interval *intv_trn = (elem.second.beforeIdx > 0) ? d->tokens_transcription.at(elem.second.beforeIdx - 1) : nullptr;
        Interval *intv_rec = (elem.second.afterIdx > 0) ? d->tokens_recognised.at(elem.second.afterIdx - 1) : nullptr;
        dtl::edit_t editType = elem.second.type;

    }


    return true;
}

bool SphinxLongSoundAligner::stepAlignTranscription()
{

    return true;
}

//void SphinxLongSoundAligner::diffTranscriptionWithRecogniser()

//
//    DiffSESforIntervalsTableModel *model = new DiffSESforIntervalsTableModel(
//                d->sesSequence, tokens_transcript, tokens_recognised,
//                "", "", QStringList() << "anchor", QStringList(), this);
//    int anchorContinuousMatches(4), anchorSkipMismatches(2);
//    bool inAnchor(false);
//    int i = 0;
//    while (i < model->rowCount()) {
//        QString op = model->data(model->index(i, 0), Qt::DisplayRole).toString();
//        if (inAnchor) {
//        }
//        else {
//            bool check(true);
//            for (int j = i; (j < i + anchorContinuousMatches) && (j < model->rowCount()); ++j) {
//                check = check && (model->data(model->index(j, 0), Qt::DisplayRole).toString() == "=");
//            }
//            if (check) {
//                inAnchor = true;
//                for (int j = i; (j < i + anchorContinuousMatches) && (j < model->rowCount()); ++j) {
//                }
//            }
//        }
//        ++i;
//    }
//}

} // namespace ASR
} // namespace Praaline
