#ifndef XMLSERIALISERANNOTATION_H
#define XMLSERIALISERANNOTATION_H

#include "xmlserialiserbase.h"
#include "structure/annotationstructure.h"
#include "annotation/annotationtiergroup.h"
#include "annotation/annotationtier.h"

class XMLSerialiserAnnotation : public XMLSerialiserBase
{

public:
    static bool saveTiers(const QString &annotationID, const QString &speakerID,
                          AnnotationTierGroup *tiers, AnnotationStructure *structure, const QString &filename);
    static bool saveTier(const QString &annotationID, const QString &speakerID,
                         AnnotationTier *tier, AnnotationStructure *structure, const QString &filename);

    static AnnotationTierGroup *getTiers(const QString &annotationID, const QString &speakerID,
                                         const AnnotationStructure *structure, const QString &filename,
                                         const QStringList &levelIDs = QStringList());
    static AnnotationTier *getTier(const QString &annotationID, const QString &speakerID,
                                   const AnnotationStructure *structure, const QString &filename,
                                   const QString &levelID, const QStringList &attributeIDs = QStringList());

    static QMap<RealTime, Interval *> getSpeakerTimeline(const QString &annotationID, const QString &levelID);

private:
    XMLSerialiserAnnotation();    
};

#endif // XMLSERIALISERANNOTATION_H
