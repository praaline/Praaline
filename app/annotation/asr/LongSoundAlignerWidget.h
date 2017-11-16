#ifndef LONGSOUNDALIGNERWIDGET_H
#define LONGSOUNDALIGNERWIDGET_H

#include "ASRModuleVisualiserWidgetBase.h"

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class LongSoundAlignerWidget;
}

struct LongSoundAlignerWidgetData;

class LongSoundAlignerWidget : public ASRModuleVisualiserWidgetBase
{
    Q_OBJECT

public:
    explicit LongSoundAlignerWidget(QWidget *parent = 0);
    ~LongSoundAlignerWidget();

public slots:
    void open(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com,
              Praaline::Core::CorpusRecording *rec, Praaline::Core::CorpusAnnotation *annot) override;

protected slots:
    void diffTranscriptionWithRecogniser();

private:
    Ui::LongSoundAlignerWidget *ui;
    LongSoundAlignerWidgetData *d;
};

#endif // LONGSOUNDALIGNERWIDGET_H
