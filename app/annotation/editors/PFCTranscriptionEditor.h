#ifndef PFCTRANSCRIPTIONEDITOR_H
#define PFCTRANSCRIPTIONEDITOR_H

#include <QObject>
#include <QWidget>
#include "AnnotationEditorBase.h"

namespace Ui {
class PFCTranscriptionEditor;
}

struct PFCTranscriptionEditorData;

class PFCTranscriptionEditor : public QMainWindow, public AnnotationEditorBase
{
    Q_OBJECT

public:
    explicit PFCTranscriptionEditor(QWidget *parent = 0);
    virtual ~PFCTranscriptionEditor();
    QWidget *editorWidget() override { return this; }

public slots:
    void open(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com,
              Praaline::Core::CorpusRecording *rec, Praaline::Core::CorpusAnnotation *annot) override;
    void jumpToTime(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com, Praaline::Core::CorpusAnnotation *annot,
                    const RealTime &time) override;

private slots:
    void previous();
    void next();
    void edit();
    void update();
    void save();
    void updateTableView();

private:
    PFCTranscriptionEditorData *d;
    Ui::PFCTranscriptionEditor *ui;
};

#endif // PFCTRANSCRIPTIONEDITOR_H
