#ifndef XMLSERIALISERANNOTATIONSTRUCTURE_H
#define XMLSERIALISERANNOTATIONSTRUCTURE_H

#include "pncore_global.h"
#include "structure/annotationstructure.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class PRAALINE_CORE_SHARED_EXPORT XMLSerialiserAnnotationStructure
{
public:
    static bool write(AnnotationStructure *data, const QString &filename);
    static void write(AnnotationStructure *data, QXmlStreamWriter &xml);
    static AnnotationStructure *read(const QString &filename);
    static AnnotationStructure *read(QXmlStreamReader &xml);

private:
    XMLSerialiserAnnotationStructure();
    static void writeLevel(AnnotationStructureLevel *level, QXmlStreamWriter &xml);
    static AnnotationStructureLevel *readLevel(QXmlStreamReader &xml);
    static void writeAttribute(AnnotationStructureAttribute *attribute, QXmlStreamWriter &xml);
    static AnnotationStructureAttribute  *readAttribute(QXmlStreamReader &xml);

    static QString xmlElementName_Structure;
    static QString xmlElementName_Level;
    static QString xmlElementName_Attribute;
};

#endif // XMLSERIALISERANNOTATIONSTRUCTURE_H
