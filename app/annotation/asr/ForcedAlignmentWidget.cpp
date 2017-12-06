#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "ForcedAlignmentWidget.h"
#include "ui_ForcedAlignmentWidget.h"

struct ForcedAlignmentWidgetData {
    QPointer<CorpusRepository> repository;
    QPointer<Corpus> corpus;
    QPointer<CorpusCommunication> communication;
    QPointer<CorpusRecording> recording;
    QPointer<CorpusAnnotation> annotation;
};

ForcedAlignmentWidget::ForcedAlignmentWidget(QWidget *parent) :
    ASRModuleVisualiserWidgetBase(parent), ui(new Ui::ForcedAlignmentWidget), d(new ForcedAlignmentWidgetData)
{
    ui->setupUi(this);
}

ForcedAlignmentWidget::~ForcedAlignmentWidget()
{
    delete ui;
    delete d;
}

void ForcedAlignmentWidget::open(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot)
{
    if (!corpus) return;
    if ((d->corpus == corpus) && (d->communication == com) && (d->recording == rec) && (d->annotation == annot)) return;

}
