#ifndef DATASTOREFACTORY_H
#define DATASTOREFACTORY_H

#include "pncore_global.h"
#include "abstractannotationdatastore.h"
#include "abstractmetadatadatastore.h"

class PRAALINE_CORE_SHARED_EXPORT DatastoreFactory
{
public:
    static AbstractAnnotationDatastore *getAnnotationDatastore(const DatastoreInfo &dsi, QPointer<AnnotationStructure> structure);
    static AbstractMetadataDatastore *getMetadataDatastore(const DatastoreInfo &dsi, QPointer<MetadataStructure> structure);

private:
    DatastoreFactory() {}
    ~DatastoreFactory() {}
};

#endif // DATASTOREFACTORY_H
