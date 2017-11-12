#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "SphinxLongSoundAligner.h"

namespace Praaline {
namespace ASR {

struct SphinxLongSoundAlignerData {
    QPointer<Corpus> corpus;
    QPointer<CorpusCommunication> communication;
    QPointer<CorpusRecording> recording;
    QPointer<CorpusAnnotation> annotation;
};

SphinxLongSoundAligner::SphinxLongSoundAligner(QObject *parent) :
    QObject(parent), d(new SphinxLongSoundAlignerData())
{
}

SphinxLongSoundAligner::~SphinxLongSoundAligner()
{
    delete d;
}

//void SphinxLongSoundAligner::diffTranscriptionWithRecogniser()
//{
//    if (!d->annotation) return;

//    QString path = "/home/george/test_lsa/";
//    QList<Interval *> tokens_recognised = SphinxSegmentation::readContinuousFile(path + "output.txt");

//    QString s;
//    foreach (Interval *intv, tokens_recognised) {
//        s = s.append(QString("%1\t%2\t%3\n")
//                     .arg(intv->tMin().toDouble()).arg(intv->tMax().toDouble()).arg(intv->text()));
//    }
//    ui->textMessages->clear();
//    ui->textMessages->appendPlainText(s);

//    QList<Interval *> tokens_transcript = d->annotation->repository()->annotations()->getIntervals(
//                AnnotationDatastore::Selection(d->annotation->ID(), "", "tok_min"));

//    d->sesSequence = DiffIntervals::intervalDiff(
//                tokens_transcript, tokens_recognised, false).getSes().getSequence();

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
