#ifndef SQLSERIALISERSYSTEM_H
#define SQLSERIALISERSYSTEM_H

#include <QString>
#include <corpus/CorpusObject.h>

namespace Praaline {
namespace Core {

class SQLSerialiserSystem
{
public:
    static QString tableNameForCorpusObjectType(CorpusObject::Type type);
    static CorpusObject::Type corpusObjectTypeFromCode(const QString &code);
    static QString corpusObjectCodeFromType(const CorpusObject::Type type);
    static QString defaultSectionID(const CorpusObject::Type type);

private:
    SQLSerialiserSystem() {}
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERSYSTEM_H
