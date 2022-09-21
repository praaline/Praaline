#include <QDebug>
#include <QMap>
#include <QSharedPointer>
// Asynchronous execution
#include <QApplication>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include <QThreadPool>

#include "PraalineCore/Corpus/Corpus.h"
using namespace Praaline::Core;

#include "AnalyserTemporal.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

struct AnalyserTemporalData {
    AnalyserTemporalData() :
        corpus(nullptr)
    {
        levelSyllables = "syll";
        levelTokens = "tok_min";
    }
    QPointer<Corpus> corpus;
    QString levelSyllables;
    QString levelTokens;
    QMap<QString, QSharedPointer<AnalyserTemporalItem> > items;
    // Asynchronous execution
    QFuture<QString> future;
    QFutureWatcher<QString> watcher;
};

AnalyserTemporal::AnalyserTemporal(QObject *parent) :
    QObject(parent), d(new AnalyserTemporalData)
{
    connect(&(d->watcher), &QFutureWatcherBase::resultReadyAt, this, &AnalyserTemporal::futureResultReadyAt);
    connect(&(d->watcher), &QFutureWatcherBase::progressValueChanged, this, &AnalyserTemporal::futureProgressValueChanged);
    connect(&(d->watcher), &QFutureWatcherBase::finished, this, &AnalyserTemporal::futureFinished);
}

AnalyserTemporal::~AnalyserTemporal()
{
    delete d;
}

// ====================================================================================================================
// Properties
// ====================================================================================================================

Corpus *AnalyserTemporal::corpus() const
{
    return d->corpus;
}

void AnalyserTemporal::setCorpus(Corpus *corpus)
{
    d->items.clear();
    d->corpus = corpus;
}

QString AnalyserTemporal::levelIDSyllables() const
{
    return d->levelSyllables;
}

void AnalyserTemporal::setLevelIDSyllables(const QString &levelID)
{
    d->levelSyllables = levelID;
}

QString AnalyserTemporal::levelIDTokens() const
{
    return d->levelTokens;
}

void AnalyserTemporal::setLevelIDTokens(const QString &levelID)
{
    d->levelTokens = levelID;
}


// ====================================================================================================================
// Asynchronous execution: basic event handling
// ====================================================================================================================

void AnalyserTemporal::futureResultReadyAt(int index)
{
    QString result = d->watcher.resultAt(index);
    qDebug() << d->watcher.progressValue() << result;
    emit printMessage(result);
    if (d->corpus->communicationsCount() > 0)
        emit madeProgress(d->items.count() * 100 / d->corpus->communicationsCount());
    else
        emit madeProgress(100);
}

void AnalyserTemporal::futureProgressValueChanged(int progressValue)
{
    qDebug() << progressValue;
    if (d->corpus->communicationsCount() > 0)
        emit madeProgress(d->items.count() * 100 / d->corpus->communicationsCount());
    else
        emit madeProgress(100);
}

void AnalyserTemporal::futureFinished()
{
    emit madeProgress(100);
    emit printMessage("Finished");
    qDebug() << "Finished";
}

// ====================================================================================================================
// Asynchronous execution step
// ====================================================================================================================

struct RunAnalysisStep
{
    QString levelSyllables;
    QString levelTokens;
    QMap<QString, QSharedPointer<AnalyserTemporalItem> > &items;

    RunAnalysisStep(const QString &levelSyllables, const QString &levelTokens,
                    QMap<QString, QSharedPointer<AnalyserTemporalItem> > &items) :
        levelSyllables(levelSyllables), levelTokens(levelTokens), items(items)
    {}
    typedef QString result_type;

    QString operator() (CorpusCommunication *com)
    {
        if (!com) return QString("Empty Communication.");
        QSharedPointer<AnalyserTemporalItem> item(new AnalyserTemporalItem());
        item->setLevelIDSyllables(levelSyllables);
        item->setLevelIDTokens(levelTokens);
        item->analyse(com);
        items.insert(com->ID(), item);
        return com->ID();
    }
};



void AnalyserTemporal::analyse()
{
    if (!d->corpus) return;
    emit madeProgress(0);
    emit printMessage("Starting");
    QElapsedTimer timer;

    QThreadPool::globalInstance()->setMaxThreadCount(64);
    timer.start();

    d->future = QtConcurrent::mapped(d->corpus->communications(),
                                     RunAnalysisStep(d->levelSyllables, d->levelTokens, d->items));
    d->watcher.setFuture(d->future);
    while (d->watcher.isRunning()) QApplication::processEvents();

    emit printMessage(QString("Time: %1 seconds").arg(static_cast<double>(timer.elapsed()) / 1000.0));
    emit madeProgress(100);

//    int i = 0;
//    foreach (CorpusCommunication *com, d->corpus->communications()) {
//        i++;
//        emit madeProgress(i);
//    }
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
    foreach (CorpusCommunication *com, d->corpus->communications()) {
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
    foreach (CorpusCommunication *com, d->corpus->communications()) {
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
            // Find measure. If it is a vector, add the entire vector to the aggregated list; otherwise just the value.
            if (AnalyserTemporalItem::measureDefinition(measureID).isVector()) {
                aggregates[id].append(theItem->vectorMeasureSpk(speakerID, measureID));
            } else {
                aggregates[id].append(theItem->measureSpk(speakerID, measureID));
            }
        }
    }
    return aggregates;
}

QStringList AnalyserTemporal::measureIDsForCommunication()
{
    return AnalyserTemporalItem::measureIDsForCommunication();
}

QStringList AnalyserTemporal::measureIDsForSpeaker()
{
    return AnalyserTemporalItem::measureIDsForSpeaker();
}

QStringList AnalyserTemporal::vectorMeasureIDsForCommunication()
{
    return AnalyserTemporalItem::vectorMeasureIDsForCommunication();
}

QStringList AnalyserTemporal::vectorMeasureIDsForSpeaker()
{
    return AnalyserTemporalItem::vectorMeasureIDsForSpeaker();
}

Praaline::Core::StatisticalMeasureDefinition AnalyserTemporal::measureDefinition(const QString &measureID)
{
    return AnalyserTemporalItem::measureDefinition(measureID);
}

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
