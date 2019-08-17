#ifndef SPEECHRECOGNITIONWIDGET_H
#define SPEECHRECOGNITIONWIDGET_H

#include <QMainWindow>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

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
    void selectedCorpusCommunication(QPointer<Corpus>, QPointer<CorpusCommunication>);
    void selectedCorpusRecording(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusRecording>);
    void selectedCorpusAnnotation(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusAnnotation>);
    void selectionChanged(QPointer<Corpus>, QPointer<CorpusCommunication>,
                          QPointer<CorpusRecording>, QPointer<CorpusAnnotation>);
    void moduleTabNew();
    void moduleTabCloseRequested(int);

private:
    Ui::SpeechRecognitionWidget *ui;
    SpeechRecognitionWidgetData *d;
};

#endif // SPEECHRECOGNITIONWIDGET_H
