#ifndef TIMELINEANNOTATIONEDITOR_H
#define TIMELINEANNOTATIONEDITOR_H

#include <QObject>
#include <QWidget>
#include "AnnotationEditorBase.h"

struct TimelineAnnotationEditorData;

class TimelineAnnotationEditor : public AnnotationEditorBase
{
    Q_OBJECT

public:
    TimelineAnnotationEditor(QWidget *parent = 0);
    virtual ~TimelineAnnotationEditor();

public slots:
    void open(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com,
              Praaline::Core::CorpusRecording *rec, Praaline::Core::CorpusAnnotation *annot) override;
    void jumpToTime(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com, Praaline::Core::CorpusAnnotation *annot,
                    const RealTime &time) override;

protected slots:
    void saveAnnotations();
    void speakerAdded(const QString &speakerID);
    void speakerRemoved(const QString &speakerID);
    void selectedLevelsAttributesChanged();

    void timelineSelectedRowsChanged(QList<int> rows);
    void timelineCurrentIndexChanged(const QModelIndex &current, const QModelIndex &previous);
    void toggleOrientation();
    void toggleTimelineConfig();

    void mediaPositionChanged(qint64 position);
    void mediaPlay();
    void mediaPause();
    void mediaStop();

    void moveMinBoundaryLeft();
    void moveMaxBoundaryRight();
    void intervalJoin();
    void intervalSplit();

private:
    TimelineAnnotationEditorData *d;

    void setupActions();
    void updateAnnotationControls();
    void openForEditing(Praaline::Core::Corpus *corpus, const QString &annotationID);
};

#endif // TIMELINEANNOTATIONEDITOR_H
