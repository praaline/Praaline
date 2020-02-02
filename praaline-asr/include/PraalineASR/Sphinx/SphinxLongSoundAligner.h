#ifndef SPHINXLONGSOUNDALIGNER_H
#define SPHINXLONGSOUNDALIGNER_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

#include "PraalineCore/Diff/DiffIntervals.h"
#include "PraalineASR/PraalineASR_Global.h"
#include "PraalineASR/Sphinx/SphinxConfiguration.h"

namespace Praaline {

namespace Core {
class Interval;
class Corpus;
class CorpusCommunication;
}

namespace ASR {

struct SphinxLongSoundAlignerData;

class PRAALINE_ASR_SHARED_EXPORT SphinxLongSoundAligner : public QObject
{
    Q_OBJECT
public:
    enum State {
        StateInitial,
        StateFeaturesFileExtracted,
        StateVADComplete,
        StateFirstPass,
        StateSecondPass
    };

    explicit SphinxLongSoundAligner(QObject *parent = nullptr);
    virtual ~SphinxLongSoundAligner();

    SphinxConfiguration config() const;
    bool initialize(const SphinxConfiguration &config);

    State currentState() const;
    QString tableTokensRecognised() const;
    QList<Praaline::Core::Interval *> tokensTranscription() const;
    QList<Praaline::Core::Interval *> tokensRecognised() const;
    dtl::Ses<Praaline::Core::Interval *>::sesElemVec diffSequence() const;

signals:
    void madeProgress(int progress);
    void printMessage(QString message);

public slots:

private:
    SphinxLongSoundAlignerData *d;

    bool stepExtractFeaturesFile();
    bool stepVoiceActivityDetection();
    bool stepCreateContrainedLanguageModel();
    bool stepFirstPassRecogniser();
    bool stepDiffTranscriptionWithRecogniser();
    bool stepFindAnchors();
    bool stepAlignTranscription();
};

} // namespace ASR
} // namespace Praaline

#endif // SPHINXLONGSOUNDALIGNER_H
