#ifndef SPHINXACOUSTICMODELADAPTER_H
#define SPHINXACOUSTICMODELADAPTER_H

#include <QObject>
#include "pncore/corpus/corpus.h"

struct SphinxAcousticModelAdapterData;

class SphinxAcousticModelAdapter : public QObject
{
    Q_OBJECT
public:
    explicit SphinxAcousticModelAdapter(QObject *parent = 0);
    ~SphinxAcousticModelAdapter();

    bool loadDictionary(const QString &filename);
    bool unknownWordsInVocabFile(const QString &filename, QStringList &unknownList);
    bool createAdaptationFiles(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications,
                               QStringList &listUnknownWords, bool segmentSound = false);

signals:
    void printMessage(QString message);

public slots:

private:
    SphinxAcousticModelAdapterData *d;
};

#endif // SPHINXACOUSTICMODELADAPTER_H
