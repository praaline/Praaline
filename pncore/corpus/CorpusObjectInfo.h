#ifndef CORPUSOBJECTINFO_H
#define CORPUSOBJECTINFO_H

#include <QString>
#include "CorpusObject.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT CorpusObjectInfo
{
public:
    CorpusObjectInfo(const QString &ID, const QString &corpusID, const QString &name, CorpusObject::Type type);

    QString ID() const;
    QString corpusID() const;
    QString name() const;
    CorpusObject::Type type() const;

private:
    QString m_ID;
    QString m_corpusID;
    QString m_name;
    CorpusObject::Type m_type;
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSOBJECTINFO_H
