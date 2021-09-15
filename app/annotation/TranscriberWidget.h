#ifndef TRANSCRIBERWIDGET_H
#define TRANSCRIBERWIDGET_H

#include <QMainWindow>
#include "PraalineCore/Base/RealTime.h"
#include "visualisation/SimpleVisualiserWidget.h"

namespace Ui {
class TranscriberWidget;
}

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class CorpusRecording;
class CorpusAnnotation;
}
}

struct TranscriberWidgetData;

class TranscriberWidget : public SimpleVisualiserWidget
{
    Q_OBJECT

public:
    explicit TranscriberWidget(QWidget *parent = nullptr);
    ~TranscriberWidget();

protected slots:
    virtual void newSession() override;

    void selectedCorpusCommunication(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>);
    void selectedCorpusRecording(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>,
                                 QPointer<Praaline::Core::CorpusRecording>);
    void selectedCorpusAnnotation(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>,
                                  QPointer<Praaline::Core::CorpusAnnotation>);
    void selectionChanged(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>,
                          QPointer<Praaline::Core::CorpusRecording>, QPointer<Praaline::Core::CorpusAnnotation>);
    void moveToAnnotationTime(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>,
                              QPointer<Praaline::Core::CorpusAnnotation>, RealTime);


private:
    Ui::TranscriberWidget *ui;
    TranscriberWidgetData *d;
};

#endif // TRANSCRIBERWIDGET_H
