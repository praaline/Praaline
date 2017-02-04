#include "structure/AnnotationStructure.h"
#include "annotation/AnnotationTierGroup.h"
#include "annotation/AnnotationTier.h"
#include "annotation/Interval.h"
#include "XMLSerialiserAnnotation.h"

namespace Praaline {
namespace Core {

XMLSerialiserAnnotation::XMLSerialiserAnnotation()
{
}

// static
bool XMLSerialiserAnnotation::saveTiers(const QString &annotationID, const QString &speakerID,
                                        AnnotationTierGroup *tiers, AnnotationStructure *structure, const QString &filename)
{
    Q_UNUSED(annotationID)
    Q_UNUSED(speakerID)
    Q_UNUSED(tiers)
    Q_UNUSED(structure)
    Q_UNUSED(filename)
    return false;
}

//static
bool XMLSerialiserAnnotation::saveTier(const QString &annotationID, const QString &speakerID,
                                       AnnotationTier *tier, AnnotationStructure *structure, const QString &filename)
{
    Q_UNUSED(annotationID)
    Q_UNUSED(speakerID)
    Q_UNUSED(tier)
    Q_UNUSED(structure)
    Q_UNUSED(filename)
    return false;
}

// static
AnnotationTierGroup *XMLSerialiserAnnotation::getTiers(const QString &annotationID, const QString &speakerID,
                                                       const AnnotationStructure *structure, const QString &filename,
                                                       const QStringList &levelIDs)
{
    Q_UNUSED(annotationID)
    Q_UNUSED(speakerID)
    Q_UNUSED(structure)
    Q_UNUSED(filename)
    Q_UNUSED(levelIDs)
    return 0;
}

// static
AnnotationTier *XMLSerialiserAnnotation::getTier(const QString &annotationID, const QString &speakerID,
                                                 const AnnotationStructure *structure, const QString &filename,
                                                 const QString &levelID, const QStringList &attributeIDs)
{
    Q_UNUSED(annotationID)
    Q_UNUSED(speakerID)
    Q_UNUSED(structure)
    Q_UNUSED(filename)
    Q_UNUSED(levelID)
    Q_UNUSED(attributeIDs)
    return 0;
}

// static
QMap<RealTime, Interval *> XMLSerialiserAnnotation::getSpeakerTimeline(const QString &annotationID, const QString &levelID)
{
    Q_UNUSED(annotationID)
    Q_UNUSED(levelID)
    QMap<RealTime, Interval *> ret;
    return ret;
}

} // namespace Core
} // namespace Praaline
