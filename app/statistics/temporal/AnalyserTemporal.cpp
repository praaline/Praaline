#include <QDebug>
#include <QMap>
#include <QSharedPointer>
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "AnalyserTemporal.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

struct AnalyserTemporalData {
    AnalyserTemporalData() :
        corpus(0)
    {}

    QPointer<Corpus> corpus;
    QMap<QString, QSharedPointer<AnalyserTemporalItem> > items;
};

AnalyserTemporal::AnalyserTemporal(QObject *parent) :
    QObject(parent), d(new AnalyserTemporalData)
{
}

AnalyserTemporal::~AnalyserTemporal()
{
    delete d;
}

Corpus *AnalyserTemporal::corpus() const
{
    return d->corpus;
}

void AnalyserTemporal::setCorpus(Corpus *corpus)
{
    d->items.clear();
    d->corpus = corpus;
}

void AnalyserTemporal::analyse()
{
    if (!d->corpus) return;
    int i = 0;
    foreach (QPointer<CorpusCommunication> com, d->corpus->communications()) {
        QSharedPointer<AnalyserTemporalItem> item(new AnalyserTemporalItem());
        item->analyse(com);
        d->items.insert(com->ID(), item);
        i++;
        emit madeProgress(i);
    }
}

AnalyserTemporalItem *AnalyserTemporal::item(const QString communicationID)
{
    return d->items.value(communicationID).data();
}

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
