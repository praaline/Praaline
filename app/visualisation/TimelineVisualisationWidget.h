#ifndef TIMELINEVISUALISATIONWIDGET_H
#define TIMELINEVISUALISATIONWIDGET_H

#include <QString>
#include <QMainWindow>
#include <QModelIndex>
#include "pncore/base/RealTime.h"

namespace Ui {
class TimelineVisualisationWidget;
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

struct TimelineVisualisationWidgetData;

class TimelineVisualisationWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit TimelineVisualisationWidget(QWidget *parent = nullptr);
    ~TimelineVisualisationWidget();

protected slots:
    // Visualisation selector
    void selectedVisualisationChanged(int selectorIndex);
    // Corpus item selection
    void selectedCorpusCommunication(QPointer<Corpus>, QPointer<CorpusCommunication>);
    void selectedCorpusRecording(QPointer<Corpus>, QPointer<CorpusCommunication>,
                                 QPointer<CorpusRecording>);
    void selectedCorpusAnnotation(QPointer<Corpus>, QPointer<CorpusCommunication>,
                                  QPointer<CorpusAnnotation>);
    void moveToAnnotationTime(QPointer<Corpus>, QPointer<CorpusCommunication>,
                              QPointer<CorpusAnnotation>, RealTime);
    // Timeline configuration changes
    void selectedLevelsAttributesChanged();
    void speakerAdded(const QString &speakerID);
    void speakerRemoved(const QString &speakerID);
    // Sync visualiser and annotator
    void annotationEditorCurrentIndexChanged(QModelIndex, QModelIndex);
    void visualiserUserScrolledToTime(RealTime);
    void visualiserPlaybackScrolledToTime(RealTime);
    // Annotation editor
    void setAnnotationEditorOrientation(const Qt::Orientation);

private:
    Ui::TimelineVisualisationWidget *ui;
    TimelineVisualisationWidgetData *d;

    void corpusItemSelectionChanged(QPointer<Corpus>, QPointer<CorpusCommunication>,
                                    QPointer<CorpusRecording>, QPointer<CorpusAnnotation>);
    void visualiserNewSession(QPointer<Corpus>, QPointer<CorpusCommunication>);
    void annotationTimelineEditorOpen(QPointer<Corpus>, const QString &);

    void readVisualisationIndexFromConfigFile();
    void loadVisualisation();

    void loadVisualisationDefault();
    void loadVisualisationAnnotationsProsogram();
    void loadVisualisationBoundariesLOCAS();
    void loadVisualisationBoundariesTITEUF();
    void loadVisualisationMacroprosodyTITEUF(const QString &measure);

};

#endif // TIMELINEVISUALISATIONWIDGET_H
