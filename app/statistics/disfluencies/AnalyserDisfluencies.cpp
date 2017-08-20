#include <QDebug>
#include <QMap>
#include <QSharedPointer>
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "AnalyserDisfluencies.h"
#include "AnalyserDisfluenciesItem.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginDisfluencies {

struct AnalyserDisfluenciesData {
    AnalyserDisfluenciesData() :
        corpus(0)
    {
        levelSyllables = "syll";
        levelTokens = "tok_min";
        levelDisfluencySequences = "disfluencies";
    }
    QPointer<Corpus> corpus;
    QString levelSyllables;
    QString levelTokens;
    QString levelDisfluencySequences;
    QMap<QString, QSharedPointer<AnalyserDisfluenciesItem> > items;
};

AnalyserDisfluencies::AnalyserDisfluencies(QObject *parent) :
    QObject(parent), d(new AnalyserDisfluenciesData)
{
}

AnalyserDisfluencies::~AnalyserDisfluencies()
{
    delete d;
}

Corpus *AnalyserDisfluencies::corpus() const
{
    return d->corpus;
}

void AnalyserDisfluencies::setCorpus(Corpus *corpus)
{
    d->items.clear();
    d->corpus = corpus;
}

QString AnalyserDisfluencies::levelSyllables() const
{
    return d->levelSyllables;
}

void AnalyserDisfluencies::setLevelSyllables(const QString &levelID)
{
    d->levelSyllables = levelID;
}

QString AnalyserDisfluencies::levelTokens() const
{
    return d->levelTokens;
}

void AnalyserDisfluencies::setLevelTokens(const QString &levelID)
{
    d->levelTokens = levelID;
}

QString AnalyserDisfluencies::levelDisfluencySequences() const
{
    return d->levelDisfluencySequences;
}

void AnalyserDisfluencies::setLevelDisfluencySequences(const QString &levelID)
{
    d->levelDisfluencySequences = levelID;
}

void AnalyserDisfluencies::analyse()
{
    if (!d->corpus) return;
    int i = 0;
    foreach (QPointer<CorpusCommunication> com, d->corpus->communications()) {
        QSharedPointer<AnalyserDisfluenciesItem> item(new AnalyserDisfluenciesItem());
        item->setLevelSyllables(d->levelSyllables);
        item->setLevelTokens(d->levelTokens);
        item->setLevelDisfluencySequences(d->levelDisfluencySequences);
        item->analyse(com);
        d->items.insert(com->ID(), item);
        i++;
        emit madeProgress(i);
    }
}

AnalyserDisfluenciesItem *AnalyserDisfluencies::item(const QString communicationID)
{
    return d->items.value(communicationID).data();
}

QMap<QString, QList<double> > AnalyserDisfluencies::aggregateMeasureCom(
        const QString &measureID, const QStringList &groupAttributeIDsCom)
{
    QMap<QString, QList<double> > aggregates;
    if (!d->corpus) return aggregates;
    foreach (QPointer<CorpusCommunication> com, d->corpus->communications()) {
        if (!com) continue;
        AnalyserDisfluenciesItem *theItem = item(com->ID());
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
        if (AnalyserDisfluenciesDefinitions::measureDefinition(measureID).isVector()) {
            aggregates[id].append(theItem->vectorMeasureCom(measureID));
        } else {
            aggregates[id].append(theItem->measureCom(measureID));
        }
    }
    return aggregates;
}

QMap<QString, QList<double> > AnalyserDisfluencies::aggregateMeasureSpk(
        const QString &measureID, const QStringList &groupAttributeIDsCom, const QStringList &groupAttributeIDsSpk)
{
    QMap<QString, QList<double> > aggregates;
    if (!d->corpus) return aggregates;
    foreach (QPointer<CorpusCommunication> com, d->corpus->communications()) {
        if (!com) continue;
        AnalyserDisfluenciesItem *theItem = item(com->ID());
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
            // Find measure. If it is a vector, add the entire vector to the aggregated list; otherwise just the value.
            if (AnalyserDisfluenciesDefinitions::measureDefinition(measureID).isVector()) {
                aggregates[id].append(theItem->vectorMeasureSpk(speakerID, measureID));
            } else {
                aggregates[id].append(theItem->measureSpk(speakerID, measureID));
            }
        }
    }
    return aggregates;
}


} // namespace StatisticsPluginDisfluencies
} // namespace Plugins
} // namespace Praaline
