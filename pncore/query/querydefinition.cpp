#include "querydefinition.h"

QueryDefinition::QueryDefinition()
{

}

QueryDefinition::~QueryDefinition()
{

}

int QueryDefinition::longestSequenceLength() const
{
    int length = 0;
    foreach (QueryFilterGroup filterGroup, filterGroups) {
        if (length < filterGroup.longestSequenceLength())
            length = filterGroup.longestSequenceLength();
    }
    return length;
}
