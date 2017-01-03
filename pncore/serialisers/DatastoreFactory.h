#ifndef DATASTOREFACTORY_H
#define DATASTOREFACTORY_H

#include "pncore_global.h"
#include "AnnotationDatastore.h"
#include "MetadataDatastore.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT DatastoreFactory
{
public:
    static AnnotationDatastore *getAnnotationDatastore(const DatastoreInfo &dsi, QPointer<AnnotationStructure> structure);
    static MetadataDatastore *getMetadataDatastore(const DatastoreInfo &dsi, QPointer<MetadataStructure> structure);

private:
    DatastoreFactory() {}
    ~DatastoreFactory() {}
};

} // namespace Core
} // namespace Praaline

#endif // DATASTOREFACTORY_H
