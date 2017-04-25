#ifndef PHONTRANSCRIPTION_H
#define PHONTRANSCRIPTION_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QList>
#include <QXmlStreamReader>
#include "base/RealTime.h"

namespace Praaline {
namespace Core {

struct PhonTranscriptionData;

class PRAALINE_CORE_SHARED_EXPORT PhonTranscription
{
public:
    class ParticipantInfo {
    public:
        QString ID;
        QString name;
        QString role;
        QString sex;
        QString language;
    };

    class TierInfo {
    public:
        TierInfo() :
            tierName(QString()), userTier(false), grouped(false), visible(true), locked(false), font(QString())
        {}
        QString tierName;
        bool userTier;
        bool grouped;
        bool visible;
        bool locked;
        QString font;
    };

    class PhoneData {
    public:
        QList<int> indexes;
        QString scType;
        bool hiatus;
    };

    class IPAData {
    public:
        QString form;
        QStringList pg;
        QList<PhoneData> sb;
    };

    class Segment {
    public:
        QString speakerID;
        QString segmentID;
        bool excludeFromSearches;
        RealTime startTime;
        RealTime duration;
        QStringList orthography;
        IPAData ipaModel;
        IPAData ipaActual;
        QString notes;
        QHash<QString, QStringList> groupTiers;
    };

    PhonTranscription();
    ~PhonTranscription();

    bool load(const QString &filename);

    QString sessionID() const;
    QString corpusID() const;
    QString phonVersion() const;
    QDate recordingDate() const;
    QString recordingFilename() const;
    QStringList participantIDs() const;
    PhonTranscription::ParticipantInfo participant(const QString &participantID) const;
    QStringList phonTierNames() const;
    PhonTranscription::TierInfo phonTierInfo(const QString &tierName) const;
    QStringList phonTierOrder() const;
    QList<PhonTranscription::Segment> &segments() const;

private:
    PhonTranscriptionData *d;

    bool readSession(QXmlStreamReader &xml);
    bool readHeader(QXmlStreamReader &xml);
    bool readParticipant(QXmlStreamReader &xml);
    bool readUserTier(QXmlStreamReader &xml);
    bool readTierOrder(QXmlStreamReader &xml);
    bool readSegment(QXmlStreamReader &xml);
    bool readSegmentOrthography(QXmlStreamReader &xml, Segment &segment);
    bool readSegmentIPATier(QXmlStreamReader &xml, Segment &segment);
    bool readSegmentAlignment(QXmlStreamReader &xml, Segment &segment);
    bool readSegmentGroupTier(QXmlStreamReader &xml, Segment &segment);

    // XML element names
    static QString xmlElementName_Session;
    static QString xmlElementName_Header;
    static QString xmlElementName_Participant;
    static QString xmlElementName_UserTier;
    static QString xmlElementName_Tier;
};

} // namespace Core
} // namespace Praaline

#endif // PHONTRANSCRIPTION_H
