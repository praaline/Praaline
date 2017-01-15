#include "DatastoreFactory.h"
#include "AnnotationDatastore.h"
#include "MetadataDatastore.h"
#include "DatastoreInfo.h"
#include "CorpusRepository.h"
#include "structure/AnnotationStructure.h"
#include "structure/MetadataStructure.h"
#include "serialisers/sql/SQLAnnotationDatastore.h"
#include "serialisers/sql/SQLMetadataDatastore.h"

namespace Praaline {
namespace Core {

// static
AnnotationDatastore *DatastoreFactory::getAnnotationDatastore(const DatastoreInfo &dsi, AnnotationStructure *structure, CorpusRepository *repository)
{
    if (dsi.type == DatastoreInfo::SQL) {
        return new SQLAnnotationDatastore(structure, repository, repository);
    }
    return 0;
}

// static
MetadataDatastore *DatastoreFactory::getMetadataDatastore(const DatastoreInfo &dsi, MetadataStructure *structure, CorpusRepository *repository)
{
    if (dsi.type == DatastoreInfo::SQL) {
        return new SQLMetadataDatastore(structure, repository, repository);
    }
    return 0;
}

} // namespace Core
} // namespace Praaline

