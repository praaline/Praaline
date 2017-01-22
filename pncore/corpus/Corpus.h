#ifndef CORPUS_H
#define CORPUS_H

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
#include <QPointer>
#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QMultiMap>
#include "CorpusObject.h"
#include "CorpusCommunication.h"
#include "CorpusSpeaker.h"
#include "CorpusParticipation.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT Corpus : public CorpusObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    explicit Corpus(CorpusRepository *repository = 0, QObject *parent = 0);
    explicit Corpus(const QString &ID, CorpusRepository *repository = 0, QObject *parent = 0);
    ~Corpus();

    // ----------------------------------------------------------------------------------------------------------------
    // Corpus
    // ----------------------------------------------------------------------------------------------------------------
    CorpusObject::Type type() const override { return CorpusObject::Type_Corpus; }
    bool save() override;

    QString name() const { return m_name; }
    void setName(const QString &name);

    QString description() const { return m_description; }
    void setDescription(const QString &description);

    // Override corpusID = ID
    QString corpusID() const override { return m_ID; }
    void setCorpusID(const QString &corpusID) override;

    void clear();

    // ----------------------------------------------------------------------------------------------------------------
    // Communications
    // ----------------------------------------------------------------------------------------------------------------
    QPointer<CorpusCommunication> communication(const QString &communicationID) const;
    int communicationsCount() const;
    bool hasCommunications() const;
    bool hasCommunication(const QString &communicationID) const;
    QStringList communicationIDs() const;
    const QMap<QString, QPointer<CorpusCommunication> > &communications() const;
    void addCommunication(CorpusCommunication *communication);
    void removeCommunication(const QString &communicationID);

    // ----------------------------------------------------------------------------------------------------------------
    // Speakers
    // ----------------------------------------------------------------------------------------------------------------
    QPointer<CorpusSpeaker> speaker(const QString &speakerID) const;
    int speakersCount() const;
    bool hasSpeakers() const;
    bool hasSpeaker(const QString &speakerID) const;
    QStringList speakerIDs() const;
    const QMap<QString, QPointer<CorpusSpeaker> > &speakers() const;
    void addSpeaker(CorpusSpeaker *speaker);
    void removeSpeaker(const QString &speakerID);

    // ----------------------------------------------------------------------------------------------------------------
    // Speaker Participation in Communications
    // ----------------------------------------------------------------------------------------------------------------
    QPointer<CorpusParticipation> participation(const QString &communicationID, const QString &speakerID);
    bool hasParticipation(const QString &communicationID, const QString &speakerID);
    QList<QPointer<CorpusParticipation> > participations();
    QList<QPointer<CorpusParticipation> > participationsForCommunication(const QString &communicationID);
    QList<QPointer<CorpusParticipation> > participationsForSpeaker(const QString &speakerID);
    QPointer<CorpusParticipation> addParticipation(const QString &communicationID, const QString &speakerID, const QString &role = QString());
    void removeParticipation(const QString &communicationID, const QString &speakerID);

    // ----------------------------------------------------------------------------------------------------------------
    // Cross Recordings x Annotations
    // ----------------------------------------------------------------------------------------------------------------
    QStringList recordingIDs() const;
    QStringList annotationIDs() const;
    QList<QPointer<CorpusCommunication> > communicationsList() const;
    QList<QPointer<CorpusSpeaker> > speakersList() const;
    QList<QPointer<CorpusRecording> > recordingsList() const;
    QList<QPointer<CorpusAnnotation> > annotationsList() const;
    QMap<QString, QPair<QPointer<CorpusRecording>, QPointer<CorpusAnnotation> > > getRecordings_x_Annotations() const;
    QMap<QString, QPair<QPointer<CorpusAnnotation>, QPointer<CorpusRecording> > > getAnnotations_x_Recordings() const;
    QList<QPair<QString, QString> > getCommunicationsAnnotationsIDs() const;
    QList<QPair<QString, QString> > getCommunicationsRecordingsIDs() const;

    // ----------------------------------------------------------------------------------------------------------------
    // Database helpers
    // ----------------------------------------------------------------------------------------------------------------
    QList<QString> deletedCommunicationIDs;
    QList<QString> deletedSpeakerIDs;
    QList<QPair<QString, QString> > deletedParticipationIDs;

signals:
    void communicationAdded(QPointer<Praaline::Core::CorpusCommunication> communication);
    void communicationDeleted(QString ID);
    void speakerAdded(QPointer<Praaline::Core::CorpusSpeaker> speaker);
    void speakerDeleted(QString ID);

private slots:
    void communicationChangedID(const QString &oldID, const QString &newID);
    void speakerChangedID(const QString &oldID, const QString &newID);

private:
    QString m_name;
    QString m_description;
    QMap<QString, QPointer<CorpusCommunication> > m_communications;
    QMap<QString, QPointer<CorpusSpeaker> > m_speakers;
    QMultiMap<QString, QPointer<CorpusParticipation> > m_participationsByCommunication;
    QMultiMap<QString, QPointer<CorpusParticipation> > m_participationsBySpeaker;

    Q_DISABLE_COPY(Corpus)
};

} // namespace Core
} // namespace Praaline

#endif // CORPUS_H
