#include <QObject>
#include <QString>
#include <QFileInfo>

#include "corpus.h"
#include "corpusobject.h"
#include "corpusrecording.h"
#include "corpuscommunication.h"

CorpusRecording::CorpusRecording(QObject *parent)
    : CorpusObject(parent), m_channels(0), m_sampleRate(0), m_precisionBits(0), m_bitRate(0), m_fileSize(0)
{
}

CorpusRecording::CorpusRecording(const QString ID, QObject *parent)
    : CorpusObject(ID, parent), m_channels(0), m_sampleRate(0), m_precisionBits(0), m_bitRate(0), m_fileSize(0)
{
}

CorpusRecording::CorpusRecording(CorpusRecording *other, QObject *parent) :
    CorpusObject(parent)
{
    if (!other) return;
    m_ID = other->m_ID;
    m_originalID = other->m_originalID;
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
    setDirty(true);
    setNew(true);
}

QString CorpusRecording::basePath() const
{
    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(this->parent());
    if (com) return com->basePath();
    return QString();
}

QString CorpusRecording::baseMediaPath() const
{
    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(this->parent());
    if (com) return com->baseMediaPath();
    return QString();
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

void CorpusRecording::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        setDirty(true);
    }
}

void CorpusRecording::setFilename(const QString &filename)
{
    if (m_filename != filename) {
        m_filename = filename;
        setDirty(true);
    }
}

void CorpusRecording::setFormat(const QString &format)
{
    if (m_format != format) {
        m_format = format;
        setDirty(true);
    }
}

void CorpusRecording::setDuration(const RealTime &duration)
{
    if (m_duration != duration) {
        m_duration = duration;
        setDirty(true);
    }
}

void CorpusRecording::setDurationSec(double duration)
{
    if (m_duration != RealTime::fromSeconds(duration)) {
        m_duration = RealTime::fromSeconds(duration);
        setDirty(true);
    }
}

void CorpusRecording::setChannels(int channels)
{
    if (m_channels != channels) {
        m_channels = channels;
        setDirty(true);
    }
}

void CorpusRecording::setSampleRate(int sampleRate)
{
    if (m_sampleRate != sampleRate) {
        m_sampleRate = sampleRate;
        setDirty(true);
    }
}

void CorpusRecording::setPrecisionBits(int precisionBits)
{
    if (m_precisionBits != precisionBits) {
        m_precisionBits = precisionBits;
        setDirty(true);
    }
}

void CorpusRecording::setBitRate(int bitRate)
{
    if (m_bitRate != bitRate) {
        m_bitRate = bitRate;
        setDirty(true);
    }
}

void CorpusRecording::setEncoding(const QString &encoding)
{
    if (m_encoding != encoding) {
        m_encoding = encoding;
        setDirty(true);
    }
}

void CorpusRecording::setFileSize(long long fileSize)
{
    if (m_fileSize != fileSize) {
        m_fileSize = fileSize;
        setDirty(true);
    }
}

void CorpusRecording::setChecksumMD5(const QString &checksumMD5)
{
    if (m_checksumMD5 != checksumMD5) {
        m_checksumMD5 = checksumMD5;
        setDirty(true);
    }
}

bool CorpusRecording::isFileAvailable() const
{
    return QFile::exists(basePath() + m_filename);
}

