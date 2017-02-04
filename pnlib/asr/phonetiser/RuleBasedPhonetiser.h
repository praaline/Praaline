#ifndef RULEBASEDPHONETISER_H
#define RULEBASEDPHONETISER_H

#include <QObject>
#include "Phonetiser.h"

namespace Praaline {
namespace ASR {

struct RuleBasedPhonetiserData;

class RuleBasedPhonetiser : public Phonetiser
{
    Q_OBJECT
public:
    explicit RuleBasedPhonetiser(QObject *parent = 0);
    ~RuleBasedPhonetiser();

    bool readRuleFile(const QString &filename);
    QString phonetise(const QString &input);

    QString phonetiseWord(const QString &word) override {}
    QList<Core::Interval *> phonetiseUtterance(Core::Interval *utterance) override {}

signals:

public slots:

private:
    RuleBasedPhonetiserData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // RULEBASEDPHONETISER_H
