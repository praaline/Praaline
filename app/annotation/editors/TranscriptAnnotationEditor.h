#ifndef TRANSCRIPTANNOTATIONEDITOR_H
#define TRANSCRIPTANNOTATIONEDITOR_H

#include <QObject>
#include <QWidget>
#include "AnnotationEditorBase.h"

namespace Praaline {
namespace Core {
class IntervalTier;
}
}

struct TranscriptAnnotationEditorData;

class TranscriptAnnotationEditor : public AnnotationEditorBase
{
    Q_OBJECT
public:
    TranscriptAnnotationEditor(QWidget *parent = 0);
    virtual ~TranscriptAnnotationEditor();

public slots:
    void open(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com,
              Praaline::Core::CorpusRecording *rec, Praaline::Core::CorpusAnnotation *annot) override;
    void jumpToTime(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com, Praaline::Core::CorpusAnnotation *annot,
                    const RealTime &time) override;

protected slots:
    void saveAnnotations();
    void mediaPositionChanged(qint64 position);
    void mediaPlay();
    void mediaPause();
    void mediaStop();

private:
    TranscriptAnnotationEditorData *d;

    void setupActions();
    void openForEditing(Praaline::Core::Corpus *corpus, const QString &annotationID);
    void createTranscriptionText();


};

#endif // TRANSCRIPTANNOTATIONEDITOR_H
