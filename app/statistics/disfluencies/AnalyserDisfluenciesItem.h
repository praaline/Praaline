#ifndef ANALYSERDISFLUENCIESITEM_H
#define ANALYSERDISFLUENCIESITEM_H

#include <QObject>
#include <QPointer>
#include "pncore/statistics/StatisticalMeasureDefinition.h"
#include "statistics/AnalyserItemBase.h"

namespace Praaline {
namespace Core {
class CorpusCommunication;
class IntervalTier;
}
}

struct AnalyserDisfluenciesItemData;

class AnalyserDisfluenciesDefinitions : public AnalyserItemDefinitionsBase<AnalyserDisfluenciesDefinitions>
{
public:
    static QStringList measureIDsForCommunication();
    static QStringList measureIDsForSpeaker();
    static QStringList vectorMeasureIDsForCommunication();
    static QStringList vectorMeasureIDsForSpeaker();
    static Praaline::Core::StatisticalMeasureDefinition measureDefinition(const QString &measureID);
};

class AnalyserDisfluenciesItem : public AnalyserItemBase
{
    Q_OBJECT
public:
    explicit AnalyserDisfluenciesItem(QObject *parent = nullptr);
    virtual ~AnalyserDisfluenciesItem();

    QString levelSyllables() const;
    void setLevelSyllables(const QString &levelID);
    QString levelTokens() const;
    void setLevelTokens(const QString &levelID);
    QString levelDisfluencySequences() const;
    void setLevelDisfluencySequences(const QString &levelID);

    QStringList speakerIDs() const override;
    double measureCom(const QString &measureID) const override;
    double measureSpk(const QString &speakerID, const QString &measureID) const override;
    QList<double> vectorMeasureCom(const QString &measureID) const override;
    QList<double> vectorMeasureSpk(const QString &speakerID, const QString &measureID) const override;

    void analyse(Praaline::Core::CorpusCommunication *com) override;

private:
    AnalyserDisfluenciesItemData *d;
};

#endif // ANALYSERDISFLUENCIESITEM_H

