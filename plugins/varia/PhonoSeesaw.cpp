#include <QDebug>
#include <QSharedPointer>
#include <QString>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QTime>

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
#include "pncore/interfaces/praat/PraatPitchFile.h"
using namespace Praaline::Core;

#include "math.h"

#include "PhonoSeesaw.h"

struct PhonoSeesawData {
    PhonoSeesawData() :
        distributionCountsTotal(0)
    {}

    QString levelTokens;
    QString levelPhones;
    QString attributePhonetisation;
    QStringList filledPauseTokens;
    QStringList phonemeSet;

    int distributionCountsTotal;
    QMap<QString, int> distributionCounts;
    QMap<QString, double> distributionFreqs;

    QHash<QString, QStringList> availableUnits;
    int numberOfUnitsToSelect;

};

PhonoSeesaw::PhonoSeesaw() :
    d(new PhonoSeesawData)
{
    d->levelPhones = "phone";
    d->levelTokens = "tok_min";
    d->attributePhonetisation = "phonetisation";
    d->filledPauseTokens << "euh" << "euhm" << "mh" << "mhm";
    d->phonemeSet << "9~" << "a~" << "e~" << "o~"
                  << "2" << "9" << "A" << "@" << "E" << "H" << "O" << "R" << "S" << "Z"
                  << "a" << "b" << "d" << "e" << "f" << "g" << "i" << "j" << "k" << "l"
                  << "m" << "n" << "o" << "p" << "s" << "t" << "u" << "v" << "w" << "y" << "z";
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
}

PhonoSeesaw::~PhonoSeesaw()
{
    delete d;
}

void PhonoSeesaw::resetDistributionCounts()
{
    d->distributionCounts.clear();
    d->distributionCountsTotal = 0;
    d->distributionFreqs.clear();
}

QString PhonoSeesaw::accumulateRefDistribution(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "No Communication";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->
                getTiersAllSpeakers(annot->ID(), QStringList() << d->levelPhones << d->levelTokens);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName(d->levelTokens);
            if (!tier_tok_min) continue;
            IntervalTier *tier_phones = tiers->getIntervalTierByName(d->levelPhones);
            if (!tier_phones) continue;

            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                // Skip pauses, filled pauses and false starts
                if (tok_min->isPauseSilent()) continue;
                if (d->filledPauseTokens.contains(tok_min->text())) continue;
                if (tok_min->text().endsWith("/")) continue;
                QList<Interval *> phones = tier_phones->getIntervalsContainedIn(tok_min);
                foreach (Interval *phone, phones) {
                    QString p = phone->text();
                    if (d->phonemeSet.contains(p)) {
                        d->distributionCounts[p] = d->distributionCounts.value(p, 0) + 1;
                        d->distributionCountsTotal++;
                    }
                }
            }
        }
        qDeleteAll(tiersAll);
    }
    ret = QString("PhonoSeesaw: Accumulated for distribution: %1").arg(com->ID());
    return ret;
}

QString PhonoSeesaw::calculateRefDistribution()
{
    QString ret;
    foreach (QString phoneme, d->phonemeSet) {
        int count = d->distributionCounts.value(phoneme, 0);
        double freq = ((double)count) / ((double) d->distributionCountsTotal);
        d->distributionFreqs[phoneme] = freq;
        ret.append(phoneme).append("\t").append(QString::number(freq)).append("\n");
    }
    return ret;
}

// ====================================================================================================================

QString PhonoSeesaw::addUnitsToListOfAvailable(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "No Communication";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->
                getTiersAllSpeakers(annot->ID(), QStringList() << d->levelPhones << d->levelTokens);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName(d->levelTokens);
            if (!tier_tok_min) continue;
            IntervalTier *tier_phones = tiers->getIntervalTierByName(d->levelPhones);
            if (!tier_phones) continue;

            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                // Skip pauses, filled pauses and false starts
                if (tok_min->isPauseSilent()) continue;
                if (d->filledPauseTokens.contains(tok_min->text())) continue;
                if (tok_min->text().endsWith("/")) continue;
                if (d->availableUnits.contains(tok_min->text())) continue;
                // otherwise, add it
                QList<Interval *> phone_intervals = tier_phones->getIntervalsContainedIn(tok_min);
                QStringList phones;
                foreach (Interval *phone, phone_intervals) {
                    QString p = phone->text();
                    if (d->phonemeSet.contains(p)) phones << p;
                }
                d->availableUnits.insert(tok_min->text(), phones);
            }
        }
        qDeleteAll(tiersAll);
    }
    ret = QString("PhonoSeesaw: Processed %1").arg(com->ID());
    return ret;
}

