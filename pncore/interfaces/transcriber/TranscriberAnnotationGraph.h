#ifndef TRANSCRIBERANNOTATIONGRAPH_H
#define TRANSCRIBERANNOTATIONGRAPH_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QList>
#include <QMap>
#include <QString>
#include "base/RealTime.h"
#include "corpus/CorpusSpeaker.h"
#include "annotation/AnnotationTierGroup.h"
#include "annotation/AnnotationTier.h"
#include "annotation/IntervalTier.h"
#include "annotation/PointTier.h"
#include "interfaces/InterfaceTextFile.h"

class QXmlStreamReader;

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT TranscriberAnnotationGraph : InterfaceTextFile
{
public:    
    // Public static methods to read and write Transcriber Annotation Graphs in XML format
    static bool load(const QString &filename, QList<QPointer<CorpusSpeaker> > &speakers,
                     QMap<QString, QPointer<AnnotationTierGroup> > &tiersAll);
    static bool save(const QString &filename, QList<QPointer<CorpusSpeaker> > &speakers,
                     QMap<QString, QPointer<AnnotationTierGroup> > &tiersAll);

private:
    struct SpeakerData {
        QString ID;
        QString name;
        QString check;
        QString dialect;
        QString accent;
        QString scope;
    };

    struct TurnData {
        QStringList speakerIDs;
        RealTime startTime;
        RealTime endTime;
        QMap<QString, QList<Interval *> > intervals;
        QList<Point *> events;
    };

    struct SectionData {
        QString type;
        RealTime startTime;
        RealTime endTime;
        QString topicID;
        QList<TurnData *> turns;
    };

    struct TransData {
        QMap<QString, QString> topics;
        QList<SpeakerData *> speakers;
        QList<SectionData *> sections;
    };

    TranscriberAnnotationGraph();
    static SpeakerData  *readSpeaker(QXmlStreamReader &xml);
    static TurnData     *readTurn(QXmlStreamReader &xml);
    static SectionData  *readSection(QXmlStreamReader &xml);
    static TransData    *readTrans(QXmlStreamReader &xml);

    // XML element names
    static QString xmlElementName_Trans;
    static QString xmlElementName_Topic;
    static QString xmlElementName_Speaker;
    static QString xmlElementName_Section;
    static QString xmlElementName_Turn;
};

} // namespace Core
} // namespace Praaline

#endif // TRANSCRIBERANNOTATIONGRAPH_H
