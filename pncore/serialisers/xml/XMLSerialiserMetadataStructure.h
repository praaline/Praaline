#ifndef XMLSERIALISERMETADATASTRUCTURE_H
#define XMLSERIALISERMETADATASTRUCTURE_H

#include "pncore_global.h"
#include "structure/MetadataStructure.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT XMLSerialiserMetadataStructure
{
public:
    static bool write(MetadataStructure *structure, const QString &filename);
    static void write(MetadataStructure *structure, QXmlStreamWriter &xml);
    static MetadataStructure *read(const QString &filename);
    static MetadataStructure *read(QXmlStreamReader &xml);
    static void writePartial(MetadataStructure *structure, CorpusObject::Type what, QXmlStreamWriter &xml);
    static MetadataStructure *readPartial(CorpusObject::Type what, QXmlStreamReader &xml);

private:
    XMLSerialiserMetadataStructure();
    static void writeSection(CorpusObject::Type what, MetadataStructureSection *section, QXmlStreamWriter &xml);
    static MetadataStructureSection *readSection(QXmlStreamReader &xml);
    static void writeAttribute(MetadataStructureAttribute *attribute, QXmlStreamWriter &xml);
    static MetadataStructureAttribute  *readAttribute(QXmlStreamReader &xml);

    static QString xmlElementName_Structure;
    static QString xmlElementName_Section;
    static QString xmlElementName_Attribute;
};

} // namespace Core
} // namespace Praaline

#endif // XMLSERIALISERMETADATASTRUCTURE_H
