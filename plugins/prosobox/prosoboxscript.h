#ifndef PROSOBOXSCRIPT_H
#define PROSOBOXSCRIPT_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusAnnotation.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

#include "annotationpluginpraatscript.h"

class ProsoboxScript : public AnnotationPluginPraatScript
{
    Q_OBJECT
public:
    explicit ProsoboxScript(QObject *parent = nullptr);
    ~ProsoboxScript();

    static void createTextGrid(QString path, Corpus *corpus, QString annotationID, QString speakerID);

    void runCreateSyllTable(QString path, Corpus *corpus, QString annotationID, QString speakerID);
    void runProsoProm(Corpus *corpus, CorpusRecording *rec, AnnotationTierGroup *tiers, QString annotationID, QString speakerID);
    void runMakeIFtier(Corpus *corpus, CorpusRecording *rec, AnnotationTierGroup *tiers, QString annotationID, QString speakerID);

signals:

public slots:

};

#endif // PROSOBOXSCRIPT_H
