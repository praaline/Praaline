#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "Corpus.h"
#include "CorpusObject.h"
#include "CorpusSpeaker.h"
#include "datastore/CorpusRepository.h"
#include "datastore/MetadataDatastore.h"

namespace Praaline {
namespace Core {

CorpusSpeaker::CorpusSpeaker(CorpusRepository *repository, QObject *parent) :
    CorpusObject(repository, parent)
{
}

CorpusSpeaker::CorpusSpeaker(const QString &ID, CorpusRepository *repository, QObject *parent) :
    CorpusObject(ID, repository, parent)
{
}

CorpusSpeaker::CorpusSpeaker(CorpusSpeaker *other, QObject *parent) :
    CorpusObject(0, parent)
{
    if (!other) return;
    m_ID = other->m_ID;
    m_originalID = other->m_originalID;
    m_name = other->m_name;
    m_corpusID = other->m_corpusID;
    m_repository = other->m_repository;
    copyPropertiesFrom(other);
}

QPointer<Corpus> CorpusSpeaker::corpus() const
{
    return qobject_cast<Corpus *>(this->parent());
}

void CorpusSpeaker::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        m_isDirty = true;
    }
}

bool CorpusSpeaker::save() {
    if (!m_repository) return false;
    if (!m_repository->metadata()) return false;
    return m_repository->metadata()->saveSpeakers(QList<QPointer<CorpusSpeaker> >() << this);
}

} // namespace Core
} // namespace Praaline
