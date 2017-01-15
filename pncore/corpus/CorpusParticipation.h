#ifndef CORPUSPARTICIPATION_H
#define CORPUSPARTICIPATION_H

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

    CorpusObject::Type type() const override { return CorpusObject::Type_Participation; }
    bool save() override;

    // override CorpusObject ID methods
    QString ID() const override;
    void setID(const QString &ID) override;

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

    Q_DISABLE_COPY(CorpusParticipation)
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSPARTICIPATION_H
