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

CorpusObject::Type SQLSerialiserSystem::corpusObjectTypeFromCode(const QString &code)
{
    CorpusObject::Type objectType = CorpusObject::Type_Undefined;
    if      (code == "X") objectType = CorpusObject::Type_Corpus;
    else if (code == "C") objectType = CorpusObject::Type_Communication;
    else if (code == "S") objectType = CorpusObject::Type_Speaker;
    else if (code == "R") objectType = CorpusObject::Type_Recording;
    else if (code == "A") objectType = CorpusObject::Type_Annotation;
    else if (code == "P") objectType = CorpusObject::Type_Participation;
    return objectType;
}

QString SQLSerialiserSystem::corpusObjectCodeFromType(const CorpusObject::Type type)
{
    if      (type == CorpusObject::Type_Corpus)         return "X";
    else if (type == CorpusObject::Type_Communication)  return "C";
    else if (type == CorpusObject::Type_Speaker)        return "S";
    else if (type == CorpusObject::Type_Recording)      return "R";
    else if (type == CorpusObject::Type_Annotation)     return "A";
    else if (type == CorpusObject::Type_Participation)  return "P";
    return QString();
}

QString SQLSerialiserSystem::defaultSectionID(const CorpusObject::Type type)
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
