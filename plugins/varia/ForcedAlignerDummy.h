#ifndef FORCEDALIGNERDUMMY_H
#define FORCEDALIGNERDUMMY_H

#include <QString>

namespace Praaline {
namespace Core {
class IntervalTier;
}
}

struct ForcedAlignerDummyData;

class ForcedAlignerDummy
{
public:
    ForcedAlignerDummy();
    ~ForcedAlignerDummy();

    Praaline::Core::IntervalTier *tierTokens() const;
    void setTierTokens(Praaline::Core::IntervalTier *tierTokens);

    QString attributePhonetisation() const;
    void setAttributePhonetisation(const QString &attributeID);

    Praaline::Core::IntervalTier *tierPhones() const;
    void setTierPhones(Praaline::Core::IntervalTier *tierPhones);

    bool processUnalignedSegment(int indexOnPhonesTier);
    bool processUnalignedSegment(int indexOnPhonesTier, int indexOnTokensTier, QStringList tokens, QStringList phonetisations);

private:
    ForcedAlignerDummyData *d;
};

#endif // FORCEDALIGNERDUMMY_H

