#ifndef SPHINXFEATUREEXTRACTOR_H
#define SPHINXFEATUREEXTRACTOR_H

#include <QObject>
#include <QPointer>

namespace Praaline {
namespace ASR {

class SphinxFeatureExtractor : public QObject
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
