#ifndef CORPUSOBJECTINFO_H
#define CORPUSOBJECTINFO_H

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

#include <QString>
#include <QDateTime>
#include "CorpusObject.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT CorpusObjectInfo
{
public:
    CorpusObjectInfo(CorpusObject::Type type, const QString &ID, const QString &parentID, const QString &name, const QString &description,
                     const QString &createdBy, const QDateTime &createdTimestamp,
                     const QString &lastUpdatedBy, const QDateTime &lastUpdatedTimestamp);

    CorpusObject::Type type() const;
    QString ID() const;
    QString parentID() const;
    QString name() const;
    QString description() const;

    QString createdBy() const;
    QDateTime createdTimestamp() const;
    QString lastUpdatedBy() const;
    QDateTime lastUpdatedTimestamp() const;

private:
    CorpusObject::Type m_type;
    QString m_ID;
    QString m_parentID;
    QString m_name;
    QString m_description;
    QString m_createdBy;
    QDateTime m_createdTimestamp;
    QString m_lastUpdatedBy;
    QDateTime m_lastUpdatedTimestamp;
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSOBJECTINFO_H
