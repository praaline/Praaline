#include "corpus/CorpusObject.h"
#include "corpus/CorpusObjectInfo.h"
#include "structure/MetadataStructure.h"
#include "structure/MetadataStructureAttribute.h"
#include "datastore/DatastoreInfo.h"
#include "datastore/CorpusRepository.h"
#include "XMLMetadataDatastore.h"

namespace Praaline {
namespace Core {

XMLMetadataDatastore::XMLMetadataDatastore(MetadataStructure *structure, CorpusRepository *repository, QObject *parent) :
    MetadataDatastore(repository, parent), m_structure(structure)
{
}

XMLMetadataDatastore::~XMLMetadataDatastore()
{
}

// ==========================================================================================================================
// Datastore
// ==========================================================================================================================
bool XMLMetadataDatastore::createDatastore(const DatastoreInfo &info)
{
    return false;
}

bool XMLMetadataDatastore::openDatastore(const DatastoreInfo &info)
{
    return false;
}

bool XMLMetadataDatastore::closeDatastore()
{
    return false;
}

// ==========================================================================================================================
// Metadata Structure
// ==========================================================================================================================
bool XMLMetadataDatastore::loadMetadataStructure()
{
    return false;
}

bool XMLMetadataDatastore::createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute)
{
    return false;
}

bool XMLMetadataDatastore::renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID)
{
    return false;
}

bool XMLMetadataDatastore::deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID)
{
    return false;
}

bool XMLMetadataDatastore::retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const DataType &newDatatype)
{
    return false;
}

// ==========================================================================================================================
// Name-value lists
// ==========================================================================================================================
NameValueList *XMLMetadataDatastore::getNameValueList(const QString &listID)
{
    return Q_NULLPTR;
}

QStringList XMLMetadataDatastore::getAllNameValueListIDs()
{
    return QStringList();
}

QMap<QString, QPointer<NameValueList> > XMLMetadataDatastore::getAllNameValueLists()
{
    QMap<QString, QPointer<NameValueList> >  ret;
    return ret;
}

bool XMLMetadataDatastore::createNameValueList(NameValueList *list)
{
    return false;
}

bool XMLMetadataDatastore::updateNameValueList(NameValueList *list)
{
    return false;
}

bool XMLMetadataDatastore::deleteNameValueList(const QString &listID)
{
    return false;
}

// ==========================================================================================================================
// Corpus object info lists
// ==========================================================================================================================
QList<CorpusObjectInfo> XMLMetadataDatastore::getCorpusObjectInfoList(CorpusObject::Type type, const Selection &selection)
{
    QList<CorpusObjectInfo> ret;
    return ret;
}

// ==========================================================================================================================
// Corpus
// ==========================================================================================================================
// Load metadata information
Corpus *XMLMetadataDatastore::getCorpus(const QString &corpusID)
{
    return Q_NULLPTR;
}

QList<QPointer<CorpusCommunication> > XMLMetadataDatastore::getCommunications(const Selection &selection)
{
    QList<QPointer<CorpusCommunication> > list;
    return list;
}

QList<QPointer<CorpusSpeaker> > XMLMetadataDatastore::getSpeakers(const Selection &selection)
{
    QList<QPointer<CorpusSpeaker> > list;
    return list;
}

QList<QPointer<CorpusRecording> > XMLMetadataDatastore::getRecordings(const Selection &selection)
{
    QList<QPointer<CorpusRecording> > list;
    return list;
}

QList<QPointer<CorpusAnnotation> > XMLMetadataDatastore::getAnnotations(const Selection &selection)
{
    QList<QPointer<CorpusAnnotation> > list;
    return list;
}

QList<QPointer<CorpusParticipation> > XMLMetadataDatastore::getParticipations(const Selection &selection)
{
    QList<QPointer<CorpusParticipation> > list;
    return list;
}

// Save (insert or update) corpus objects
bool XMLMetadataDatastore::saveCorpus(Corpus *corpus)
{
    return false;
}

bool XMLMetadataDatastore::saveCommunications(QList<QPointer<CorpusCommunication> > &communications)
{
    return false;
}

bool XMLMetadataDatastore::saveSpeakers(QList<QPointer<CorpusSpeaker> > &speakers)
{
    return false;
}

bool XMLMetadataDatastore::saveRecordings(QList<QPointer<CorpusRecording> > &recordings)
{
    return false;
}

bool XMLMetadataDatastore::saveAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations)
{
    return false;
}

bool XMLMetadataDatastore::saveParticipations(QList<QPointer<CorpusParticipation> >  &participations)
{
    return false;
}

// Delete corpus objects
bool XMLMetadataDatastore::deleteCorpus(const QString &corpusID)
{
    return false;
}

bool XMLMetadataDatastore::deleteCommunication(const QString &communicationID)
{
    return false;
}

bool XMLMetadataDatastore::deleteSpeaker(const QString &speakerID)
{
    return false;
}

bool XMLMetadataDatastore::deleteRecording(const QString &recordingID)
{
    return false;
}

bool XMLMetadataDatastore::deleteAnnotation(const QString &annotationID)
{
    return false;
}

bool XMLMetadataDatastore::deleteParticipation(const QString &communicationID, const QString &speakerID)
{
    return false;
}

} // namespace Core
} // namespace Praaline