void PhonoSeesaw::calculateDistributionFromPhonemeList(QStringList phonemes, QMap<QString, int> &distributionCounts,
                                                       QMap<QString, double> &distributionFreqs)
{
    int distributionCountsTotal(0);
    distributionCounts.clear();
    distributionFreqs.clear();

    foreach (QString phoneme, phonemes) {
        distributionCounts[phoneme] = distributionCounts[phoneme] + 1;
        distributionCountsTotal++;
    }
    foreach (QString phoneme, d->phonemeSet) {
        int count = distributionCounts.value(phoneme, 0);
        double freq = ((double)count) / ((double) distributionCountsTotal);
        distributionFreqs[phoneme] = freq;
    }
}

int PhonoSeesaw::randInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

QList<int> PhonoSeesaw::selection(int numberOfUnits)
{
    QList<int> sel;
    if (d->availableUnits.count() < numberOfUnits) return sel;
    while (sel.count() < numberOfUnits) {
        int i = randInt(0, d->availableUnits.count() - 1);
        if (!sel.contains(i)) sel << i;
    }
    return sel;
}

QString PhonoSeesaw::checkSelection(QList<int> selection, int &countRejects)
{
    QString ret;
    QStringList selectionPhonemes;
    QMap<QString, int> distributionCounts;
    QMap<QString, double> distributionFreqs;
    ret.append("Selection:\n");
    foreach (int isel, selection) {
        QString unit = d->availableUnits.keys().at(isel);
        QStringList unitPhonemes = d->availableUnits.value(unit);
        selectionPhonemes << unitPhonemes;
        ret.append(unit).append("\t").append(unitPhonemes.join(" ")).append("\n");
    }
    calculateDistributionFromPhonemeList(selectionPhonemes, distributionCounts, distributionFreqs);
    double n1 = d->distributionCountsTotal;
    double n2 = selectionPhonemes.count();

    ret.append("\nPhoneme\tRef Freq\tSel Freq\tz value\tTest\n");
    countRejects = 0;
    foreach (QString phoneme, d->phonemeSet) {
        double p1 = d->distributionFreqs.value(phoneme);
        double p2 = distributionFreqs.value(phoneme);
        double x1 = d->distributionCounts.value(phoneme);
        double x2 = distributionCounts.value(phoneme);
        double p = (x1 + x2) / (n1 + n2);
        double z = (p1 - p2) / (sqrt(p * (1 - p)) * sqrt((1/n1) + (1/n2)));
        bool reject(false);
        if (fabs(z) > 1.96) reject = true;
        QString message = "ok"; if (reject) message = "REJECT";
        ret.append(phoneme).append("\t");
        ret.append(QString::number(d->distributionFreqs.value(phoneme))).append("\t");
        ret.append(QString::number(distributionFreqs.value(phoneme))).append("\t");
        ret.append(QString::number(z)).append("\t");
        ret.append(message).append("\n");
        if (reject) countRejects++;
    }
    return ret;
}

QString PhonoSeesaw::makeBestSelection(int numberOfUnits, int numberOfAttempts)
{
    QString ret;
    QMap<int, QString> results;
    for (int attempt = 0; attempt < numberOfAttempts; ++attempt) {
        QList<int> sel = selection(numberOfUnits);
        int countRejects;
        QString result = checkSelection(sel, countRejects);
        results.insert(countRejects, result);
    }
    ret.append(QString("\nNumber of units: %1 Number of attempts: %2\nBest Result:\n").arg(numberOfUnits).arg(numberOfAttempts));
    if (results.isEmpty()) return ret.append("No results");
    return ret.append(results.first());
}


