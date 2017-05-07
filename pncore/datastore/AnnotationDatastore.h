#ifndef ANNOTATIONDATASTORE_H
#define ANNOTATIONDATASTORE_H

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
#include <QString>
#include <QList>
#include <QStringList>
#include <QPair>
#include <QMap>
#include "base/IDatastore.h"
#include "CorpusDatastore.h"
#include "CorpusRepository.h"
#include "NameValueListDatastore.h"
#include "base/RealTime.h"
#include "base/DataType.h"

namespace Praaline {
namespace Core {

class DatastoreInfo;
class AnnotationStructureLevel;
class AnnotationStructureAttribute;
class NameValueList;
class AnnotationElement;
class Point;
class Interval;
class Sequence;
class Relation;
class AnnotationTier;
class AnnotationTierGroup;
class IntervalTier;
class QueryDefinition;
class QueryOccurrence;
struct QueryOccurrencePointer;

class PRAALINE_CORE_SHARED_EXPORT AnnotationDatastore : public CorpusDatastore, public IDatastore, public NameValueListDatastore
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

    AnnotationDatastore(CorpusRepository *repository, QObject *parent = 0) :
        CorpusDatastore(repository, parent) {}
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
    virtual bool createAnnotationLevel(AnnotationStructureLevel *newLevel) = 0;
    virtual bool renameAnnotationLevel(const QString &levelID, const QString &newLevelID) = 0;
    virtual bool deleteAnnotationLevel(const QString &levelID) = 0;
    virtual bool createAnnotationAttribute(const QString &levelID, AnnotationStructureAttribute *newAttribute) = 0;
    virtual bool renameAnnotationAttribute(const QString &levelID, const QString &attributeID, const QString &newAttributeID) = 0;
    virtual bool deleteAnnotationAttribute(const QString &levelID, const QString &attributeID) = 0;
    virtual bool retypeAnnotationAttribute(const QString &levelID, const QString &attributeID, const DataType &newDatatype) = 0;

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
    // Annotation Elements
    // ==========================================================================================================================
    virtual QList<AnnotationElement *> getAnnotationElements(const Selection &selection) = 0;
    virtual QList<Point *> getPoints(const Selection &selection) = 0;
    virtual QList<Interval *> getIntervals(const Selection &selection) = 0;
    virtual QList<Sequence *> getSequences(const Selection &selection) = 0;
    virtual bool saveAnnotationElements(const QList<AnnotationElement *> &elements, const QString &levelID,
                                        const QStringList &attributeIDs) = 0;

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
    // Speakers and Timeline
    // ==========================================================================================================================
    virtual QStringList getSpeakersInLevel(const QString &annotationID, const QString &levelID) = 0;
    virtual QStringList getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID) = 0;
    virtual QStringList getSpeakersInAnnotation(const QString &annotationID) = 0;
    virtual QStringList getSpeakersActiveInAnnotation(const QString &annotationID) = 0;
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
    virtual QList<QPair<QList<QVariant>, long long> > countItems(const QString &levelID, const QStringList &groupByAttributeIDs,
                                                                 bool excludeNULL, QStringList excludeValues) = 0;

};

} // namespace Core
} // namespace Praaline

#endif // ANNOTATIONDATASTORE_H
