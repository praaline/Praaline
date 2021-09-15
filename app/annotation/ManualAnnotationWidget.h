#ifndef MANUALANNOTATIONWIDGET_H
#define MANUALANNOTATIONWIDGET_H

#include <QMainWindow>
#include <QModelIndex>
#include <QStandardItem>
#include "PraalineCore/Corpus/Corpus.h"

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
    void selectedCorpusCommunication(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>);
    void selectedCorpusRecording(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>, QPointer<Praaline::Core::CorpusRecording>);
    void selectedCorpusAnnotation(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>, QPointer<Praaline::Core::CorpusAnnotation>);
    void selectionChanged(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>, QPointer<Praaline::Core::CorpusRecording>, QPointer<Praaline::Core::CorpusAnnotation>);
    void moveToAnnotationTime(QPointer<Praaline::Core::Corpus>, QPointer<Praaline::Core::CorpusCommunication>, QPointer<Praaline::Core::CorpusAnnotation>, RealTime);
    void editorTabNew();
    void editorTabCloseRequested(int);


private:
    Ui::ManualAnnotationWidget *ui;
    ManualAnnotationWidgetData *d;

    void setupActions();
    void openForEditing(Praaline::Core::Corpus *corpus, const QString &annotationID);
};

#endif // MANUALANNOTATIONWIDGET_H
