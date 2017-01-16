#ifndef ABSTRACTMETADATADATASTORE_H
#define ABSTRACTMETADATADATASTORE_H

/*
    Praaline - Core module - Datastores
    Copyright (c) 2011-2017 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QList>
#include <QMap>
#include "base/DataType.h"
#include "base/IDatastore.h"
#include "CorpusDatastore.h"
#include "CorpusRepository.h"
#include "NameValueListDatastore.h"
#include "corpus/CorpusObject.h"

namespace Praaline {
namespace Core {

class DatastoreInfo;
class MetadataStructureAttribute;
class NameValueList;
class CorpusObjectInfo;
class Corpus;
class CorpusCommunication;
class CorpusSpeaker;
class CorpusParticipation;
class CorpusRecording;
class CorpusAnnotation;

class PRAALINE_CORE_SHARED_EXPORT MetadataDatastore : public CorpusDatastore, public NameValueListDatastore, public IDatastore
{
    Q_OBJECT
public:
    MetadataDatastore(CorpusRepository *repository, QObject *parent = 0) :
        CorpusDatastore(repository, parent) {}
    virtual ~MetadataDatastore() {}

    // ==========================================================================================================================
    // Datastore
    // ==========================================================================================================================
    virtual bool createDatastore(const DatastoreInfo &info) = 0;
    virtual bool openDatastore(const DatastoreInfo &info) = 0;
    virtual bool closeDatastore() = 0;

    // ==========================================================================================================================
    // Metadata Structure
    // ==========================================================================================================================
    virtual bool loadMetadataStructure() = 0;
    virtual bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute) = 0;
    virtual bool renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID) = 0;
    virtual bool deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID) = 0;
    virtual bool retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const DataType &newDatatype) = 0;

    // ==========================================================================================================================
    // Name-value lists
    // ==========================================================================================================================
    virtual NameValueList *getNameValueList(const QString &listID) = 0;
    virtual QStringList getAllNameValueListIDs() = 0;
    virtual QMap<QString, QPointer<NameValueList> > getAllNameValueLists() = 0;
    virtual bool createNameValueList(NameValueList *list) = 0;
    virtual bool updateNameValueList(NameValueList *list) = 0;
    virtual bool deleteNameValueList(const QString &listID) = 0;

    // ==========================================================================================================================
    // Corpus object info lists
    // ==========================================================================================================================
    virtual QList<CorpusObjectInfo> getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID) = 0;

    // ==========================================================================================================================
    // Corpus
    // ==========================================================================================================================
    // Load metadata information in already created corpus objects
    virtual bool loadCorpus(Corpus *corpus) = 0;
    virtual bool loadCommunications(QList<QPointer<CorpusCommunication> > &communications) = 0;
    virtual bool loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers) = 0;
    virtual bool loadRecordings(QList<QPointer<CorpusRecording> > &recordings) = 0;
    virtual bool loadAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations) = 0;

    // Save (insert or update) corpus objects
    virtual bool saveCorpus(Corpus *corpus) = 0;
    virtual bool saveCommunications(QList<QPointer<CorpusCommunication> > &communications) = 0;
    virtual bool saveSpeakers(QList<QPointer<CorpusSpeaker> > &speakers) = 0;
    virtual bool saveRecordings(QList<QPointer<CorpusRecording> > &recordings) = 0;
    virtual bool saveAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations) = 0;
    virtual bool saveParticipations(QList<QPointer<CorpusParticipation> >  &participations) = 0;

    // Delete corpus objects
    virtual bool deleteCorpus(const QString &corpusID) = 0;
    virtual bool deleteCommunication(const QString &communicationID) = 0;
    virtual bool deleteSpeaker(const QString &speakerID) = 0;
    virtual bool deleteRecording(const QString &recordingID) = 0;
    virtual bool deleteAnnotation(const QString &annotationID) = 0;
    virtual bool deleteParticipation(const QString &communicationID, const QString &speakerID) = 0;
};

} // namespace Core
} // namespace Praaline

#endif // ABSTRACTMETADATADATASTORE_H
