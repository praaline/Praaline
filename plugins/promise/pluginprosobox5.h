#ifndef PLUGINPROSOBOX5_H
#define PLUGINPROSOBOX5_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusAnnotation.h"
using namespace Praaline::Core;

#include "annotationpluginpraatscript.h"

class PluginProsobox5 : public AnnotationPluginPraatScript
{
    Q_OBJECT
public:
    explicit PluginProsobox5(QObject *parent = nullptr);

    void ProsoGram(Corpus *corpus, CorpusRecording *rec, CorpusAnnotation *annot);
    void CreateSyllTable(CorpusCommunication *com);
    void ProsoProm(CorpusCommunication *com);
    void MakeIFtier(CorpusCommunication *com);

signals:

public slots:

};

#endif // PLUGINPROSOBOX5_H
