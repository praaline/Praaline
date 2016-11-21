#ifndef CORPUSOBJECTINFO_H
#define CORPUSOBJECTINFO_H

#include <QString>
#include <QDateTime>
#include "CorpusObject.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT CorpusObjectInfo
{
public:
    CorpusObjectInfo(CorpusObject::Type type, const QString &ID, const QString &parentID, const QString &name, const QString &description);

    CorpusObject::Type type() const;
    QString ID() const;
    QString parentID() const;
    QString name() const;
    QString description() const;

    QString createdBy() const;
    QDateTime createdTimestamp() const;
    QString lastUpdatedBy() const;
    QDateTime lastUpdatedTimestamp() const;
    void setCreated(const QString &createdBy, const QDateTime &createdTimestamp);
    void setUpdated(const QString &updatedBy, const QDateTime &updatedTimestamp);

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
