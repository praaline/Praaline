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
    return true;
}

//static
bool XMLSerialiserAnnotation::saveTier(const QString &annotationID, const QString &speakerID,
                                       AnnotationTier *tier, AnnotationStructure *structure, const QString &filename)
{

}

// static
AnnotationTierGroup *XMLSerialiserAnnotation::getTiers(const QString &annotationID, const QString &speakerID,
                                                       const AnnotationStructure *structure, const QString &filename,
                                                       const QStringList &levelIDs)
{
    return 0;
}

// static
AnnotationTier *XMLSerialiserAnnotation::getTier(const QString &annotationID, const QString &speakerID,
                                                 const AnnotationStructure *structure, const QString &filename,
                                                 const QString &levelID, const QStringList &attributeIDs)
{
    return 0;
}

// static
QMap<RealTime, Interval *> XMLSerialiserAnnotation::getSpeakerTimeline(const QString &annotationID, const QString &levelID)
{

}

} // namespace Core
} // namespace Praaline
