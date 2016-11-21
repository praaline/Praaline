#include "MocaDBSerialiserSystem.h"

namespace Praaline {
namespace Core {

// static
QString MocaDBSerialiserSystem::getMocaBaseTableForPraalineCorpusObjectType(CorpusObject::Type type)
{
    switch (type) {
    case CorpusObject::Type_Corpus:         return "data_corpus";       break;
    case CorpusObject::Type_Communication:  return "data_sample";       break;
    case CorpusObject::Type_Speaker:        return "data_speaker";      break;
    case CorpusObject::Type_Recording:      return "data_recording";    break;
    case CorpusObject::Type_Annotation:     return "data_annotation";   break;
    default:                                return QString();           break;
    }
    return QString();
}

// static
MocaDBSerialiserSystem::MocaDataType MocaDBSerialiserSystem::getMocaDataTypeIdForPraalineCorpusObjectType(CorpusObject::Type type)
{
    switch (type) {
    case CorpusObject::Type_Corpus:         return MocaDataType::Corpus;        break;
    case CorpusObject::Type_Communication:  return MocaDataType::Communication; break;
    case CorpusObject::Type_Speaker:        return MocaDataType::Speaker;       break;
    case CorpusObject::Type_Recording:      return MocaDataType::Recording;     break;
    case CorpusObject::Type_Annotation:     return MocaDataType::Annotation;    break;
    default:                                return MocaDataType::NotSupported;  break;
    }
    return MocaDataType::NotSupported;
    // Not handled: Type_Participation, Type_Bookmark, Type_Undefined
}

// static
CorpusObject::Type MocaDBSerialiserSystem::getPraalineCorpusObjectTypeForMocaDataTypeId(int mocaDataTypeId)
{
    if (mocaDataTypeId == static_cast<int>(MocaDataType::Corpus))           return CorpusObject::Type_Corpus;
    if (mocaDataTypeId == static_cast<int>(MocaDataType::Communication))    return CorpusObject::Type_Communication;
    if (mocaDataTypeId == static_cast<int>(MocaDataType::Speaker))          return CorpusObject::Type_Speaker;
    if (mocaDataTypeId == static_cast<int>(MocaDataType::Recording))        return CorpusObject::Type_Recording;
    if (mocaDataTypeId == static_cast<int>(MocaDataType::Annotation))       return CorpusObject::Type_Annotation;
    return CorpusObject::Type_Undefined;
}

// static
MocaDBSerialiserSystem::MocaValueType MocaDBSerialiserSystem::getMocaValueTypeIdForPraalineDataType(const DataType &datatype)
{
    switch (datatype.base()) {
    case DataType::Invalid:     return MocaValueType::Invalid;  break;
    case DataType::Char:        return MocaValueType::Text;     break;
    case DataType::VarChar:
        if (datatype.precision() <= 255)
            return MocaValueType::Text;
        else
            return MocaValueType::LongText;
        break;
    case DataType::Binary:      return MocaValueType::LongText; break;
    case DataType::VarBinary:   return MocaValueType::LongText; break;
    case DataType::Boolean:     return MocaValueType::Bool;     break;
    case DataType::SmallInt:    return MocaValueType::Integer;  break;
    case DataType::Integer:     return MocaValueType::Integer;  break;
    case DataType::BigInt:      return MocaValueType::Double;   break;
    case DataType::Decimal:     return MocaValueType::Double;   break;
    case DataType::Float:       return MocaValueType::Double;   break;
    case DataType::Double:      return MocaValueType::Double;   break;
    case DataType::Date:        return MocaValueType::DateTime; break;
    case DataType::Time:        return MocaValueType::DateTime; break;
    case DataType::DateTime:    return MocaValueType::DateTime; break;
    case DataType::Interval:    return MocaValueType::LongText; break;
    case DataType::Array:       return MocaValueType::LongText; break;
    case DataType::Multiset:    return MocaValueType::LongText; break;
    case DataType::Xml:         return MocaValueType::LongText; break;
    case DataType::Geometry:    return MocaValueType::Geometry; break;
    default:
        return MocaValueType::Invalid; break;
    }
    return MocaValueType::Invalid;
    // Notes: binary data should be UUEncoded to be saved in a Moca3-type database.
    // When exact correspondances do not exist, using upscaled data types.
}

// static
DataType MocaDBSerialiserSystem::getPraalineDataTypeForMocaValueTypeId(int mocaValueTypeId)
{
    if (mocaValueTypeId == static_cast<int>(MocaValueType::Bool))           return DataType(DataType::Boolean);
    if (mocaValueTypeId == static_cast<int>(MocaValueType::Text))           return DataType(DataType::VarChar, 255);
    if (mocaValueTypeId == static_cast<int>(MocaValueType::LongText))       return DataType(DataType::VarChar, 1024);
    if (mocaValueTypeId == static_cast<int>(MocaValueType::Integer))        return DataType(DataType::Integer);
    if (mocaValueTypeId == static_cast<int>(MocaValueType::DateTime))       return DataType(DataType::DateTime);
    if (mocaValueTypeId == static_cast<int>(MocaValueType::Double))         return DataType(DataType::Double);
    if (mocaValueTypeId == static_cast<int>(MocaValueType::Geometry))       return DataType(DataType::Geometry);
    return DataType::Invalid;
}

} // namespace Core
} // namespace Praaline
