#ifndef MANUALANNOTATIONWIDGET_H
#define MANUALANNOTATIONWIDGET_H

#include <QMainWindow>
#include <QModelIndex>
#include <QStandardItem>
#include "pncore/corpus/corpus.h"

namespace Ui {
class ManualAnnotationWidget;
}

struct ManualAnnotationWidgetData;

class ManualAnnotationWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit ManualAnnotationWidget(QWidget *parent = 0);
    ~ManualAnnotationWidget();

protected slots:
    void selectedCorpusCommunication(QPointer<Corpus>, QPointer<CorpusCommunication>);
    void selectedCorpusRecording(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusRecording>);
    void selectedCorpusAnnotation(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusAnnotation>);
    void selectionChanged(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusRecording>, QPointer<CorpusAnnotation>);
    void moveToAnnotationTime(QPointer<Corpus>, QPointer<CorpusCommunication>, QPointer<CorpusAnnotation>, RealTime);

    void saveAnnotations();

    void selectedLevelsAttributesChanged();
    void speakerAdded(const QString &speakerID);
    void speakerRemoved(const QString &speakerID);

    void verticalTimelineSelectedRowsChanged(QList<int> rows);
    void verticalTimelineCurrentIndexChanged(const QModelIndex &current, const QModelIndex &previous);
    void toggleOrientation();

    void mediaPositionChanged(qint64 position);
    void mediaPlay();
    void mediaPause();
    void mediaStop();

    void moveMinBoundaryLeft();
    void moveMaxBoundaryRight();
    void intervalJoin();
    void intervalSplit();

private:
    Ui::ManualAnnotationWidget *ui;
    ManualAnnotationWidgetData *d;

    void setupActions();
    void openForEditing(Corpus *corpus, const QString &annotationID);
    void updateAnnotationControls();
};

#endif // MANUALANNOTATIONWIDGET_H
