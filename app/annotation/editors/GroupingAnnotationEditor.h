#ifndef GROUPINGANNOTATIONEDITOR_H
#define GROUPINGANNOTATIONEDITOR_H

#include <QObject>
#include <QWidget>
#include "AnnotationEditorBase.h"

struct GroupingAnnotationEditorData;

class GroupingAnnotationEditor : public QMainWindow, public AnnotationEditorBase
{
    Q_OBJECT

public:
    GroupingAnnotationEditor(QWidget *parent = nullptr);
    virtual ~GroupingAnnotationEditor();
    QWidget *editorWidget() override { return this; }

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
    void toggleTimelineConfig();

    void mediaPositionChanged(qint64 position);
    void mediaPlay();
    void mediaPause();
    void mediaStop();

private:
    GroupingAnnotationEditorData *d;

    void setupActions();
    void openForEditing(Praaline::Core::Corpus *corpus, const QString &annotationID);
};

#endif // GROUPINGANNOTATIONEDITOR_H
