#ifndef AUTOMATICTRANSCRIPTIONWIDGET_H
#define AUTOMATICTRANSCRIPTIONWIDGET_H

#include "ASRModuleVisualiserWidgetBase.h"

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class AutomaticTranscriptionWidget;
}

struct AutomaticTranscriptionWidgetData;

class AutomaticTranscriptionWidget : public ASRModuleVisualiserWidgetBase
{
    Q_OBJECT

public:
    explicit AutomaticTranscriptionWidget(QWidget *parent = 0);
    ~AutomaticTranscriptionWidget();

public slots:
    void open(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com,
              Praaline::Core::CorpusRecording *rec, Praaline::Core::CorpusAnnotation *annot) override;

protected slots:
    void annotationLevelChanged(QString text);
    void futureResultReadyAt(int);
    void futureProgressValueChanged(int);
    void futureFinished();

    void transcribe();

private:
    Ui::AutomaticTranscriptionWidget *ui;
    AutomaticTranscriptionWidgetData *d;

    void prepareFile();

    struct SphinxAutomaticTranscriptionStep;
};

#endif // AUTOMATICTRANSCRIPTIONWIDGET_H
