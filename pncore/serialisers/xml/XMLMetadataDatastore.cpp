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
    Q_UNUSED(info)
    return false;
}

bool XMLMetadataDatastore::openDatastore(const DatastoreInfo &info)
{
    Q_UNUSED(info)
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

bool XMLMetadataDatastore::saveMetadataStructure()
{
    return false;
}

bool XMLMetadataDatastore::createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute)
{
    Q_UNUSED(type)
    Q_UNUSED(newAttribute)
    return false;
}

bool XMLMetadataDatastore::renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID)
{
    Q_UNUSED(type)
    Q_UNUSED(attributeID)
    Q_UNUSED(newAttributeID)
    return false;
}

bool XMLMetadataDatastore::deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID)
{
    Q_UNUSED(type)
    Q_UNUSED(attributeID)
    return false;
}

bool XMLMetadataDatastore::retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const DataType &newDatatype)
{
    Q_UNUSED(type)
    Q_UNUSED(attributeID)
    Q_UNUSED(newDatatype)
    return false;
}

// ==========================================================================================================================
// Name-value lists
// ==========================================================================================================================
NameValueList *XMLMetadataDatastore::getNameValueList(const QString &listID)
{
    Q_UNUSED(listID)
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
    Q_UNUSED(list)
    return false;
}

bool XMLMetadataDatastore::updateNameValueList(NameValueList *list)
{
    Q_UNUSED(list)
    return false;
}

bool XMLMetadataDatastore::deleteNameValueList(const QString &listID)
{
    Q_UNUSED(listID)
    return false;
}

// ==========================================================================================================================
// Corpus object info lists
// ==========================================================================================================================
QList<CorpusObjectInfo> XMLMetadataDatastore::getCorpusObjectInfoList(CorpusObject::Type type, const Selection &selection)
{
    Q_UNUSED(type)
    Q_UNUSED(selection)
    QList<CorpusObjectInfo> ret;
    return ret;
}

// ==========================================================================================================================
// Corpus
// ==========================================================================================================================
// Load metadata information
Corpus *XMLMetadataDatastore::getCorpus(const QString &corpusID)
{
    Q_UNUSED(corpusID)
    return Q_NULLPTR;
}

QList<QPointer<CorpusCommunication> > XMLMetadataDatastore::getCommunications(const Selection &selection)
{
    Q_UNUSED(selection)
    QList<QPointer<CorpusCommunication> > list;
    return list;
}

QList<QPointer<CorpusSpeaker> > XMLMetadataDatastore::getSpeakers(const Selection &selection)
{
    Q_UNUSED(selection)
    QList<QPointer<CorpusSpeaker> > list;
    return list;
}

QList<QPointer<CorpusRecording> > XMLMetadataDatastore::getRecordings(const Selection &selection)
{
    Q_UNUSED(selection)
    QList<QPointer<CorpusRecording> > list;
    return list;
}

QList<QPointer<CorpusAnnotation> > XMLMetadataDatastore::getAnnotations(const Selection &selection)
{
    Q_UNUSED(selection)
    QList<QPointer<CorpusAnnotation> > list;
    return list;
}

QList<QPointer<CorpusParticipation> > XMLMetadataDatastore::getParticipations(const Selection &selection)
{
    Q_UNUSED(selection)
    QList<QPointer<CorpusParticipation> > list;
    return list;
}

// Save (insert or update) corpus objects
bool XMLMetadataDatastore::saveCorpus(Corpus *corpus)
{
    Q_UNUSED(corpus)
    return false;
}

bool XMLMetadataDatastore::saveCommunication(CorpusCommunication *communication)
{
    Q_UNUSED(communication)
    return false;
}

bool XMLMetadataDatastore::saveCommunications(QList<QPointer<CorpusCommunication> > &communications)
{
    Q_UNUSED(communications)
    return false;
}

bool XMLMetadataDatastore::saveSpeaker(CorpusSpeaker *speaker)
{
    Q_UNUSED(speaker)
    return false;
}

bool XMLMetadataDatastore::saveSpeakers(QList<QPointer<CorpusSpeaker> > &speakers)
{
    Q_UNUSED(speakers)
    return false;
}

bool XMLMetadataDatastore::saveRecordings(QList<QPointer<CorpusRecording> > &recordings)
{
    Q_UNUSED(recordings)
    return false;
}

bool XMLMetadataDatastore::saveAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations)
{
    Q_UNUSED(annotations)
    return false;
}

bool XMLMetadataDatastore::saveParticipations(QList<QPointer<CorpusParticipation> >  &participations)
{
    Q_UNUSED(participations)
    return false;
}

// Delete corpus objects
bool XMLMetadataDatastore::deleteCorpus(const QString &corpusID)
{
    Q_UNUSED(corpusID)
    return false;
}

bool XMLMetadataDatastore::deleteCommunication(const QString &communicationID)
{
    Q_UNUSED(communicationID)
    return false;
}

bool XMLMetadataDatastore::deleteSpeaker(const QString &speakerID)
{
    Q_UNUSED(speakerID)
    return false;
}

bool XMLMetadataDatastore::deleteRecording(const QString &recordingID)
{
    Q_UNUSED(recordingID)
    return false;
}

bool XMLMetadataDatastore::deleteAnnotation(const QString &annotationID)
{
    Q_UNUSED(annotationID)
    return false;
}

bool XMLMetadataDatastore::deleteParticipation(const QString &communicationID, const QString &speakerID)
{
    Q_UNUSED(communicationID)
    Q_UNUSED(speakerID)
    return false;
}

} // namespace Core
} // namespace Praaline
