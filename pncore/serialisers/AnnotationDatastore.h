#ifndef ANNOTATIONDATASTORE_H
#define ANNOTATIONDATASTORE_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QList>
#include <QPair>
#include <QMap>

#include "DatastoreInfo.h"
#include "base/DataType.h"
#include "structure/AnnotationStructure.h"
#include "annotation/AnnotationTierGroup.h"
#include "query/QueryDefinition.h"
#include "query/QueryOccurrence.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT AnnotationDatastore : public QObject
{
    Q_OBJECT
public:
    class Selection {
    public:
        QString annotationID;
        QString speakerID;
        QString levelID;
        QStringList attributeIDs;
        int indexMin;
        int indexMax;
        RealTime timeMin;
        RealTime timeMax;

        Selection(const QString &annotationID, const QString &speakerID, const QString &levelID) :
            annotationID(annotationID), speakerID(speakerID), levelID(levelID),
            indexMin(-1), indexMax(-1), timeMin(RealTime(-1, 0)), timeMax(RealTime(-1, 0)) {}
        Selection(const QString &annotationID, const QString &speakerID, const QString &levelID, const QStringList &attributeIDs) :
            annotationID(annotationID), speakerID(speakerID), levelID(levelID), attributeIDs(attributeIDs),
            indexMin(-1), indexMax(-1), timeMin(RealTime(-1, 0)), timeMax(RealTime(-1, 0)) {}
        Selection(const QString &annotationID, const QString &speakerID, const QString &levelID, int indexMin, int indexMax) :
            annotationID(annotationID), speakerID(speakerID), levelID(levelID),
            indexMin(indexMin), indexMax(indexMax), timeMin(RealTime(-1, 0)), timeMax(RealTime(-1, 0)) {}
        Selection(const QString &annotationID, const QString &speakerID, const QString &levelID, const QStringList &attributeIDs,
                  int indexMin, int indexMax) :
            annotationID(annotationID), speakerID(speakerID), levelID(levelID), attributeIDs(attributeIDs),
            indexMin(indexMin), indexMax(indexMax), timeMin(RealTime(-1, 0)), timeMax(RealTime(-1, 0)) {}
        Selection(const QString &annotationID, const QString &speakerID, const QString &levelID, RealTime timeMin, RealTime timeMax) :
            annotationID(annotationID), speakerID(speakerID), levelID(levelID),
            indexMin(-1), indexMax(-1), timeMin(timeMin), timeMax(timeMax) {}
        Selection(const QString &annotationID, const QString &speakerID, const QString &levelID, const QStringList &attributeIDs,
                  RealTime timeMin, RealTime timeMax) :
            annotationID(annotationID), speakerID(speakerID), levelID(levelID), attributeIDs(attributeIDs),
            indexMin(-1), indexMax(-1), timeMin(timeMin), timeMax(timeMax) {}
    };

    AnnotationDatastore(QObject *parent = 0) : QObject(parent) {}
    virtual ~AnnotationDatastore() {}

    // ==========================================================================================================================
    // Datastore
    // ==========================================================================================================================
    virtual bool createDatastore(const DatastoreInfo &info) = 0;
    virtual bool openDatastore(const DatastoreInfo &info) = 0;
    virtual bool closeDatastore() = 0;

    // ==========================================================================================================================
    // Annotation Structure
    // ==========================================================================================================================
    virtual bool loadAnnotationStructure() = 0;
    virtual bool saveAnnotationStructure() = 0;
    virtual bool createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel) = 0;
    virtual bool renameAnnotationLevel(const QString &levelID, const QString &newLevelID) = 0;
    virtual bool deleteAnnotationLevel(const QString &levelID) = 0;
    virtual bool createAnnotationAttribute(const QString &levelID, QPointer<AnnotationStructureAttribute> newAttribute) = 0;
    virtual bool renameAnnotationAttribute(const QString &levelID, const QString &attributeID, const QString &newAttributeID) = 0;
    virtual bool deleteAnnotationAttribute(const QString &levelID, const QString &attributeID) = 0;
    virtual bool retypeAnnotationAttribute(const QString &levelID, const QString &attributeID, const DataType &newDatatype) = 0;

    // ==========================================================================================================================
    // Annotation Tiers
    // ==========================================================================================================================
    virtual AnnotationTier *getTier(const QString &annotationID, const QString &speakerID, const QString &levelID,
                                    const QStringList &attributeIDs = QStringList()) = 0;
    virtual AnnotationTierGroup *getTiers(const QString &annotationID, const QString &speakerID,
                                          const QStringList &levelIDs = QStringList()) = 0;
    virtual QMap<QString, QPointer<AnnotationTierGroup> > getTiersAllSpeakers(const QString &annotationID,
                                                                              const QStringList &levelIDs = QStringList()) = 0;
    virtual bool saveTier(const QString &annotationID, const QString &speakerID, AnnotationTier *tier) = 0;
    virtual bool saveTiers(const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers) = 0;
    virtual bool saveTiersAllSpeakers(const QString &annotationID, QMap<QString, QPointer<AnnotationTierGroup> > &tiersAllSpeakers) = 0;
    virtual bool deleteTier(const QString &annotationID, const QString &speakerID, const QString &levelID) = 0;
    virtual bool deleteTiers(const QString &annotationID, const QString &speakerID, const QStringList &levelIDs = QStringList()) = 0;
    virtual bool deleteAllTiersAllSpeakers(const QString &annotationID) = 0;

    // ==========================================================================================================================
    // Annotation Elements
    // ==========================================================================================================================
    virtual QList<AnnotationElement *> getAnnotationElements(const Selection &selection) = 0;
    virtual QList<Interval *> getIntervals(const Selection &selection) = 0;
    virtual QList<Point *> getPoints(const Selection &selection) = 0;

    // ==========================================================================================================================
    // Speakers and Timeline
    // ==========================================================================================================================
    virtual QList<QString> getSpeakersInLevel(const QString &annotationID, const QString &levelID) = 0;
    virtual QList<QString> getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID) = 0;
    virtual QList<QString> getSpeakersInAnnotation(const QString &annotationID) = 0;
    virtual QList<QString> getSpeakersActiveInAnnotation(const QString &annotationID) = 0;
    virtual IntervalTier *getSpeakerTimeline(const QString &communicationID, const QString &annotationID,
                                             const QString &levelID, bool detailed = false) = 0;

    // ==========================================================================================================================
    // Queries
    // ==========================================================================================================================
    virtual QList<QueryOccurrencePointer *> runQuery(QueryDefinition *qdef) = 0;
    virtual QueryOccurrence *getOccurrence(QueryOccurrencePointer *pointer, QueryDefinition *qdef) = 0;
    virtual bool updateAnnotationsFromQueryOccurrences(const QList<QueryOccurrence *> &occurrences) = 0;

    // ==========================================================================================================================
    // Batch Processing
    // ==========================================================================================================================
    virtual QList<QPair<QList<QVariant>, long> > getDistinctLabels(const QString &levelID, const QStringList &attributeIDs) = 0;
    virtual bool batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                             const QList<QPair<QString, QVariant> > &criteria) = 0;

    // ==========================================================================================================================
    // Statistics
    // ==========================================================================================================================
    virtual QList<QPair<QList<QVariant>, long> > countItems(const QString &levelID, const QStringList &groupByAttributeIDs) = 0;

    // ==========================================================================================================================
    // Error handling
    // ==========================================================================================================================
    virtual QString lastError() const { return m_lastError; }
    virtual void clearError() { m_lastError.clear(); }

protected:
    QString m_lastError;
};

} // namespace Core
} // namespace Praaline

#endif // ANNOTATIONDATASTORE_H
