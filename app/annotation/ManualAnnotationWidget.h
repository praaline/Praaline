#ifndef MANUALANNOTATIONWIDGET_H
#define MANUALANNOTATIONWIDGET_H

#include <QMainWindow>
#include <QModelIndex>
#include <QStandardItem>
#include "PraalineCore/Corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class ManualAnnotationWidget;
}

struct ManualAnnotationWidgetData;

class ManualAnnotationWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit ManualAnnotationWidget(QWidget *parent = nullptr);
    ~ManualAnnotationWidget();

protected slots:
    void selectedCorpusCommunication(QPointer<Corpus>, QPointer<CorpusCommunication>);
    void selectedCorpusRecording(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusRecording>);
    void selectedCorpusAnnotation(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusAnnotation>);
    void selectionChanged(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusRecording>, QPointer<CorpusAnnotation>);
    void moveToAnnotationTime(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusAnnotation>, RealTime);
    void editorTabNew();
    void editorTabCloseRequested(int);


private:
    Ui::ManualAnnotationWidget *ui;
    ManualAnnotationWidgetData *d;

    void setupActions();
    void openForEditing(Corpus *corpus, const QString &annotationID);
};

#endif // MANUALANNOTATIONWIDGET_H
