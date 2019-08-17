#ifndef TRANSCRIBERWIDGET_H
#define TRANSCRIBERWIDGET_H

#include <QMainWindow>
#include "pncore/base/RealTime.h"
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
using namespace Praaline::Core;

struct TranscriberWidgetData;

class TranscriberWidget : public SimpleVisualiserWidget
{
    Q_OBJECT

public:
    explicit TranscriberWidget(QWidget *parent = nullptr);
    ~TranscriberWidget();

protected slots:
    virtual void newSession() override;

    void selectedCorpusCommunication(QPointer<Corpus>, QPointer<CorpusCommunication>);
    void selectedCorpusRecording(QPointer<Corpus>, QPointer<CorpusCommunication>,
                                 QPointer<CorpusRecording>);
    void selectedCorpusAnnotation(QPointer<Corpus>, QPointer<CorpusCommunication>,
                                  QPointer<CorpusAnnotation>);
    void selectionChanged(QPointer<Corpus>, QPointer<CorpusCommunication>,
                          QPointer<CorpusRecording>, QPointer<CorpusAnnotation>);
    void moveToAnnotationTime(QPointer<Corpus>, QPointer<CorpusCommunication>,
                              QPointer<CorpusAnnotation>, RealTime);


private:
    Ui::TranscriberWidget *ui;
    TranscriberWidgetData *d;
};

#endif // TRANSCRIBERWIDGET_H
