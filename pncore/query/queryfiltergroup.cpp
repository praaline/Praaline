#include "queryfiltergroup.h"

QueryFilterGroup::QueryFilterGroup()
{

}

QueryFilterGroup::~QueryFilterGroup()
{

}

int QueryFilterGroup::longestSequenceLength() const
{
    int length = 0;
    foreach (QueryFilterSequence filterSequence, filterSequences) {
        if (length < filterSequence.sequenceLength())
            length = filterSequence.sequenceLength();
    }
    return length;
}
