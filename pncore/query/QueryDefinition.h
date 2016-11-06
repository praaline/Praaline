#ifndef QUERYDEFINITION_H
#define QUERYDEFINITION_H

#include "pncore_global.h"
#include <QString>
#include <QPair>
#include <QList>
#include "QueryFilterGroup.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT QueryDefinition
{
public:
    QueryDefinition();
    ~QueryDefinition();

    int longestSequenceLength() const;

    QString name;
    QList<QueryFilterGroup> filterGroups;
    QList<QPair<QString, QString> > resultLevelsAttributes;
};

} // namespace Core
} // namespace Praaline

#endif // QUERYDEFINITION_H
