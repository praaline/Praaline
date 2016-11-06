#ifndef SPHINXACOUSTICMODELADAPTER_H
#define SPHINXACOUSTICMODELADAPTER_H

#include <QObject>
#include "pncore/corpus/Corpus.h"
#include "SphinxPronunciationDictionary.h"

struct SphinxAcousticModelAdapterData;

class SphinxAcousticModelAdapter : public QObject
{
    Q_OBJECT
public:
    explicit SphinxAcousticModelAdapter(QObject *parent = 0);
    ~SphinxAcousticModelAdapter();

signals:
    void printMessage(QString message);

public slots:

private:
    SphinxAcousticModelAdapterData *d;
};

#endif // SPHINXACOUSTICMODELADAPTER_H
