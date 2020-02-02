#ifndef ANNOTATIONEDITORBASE_H
#define ANNOTATIONEDITORBASE_H

#include <QString>
#include <QWidget>
#include <QMainWindow>
#include "PraalineCore/Base/RealTime.h"

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class CorpusRecording;
class CorpusAnnotation;
}
}

class AnnotationEditorBase
{
public:
    explicit AnnotationEditorBase() {}
    virtual ~AnnotationEditorBase() {}

    virtual QWidget *editorWidget() = 0;
    virtual void open(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com,
                      Praaline::Core::CorpusRecording *rec, Praaline::Core::CorpusAnnotation *annot) = 0;
    virtual void jumpToTime(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com, Praaline::Core::CorpusAnnotation *annot,
                            const RealTime &time) = 0;
};

#endif // ANNOTATIONEDITORBASE_H
