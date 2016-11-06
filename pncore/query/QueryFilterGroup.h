#ifndef QUERYFILTERGROUP_H
#define QUERYFILTERGROUP_H

#include <QString>
#include <QList>
#include "pncore_global.h"
#include "QueryFilterSequence.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT QueryFilterGroup
{
public:
    QueryFilterGroup();
    ~QueryFilterGroup();

    QString name;
    QList<QueryFilterSequence> filterSequences;

    int longestSequenceLength() const;
};

} // namespace Core
} // namespace Praaline

#endif // QUERYFILTERGROUP_H
