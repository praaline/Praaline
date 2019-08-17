#ifndef LANGUAGEMODELBUILDERWIDGET_H
#define LANGUAGEMODELBUILDERWIDGET_H

#include "ASRModuleWidgetBase.h"

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class LanguageModelBuilderWidget;
}

struct LanguageModelBuilderWidgetData;

class LanguageModelBuilderWidget : public ASRModuleWidgetBase
{
    Q_OBJECT

public:
    explicit LanguageModelBuilderWidget(QWidget *parent = nullptr);
    ~LanguageModelBuilderWidget();

public slots:
    void open(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com,
              Praaline::Core::CorpusRecording *rec, Praaline::Core::CorpusAnnotation *annot) override;

protected slots:
    void utterancesLevelChanged(QString text);
    void tokensLevelChanged(QString text);
    void createLanguageModel();

private:
    Ui::LanguageModelBuilderWidget *ui;
    LanguageModelBuilderWidgetData *d;
};

#endif // LANGUAGEMODELBUILDERWIDGET_H
