#ifndef SPEECHRECOGNITIONWIDGET_H
#define SPEECHRECOGNITIONWIDGET_H

#include <QMainWindow>

#include "PraalineCore/Corpus/Corpus.h"

namespace Ui {
class SpeechRecognitionWidget;
}

struct SpeechRecognitionWidgetData;

class SpeechRecognitionWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit SpeechRecognitionWidget(QWidget *parent = nullptr);
    ~SpeechRecognitionWidget();

protected slots:
    void selectedCorpusCommunication(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>);
    void selectedCorpusRecording(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>, QPointer<Praaline::Core::CorpusRecording>);
    void selectedCorpusAnnotation(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>, QPointer<Praaline::Core::CorpusAnnotation>);
    void selectionChanged(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>,
                          QPointer<Praaline::Core::CorpusRecording>, QPointer<Praaline::Core::CorpusAnnotation>);
    void moduleTabNew();
    void moduleTabCloseRequested(int);

private:
    Ui::SpeechRecognitionWidget *ui;
    SpeechRecognitionWidgetData *d;
};

#endif // SPEECHRECOGNITIONWIDGET_H
