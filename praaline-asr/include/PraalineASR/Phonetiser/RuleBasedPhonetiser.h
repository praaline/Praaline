#ifndef PHONETISERRULEBASED_H
#define PHONETISERRULEBASED_H

#include <QObject>
#include <QString>

#include "PraalineASR/PraalineASR_Global.h"
#include "PraalineASR/Phonetiser.h"

namespace Praaline {
namespace ASR {

struct RuleBasedPhonetiserData;

class PRAALINE_ASR_SHARED_EXPORT RuleBasedPhonetiser : public Phonetiser
{
    Q_OBJECT
public:
    explicit RuleBasedPhonetiser(QObject *parent = nullptr);
    ~RuleBasedPhonetiser() override;

    QString phonetiseWord(const QString &word) override;
    QList<Core::Interval *> phonetiseUtterance(Core::Interval *utterance) override;

    bool readRuleFile(const QString &filename);
    QString phonetise(const QString &input);

signals:

public slots:

private:
    RuleBasedPhonetiserData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // EXTERNALPHONETISER_H
