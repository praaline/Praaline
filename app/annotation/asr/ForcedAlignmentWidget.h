#ifndef FORCEDALIGNMENTWIDGET_H
#define FORCEDALIGNMENTWIDGET_H

#include "ASRModuleVisualiserWidgetBase.h"

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class ForcedAlignmentWidget;
}

struct ForcedAlignmentWidgetData;

class ForcedAlignmentWidget : public ASRModuleVisualiserWidgetBase
{
    Q_OBJECT

public:
    explicit ForcedAlignmentWidget(QWidget *parent = 0);
    ~ForcedAlignmentWidget();

public slots:
    void open(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com,
              Praaline::Core::CorpusRecording *rec, Praaline::Core::CorpusAnnotation *annot) override;

private slots:
    void stepPhonetiseAll();
    void stepAlignAll();
    void stepSpeakerAdaptation();

private:
    Ui::ForcedAlignmentWidget *ui;
    ForcedAlignmentWidgetData *d;
};

#endif // FORCEDALIGNMENTWIDGET_H
