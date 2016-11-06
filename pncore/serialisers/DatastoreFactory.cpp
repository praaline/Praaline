#include "DatastoreFactory.h"
#include "sql/SQLAnnotationDatastore.h"
#include "sql/SQLMetadataDatastore.h"

namespace Praaline {
namespace Core {

// static
AbstractAnnotationDatastore *DatastoreFactory::getAnnotationDatastore(const DatastoreInfo &dsi, QPointer<AnnotationStructure> structure)
{
    if (dsi.type == DatastoreInfo::SQL) {
        return new SQLAnnotationDatastore(structure);
    }
    return 0;
}

// static
AbstractMetadataDatastore *DatastoreFactory::getMetadataDatastore(const DatastoreInfo &dsi, QPointer<MetadataStructure> structure)
{
    if (dsi.type == DatastoreInfo::SQL) {
        return new SQLMetadataDatastore(structure);
    }
    return 0;
}

} // namespace Core
} // namespace Praaline
