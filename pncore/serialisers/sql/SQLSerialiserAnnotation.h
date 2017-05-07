#ifndef SQLSERIALISERANNOTATION_H
#define SQLSERIALISERANNOTATION_H

#include <QSqlDatabase>
#include "base/IDatastore.h"
#include "SQLSerialiserBase.h"
#include "annotation/AnnotationElement.h"
#include "annotation/Point.h"
#include "annotation/Interval.h"
#include "annotation/Sequence.h"
#include "annotation/Relation.h"
#include "annotation/AnnotationTier.h"
#include "annotation/AnnotationTierGroup.h"
#include "structure/AnnotationStructure.h"
#include "datastore/AnnotationDatastore.h"

namespace Praaline {
namespace Core {

class SQLSerialiserAnnotation : public SQLSerialiserBase, public IDatastore
{
public:
    // ==========================================================================================================================
    // Annotation Elements
    // ==========================================================================================================================
    static QList<AnnotationElement *> getAnnotationElements(
            const AnnotationDatastore::Selection &selection, AnnotationStructure *structure, QSqlDatabase &db);
    static QList<Point *> getPoints(
            const AnnotationDatastore::Selection &selection, AnnotationStructure *structure, QSqlDatabase &db);
    static QList<Interval *> getIntervals(
            const AnnotationDatastore::Selection &selection, AnnotationStructure *structure, QSqlDatabase &db);
    static QList<Sequence *> getSequences(
            const AnnotationDatastore::Selection &selection, AnnotationStructure *structure, QSqlDatabase &db);
    static bool saveAnnotationElements(
            const QList<AnnotationElement *> &elements, const QString &levelID, const QStringList &attributeIDs,
            AnnotationStructure *structure, QSqlDatabase &db);

    // ==========================================================================================================================
    // Annotation Tiers
    // ==========================================================================================================================
    static AnnotationTier *getTier(
            const QString &annotationID, const QString &speakerID, const QString &levelID, const QStringList &attributeIDs,
            AnnotationStructure *structure, QSqlDatabase &db);
    static AnnotationTierGroup *getTiers(
            const QString &annotationID, const QString &speakerID, const QStringList &levelIDs,
            AnnotationStructure *structure, QSqlDatabase &db);
    static QMap<QString, QPointer<AnnotationTierGroup> > getTiersAllSpeakers(
            const QString &annotationID, const QStringList &levelIDs, AnnotationStructure *structure, QSqlDatabase &db);
    static bool saveTier(const QString &annotationID, const QString &speakerID, AnnotationTier *tier,
                         AnnotationStructure *structure, QSqlDatabase &db);
    static bool saveTiers(const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers,
                          AnnotationStructure *structure, QSqlDatabase &db);
    static bool deleteTier(const QString &annotationID, const QString &speakerID, const QString &levelID,
                           AnnotationStructure *structure, QSqlDatabase &db);
    static bool deleteAllTiersAllSpeakers(const QString &annotationID, AnnotationStructure *structure, QSqlDatabase &db);

    // ==========================================================================================================================
    // Speakers and Timeline
    // ==========================================================================================================================
    static QList<QString> getSpeakersInLevel(
            const QString &annotationID, const QString &levelID,
            AnnotationStructure *structure, QSqlDatabase &db, bool filter = false);
    static QList<QString> getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID,
                                                   AnnotationStructure *structure, QSqlDatabase &db);
    static QList<QString> getSpeakersInAnnotation(const QString &annotationID,
                                                  AnnotationStructure *structure, QSqlDatabase &db);
    static QList<QString> getSpeakersActiveInAnnotation(const QString &annotationID,
                                                        AnnotationStructure *structure, QSqlDatabase &db);
    static IntervalTier *getSpeakerTimeline(const QString &communicationID, const QString &annotationID,
                                            const QString &levelID, bool detailed,
                                            AnnotationStructure *structure, QSqlDatabase &db);

    // ==========================================================================================================================
    // Batch Processing
    // ==========================================================================================================================
    static QList<QPair<QList<QVariant>, long> > getDistinctLabels(const QString &levelID, const QStringList &attributeIDs,
                                                                 AnnotationStructure *structure, QSqlDatabase &db);
    static bool batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                            const QList<QPair<QString, QVariant> > &criteria,
                            AnnotationStructure *structure, QSqlDatabase &db);

    // ==========================================================================================================================
    // Statistics
    // ==========================================================================================================================
    static QList<QPair<QList<QVariant>, long long> > countItems(
            const QString &levelID, const QStringList &groupByAttributeIDs, bool excludeNULL, QStringList excludeValues,
            AnnotationStructure *structure, QSqlDatabase &db);

private:
    SQLSerialiserAnnotation();
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERANNOTATION_H
