#ifndef SPHINXFEATUREEXTRACTOR_H
#define SPHINXFEATUREEXTRACTOR_H

#include <QObject>
#include <QPointer>

class SphinxFeatureExtractor : public QObject
{
    Q_OBJECT
public:
    explicit SphinxFeatureExtractor(QObject *parent = 0);
    ~SphinxFeatureExtractor();

    void setFeatureParametersFile(const QString &filename);
    void setSampleRate(quint64 samplerate);

    bool batchCreateSphinxMFC(const QString &basePath, QStringList filenamesWave16k);

signals:

public slots:

private:
    QString m_filenameSphinxFeatParams;
    quint64 m_sampleRate;

};

#endif // SPHINXFEATUREEXTRACTOR_H
