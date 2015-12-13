#include "xmlmetadatadatastore.h"

XMLMetadataDatastore::XMLMetadataDatastore(QPointer<MetadataStructure> structure, QObject *parent) :
    QObject(parent), m_structure(structure)
{

}

XMLMetadataDatastore::~XMLMetadataDatastore()
{
}

bool XMLMetadataDatastore::createDatastore(const DatastoreInfo &info)
{

}

bool XMLMetadataDatastore::openDatastore(const DatastoreInfo &info)
{

}

bool XMLMetadataDatastore::closeDatastore()
{

}

bool XMLMetadataDatastore::loadAnnotationStructure()
{

}

bool XMLMetadataDatastore::saveAnnotationStructure()
{

}

bool XMLMetadataDatastore::createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute)
{

}

bool XMLMetadataDatastore::renameMetadataAttribute(CorpusObject::Type type, QString attributeID, QString newAttributeID)
{

}

bool XMLMetadataDatastore::deleteMetadataAttribute(CorpusObject::Type type, QString attributeID)
{

}

bool XMLMetadataDatastore::loadCorpus(Corpus *corpus)
{

}

bool XMLMetadataDatastore::saveCorpus(Corpus *corpus)
{

}
