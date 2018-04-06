#ifndef CORPUSCOMMUNICATIONSPLICER_H
#define CORPUSCOMMUNICATIONSPLICER_H

#include <QString>
#include <QPointer>
#include "pncore/base/RealTime.h"

namespace Praaline {
namespace Core {
class CorpusRepository;
class CorpusCommunication;
}
}

struct CorpusCommunicationSplicerData;

class CorpusCommunicationSplicer
{
public:
    class Splice {
    public:
        Splice(const QString &communicationID, const QString &recordingID, const QString &annotationID,
               RealTime timeFrom, RealTime timeTo) :
            communicationID(communicationID), recordingID(recordingID), annotationID(annotationID),
            timeFrom(timeFrom), timeTo(timeTo)
        {}
        Splice(RealTime duration) :
            communicationID(QString()), recordingID(QString()), annotationID(QString()),
            timeFrom(RealTime::zeroTime), timeTo(duration)
        {}

        QString communicationID;
        QString recordingID;
        QString annotationID;
        RealTime timeFrom;
        RealTime timeTo;
        bool isSilence() {
            return communicationID.isEmpty();
        }
        RealTime duration() {
            return timeTo - timeFrom;
        }
    };

    class Composition {
    public:
        Composition(const QString &communicationID, const QString &recordingID, const QString &annotationID,
                    const QString &recordingFilepath, long recordingSampleRate, const QList<Splice> &splices = QList<Splice>()) :
            communicationID(communicationID), recordingID(recordingID), annotationID(annotationID),
            recordingFilepath(recordingFilepath), recordingSampleRate(recordingSampleRate), splices(splices)
        {}

        QString communicationID;
        QString recordingID;
        QString annotationID;
        QString recordingFilepath;
        long recordingSampleRate;
        QList<Splice> splices;
    };

    CorpusCommunicationSplicer();
    ~CorpusCommunicationSplicer();

    void setRepositorySource(Praaline::Core::CorpusRepository *repository);
    void setRepositoryDestination(Praaline::Core::CorpusRepository *repository);

private:
    CorpusCommunicationSplicerData *d;
};

#endif // CORPUSCOMMUNICATIONSPLICER_H
