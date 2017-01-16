#ifndef SQLMETADATADATASTORE_H
#define SQLMETADATADATASTORE_H

#include <QObject>
#include "datastore/MetadataDatastore.h"

namespace Praaline {
namespace Core {

class MetadataStructure;
class CorpusRepository;

struct SQLMetadataDatastoreData;

class SQLMetadataDatastore : public MetadataDatastore
{
    Q_OBJECT
public:
    SQLMetadataDatastore(MetadataStructure *structure, CorpusRepository *repository, QObject *parent = 0);
    ~SQLMetadataDatastore();

    // ==========================================================================================================================
    // Datastore
    // ==========================================================================================================================
    bool createDatastore(const DatastoreInfo &info) override;
    bool openDatastore(const DatastoreInfo &info) override;
    bool closeDatastore() override;

    // ==========================================================================================================================
    // Metadata Structure
    // ==========================================================================================================================
    bool loadMetadataStructure() override;
    bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute) override;
    bool renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID) override;
    bool deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID) override;
    bool retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const DataType &newDatatype) override;

    // ==========================================================================================================================
    // Name-value lists
    // ==========================================================================================================================
    NameValueList *getNameValueList(const QString &listID) override;
    QStringList getAllNameValueListIDs() override;
    QMap<QString, QPointer<NameValueList> > getAllNameValueLists() override;
    bool createNameValueList(NameValueList *list) override;
    bool updateNameValueList(NameValueList *list) override;
    bool deleteNameValueList(const QString &listID) override;

    // ==========================================================================================================================
    // Corpus object info lists
    // ==========================================================================================================================
    QList<CorpusObjectInfo> getCorpusObjectInfoList(CorpusObject::Type type, const QString &parentID) override;

    // ==========================================================================================================================
    // Corpus
    // ==========================================================================================================================
    // Load metadata information in already created corpus objects
    bool loadCorpus(Corpus *corpus) override;
    bool loadCommunications(QList<QPointer<CorpusCommunication> > &communications) override;
    bool loadSpeakers(QList<QPointer<CorpusSpeaker> > &speakers) override;
    bool loadRecordings(QList<QPointer<CorpusRecording> > &recordings) override;
    bool loadAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations) override;

    // Save (insert or update) corpus objects
    bool saveCorpus(Corpus *corpus) override;
    bool saveCommunications(QList<QPointer<CorpusCommunication> > &communications) override;
    bool saveSpeakers(QList<QPointer<CorpusSpeaker> > &speakers) override;
    bool saveRecordings(QList<QPointer<CorpusRecording> > &recordings) override;
    bool saveAnnotations(QList<QPointer<CorpusAnnotation> >  &annotations) override;
    bool saveParticipations(QList<QPointer<CorpusParticipation> >  &participations) override;

    // Delete corpus objects
    bool deleteCorpus(const QString &corpusID) override;
    bool deleteCommunication(const QString &communicationID) override;
    bool deleteSpeaker(const QString &speakerID) override;
    bool deleteRecording(const QString &recordingID) override;
    bool deleteAnnotation(const QString &annotationID) override;
    bool deleteParticipation(const QString &communicationID, const QString &speakerID) override;

private:
    SQLMetadataDatastoreData *d;
};

} // namespace Core
} // namespace Praaline

#endif // SQLMETADATADATASTORE_H
