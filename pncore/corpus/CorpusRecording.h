#ifndef CORPUSRECORDING_H
#define CORPUSRECORDING_H

/*
    Praaline - Core module - Corpus metadata
    Copyright (c) 2011-2017 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include <QDir>
#include <QUrl>
#include "base/RealTime.h"
#include "CorpusObject.h"

namespace Praaline {
namespace Core {

class Corpus;

class PRAALINE_CORE_SHARED_EXPORT CorpusRecording : public CorpusObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString filename READ filename WRITE setFilename)
    Q_PROPERTY(QString format READ format WRITE setFormat)
    Q_PROPERTY(double durationSec READ durationSec WRITE setDurationSec)
    Q_PROPERTY(int channels READ channels WRITE setChannels)
    Q_PROPERTY(int sampleRate READ sampleRate WRITE setSampleRate)
    Q_PROPERTY(int precisionBits READ precisionBits WRITE setPrecisionBits)
    Q_PROPERTY(int bitRate READ bitRate WRITE setBitRate)
    Q_PROPERTY(QString encoding READ encoding WRITE setEncoding)
    Q_PROPERTY(long long fileSize READ fileSize WRITE setFileSize)
    Q_PROPERTY(QString checksumMD5 READ checksumMD5 WRITE setChecksumMD5)
    Q_PROPERTY(QString communicationID READ communicationID)

public:
    explicit CorpusRecording(CorpusRepository *repository = 0, QObject *parent = 0);
    explicit CorpusRecording(const QString ID, CorpusRepository *repository = 0, QObject *parent = 0);
    explicit CorpusRecording(CorpusRecording *other, QObject *parent = 0);
    ~CorpusRecording() {}

    CorpusObject::Type type() const override { return CorpusObject::Type_Recording; }
    bool save() override;

    QString communicationID() const;
    QPointer<Corpus> corpus() const;

    QString basePath() const;
    QString filePath() const;

    QString name() const { return m_name; }
    void setName(const QString &name);

    QString filename() const { return m_filename; }
    void setFilename(const QString &filename);

    QString format() const { return m_format; }
    void setFormat(const QString &format);

    RealTime duration() const { return m_duration; }
    void setDuration(const RealTime &duration);

    double durationSec() const { return m_duration.toDouble(); }
    void setDurationSec(double duration);

    int channels() const { return m_channels; }
    void setChannels(int channels);

    int sampleRate() const { return m_sampleRate; }
    void setSampleRate(int sampleRate);

    int precisionBits() const { return m_precisionBits; }
    void setPrecisionBits(int precisionBits);

    int bitRate() const { return m_bitRate; }
    void setBitRate(int bitRate);

    QString encoding() const { return m_encoding; }
    void setEncoding(const QString &encoding);

    long long fileSize() const { return m_fileSize; }
    void setFileSize(long long fileSize);

    QString checksumMD5() const { return m_checksumMD5; }
    void setChecksumMD5(const QString &checksumMD5);

    bool isFileAvailable() const;
    QUrl mediaUrl() const;

private:
    QString m_name;
    QString m_filename;
    QString m_format;
    RealTime m_duration;
    int m_channels;
    int m_sampleRate;
    int m_precisionBits;
    int m_bitRate;
    QString m_encoding;
    long long m_fileSize;
    QString m_checksumMD5;

    Q_DISABLE_COPY(CorpusRecording)
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSRECORDING_H
