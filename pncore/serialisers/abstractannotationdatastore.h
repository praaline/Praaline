#ifndef ABSTRACTANNOTATIONDATASTORE_H
#define ABSTRACTANNOTATIONDATASTORE_H

#include "pncore_global.h"
#include <QObject>
#include <QPointer>
#include <QList>
#include <QPair>
#include <QMap>

#include "datastoreinfo.h"
#include "structure/annotationstructure.h"
#include "annotation/annotationtiergroup.h"
#include "query/querydefinition.h"
#include "query/queryoccurrence.h"

class PRAALINE_CORE_SHARED_EXPORT AbstractAnnotationDatastore : public QObject
{
    Q_OBJECT
public:
    AbstractAnnotationDatastore(QObject *parent = 0) : QObject(parent) {}
    virtual ~AbstractAnnotationDatastore() {}

    virtual bool createDatastore(const DatastoreInfo &info) = 0;
    virtual bool openDatastore(const DatastoreInfo &info) = 0;
    virtual bool closeDatastore() = 0;

    virtual bool loadAnnotationStructure() = 0;
    virtual bool saveAnnotationStructure() = 0;
    virtual bool createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel) = 0;
    virtual bool renameAnnotationLevel(QString levelID, QString newLevelID) = 0;
    virtual bool deleteAnnotationLevel(QString levelID) = 0;
    virtual bool createAnnotationAttribute(QString levelID, QPointer<AnnotationStructureAttribute> newAttribute) = 0;
    virtual bool renameAnnotationAttribute(QString levelID, QString attributeID, QString newAttributeID) = 0;
    virtual bool deleteAnnotationAttribute(QString levelID, QString attributeID) = 0;
    virtual bool retypeAnnotationAttribute(QString levelID, QString attributeID, QString newDatatype, int newDatalength) = 0;

    virtual bool saveTiersAllSpeakers(const QString &annotationID, QMap<QString, QPointer<AnnotationTierGroup> > &tiersAllSpeakers) = 0;
    virtual bool saveTiers(const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers) = 0;
    virtual bool saveTier(const QString &annotationID, const QString &speakerID, AnnotationTier *tier) = 0;

    virtual AnnotationTierGroup *getTiers(const QString &annotationID, const QString &speakerID,
                                          const QStringList &levelIDs = QStringList()) = 0;
    virtual QMap<QString, QPointer<AnnotationTierGroup> > getTiersAllSpeakers(const QString &annotationID,
                                                                              const QStringList &levelIDs = QStringList()) = 0;
    virtual AnnotationTier *getTier(const QString &annotationID, const QString &speakerID,
                                    const QString &levelID, const QStringList &attributeIDs = QStringList()) = 0;
    virtual QList<Interval *> getIntervals(const QString &annotationID, const QString &speakerID,
                                           const QString &levelID, int intervalNoMin, int intervalNoMax,
                                           const QStringList &attributeIDs = QStringList()) = 0;
    virtual QList<Interval *> getIntervals(const QString &annotationID, const QString &speakerID,
                                           const QString &levelID, RealTime tMin, RealTime tMax,
                                           const QStringList &attributeIDs = QStringList()) = 0;

    virtual QList<QString> getSpeakersInLevel(const QString &annotationID, const QString &levelID) = 0;
    virtual QList<QString> getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID) = 0;

    virtual QList<QString> getSpeakersInAnnotation(const QString &annotationID) = 0;
    virtual QList<QString> getSpeakersActiveInAnnotation(const QString &annotationID) = 0;

    virtual IntervalTier *getSpeakerTimeline(const QString &communicationID, const QString &annotationID,
                                             const QString &levelID) = 0;

    virtual bool deleteAllForAnnotationID(QString annotationID) = 0;

    virtual QList<QueryOccurrencePointer *> runQuery(QueryDefinition *qdef) = 0;
    virtual QueryOccurrence *getOccurrence(QueryOccurrencePointer *pointer, QueryDefinition *qdef) = 0;
    virtual bool updateAnnotationsFromQueryOccurrences(const QList<QueryOccurrence *> &occurrences) = 0;

    virtual QList<QPair<QList<QVariant>, long> > getDistinctLabels(const QString &levelID, const QStringList &attributeIDs) = 0;
    virtual bool batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                             const QList<QPair<QString, QVariant> > &criteria) = 0;

    virtual QList<QPair<QList<QVariant>, long> > countItems(const QString &levelID, const QStringList &groupByAttributeIDs) = 0;

    virtual QString lastError() const { return m_lastError; }
    virtual void clearError() { m_lastError.clear(); }

protected:
    QString m_lastError;
};

#endif // ABSTRACTANNOTATIONDATASTORE_H
