#ifndef ANALYSERDISFLUENCIES_H
#define ANALYSERDISFLUENCIES_H

#include <QObject>
#include <QString>
#include <QMap>
#include "AnalyserDisfluenciesItem.h"

namespace Praaline {

namespace Core {
class Corpus;
}

namespace Plugins {
namespace StatisticsPluginDisfluencies {

struct AnalyserDisfluenciesData;

class AnalyserDisfluencies : public QObject
{
    Q_OBJECT
public:
    explicit AnalyserDisfluencies(QObject *parent = 0);
    ~AnalyserDisfluencies();

    Praaline::Core::Corpus *corpus() const;
    void setCorpus(Praaline::Core::Corpus *corpus);

    QString levelSyllables() const;
    void setLevelSyllables(const QString &levelID);
    QString levelTokens() const;
    void setLevelTokens(const QString &levelID);
    QString levelDisfluencySequences() const;
    void setLevelDisfluencySequences(const QString &levelID);

    void analyse();
    AnalyserDisfluenciesItem *item(const QString communicationID);
    QMap<QString, QList<double> > aggregateMeasureCom(
            const QString &measureID, const QStringList &groupAttributeIDsCom);
    QMap<QString, QList<double> > aggregateMeasureSpk(
            const QString &measureID, const QStringList &groupAttributeIDsCom, const QStringList &groupAttributeIDsSpk);
signals:
    void madeProgress(int);

public slots:

private:
    AnalyserDisfluenciesData *d;
};

} // namespace StatisticsPluginDisfluencies
} // namespace Plugins
} // namespace Praaline

#endif // ANALYSERDISFLUENCIES_H
