#ifndef ASRMODULEWIDGETBASE_H
#define ASRMODULEWIDGETBASE_H

#include <QString>
#include <QWidget>
#include <QMainWindow>
#include "pncore/base/RealTime.h"

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class CorpusRecording;
class CorpusAnnotation;
}
}

class ASRModuleWidgetBase : public QMainWindow
{
    Q_OBJECT
public:
    explicit ASRModuleWidgetBase(QWidget *parent = nullptr);
    virtual ~ASRModuleWidgetBase();

public slots:
    virtual void open(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusCommunication *com,
                      Praaline::Core::CorpusRecording *rec, Praaline::Core::CorpusAnnotation *annot) = 0;
};

#endif // ASRMODULEWIDGETBASE_H
