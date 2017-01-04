#ifndef SQLANNOTATIONDATASTORE_H
#define SQLANNOTATIONDATASTORE_H

#include <QObject>
#include <QSqlDatabase>
#include "serialisers/AnnotationDatastore.h"

namespace Praaline {
namespace Core {

class SQLAnnotationDatastore : public AnnotationDatastore
{
    Q_OBJECT
public:
    explicit SQLAnnotationDatastore(QPointer<AnnotationStructure> structure, QObject *parent = 0);
    ~SQLAnnotationDatastore();

    // ==========================================================================================================================
    // Datastore
    // ==========================================================================================================================
    bool createDatastore(const DatastoreInfo &info);
    bool openDatastore(const DatastoreInfo &info);
    bool closeDatastore();

    // ==========================================================================================================================
    // Annotation Structure
    // ==========================================================================================================================
    bool loadAnnotationStructure();
    bool saveAnnotationStructure();
    bool createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel);
    bool renameAnnotationLevel(const QString &levelID, const QString &newLevelID);
    bool deleteAnnotationLevel(const QString &levelID);
    bool createAnnotationAttribute(const QString &levelID, QPointer<AnnotationStructureAttribute> newAttribute);
    bool renameAnnotationAttribute(const QString &levelID, const QString &attributeID, const QString &newAttributeID);
    bool deleteAnnotationAttribute(const QString &levelID, const QString &attributeID);
    bool retypeAnnotationAttribute(const QString &levelID, const QString &attributeID, const DataType &newDatatype);

    // ==========================================================================================================================
    // Annotation Tiers
    // ==========================================================================================================================
    AnnotationTier *getTier(const QString &annotationID, const QString &speakerID,
                            const QString &levelID, const QStringList &attributeIDs = QStringList());
    AnnotationTierGroup *getTiers(const QString &annotationID, const QString &speakerID,
                                  const QStringList &levelIDs = QStringList());
    QMap<QString, QPointer<AnnotationTierGroup> > getTiersAllSpeakers(const QString &annotationID,
                                                                      const QStringList &levelIDs = QStringList());
    bool saveTier(const QString &annotationID, const QString &speakerID, AnnotationTier *tier);
    bool saveTiers(const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers);
    bool saveTiersAllSpeakers(const QString &annotationID, QMap<QString, QPointer<AnnotationTierGroup> > &tiersAllSpeakers);
    bool deleteTier(const QString &annotationID, const QString &speakerID, const QString &levelID);
    bool deleteTiers(const QString &annotationID, const QString &speakerID, const QStringList &levelIDs = QStringList());
    bool deleteAllTiersAllSpeakers(const QString &annotationID);

    // ==========================================================================================================================
    // Annotation Elements
    // ==========================================================================================================================
    QList<AnnotationElement *> getAnnotationElements(const Selection &selection);
    QList<Point *> getPoints(const Selection &selection);
    QList<Interval *> getIntervals(const Selection &selection);
    QList<Sequence *> getSequences(const Selection &selection);

    // ==========================================================================================================================
    // Speakers and Timeline
    // ==========================================================================================================================
    QList<QString> getSpeakersInLevel(const QString &annotationID, const QString &levelID);
    QList<QString> getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID);
    QList<QString> getSpeakersInAnnotation(const QString &annotationID);
    QList<QString> getSpeakersActiveInAnnotation(const QString &annotationID);
    IntervalTier *getSpeakerTimeline(const QString &communicationID,const QString &annotationID,
                                     const QString &levelID, bool detailed = false);

    // ==========================================================================================================================
    // Queries
    // ==========================================================================================================================
    QList<QueryOccurrencePointer *> runQuery(QueryDefinition *qdef);
    QueryOccurrence *getOccurrence(QueryOccurrencePointer *pointer, QueryDefinition *qdef);
    bool updateAnnotationsFromQueryOccurrences(const QList<QueryOccurrence *> &occurrences);

    // ==========================================================================================================================
    // Batch Processing
    // ==========================================================================================================================
    QList<QPair<QList<QVariant>, long> > getDistinctLabels(const QString &levelID, const QStringList &attributeIDs);
    bool batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                     const QList<QPair<QString, QVariant> > &criteria);

    // ==========================================================================================================================
    // Statistics
    // ==========================================================================================================================
    QList<QPair<QList<QVariant>, long> > countItems(const QString &levelID, const QStringList &groupByAttributeIDs);

private:
    QSqlDatabase m_database;
    QPointer<AnnotationStructure> m_structure;
};

} // namespace Core
} // namespace Praaline

#endif // SQLANNOTATIONDATASTORE_H
