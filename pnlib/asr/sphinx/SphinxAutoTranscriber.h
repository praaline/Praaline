#ifndef SPHINXAUTOTRANSCRIBER_H
#define SPHINXAUTOTRANSCRIBER_H

#include <QObject>
#include "pncore/base/RealTime.h"
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

    State currentState() const;
    QString filePathRecording() const;
    void setFilePathRecording(const QString &);
    QString workingDirectory() const;
    void setWorkingDirectory(const QString &);
    bool keepDownsampledFile() const;
    void setKeepDownsampledFile(bool);
    SphinxConfiguration configuration() const;
    bool setConfiguration(const SphinxConfiguration &);
    QList<QPair<double, double> > &vadResults();
    QList<Praaline::Core::Interval *> utterances();
    QList<Praaline::Core::Interval *> tokens();

    bool stepExtractFeaturesFile();
    bool stepVoiceActivityDetection();
    bool stepAutoTranscribe();

signals:
    void madeProgress(int progress);
    void printMessage(QString message);

public slots:
    void recogniserError(QString message);
    void recogniserInfo(QString message);

private:
    SphinxAutoTranscriberData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXAUTOTRANSCRIBER_H
