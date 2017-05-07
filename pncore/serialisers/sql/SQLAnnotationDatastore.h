#ifndef SQLANNOTATIONDATASTORE_H
#define SQLANNOTATIONDATASTORE_H

#include <QObject>
#include "datastore/AnnotationDatastore.h"

namespace Praaline {
namespace Core {

class AnnotationStructure;
class CorpusRepository;

struct SQLAnnotationDatastoreData;

class SQLAnnotationDatastore : public AnnotationDatastore
{
    Q_OBJECT
public:
    explicit SQLAnnotationDatastore(AnnotationStructure *structure, CorpusRepository *repository, QObject *parent = 0);
    ~SQLAnnotationDatastore();

    // ==========================================================================================================================
    // Datastore
    // ==========================================================================================================================
    bool createDatastore(const DatastoreInfo &info) override;
    bool openDatastore(const DatastoreInfo &info) override;
    bool closeDatastore() override;

    // ==========================================================================================================================
    // Annotation Structure
    // ==========================================================================================================================
    bool loadAnnotationStructure() override;
    bool saveAnnotationStructure() override;
    bool createAnnotationLevel(AnnotationStructureLevel *newLevel) override;
    bool renameAnnotationLevel(const QString &levelID, const QString &newLevelID) override;
    bool deleteAnnotationLevel(const QString &levelID) override;
    bool createAnnotationAttribute(const QString &levelID, AnnotationStructureAttribute *newAttribute) override;
    bool renameAnnotationAttribute(const QString &levelID, const QString &attributeID, const QString &newAttributeID) override;
    bool deleteAnnotationAttribute(const QString &levelID, const QString &attributeID) override;
    bool retypeAnnotationAttribute(const QString &levelID, const QString &attributeID, const DataType &newDatatype) override;

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
    // Annotation Tiers
    // ==========================================================================================================================
    AnnotationTier *getTier(const QString &annotationID, const QString &speakerID,
                            const QString &levelID, const QStringList &attributeIDs = QStringList()) override;
    AnnotationTierGroup *getTiers(const QString &annotationID, const QString &speakerID,
                                  const QStringList &levelIDs = QStringList()) override;
    QMap<QString, QPointer<AnnotationTierGroup> > getTiersAllSpeakers(const QString &annotationID,
                                                                      const QStringList &levelIDs = QStringList()) override;
    bool saveTier(const QString &annotationID, const QString &speakerID, AnnotationTier *tier) override;
    bool saveTiers(const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers) override;
    bool saveTiersAllSpeakers(const QString &annotationID, QMap<QString, QPointer<AnnotationTierGroup> > &tiersAllSpeakers) override;
    bool deleteTier(const QString &annotationID, const QString &speakerID, const QString &levelID) override;
    bool deleteTiers(const QString &annotationID, const QString &speakerID, const QStringList &levelIDs = QStringList()) override;
    bool deleteAllTiersAllSpeakers(const QString &annotationID) override;

    // ==========================================================================================================================
    // Annotation Elements
    // ==========================================================================================================================
    QList<AnnotationElement *> getAnnotationElements(const Selection &selection) override;
    QList<Point *> getPoints(const Selection &selection) override;
    QList<Interval *> getIntervals(const Selection &selection) override;
    QList<Sequence *> getSequences(const Selection &selection) override;
    bool saveAnnotationElements(const QList<AnnotationElement *> &elements, const QString &levelID,
                                const QStringList &attributeIDs) override;

    // ==========================================================================================================================
    // Speakers and Timeline
    // ==========================================================================================================================
    QStringList getSpeakersInLevel(const QString &annotationID, const QString &levelID) override;
    QStringList getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID) override;
    QStringList getSpeakersInAnnotation(const QString &annotationID) override;
    QStringList getSpeakersActiveInAnnotation(const QString &annotationID) override;
    IntervalTier *getSpeakerTimeline(const QString &communicationID,const QString &annotationID,
                                     const QString &levelID, bool detailed = false) override;

    // ==========================================================================================================================
    // Queries
    // ==========================================================================================================================
    QList<QueryOccurrencePointer *> runQuery(QueryDefinition *qdef) override;
    QueryOccurrence *getOccurrence(QueryOccurrencePointer *pointer, QueryDefinition *qdef) override;
    bool updateAnnotationsFromQueryOccurrences(const QList<QueryOccurrence *> &occurrences) override;

    // ==========================================================================================================================
    // Batch Processing
    // ==========================================================================================================================
    QList<QPair<QList<QVariant>, long> > getDistinctLabels(const QString &levelID, const QStringList &attributeIDs) override;
    bool batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                     const QList<QPair<QString, QVariant> > &criteria) override;

    // ==========================================================================================================================
    // Statistics
    // ==========================================================================================================================
    QList<QPair<QList<QVariant>, long long> > countItems(const QString &levelID, const QStringList &groupByAttributeIDs,
                                                         bool excludeNULL, QStringList excludeValues) override;

private:
    SQLAnnotationDatastoreData *d;
};

} // namespace Core
} // namespace Praaline

#endif // SQLANNOTATIONDATASTORE_H
