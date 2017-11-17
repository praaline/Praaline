#ifndef SPHINXAUTOTRANSCRIBER_H
#define SPHINXAUTOTRANSCRIBER_H

#include <QObject>
#include "SphinxConfiguration.h"

namespace Praaline {

namespace Core {
class Interval;
class Corpus;
class CorpusCommunication;
}

namespace ASR {

struct SphinxAutoTranscriberData;

class SphinxAutoTranscriber : public QObject
{
    Q_OBJECT
public:
    enum State {
        StateInitial,
        StateFeaturesFileExtracted,
        StateVADComplete,
        StateTranscription
    };

    explicit SphinxAutoTranscriber(QObject *parent = nullptr);
    virtual ~SphinxAutoTranscriber();

    SphinxConfiguration config() const;
    bool initialize(const SphinxConfiguration &config);

    State currentState() const;
    QString filePathRecording() const;
    void setFilePathRecording(const QString &filePath);
    QString basePath() const;
    void setBasePath(const QString &basePath);

    bool stepExtractFeaturesFile();
    bool stepVoiceActivityDetection();
    bool stepAutoTranscribe();

signals:
    void madeProgress(int progress);
    void printMessage(QString message);

public slots:

private:
    SphinxAutoTranscriberData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXAUTOTRANSCRIBER_H
