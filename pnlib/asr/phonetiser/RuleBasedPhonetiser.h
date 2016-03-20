#ifndef RULEBASEDPHONETISER_H
#define RULEBASEDPHONETISER_H

#include <QObject>
#include "AbstractPhonetiser.h"

struct RuleBasedPhonetiserData;

class RuleBasedPhonetiser : public AbstractPhonetiser
{
    Q_OBJECT
public:
    explicit RuleBasedPhonetiser(QObject *parent = 0);
    ~RuleBasedPhonetiser();

    bool readRuleFile(const QString &filename);
    QString phonetise(const QString &input);

signals:

public slots:

private:
    RuleBasedPhonetiserData *d;
};

#endif // RULEBASEDPHONETISER_H
