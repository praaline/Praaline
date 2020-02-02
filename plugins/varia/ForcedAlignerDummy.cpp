#include <QString>
#include <QList>
#include <QStringList>

#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "ForcedAlignerDummy.h"


QStringList splitPhoneLabels(const QString &input)
{
    QStringList phonemes;
    phonemes << "2" << "9" << "9~" << "@" << "E" << "H" << "O" << "R" << "S" << "Z"
             << "a" << "a~" << "b" << "d" << "e" << "e~" << "f" << "g" << "i" << "j" << "k" << "l"
             << "m" << "n" << "o" << "o~" << "p" << "s" << "t" << "u" << "v" << "w" << "y" << "z";
    QStringList ret;
    QString temp(input);
    while (!temp.isEmpty()) {
        if (phonemes.contains(temp.left(2))) {
            ret << temp.left(2);
            temp = temp.remove(0, 2);
        } else {
            ret << temp.left(1);
            temp = temp.remove(0, 1);
        }
    }
    return ret;
}

struct ForcedAlignerDummyData {
    ForcedAlignerDummyData() :
        tier_phones(0), tier_tokens(0)
    {}
    IntervalTier *tier_phones;
    IntervalTier *tier_tokens;
    QString attributePhonetisation;
};

ForcedAlignerDummy::ForcedAlignerDummy() :
    d(new ForcedAlignerDummyData)
{
}

ForcedAlignerDummy::~ForcedAlignerDummy()
{
    delete d;
}

IntervalTier *ForcedAlignerDummy::tierTokens() const
{
    return d->tier_tokens;
}

void ForcedAlignerDummy::setTierTokens(Praaline::Core::IntervalTier *tierTokens)
{
    d->tier_tokens = tierTokens;
}

QString ForcedAlignerDummy::attributePhonetisation() const
{
    return d->attributePhonetisation;
}

void ForcedAlignerDummy::setAttributePhonetisation(const QString &attributeID)
{
    d->attributePhonetisation = attributeID;
}

IntervalTier *ForcedAlignerDummy::tierPhones() const
{
    return d->tier_phones;
}

void ForcedAlignerDummy::setTierPhones(Praaline::Core::IntervalTier *tierPhones)
{
    d->tier_phones = tierPhones;
}

// Used when there is a tier already containing the tokens
bool ForcedAlignerDummy::processUnalignedSegment(int indexOnPhonesTier)
{
    if (!d->tier_phones) return false;
    if (!d->tier_tokens) return false;
    if ((indexOnPhonesTier < 0) || (indexOnPhonesTier >= d->tier_phones->count())) return false;

    QPair<int, int> tokenIndices = d->tier_tokens->getIntervalIndexesContainedIn(d->tier_phones->interval(indexOnPhonesTier));
    if ((tokenIndices.first < 0) || (tokenIndices.first >= d->tier_tokens->count())) return false;
    if ((tokenIndices.second < 0) || (tokenIndices.second >= d->tier_tokens->count())) return false;

    QList<RealTime> updatedTokenBoundaries;
    updatedTokenBoundaries << d->tier_phones->interval(indexOnPhonesTier)->tMin();
    QList<int> tokenBoundaryIndices;
    QStringList segmentPhoneLabels;

    for (int i = tokenIndices.first; i <= tokenIndices.second; ++i) {
        QString phonetisation = d->tier_tokens->at(i)->attribute(d->attributePhonetisation).toString();
        QStringList tokenPhoneLabels = splitPhoneLabels(phonetisation);
        segmentPhoneLabels << tokenPhoneLabels;
        tokenBoundaryIndices << segmentPhoneLabels.count() - 1;
    }
    d->tier_phones->interval(indexOnPhonesTier)->setText("");
    QList<Interval *> segmentPhones = d->tier_phones->splitToEqual(indexOnPhonesTier, segmentPhoneLabels.count());
    if (segmentPhones.count() != segmentPhoneLabels.count()) return false;
    for (int i = 0; i < segmentPhones.count(); ++i)
        segmentPhones.at(i)->setText(segmentPhoneLabels.at(i));

    foreach (int i, tokenBoundaryIndices)
        updatedTokenBoundaries << segmentPhones.at(i)->tMax();
    d->tier_tokens->realignIntervals(tokenIndices.first, updatedTokenBoundaries);

    return true;
}

// Used when the tokens tier is also empty
bool ForcedAlignerDummy::processUnalignedSegment(int indexOnPhonesTier, int indexOnTokensTier, QStringList tokens, QStringList phonetisations)
{
    if (!d->tier_phones) return false;
    if (!d->tier_tokens) return false;
    if ((indexOnPhonesTier < 0) || (indexOnPhonesTier >= d->tier_phones->count())) return false;
    if ((indexOnTokensTier < 0) || (indexOnTokensTier >= d->tier_tokens->count())) return false;

    if (tokens.count() != phonetisations.count()) return false;

    QList<RealTime> updatedTokenBoundaries;
    updatedTokenBoundaries << d->tier_phones->interval(indexOnPhonesTier)->tMin();
    QList<int> tokenBoundaryIndices;
    QStringList segmentPhoneLabels;

    for (int i = 0; i < tokens.count(); ++i) {
        QString phonetisation = phonetisations.at(i);
        QStringList tokenPhoneLabels = splitPhoneLabels(phonetisation);
        segmentPhoneLabels << tokenPhoneLabels;
        tokenBoundaryIndices << segmentPhoneLabels.count() - 1;
    }

    d->tier_phones->interval(indexOnPhonesTier)->setText("");
    d->tier_tokens->interval(indexOnTokensTier)->setText("");

    QList<Interval *> segmentPhones = d->tier_phones->splitToEqual(indexOnPhonesTier, segmentPhoneLabels.count());
    if (segmentPhones.count() != segmentPhoneLabels.count()) return false;
    for (int i = 0; i < segmentPhones.count(); ++i)
        segmentPhones.at(i)->setText(segmentPhoneLabels.at(i));

    QList<Interval *> segmentTokens = d->tier_tokens->splitToEqual(indexOnTokensTier, tokens.count());
    if (segmentTokens.count() != tokens.count()) return false;
    for (int i = 0; i < tokens.count(); ++i)
        segmentTokens.at(i)->setText(tokens.at(i));

    foreach (int i, tokenBoundaryIndices)
        updatedTokenBoundaries << segmentPhones.at(i)->tMax();
    d->tier_tokens->realignIntervals(indexOnTokensTier, updatedTokenBoundaries);

    return true;
}


