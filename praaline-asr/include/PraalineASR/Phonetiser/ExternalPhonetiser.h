#ifndef EXTERNALPHONETISER_H
#define EXTERNALPHONETISER_H

#include <QObject>
#include <QString>
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineASR/PraalineASR_Global.h"
#include "PraalineASR/SpeechToken.h"
#include "PraalineASR/Phonetiser.h"

namespace Praaline {
namespace ASR {

struct ExternalPhonetiserData;

class PRAALINE_ASR_SHARED_EXPORT ExternalPhonetiser : public Phonetiser
{
    Q_OBJECT
public:
    explicit ExternalPhonetiser(QObject *parent = nullptr);
    ~ExternalPhonetiser() override;

    QString phonetiseWord(const QString &word) override {}
    QList<Core::Interval *> phonetiseUtterance(Core::Interval *utterance) override { return QList<Core::Interval *>(); }

    static QList<SpeechToken> phonetiseList(QList<Praaline::Core::Interval *> tokens, const QString &attributeID = QString());
    static QList<SpeechToken> phonetiseTier(Praaline::Core::IntervalTier *tier_tokens, const QString &attributeID = QString());
    static void addPhonetisationToTokens(Praaline::Core::IntervalTier *tier_tokens, const QString &attributeIDOrthographic,
                                         const QString &attributeIDPhonetisation);

signals:

public slots:

private:
    ExternalPhonetiserData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // EXTERNALPHONETISER_H
