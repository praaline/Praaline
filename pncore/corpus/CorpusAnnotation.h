#ifndef CORPUSANNOTATION_H
#define CORPUSANNOTATION_H

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
#include <QStringList>
#include <QSharedPointer>
#include "CorpusObject.h"

namespace Praaline {
namespace Core {

class Corpus;

class PRAALINE_CORE_SHARED_EXPORT CorpusAnnotation : public CorpusObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString filename READ filename WRITE setFilename)
    Q_PROPERTY(QString format READ format WRITE setFormat)
    Q_PROPERTY(QStringList languages READ languages)
    Q_PROPERTY(QString communicationID READ communicationID)

public:
    explicit CorpusAnnotation(QObject *parent = 0);
    CorpusAnnotation(const QString ID, QObject *parent = 0);
    CorpusAnnotation(CorpusAnnotation *other, QObject *parent = 0);
    ~CorpusAnnotation() {}

    CorpusObject::Type type() const { return CorpusObject::Type_Annotation; }

    QString basePath() const;
    QString baseMediaPath() const;
    QString communicationID() const;
    QPointer<Corpus> corpus() const;

    QString name() const { return m_name; }
    void setName(const QString &name);

    QString recordingID() const { return m_recordingID; }
    void setRecordingID(const QString &recordingID);

    QString filename() const { return m_filename; }
    void setFilename(const QString &filename);

    QString format() const { return m_format; }
    void setFormat(const QString &format);

    bool isMultiLanguage() const;
    QStringList languages() const;
    void addLanguage(const QString &languageID);
    void removeLanguage(const QString &languageID);

signals:
    
public slots:

protected:
    QString m_name;
    QString m_recordingID;
    QString m_filename;
    QString m_format;
    QStringList m_languages;
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSANNOTATION_H

