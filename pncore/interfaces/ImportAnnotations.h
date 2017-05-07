#ifndef IMPORTANNOTATIONS_H
#define IMPORTANNOTATIONS_H

#include <QString>
#include <QList>
#include "pncore_global.h"

namespace Praaline {
namespace Core {

class Corpus;
class CorpusCommunication;
class CorpusAnnotation;
class AnnotationTierGroup;

class PRAALINE_CORE_SHARED_EXPORT TierCorrespondance {
public:
    TierCorrespondance(QString filename, QString tierName, QString tierType, int itemsCount) :
        filename(filename), tierName(tierName), tierType(tierType), itemsCount(itemsCount) {}
    QString filename;
    QString tierName;
    QString tierType;
    int itemsCount;
    QString annotationLevelID;
    QString annotationAttributeID;
};

class PRAALINE_CORE_SHARED_EXPORT ImportAnnotations {
public:
    enum SpeakerPolicy {
        SpeakerPolicySingle,
        SpeakerPolicyTierNames,
        SpeakerPolicyIntervals,
        SpeakerPolicyPrimaryAndSecondary
    };

    ImportAnnotations();

    static SpeakerPolicy speakerPolicyFromInt(int i);

    static bool importBasic(Corpus *corpus, CorpusCommunication *com, CorpusAnnotation *annot,
                            SpeakerPolicy speakerPolicy, QString speakerID,
                            AnnotationTierGroup *inputTiers, QList<TierCorrespondance> &correspondances);
};

} // namespace Core
} // namespace Praaline

#endif // IMPORTANNOTATIONS_H
