#include <QObject>
#include <QString>
#include <QFileInfo>

#include "Corpus.h"
#include "CorpusObject.h"
#include "CorpusRecording.h"
#include "CorpusCommunication.h"
#include "datastore/CorpusRepository.h"
#include "datastore/FileDatastore.h"
#include "datastore/MetadataDatastore.h"

namespace Praaline {
namespace Core {

CorpusRecording::CorpusRecording(CorpusRepository *repository, QObject *parent) :
    CorpusObject(repository, parent),
    m_channels(0), m_sampleRate(0), m_precisionBits(0), m_bitRate(0), m_fileSize(0)
{
}

CorpusRecording::CorpusRecording(const QString ID, CorpusRepository *repository, QObject *parent) :
    CorpusObject(ID, repository, parent),
    m_channels(0), m_sampleRate(0), m_precisionBits(0), m_bitRate(0), m_fileSize(0)
{
}

CorpusRecording::CorpusRecording(CorpusRecording *other, QObject *parent) :
    CorpusObject(0, parent)
{
    if (!other) return;
    m_ID = other->m_ID;
    m_originalID = other->m_originalID;
    m_repository = other->m_repository;
    m_name = other->m_name;
    m_filename = other->m_filename;
    m_format = other->m_format;
    m_duration = other->m_duration;
    m_channels = other->m_channels;
    m_sampleRate = other->m_sampleRate;
    m_precisionBits = other->m_precisionBits;
    m_bitRate = other->m_bitRate;
    m_encoding = other->m_encoding;
    m_fileSize = other->m_fileSize;
    m_checksumMD5 = other->m_checksumMD5;
    copyPropertiesFrom(other);
}

QString CorpusRecording::basePath() const
{
    QString path;
    if (m_repository && m_repository->files())
        path = m_repository->files()->basePath();
    if (path.endsWith("/")) path.chop(1);
    return path;
}

QString CorpusRecording::communicationID() const
{
    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(this->parent());
    if (com) return com->ID();
    return QString();
}

QPointer<Corpus> CorpusRecording::corpus() const
{
    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(this->parent());
    if (com) return qobject_cast<Corpus *>(com->parent());
    return 0;
}

QString CorpusRecording::filePath() const
{
    return basePath() + "/" + filename();
}

bool CorpusRecording::isFileAvailable() const
{
    if (filename().startsWith("http:") || filename().startsWith("https:"))
        return false;
    // else
    return QFile::exists(basePath() + "/" + filename());
}

QUrl CorpusRecording::mediaUrl() const
{
    if (filename().startsWith("http:") || filename().startsWith("https:"))
        return QUrl(filename());
    // else
    return QUrl::fromLocalFile(basePath() + "/" + filename());
}

void CorpusRecording::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        m_isDirty = true;
    }
}

void CorpusRecording::setFilename(const QString &filename)
{
    if (m_filename != filename) {
        m_filename = filename;
        m_isDirty = true;
    }
}

void CorpusRecording::setFormat(const QString &format)
{
    if (m_format != format) {
        m_format = format;
        m_isDirty = true;
    }
}

void CorpusRecording::setDuration(const RealTime &duration)
{
    if (m_duration != duration) {
        m_duration = duration;
        m_isDirty = true;
    }
}

void CorpusRecording::setDurationSec(double duration)
{
    if (m_duration != RealTime::fromSeconds(duration)) {
        m_duration = RealTime::fromSeconds(duration);
        m_isDirty = true;
    }
}

void CorpusRecording::setChannels(int channels)
{
    if (m_channels != channels) {
        m_channels = channels;
        m_isDirty = true;
    }
}

void CorpusRecording::setSampleRate(int sampleRate)
{
    if (m_sampleRate != sampleRate) {
        m_sampleRate = sampleRate;
        m_isDirty = true;
    }
}

void CorpusRecording::setPrecisionBits(int precisionBits)
{
    if (m_precisionBits != precisionBits) {
        m_precisionBits = precisionBits;
        m_isDirty = true;
    }
}

void CorpusRecording::setBitRate(int bitRate)
{
    if (m_bitRate != bitRate) {
        m_bitRate = bitRate;
        m_isDirty = true;
    }
}

void CorpusRecording::setEncoding(const QString &encoding)
{
    if (m_encoding != encoding) {
        m_encoding = encoding;
        m_isDirty = true;
    }
}

void CorpusRecording::setFileSize(long long fileSize)
{
    if (m_fileSize != fileSize) {
        m_fileSize = fileSize;
        m_isDirty = true;
    }
}

void CorpusRecording::setChecksumMD5(const QString &checksumMD5)
{
    if (m_checksumMD5 != checksumMD5) {
        m_checksumMD5 = checksumMD5;
        m_isDirty = true;
    }
}

bool CorpusRecording::save() {
    if (!m_repository) return false;
    if (!m_repository->metadata()) return false;
    return m_repository->metadata()->saveRecordings(QList<QPointer<CorpusRecording> >() << this);
}

} // namespace Core
} // namespace Praaline
