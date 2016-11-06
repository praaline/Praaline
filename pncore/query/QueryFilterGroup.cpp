#include "QueryFilterGroup.h"

namespace Praaline {
namespace Core {

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

} // namespace Core
} // namespace Praaline
