#include "corpus.h"
#include "corpusparticipation.h"

CorpusParticipation::CorpusParticipation(QPointer<CorpusCommunication> com, QPointer<CorpusSpeaker> spk,
                                         QString role, QObject *parent)
    : CorpusObject(parent), m_communication(com), m_speaker(spk), m_role(role)
{
    if (com && spk)
        m_ID = QString("%1_x_%2").arg(m_communication->ID()).arg(m_speaker->ID());
    else
        m_ID = "(deleted)";
}

void CorpusParticipation::copyProperties(CorpusParticipation *other)
{
    if (!other) return;
    copyPropertiesFrom(other);
}

// override CorpusObject ID methods
QString CorpusParticipation::ID() const
{
    if (m_communication && m_speaker)
        return QString("%1_x_%2").arg(m_communication->ID()).arg(m_speaker->ID());
    return "(deleted)";
}

void CorpusParticipation::setID(const QString &ID)
{
    Q_UNUSED(ID)
}

QPointer<Corpus> CorpusParticipation::corpus() const
{
    return qobject_cast<Corpus *>(this->parent());
}

// read-only properties
QPointer<CorpusCommunication> CorpusParticipation::communication() const
{
    return m_communication;
}

QPointer<CorpusSpeaker> CorpusParticipation::speaker() const
{
    return m_speaker;
}

QString CorpusParticipation::communicationID() const
{
    if (m_communication) return m_communication->ID();
    return "(deleted)";
}

QString CorpusParticipation::speakerID() const
{
    if (m_speaker) return m_speaker->ID();
    return "(deleted)";
}

// speaker role in communication
QString CorpusParticipation::role() const
{
    return m_role;
}

void CorpusParticipation::setRole(const QString &role)
{
    if (m_role != role) {
        m_role = role;
        m_dirty = true;
    }
}

