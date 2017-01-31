#ifndef IMPORTANNOTATIONS
#define IMPORTANNOTATIONS

#include <QString>

class TierCorrespondance {
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

enum SpeakerPolicy {
    SpeakerPolicySingle,
    SpeakerPolicyTierNames,
    SpeakerPolicyIntervals,
    SpeakerPolicyPrimaryAndSecondary
};

#endif // IMPORTANNOTATIONS

