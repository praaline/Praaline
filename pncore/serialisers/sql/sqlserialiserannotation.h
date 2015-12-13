#ifndef SQLSERIALISERANNOTATION_H
#define SQLSERIALISERANNOTATION_H

#include <QSqlDatabase>
#include "sqlserialiserbase.h"
#include "structure/annotationstructure.h"
#include "annotation/annotationtiergroup.h"
#include "annotation/annotationtier.h"

class SQLSerialiserAnnotation : public SQLSerialiserBase
{
public:
    static bool saveTiers(const QString &annotationID, const QString &speakerID,
                          AnnotationTierGroup *tiers, AnnotationStructure *structure, QSqlDatabase &db);
    static bool saveTier(const QString &annotationID, const QString &speakerID,
                         AnnotationTier *tier, AnnotationStructure *structure, QSqlDatabase &db);

    static QMap<QString, QPointer<AnnotationTierGroup> > getTiersAllSpeakers(const QString &annotationID,
                                                                             const AnnotationStructure *structure, QSqlDatabase &db,
                                                                             const QStringList &levelIDs = QStringList());
    static AnnotationTierGroup *getTiers(const QString &annotationID, const QString &speakerID,
                                         const AnnotationStructure *structure, QSqlDatabase &db,
                                         const QStringList &levelIDs = QStringList());
    static AnnotationTier *getTier(const QString &annotationID, const QString &speakerID,
                                   const AnnotationStructure *structure, QSqlDatabase &db,
                                   const QString &levelID, const QStringList &attributeIDs = QStringList());    
    static QList<Interval *> getIntervals(const QString &annotationID, const QString &speakerID,
                                          const AnnotationStructure *structure, QSqlDatabase &db,
                                          const QString &levelID, int intervalNoMin, int intervalNoMax,
                                          const QStringList &attributeIDs = QStringList());
    static QList<Interval *> getIntervals(const QString &annotationID, const QString &speakerID,
                                          const AnnotationStructure *structure, QSqlDatabase &db,
                                          const QString &levelID, RealTime tMin, RealTime tMax,
                                          const QStringList &attributeIDs = QStringList());

    static QList<QString> getSpeakersInLevel(const QString &annotationID, const QString &levelID,
                                             const AnnotationStructure *structure, QSqlDatabase &db);
    static QList<QString> getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID,
                                                   const AnnotationStructure *structure, QSqlDatabase &db);
    static QList<QString> getSpeakersInAnnotation(const QString &annotationID,
                                                  const AnnotationStructure *structure, QSqlDatabase &db);
    static QList<QString> getSpeakersActiveInAnnotation(const QString &annotationID,
                                                        const AnnotationStructure *structure, QSqlDatabase &db);

    static IntervalTier *getSpeakerTimeline(const QString &annotationID, const QString &levelID,
                                            const AnnotationStructure *structure, QSqlDatabase &db);
    static bool deleteAllForAnnotationID(QString annotationID, const AnnotationStructure *structure, QSqlDatabase &db);

    static QList<QPair<QList<QVariant>, long> > getDistinctLabels(const QString &levelID, const QStringList &attributeIDs,
                                                                 const AnnotationStructure *structure, QSqlDatabase &db);
    static bool batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                            const QList<QPair<QString, QVariant> > &criteria,
                            const AnnotationStructure *structure, QSqlDatabase &db);

    static QList<QPair<QList<QVariant>, long> > countItems(const QString &levelID, const QStringList &groupByAttributeIDs,
                                                           const AnnotationStructure *structure, QSqlDatabase &db);
private:
    SQLSerialiserAnnotation();
};

#endif // SQLSERIALISERANNOTATION_H
