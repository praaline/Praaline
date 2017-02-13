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

QMap<QString, QList<double> > AnalyserTemporal::aggregateMeasureCom(
        const QString &measureID, const QStringList &groupAttributeIDsCom)
{
    QMap<QString, QList<double> > aggregates;
    if (!d->corpus) return aggregates;
    foreach (QPointer<CorpusCommunication> com, d->corpus->communications()) {
        if (!com) continue;
        AnalyserTemporalItem *theItem = item(com->ID());
        if (!theItem) continue;
        // Calculate ID for these data
        QString id;
        for (int i = 0; i < groupAttributeIDsCom.count(); ++i) {
            id = id.append(com->property(groupAttributeIDsCom.at(i)).toString());
            if (i < groupAttributeIDsCom.count() - 1) id = id.append("::");
        }
        // Create list if it does not exist already;
        if (!aggregates.contains(id))
            aggregates.insert(id, QList<double>());
        // Find measure
        if (AnalyserTemporalItem::measureDefinition(measureID).isVector()) {
            aggregates[id].append(theItem->vectorMeasureCom(measureID));
        } else {
            aggregates[id].append(theItem->measureCom(measureID));
        }
    }
    return aggregates;
}

QMap<QString, QList<double> > AnalyserTemporal::aggregateMeasureSpk(
        const QString &measureID, const QStringList &groupAttributeIDsCom, const QStringList &groupAttributeIDsSpk)
{
    QMap<QString, QList<double> > aggregates;
    if (!d->corpus) return aggregates;
    foreach (QPointer<CorpusCommunication> com, d->corpus->communications()) {
        if (!com) continue;
        AnalyserTemporalItem *theItem = item(com->ID());
        if (!theItem) continue;
        foreach (QString speakerID, theItem->speakerIDs()) {
            QPointer<CorpusSpeaker> spk = d->corpus->speaker(speakerID);
            // Calculate ID for these data
            QString id;
            for (int i = 0; i < groupAttributeIDsCom.count(); ++i) {
                id = id.append(com->property(groupAttributeIDsCom.at(i)).toString());
                if (i < groupAttributeIDsCom.count() - 1) id = id.append("::");
            }
            for (int i = 0; i < groupAttributeIDsSpk.count(); ++i) {
                id = id.append(spk->property(groupAttributeIDsSpk.at(i)).toString());
                if (i < groupAttributeIDsSpk.count() - 1) id = id.append("::");
            }
            // Create list if it does not exist already;
            if (!aggregates.contains(id))
                aggregates.insert(id, QList<double>());
            // Find measure
            if (AnalyserTemporalItem::measureDefinition(measureID).isVector()) {
                aggregates[id].append(theItem->vectorMeasureSpk(speakerID, measureID));
            } else {
                aggregates[id].append(theItem->measureSpk(speakerID, measureID));
            }
        }
    }
    return aggregates;
}


} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
