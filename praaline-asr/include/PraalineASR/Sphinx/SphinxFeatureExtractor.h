#ifndef SPHINXFEATUREEXTRACTOR_H
#define SPHINXFEATUREEXTRACTOR_H

#include <QObject>
#include <QPointer>
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT SphinxFeatureExtractor : public QObject
{
    Q_OBJECT
public:
    explicit SphinxFeatureExtractor(QObject *parent = nullptr);
    ~SphinxFeatureExtractor();

    void setFeatureParametersFile(const QString &filename);
    void setSampleRate(quint64 samplerate);

    bool batchCreateSphinxMFC(QStringList filenamesWave16k);

signals:

public slots:

private:
    QString m_filenameSphinxFeatParams;
    quint64 m_sampleRate;

};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXFEATUREEXTRACTOR_H
