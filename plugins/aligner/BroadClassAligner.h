#ifndef BROADCLASSALIGNER_H
#define BROADCLASSALIGNER_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QHash>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

struct BroadClassAlignerData;

class BroadClassAligner
{
public:
    BroadClassAligner();
    ~BroadClassAligner();

    void addBroadPhoneticClass(const QString &name, const QList<QString> &phonemes);
    void resetClasses();

    void initialiseFR();
    void initialiseFRSphinx();
    void initialiseFRcv();

    QString phonemeToBroadClass(const QString &phoneme) const;

    bool adaptDictionary(const QString &filenameInput, const QString &filenameOutput) const;
    void updatePhoneTierWithBroadClasses(IntervalTier *tier, const QString &attributeBroadClass) const;
    void updateTokenTierWithBroadClasses(IntervalTier *tier_phone, const QString &attributeBroadClassPhone,
                                         IntervalTier *tier_token, const QString &attributeBroadClassToken) const;
    void prepareBPCTrainingFromCommunications(QList<CorpusCommunication *> communications,
                                              const QString &outputPath);

private:
    BroadClassAlignerData *d;

    void prepareTiers(QList<CorpusCommunication *> communications);
};

#endif // BROADCLASSALIGNER_H
