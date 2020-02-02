#ifndef SPHINXACOUSTICMODELADAPTER_H
#define SPHINXACOUSTICMODELADAPTER_H

#include <QObject>
#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineASR/PraalineASR_Global.h"
#include "PraalineASR/Sphinx/SphinxPronunciationDictionary.h"

namespace Praaline {
namespace ASR {

struct SphinxAcousticModelAdapterData;

class PRAALINE_ASR_SHARED_EXPORT SphinxAcousticModelAdapter : public QObject
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
