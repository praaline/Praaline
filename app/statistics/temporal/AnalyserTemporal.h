#ifndef ANALYSERTEMPORAL_H
#define ANALYSERTEMPORAL_H

#include <QObject>
#include <QString>
#include <QMap>
#include "AnalyserTemporalItem.h"

namespace Praaline {

namespace Core {
class Corpus;
}

namespace Plugins {
namespace StatisticsPluginTemporal {

struct AnalyserTemporalData;

class AnalyserTemporal : public QObject
{
    Q_OBJECT
public:
    explicit AnalyserTemporal(QObject *parent = 0);
    ~AnalyserTemporal();

    Praaline::Core::Corpus *corpus() const;
    void setCorpus(Praaline::Core::Corpus *corpus);

    void analyse();
    AnalyserTemporalItem *item(const QString communicationID);
    QMap<QString, QList<double> > aggregateMeasureCom(
            const QString &measureID, const QStringList &groupAttributeIDsCom);
    QMap<QString, QList<double> > aggregateMeasureSpk(
            const QString &measureID, const QStringList &groupAttributeIDsCom, const QStringList &groupAttributeIDsSpk);
signals:
    void madeProgress(int);

public slots:

private:
    AnalyserTemporalData *d;
};

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline

#endif // ANALYSERTEMPORAL_H
