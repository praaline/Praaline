#ifndef PROSOBOXSCRIPT_H
#define PROSOBOXSCRIPT_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include "pncore/corpus/corpus.h"
#include "pncore/corpus/corpuscommunication.h"
#include "pncore/corpus/corpusannotation.h"
#include "pncore/annotation/annotationtiergroup.h"
#include "annotationpluginpraatscript.h"

class ProsoboxScript : public AnnotationPluginPraatScript
{
    Q_OBJECT
public:
    explicit ProsoboxScript(QObject *parent = 0);
    ~ProsoboxScript();

    static void createTextGrid(QString path, Corpus *corpus, QString annotationID, QString speakerID);

    void runCreateSyllTable(QString path, Corpus *corpus, QString annotationID, QString speakerID);
    void runProsoProm(Corpus *corpus, CorpusRecording *rec, QPointer<AnnotationTierGroup> tiers, QString annotationID, QString speakerID);
    void runMakeIFtier(Corpus *corpus, CorpusRecording *rec, QPointer<AnnotationTierGroup> tiers, QString annotationID, QString speakerID);

signals:

public slots:

};

#endif // PROSOBOXSCRIPT_H
