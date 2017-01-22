#ifndef CORPUSSPEAKER_H
#define CORPUSSPEAKER_H

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
#include "CorpusObject.h"

namespace Praaline {
namespace Core {

class Corpus;

class PRAALINE_CORE_SHARED_EXPORT CorpusSpeaker : public CorpusObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    explicit CorpusSpeaker(CorpusRepository *repository = 0, QObject *parent = 0);
    explicit CorpusSpeaker(const QString &ID, CorpusRepository *repository = 0, QObject *parent = 0);
    explicit CorpusSpeaker(CorpusSpeaker *other, QObject *parent = 0);

    CorpusObject::Type type() const override { return CorpusObject::Type_Speaker; }
    bool save() override;

    QPointer<Corpus> corpus() const;

    QString name() const { return m_name; }
    void setName(const QString &name);

private:
    QString m_name;
    
    Q_DISABLE_COPY(CorpusSpeaker)
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSSPEAKER_H
