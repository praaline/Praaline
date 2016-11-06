#include <QObject>
#include <QString>
#include <QStringList>

#include "CorpusObject.h"
#include "Corpus.h"
#include "CorpusAnnotation.h"
#include "CorpusCommunication.h"

namespace Praaline {
namespace Core {

CorpusAnnotation::CorpusAnnotation(QObject *parent)
    : CorpusObject(parent)
{
    m_filename = QString();
    m_format = "tiers"; // by default
}

CorpusAnnotation::CorpusAnnotation(const QString ID, QObject *parent)
    : CorpusObject(ID, parent)
{
    m_filename = QString();
}

CorpusAnnotation::CorpusAnnotation(CorpusAnnotation *other, QObject *parent) :
    CorpusObject(parent)
{
    if (!other) return;
    m_ID = other->m_ID;
    m_originalID = other->m_originalID;
    m_name = other->m_name;
    m_corpusID = other->m_corpusID;
    m_recordingID = other->m_recordingID;
    m_filename = other->m_filename;
    m_format = other->m_format;
    m_languages = other->m_languages;
    copyPropertiesFrom(other);
    setDirty(true);
    setNew(true);
}

QString CorpusAnnotation::basePath() const
{
    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(this->parent());
    if (com) return com->basePath();
    return QString();
}

QString CorpusAnnotation::baseMediaPath() const
{
    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(this->parent());
    if (com) return com->baseMediaPath();
    return QString();
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
        setDirty(true);
    }
}

void CorpusAnnotation::setRecordingID(const QString &recordingID)
{
    if (m_recordingID != recordingID) {
        m_recordingID = recordingID;
        setDirty(true);
    }
}

void CorpusAnnotation::setFilename(const QString &filename)
{
    if (m_filename != filename) {
        m_filename = filename;
        setDirty(true);
    }
}

void CorpusAnnotation::setFormat(const QString &format)
{
    if (m_format != format) {
        m_format = format;
        setDirty(true);
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
        setDirty(true);
    }
}

void CorpusAnnotation::removeLanguage(const QString &languageID)
{
    if (m_languages.contains(languageID)) {
        m_languages.removeOne(languageID);
        setDirty(true);
    }
}

} // namespace Core
} // namespace Praaline
