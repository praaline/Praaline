#include "SQLSerialiserSystem.h"

namespace Praaline {
namespace Core {

// static
QString SQLSerialiserSystem::tableNameForCorpusObjectType(CorpusObject::Type type)
{
    if      (type == CorpusObject::Type_Corpus)         return "corpus";
    else if (type == CorpusObject::Type_Communication)  return "communication";
    else if (type == CorpusObject::Type_Speaker)        return "speaker";
    else if (type == CorpusObject::Type_Recording)      return "recording";
    else if (type == CorpusObject::Type_Annotation)     return "annotation";
    else if (type == CorpusObject::Type_Participation)  return "participation";
    return QString();
}

} // namespace Core
} // namespace Praaline
