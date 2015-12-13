#ifndef QUERYFILTERGROUP_H
#define QUERYFILTERGROUP_H

#include <QString>
#include <QList>
#include "pncore_global.h"
#include "queryfiltersequence.h"

class PRAALINE_CORE_SHARED_EXPORT QueryFilterGroup
{
public:
    QueryFilterGroup();
    ~QueryFilterGroup();

    QString name;
    QList<QueryFilterSequence> filterSequences;

    int longestSequenceLength() const;
};

#endif // QUERYFILTERGROUP_H
