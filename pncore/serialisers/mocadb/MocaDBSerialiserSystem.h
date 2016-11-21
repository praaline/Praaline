#ifndef MOCADBSERIALISERSYSTEM_H
#define MOCADBSERIALISERSYSTEM_H

#include "base/DataType.h"
#include "corpus/CorpusObject.h"

namespace Praaline {
namespace Core {

class MocaDBSerialiserSystem
{
public:
    enum MocaDataType {
        NotSupported    = 0,
        Corpus          = 1,
        Communication   = 2,
        Speaker         = 3,
        Recording       = 4,
        Annotation      = 5
    };

    enum MocaValueType {
        Invalid         = 0,
        Bool            = 1,
        Text            = 2,
        LongText        = 3,
        Integer         = 4,
        DateTime        = 5,
        Double          = 6,
        Geometry        = 7
    };

    static QString getMocaBaseTableForPraalineCorpusObjectType(CorpusObject::Type type);

    static MocaDataType getMocaDataTypeIdForPraalineCorpusObjectType(CorpusObject::Type type);
    static CorpusObject::Type getPraalineCorpusObjectTypeForMocaDataTypeId(int mocaDataTypeId);

    static MocaValueType getMocaValueTypeIdForPraalineDataType(const DataType &datatype);
    static DataType getPraalineDataTypeForMocaValueTypeId(int mocaValueTypeId);

private:
    MocaDBSerialiserSystem() {}
};

} // namespace Core
} // namespace Praaline

#endif // MOCADBSERIALISERSYSTEM_H
