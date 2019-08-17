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
    explicit AutomaticTranscriptionWidget(QWidget *parent = nullptr);
    ~AutomaticTranscriptionWidget();

public slots:
    void open(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com,
              Praaline::Core::CorpusRecording *rec, Praaline::Core::CorpusAnnotation *annot) override;

protected slots:
    void receivedMessage(QString text);
    void annotationLevelChanged(QString text);

    void stepPrepare();
    void stepVAD();
    void stepAutoTranscribe();
    void stepSpeakerAdaptation();

private:
    Ui::AutomaticTranscriptionWidget *ui;
    AutomaticTranscriptionWidgetData *d;

    struct SphinxAutomaticTranscriptionStep;
};

#endif // AUTOMATICTRANSCRIPTIONWIDGET_H
