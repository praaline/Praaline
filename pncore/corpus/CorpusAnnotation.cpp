#include <QObject>
#include <QString>
#include <QStringList>

#include "CorpusObject.h"
#include "Corpus.h"
#include "CorpusAnnotation.h"
#include "CorpusCommunication.h"
#include "datastore/CorpusRepository.h"
#include "datastore/MetadataDatastore.h"

namespace Praaline {
namespace Core {

CorpusAnnotation::CorpusAnnotation(CorpusRepository *repository, QObject *parent) :
    CorpusObject(repository, parent)
{
    m_filename = QString();
    m_format = "tiers"; // by default
}

CorpusAnnotation::CorpusAnnotation(const QString ID, CorpusRepository *repository, QObject *parent) :
    CorpusObject(ID, repository, parent)
{
    m_filename = QString();
}

CorpusAnnotation::CorpusAnnotation(CorpusAnnotation *other, QObject *parent) :
    CorpusObject(0, parent)
{
    if (!other) return;
    m_ID = other->m_ID;
    m_originalID = other->m_originalID;
    m_name = other->m_name;
    m_corpusID = other->m_corpusID;
    m_repository = other->m_repository;
    m_recordingID = other->m_recordingID;
    m_filename = other->m_filename;
    m_format = other->m_format;
    m_languages = other->m_languages;
    copyPropertiesFrom(other);
}

QPointer<Corpus> CorpusAnnotation::corpus() const
{
    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(this->parent());
    if (com) return qobject_cast<Corpus *>(com->parent());
    return 0;
}

QString CorpusAnnotation::communicationID() const
{
    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(this->parent());
    if (com) return com->ID();
    return QString();
}

void CorpusAnnotation::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        m_isDirty = true;
    }
}

void CorpusAnnotation::setRecordingID(const QString &recordingID)
{
    if (m_recordingID != recordingID) {
        m_recordingID = recordingID;
        m_isDirty = true;
    }
}

void CorpusAnnotation::setFilename(const QString &filename)
{
    if (m_filename != filename) {
        m_filename = filename;
        m_isDirty = true;
    }
}

void CorpusAnnotation::setFormat(const QString &format)
{
    if (m_format != format) {
        m_format = format;
        m_isDirty = true;
    }
}

// ==========================================================================================================
// Language
// ==========================================================================================================
bool CorpusAnnotation::isMultiLanguage() const
{
    return (m_languages.count() > 1);
}

QStringList CorpusAnnotation::languages() const
{
    return m_languages;
}

void CorpusAnnotation::addLanguage(const QString &languageID)
{
    if (!m_languages.contains(languageID)) {
        m_languages.append(languageID);
        m_isDirty = true;
    }
}

void CorpusAnnotation::removeLanguage(const QString &languageID)
{
    if (m_languages.contains(languageID)) {
        m_languages.removeOne(languageID);
        m_isDirty = true;
    }
}

bool CorpusAnnotation::save() {
    if (!m_repository) return false;
    if (!m_repository->metadata()) return false;
    return m_repository->metadata()->saveAnnotations(QList<QPointer<CorpusAnnotation> >() << this);
}

} // namespace Core
} // namespace Praaline
