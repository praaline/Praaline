#include "xmlannotationdatastore.h"

XMLAnnotationDatastore::XMLAnnotationDatastore(QPointer<AnnotationStructure> structure, QObject *parent)
{
}

XMLAnnotationDatastore::~XMLAnnotationDatastore()
{
}

bool XMLAnnotationDatastore::createDatastore(const DatastoreInfo &info)
{
    return false;
}

bool XMLAnnotationDatastore::openDatastore(const DatastoreInfo &info)
{
    return false;
}

bool XMLAnnotationDatastore::closeDatastore()
{
    return false;
}

bool XMLAnnotationDatastore::loadAnnotationStructure()
{
    return false;
}

bool XMLAnnotationDatastore::saveAnnotationStructure()
{
    return false;
}

bool XMLAnnotationDatastore::createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel)
{
    return false;
}

bool XMLAnnotationDatastore::renameAnnotationLevel(QString levelID, QString newLevelID)
{
    return false;
}

bool XMLAnnotationDatastore::deleteAnnotationLevel(QString levelID)
{
    return false;
}

bool XMLAnnotationDatastore::createAnnotationAttribute(QString levelID, QPointer<AnnotationStructureAttribute> newAttribute)
{
    return false;
}

bool XMLAnnotationDatastore::renameAnnotationAttribute(QString levelID, QString attributeID, QString newAttributeID)
{
    return false;
}

bool XMLAnnotationDatastore::deleteAnnotationAttribute(QString levelID, QString attributeID)
{
    return false;
}

bool XMLAnnotationDatastore::retypeAnnotationAttribute(QString levelID, QString attributeID, QString newDatatype, int newDatalength)
{
    return false;
}

bool XMLAnnotationDatastore::saveTiersAllSpeakers(const QString &annotationID, QMap<QString, QPointer<AnnotationTierGroup> > &tiersAllSpeakers)
{
    return false;
}

bool XMLAnnotationDatastore::saveTiers(const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers)
{
    return false;
}

bool XMLAnnotationDatastore::saveTier(const QString &annotationID, const QString &speakerID, AnnotationTier *tier)
{
    return false;
}

AnnotationTierGroup *XMLAnnotationDatastore::getTiers(const QString &annotationID, const QString &speakerID, const QStringList &levelIDs)
{
    return 0;
}

QMap<QString, QPointer<AnnotationTierGroup> > XMLAnnotationDatastore::getTiersAllSpeakers(const QString &annotationID, const QStringList &levelIDs)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiers;
    return tiers;
}

AnnotationTier *XMLAnnotationDatastore::getTier(const QString &annotationID, const QString &speakerID, const QString &levelID, const QStringList &attributeIDs)
{
    return 0;
}

QList<Interval *> XMLAnnotationDatastore::getIntervals(const QString &annotationID, const QString &speakerID,
                                                       const QString &levelID, int intervalNoMin, int intervalNoMax,
                                                       const QStringList &attributeIDs)
{
    QList<Interval *> list;
    return list;
}

QList<Interval *> XMLAnnotationDatastore::getIntervals(const QString &annotationID, const QString &speakerID,
                                                       const QString &levelID, RealTime tMin, RealTime tMax,
                                                       const QStringList &attributeIDs)
{
    QList<Interval *> list;
    return list;
}

QList<QString> XMLAnnotationDatastore::getSpeakersInLevel(const QString &annotationID, const QString &levelID)
{
    QList<QString> speakers;
    return speakers;
}

QList<QString> XMLAnnotationDatastore::getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID)
{
    QList<QString> speakers;
    return speakers;
}

QList<QString> XMLAnnotationDatastore::getSpeakersInAnnotation(const QString &annotationID)
{
    QList<QString> speakersAll;
    return speakersAll;
}

QList<QString> XMLAnnotationDatastore::getSpeakersActiveInAnnotation(const QString &annotationID)
{
    QList<QString> speakersAll;
    return speakersAll;
}

IntervalTier *XMLAnnotationDatastore::getSpeakerTimeline(const QString &annotationID, const QString &levelID)
{
    return 0;
}

bool XMLAnnotationDatastore::deleteAllForAnnotationID(QString annotationID)
{
    return false;
}

QList<QueryOccurrencePointer *> XMLAnnotationDatastore::runQuery(QueryDefinition *qdef)
{
    QList<QueryOccurrencePointer *> occurrences;
    return occurrences;
}

QueryOccurrence *XMLAnnotationDatastore::getOccurrence(QueryOccurrencePointer *pointer, QueryDefinition *qdef)
{
    return 0;
}

bool XMLAnnotationDatastore::updateAnnotationsFromQueryOccurrences(const QList<QueryOccurrence *> &occurrences)
{
    return false;
}

