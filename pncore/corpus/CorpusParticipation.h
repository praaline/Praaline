#ifndef CORPUSPARTICIPATION_H
#define CORPUSPARTICIPATION_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include "CorpusObject.h"
#include "CorpusCommunication.h"
#include "CorpusSpeaker.h"

namespace Praaline {
namespace Core {

class Corpus;

class PRAALINE_CORE_SHARED_EXPORT CorpusParticipation : public CorpusObject
{
    Q_OBJECT
    Q_PROPERTY(QString communicationID READ communicationID)
    Q_PROPERTY(QString speakerID READ speakerID)
    Q_PROPERTY(QString role READ role WRITE setRole)

public:
    CorpusParticipation(QPointer<CorpusCommunication> com, QPointer<CorpusSpeaker> spk, QString role = QString(), QObject *parent = 0);
    ~CorpusParticipation() {}
    void copyProperties(CorpusParticipation *other);

    CorpusObject::Type type() const { return CorpusObject::Type_Participation; }

    // override CorpusObject ID methods
    QString ID() const;
    void setID(const QString &ID);

    QPointer<Corpus> corpus() const;

    // read-only properties
    QPointer<CorpusCommunication> communication() const;
    QPointer<CorpusSpeaker> speaker() const;
    QString communicationID() const;
    QString speakerID() const;

    QString role() const;
    void setRole(const QString &role);

private:
    QPointer<CorpusCommunication> m_communication;
    QPointer<CorpusSpeaker> m_speaker;
    QString m_role;
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSPARTICIPATION_H
