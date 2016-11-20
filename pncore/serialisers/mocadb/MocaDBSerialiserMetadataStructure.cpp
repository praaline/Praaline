#include "MocaDBSerialiserMetadataStructure.h"

#include <QSqlDatabase>
#include "structure/MetadataStructure.h"

namespace Praaline {
namespace Core {

// static
bool MocaDBSerialiserMetadataStructure::initialiseMetadataStructureTables(QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadataStructure::loadMetadataStructure(QPointer<MetadataStructure> structure)
{

}

// static
bool MocaDBSerialiserMetadataStructure::createMetadataAttribute(CorpusObject::Type type,
                                                                QPointer<MetadataStructureAttribute> newAttribute, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadataStructure::updateMetadataAttribute(CorpusObject::Type type,
                                                                QPointer<MetadataStructureAttribute> updatedAttribute, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadataStructure::renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID,
                                                                const QString &newAttributeID, QSqlDatabase &db)
{
    return false;
}

// static
bool MocaDBSerialiserMetadataStructure::deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID, QSqlDatabase &db)
{
    return false;
}

} // namespace Core
} // namespace Praaline
