#include <QDebug>
#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QPointer>
#include <QSharedPointer>


#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "SpeechToken.h"
#include "ForcedAligner.h"

namespace Praaline {
namespace ASR {

struct ForcedAlignerData {
    ForcedAlignerData() :
        usePronunciationVariants(true), returnDummyAlignmentOnFailure(true)
    {}

    QStringList phonemeset;
    QRegExp regexMatchPhoneme;
    QString tokenPhonetisationAttributeID;
    QString phonetisationSeparatorForVariants;
    QString phonetisationSeparatorForPhonemes;
    bool usePronunciationVariants;
    bool returnDummyAlignmentOnFailure;
};

ForcedAligner::ForcedAligner(QObject *parent) :
    QObject(parent), d(new ForcedAlignerData())
{
    // Default values
    d->tokenPhonetisationAttributeID = "phonetisation";
    d->phonetisationSeparatorForVariants = "|";
    d->phonetisationSeparatorForPhonemes = " ";
    d->usePronunciationVariants = true;
    d->returnDummyAlignmentOnFailure = true;
    // Phoneme set. The order is important. Start with the longest phonemes.
    QStringList phonemeset;
    phonemeset << "9~" << "a~" << "e~" << "o~"
               << "2" << "9" << "A" << "@" << "E" << "H" << "O" << "R" << "S" << "Z"
               << "a" << "b" << "d" << "e" << "f" << "g" << "i" << "j" << "k" << "l"
               << "m" << "n" << "o" << "p" << "s" << "t" << "u" << "v" << "w" << "y" << "z";
    setPhonemeset(phonemeset);
}

ForcedAligner::~ForcedAligner()
{
    delete d;
}

QStringList ForcedAligner::phonemeset() const
{
    return d->phonemeset;
}

void ForcedAligner::setPhonemeset(const QStringList &phonemes)
{
    d->phonemeset = phonemes;
    QString regex;
    foreach (QString phoneme, d->phonemeset)
        regex = regex.append(QString("%1\\*|%1|").arg(phoneme));
    if (!regex.isEmpty()) regex.chop(1);
    d->regexMatchPhoneme = QRegExp(regex);
}

bool ForcedAligner::usePronunciationVariants() const
{
    return d->usePronunciationVariants;
}

void ForcedAligner::setUsePronunciationVariants(bool use)
{
    d->usePronunciationVariants = use;
}

QString ForcedAligner::tokenPhonetisationAttributeID() const
{
    return d->tokenPhonetisationAttributeID;
}

void ForcedAligner::setTokenPhonetisationAttributeID(const QString &attributeID)
{
    d->tokenPhonetisationAttributeID = attributeID;
}

QString ForcedAligner::phonetisationSeparatorForVariants() const
{
    return d->phonetisationSeparatorForVariants;
}

void ForcedAligner::setPhonetisationSeparatorForVariants(const QString &sep)
{
    d->phonetisationSeparatorForVariants = sep;
}

QString ForcedAligner::phonetisationSeparatorForPhonemes() const
{
    return d->phonetisationSeparatorForPhonemes;
}

void ForcedAligner::setPhonetisationSeparatorForPhonemes(const QString &sep)
{
    d->phonetisationSeparatorForPhonemes = sep;
}


QList<SpeechToken> ForcedAligner::alignerTokensFromIntervalTier(bool insertLeadingAndTrailingPauses,
                                                                IntervalTier *tierTokens, int indexFrom, int indexTo)
{
    QList<SpeechToken> alignerTokens;
    if (!tierTokens) return alignerTokens;
    if (tierTokens->count() == 0) return alignerTokens;
    if (indexTo < 0) indexTo = tierTokens->count() - 1;
    if (indexTo >= tierTokens->count()) indexTo = tierTokens->count() - 1;
    if (indexFrom < 0) indexFrom = 0;
    if (indexTo >= tierTokens->count()) indexTo = tierTokens->count() - 1;
    if (indexTo < indexFrom) return alignerTokens;

    QSharedPointer<IntervalTier> tierTokensCopy(tierTokens->clone());

    // On the working copy: Insert silent pauses at the beginning and end, if authorised to do so.
    if (insertLeadingAndTrailingPauses) {
        if (!tierTokensCopy->at(indexFrom)->isPauseSilent()) {
            // Insert pause at the beginning of the utterance:
            // [xxx] becomes [ _ ][xxx]
            tierTokensCopy->split(indexFrom, tierTokensCopy->at(indexFrom)->tCenter(), true);
            tierTokensCopy->at(indexFrom)->setText("_");
            indexTo++;
        }
        if (!tierTokensCopy->at(indexTo)->isPauseSilent()) {
            // Insert pause at the end of the utterance
            // [xxx] becomes [xxx][ _ ]
            tierTokensCopy->split(indexTo, tierTokensCopy->at(indexTo)->tCenter(), false);
            indexTo++;
            tierTokensCopy->at(indexTo)->setText("_");
        }
    }
    // On the working copy: Remove pauses from within the utterance
    for (int i = indexTo - 1; i >= indexFrom + 1; --i) {
        if (tierTokensCopy->at(i)->isPauseSilent()) {
            tierTokensCopy->removeInterval(i);
            indexTo--;
        }
    }
    // Create tokens for alignment
    for (int index = indexFrom; index <= indexTo; ++index) {
        Interval *token = tierTokensCopy->at(index);
        SpeechToken atoken(index, index, token->text());
        QString phonetisationAttributeText = token->attribute(d->tokenPhonetisationAttributeID).toString();
        if (!d->usePronunciationVariants) {
            phonetisationAttributeText = phonetisationAttributeText.replace("*", "");
        }
        phonetisationAttributeText = phonetisationAttributeText.trimmed();
        // Split the given phonetisation text into proposed alternates (e.g. abc | def)
        QStringList phonetisationsProposed = phonetisationAttributeText.split(d->phonetisationSeparatorForVariants, QString::SkipEmptyParts);
        // Create list of alternate phonetisations
        foreach (QString phonetisationProposed, phonetisationsProposed) {
            QStringList phonemesSeparated;
            // Separate phonemes
            if (d->phonetisationSeparatorForPhonemes.isEmpty()) {
                int pos = 0;
                while ((pos = d->regexMatchPhoneme.indexIn(phonetisationProposed, pos)) != -1) {
                    phonemesSeparated << d->regexMatchPhoneme.cap(0);
                    pos += d->regexMatchPhoneme.matchedLength();
                }
            }
            else {
                phonemesSeparated = phonetisationProposed.split(d->phonetisationSeparatorForPhonemes, QString::SkipEmptyParts);
            }
            // Process alternative phonetisations
            QList<QStringList> phonetisations;
            phonetisations << QStringList();
            foreach (QString phoneme, phonemesSeparated) {
                if (phoneme.endsWith("*")) {
                    QList<QStringList> additionalPhonetisations;
                    for (int i = 0; i < phonetisations.count(); ++i) {
                        additionalPhonetisations << phonetisations[i];      // without
                        phonetisations[i].append(phoneme.replace("*", "")); // with
                    }
                    phonetisations << additionalPhonetisations;
                }
                else {
                    for (int i = 0; i < phonetisations.count(); ++i) {
                        phonetisations[i].append(phoneme);
                    }
                }
            }
            foreach (QStringList phonetisation, phonetisations) {
                atoken.phonetisations.append(phonetisation.join(" "));
                // qDebug() << atoken.orthographic << phonetisation.join(" ");
            }
        }
        alignerTokens << atoken;
    }
    return alignerTokens;
}

// public
bool ForcedAligner::alignAllTokens(const QString &waveFilepath, Praaline::Core::IntervalTier *tierTokens,
                                      QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput)
{
    if (!tierTokens) return false;
    if (tierTokens->count() == 0) return false;
    int indexFrom = 0;
    int indexTo = tierTokens->count() - 1;
    return alignTokens(waveFilepath, RealTime(-1, 0), RealTime(-1, 0),
                       tierTokens, indexFrom, indexTo, true,
                       outPhonesList, outAlignerOutput);
}

// public
bool ForcedAligner::alignUtterance(const QString &waveFilepath,
                                   IntervalTier *tierUtterances, int &indexUtteranceToAlign,
                                   IntervalTier *tierTokens, IntervalTier *tierPhones,
                                   QString &outAlignerOutput, bool insertLeadingAndTrailingPauses)
{
    if (!tierUtterances) return false;
    if (!tierTokens) return false;
    if (!tierPhones) return false;
    if (indexUtteranceToAlign < 0) return false;
    if (indexUtteranceToAlign >= tierUtterances->count()) return false;
    Interval *utterance = tierUtterances->at(indexUtteranceToAlign);

    RealTime timeFrom = utterance->tMin();
    RealTime timeTo = utterance->tMax();
    QPair<int, int> tokenIndices = tierTokens->getIntervalIndexesContainedIn(tierUtterances->at(indexUtteranceToAlign));
    int tokenIndexFrom = tokenIndices.first;
    int tokenIndexTo = tokenIndices.second;
    if ((tokenIndexFrom < 0) || (tokenIndexTo < 0)) return false;

    QList<Interval *> phonesList;

    bool ok = alignTokens(waveFilepath, timeFrom, timeTo, tierTokens, tokenIndexFrom, tokenIndexTo,
                          insertLeadingAndTrailingPauses, phonesList, outAlignerOutput);

    if (ok) {
        int i(tokenIndexFrom);
        while ((i < tierTokens->count()) && (tierTokens->at(i)->isPauseSilent())) i++;
        if ((i < tierTokens->count()) && (tierTokens->at(i)->tMin() != utterance->tMin())) {
            tierUtterances->split(indexUtteranceToAlign, tierTokens->at(i)->tMin(), true);
            tierUtterances->at(indexUtteranceToAlign)->setText("_");
            indexUtteranceToAlign++;
        }
        i = tokenIndexTo;
        while ((i - 1 >= 0) && (tierTokens->at(i - 1)->isPauseSilent())) i--;
        if ((i >= 0) && (tierTokens->at(i)->isPauseSilent())) {
            tierUtterances->split(indexUtteranceToAlign, tierTokens->at(i)->tMin(), false);
            tierUtterances->at(indexUtteranceToAlign + 1)->setText("_");
        }
        tierPhones->patchIntervals(phonesList, timeFrom, timeTo);
    }

    if ((!ok) && d->returnDummyAlignmentOnFailure) {
        QList<Interval *> dummyPhonesList;
        for (int i = tokenIndexFrom; i <= tokenIndexTo; ++i) {

            Interval *token = tierTokens->at(i);
            QString phonetisationAttributeText = token->attribute(d->tokenPhonetisationAttributeID).toString();
            QString phonetisation = phonetisationAttributeText.split(d->phonetisationSeparatorForVariants).first();
            QStringList dummyPhones = phonetisation.split(d->phonetisationSeparatorForPhonemes);
            RealTime step = (token->tMax() - token->tMin()) / dummyPhones.count();
            RealTime dummyPhone_tMin = token->tMin();
            for (int j = 0; j < dummyPhones.count(); ++j) {
                RealTime dummyPhone_tMax = dummyPhone_tMin + step;
                if (j == dummyPhones.count() - 1) dummyPhone_tMax = token->tMax();
                dummyPhonesList << new Interval(dummyPhone_tMin, dummyPhone_tMax, dummyPhones.at(j));
                dummyPhone_tMin = dummyPhone_tMax;
            }
            token->setAttribute("dummy_align", true);
        }
        tierPhones->patchIntervals(dummyPhonesList, timeFrom, timeTo);
    }

    return ok;
}

// public
bool ForcedAligner::alignAllUtterances(const QString &waveFilepath,
                                       IntervalTier *tierUtterances, IntervalTier *tierTokens, IntervalTier *tierPhones,
                                       bool insertLeadingAndTrailingPauses)
{
    if (!tierUtterances) return false;
    if (!tierTokens) return false;
    if (!tierPhones) return false;
    int indexUtterance = tierUtterances->count() - 1;
    while (indexUtterance >= 0) {
        if (tierUtterances->at(indexUtterance)->isPauseSilent()) { indexUtterance--; continue; }
        QString alignerOutput;
        bool result(false);
        result = alignUtterance(waveFilepath, tierUtterances, indexUtterance, tierTokens, tierPhones,
                                alignerOutput, insertLeadingAndTrailingPauses);
        // if (!result) qDebug() << result << tierUtterances->at(indexUtterance)->text() << alignerOutput;
        alignerMessage(alignerOutput);
        indexUtterance--;
    }
    tierTokens->mergeIdenticalAnnotations("_");
    tierUtterances->mergeIdenticalAnnotations("_");
    tierPhones->fillEmptyWith("", "_");
    tierPhones->mergeIdenticalAnnotations("_");
    return true;
}

} // namespace ASR
} // namespace Praaline

