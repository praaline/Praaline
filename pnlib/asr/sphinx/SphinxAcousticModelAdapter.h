#ifndef SPHINXACOUSTICMODELADAPTER_H
#define SPHINXACOUSTICMODELADAPTER_H

#include <QObject>
#include "pncore/corpus/Corpus.h"
#include "SphinxPronunciationDictionary.h"

namespace Praaline {
namespace ASR {

struct SphinxAcousticModelAdapterData;

class SphinxAcousticModelAdapter : public QObject
{
    Q_OBJECT
public:
    explicit SphinxAcousticModelAdapter(QObject *parent = nullptr);
    ~SphinxAcousticModelAdapter();

signals:
    void printMessage(QString message);

public slots:

private:
    SphinxAcousticModelAdapterData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXACOUSTICMODELADAPTER_H
