#include <QString>
#include <QMap>
#include "datastore/CorpusRepository.h"
#include "datastore/DatastoreInfo.h"
#include "annotation/IntervalTier.h"
#include "structure/AnnotationStructure.h"
#include "XMLAnnotationDatastore.h"

namespace Praaline {
namespace Core {

XMLAnnotationDatastore::XMLAnnotationDatastore(AnnotationStructure *structure, CorpusRepository *repository, QObject *parent) :
    AnnotationDatastore(repository, parent), m_structure(structure)
{
}

XMLAnnotationDatastore::~XMLAnnotationDatastore()
{
}

// ==========================================================================================================================
// Datastore
// ==========================================================================================================================
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

// ==========================================================================================================================
// Annotation Structure
// ==========================================================================================================================
bool XMLAnnotationDatastore::loadAnnotationStructure()
{
    return false;
}

bool XMLAnnotationDatastore::createAnnotationLevel(AnnotationStructureLevel *newLevel)
{
    return false;
}

bool XMLAnnotationDatastore::renameAnnotationLevel(const QString &levelID, const QString &newLevelID)
{
    return false;
}

bool XMLAnnotationDatastore::deleteAnnotationLevel(const QString &levelID)
{
    return false;
}

bool XMLAnnotationDatastore::createAnnotationAttribute(const QString &levelID, AnnotationStructureAttribute *newAttribute)
{
    return false;
}

bool XMLAnnotationDatastore::renameAnnotationAttribute(const QString &levelID, const QString &attributeID, const QString &newAttributeID)
{
    return false;
}

bool XMLAnnotationDatastore::deleteAnnotationAttribute(const QString &levelID, const QString &attributeID)
{
    return false;
}

bool XMLAnnotationDatastore::retypeAnnotationAttribute(const QString &levelID, const QString &attributeID, const DataType &newDatatype)
{
    return false;
}

// ==========================================================================================================================
// Name-value lists
// ==========================================================================================================================
NameValueList *XMLAnnotationDatastore::getNameValueList(const QString &listID)
{
    return Q_NULLPTR;
}

QStringList XMLAnnotationDatastore::getAllNameValueListIDs()
{
    QStringList ret;
    return ret;
}

QMap<QString, QPointer<NameValueList> > XMLAnnotationDatastore::getAllNameValueLists()
{

}

bool XMLAnnotationDatastore::createNameValueList(NameValueList *list)
{
    return false;
}

bool XMLAnnotationDatastore::updateNameValueList(NameValueList *list)
{
    return false;
}

bool XMLAnnotationDatastore::deleteNameValueList(const QString &listID)
{
    return false;
}

// ==========================================================================================================================
// Annotation Tiers
// ==========================================================================================================================
AnnotationTier *XMLAnnotationDatastore::getTier(const QString &annotationID, const QString &speakerID,
                                                const QString &levelID, const QStringList &attributeIDs)
{
    return Q_NULLPTR;
}

AnnotationTierGroup *XMLAnnotationDatastore::getTiers(const QString &annotationID, const QString &speakerID, const QStringList &levelIDs)
{
    return Q_NULLPTR;
}

QMap<QString, QPointer<AnnotationTierGroup> > XMLAnnotationDatastore::getTiersAllSpeakers(const QString &annotationID, const QStringList &levelIDs)
{
    QMap<QString, QPointer<AnnotationTierGroup> > ret;
    return ret;
}

bool XMLAnnotationDatastore::saveTier(const QString &annotationID, const QString &speakerID, AnnotationTier *tier)
{
    return false;
}

bool XMLAnnotationDatastore::saveTiers(const QString &annotationID, const QString &speakerID, AnnotationTierGroup *tiers)
{
    return false;
}

bool XMLAnnotationDatastore::saveTiersAllSpeakers(const QString &annotationID, QMap<QString, QPointer<AnnotationTierGroup> > &tiersAllSpeakers)
{
    return false;
}

bool XMLAnnotationDatastore::deleteTier(const QString &annotationID, const QString &speakerID, const QString &levelID)
{
    return false;
}

bool XMLAnnotationDatastore::deleteTiers(const QString &annotationID, const QString &speakerID, const QStringList &levelIDs)
{
    return false;
}

bool XMLAnnotationDatastore::deleteAllTiersAllSpeakers(const QString &annotationID)
{
    return false;
}

// ==========================================================================================================================
// Annotation Elements
// ==========================================================================================================================
QList<AnnotationElement *> XMLAnnotationDatastore::getAnnotationElements(const Selection &selection)
{
    QList<AnnotationElement *> ret;
    return ret;
}

QList<Point *> XMLAnnotationDatastore::getPoints(const Selection &selection)
{
    QList<Point *> ret;
    return ret;
}

QList<Interval *> XMLAnnotationDatastore::getIntervals(const Selection &selection)
{
    QList<Interval *> ret;
    return ret;
}

QList<Sequence *> XMLAnnotationDatastore::getSequences(const Selection &selection)
{
    QList<Sequence *> ret;
    return ret;
}

bool XMLAnnotationDatastore::saveAnnotationElements(const QList<AnnotationElement *> &elements, const QString &levelID,
                                                    const QStringList &attributeIDs)
{
    return false;
}

// ==========================================================================================================================
// Speakers and Timeline
// ==========================================================================================================================
QStringList XMLAnnotationDatastore::getSpeakersInLevel(const QString &annotationID, const QString &levelID)
{
    QStringList ret;
    return ret;
}

QStringList XMLAnnotationDatastore::getSpeakersActiveInLevel(const QString &annotationID, const QString &levelID)
{
    QStringList ret;
    return ret;
}

QStringList XMLAnnotationDatastore::getSpeakersInAnnotation(const QString &annotationID)
{
    QStringList ret;
    return ret;
}

QStringList XMLAnnotationDatastore::getSpeakersActiveInAnnotation(const QString &annotationID)
{
    QStringList ret;
    return ret;
}

IntervalTier *XMLAnnotationDatastore::getSpeakerTimeline(const QString &communicationID,const QString &annotationID,
                                                         const QString &levelID, bool detailed)
{
    return Q_NULLPTR;
}

// ==========================================================================================================================
// Queries
// ==========================================================================================================================
QList<QueryOccurrencePointer *> XMLAnnotationDatastore::runQuery(QueryDefinition *qdef)
{
    QList<QueryOccurrencePointer *> ret;
    return ret;
}

QueryOccurrence *XMLAnnotationDatastore::getOccurrence(QueryOccurrencePointer *pointer, QueryDefinition *qdef)
{
    return Q_NULLPTR;
}

bool XMLAnnotationDatastore::updateAnnotationsFromQueryOccurrences(const QList<QueryOccurrence *> &occurrences)
{
    return false;
}

// ==========================================================================================================================
// Batch Processing
// ==========================================================================================================================
QList<QPair<QList<QVariant>, long> > XMLAnnotationDatastore::getDistinctLabels(const QString &levelID, const QStringList &attributeIDs)
{
    QList<QPair<QList<QVariant>, long> > ret;
    return ret;
}

bool XMLAnnotationDatastore::batchUpdate(const QString &levelID, const QString &attributeID, const QVariant &newValue,
                                         const QList<QPair<QString, QVariant> > &criteria)
{
    return false;
}

// ==========================================================================================================================
// Statistics
// ==========================================================================================================================
QList<QPair<QList<QVariant>, long> > XMLAnnotationDatastore::countItems(const QString &levelID, const QStringList &groupByAttributeIDs)
{
    QList<QPair<QList<QVariant>, long> > ret;
    return ret;
}

} // namespace Core
} // namespace Praaline
