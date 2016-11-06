#include <QObject>
#include <QPointer>
#include <QString>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "XMLSerialiserCorpusBookmark.h"

namespace Praaline {
namespace Core {

QString XMLSerialiserCorpusBookmark::xmlElementName_CorpusBookmark("CorpusBookmark");

XMLSerialiserCorpusBookmark::XMLSerialiserCorpusBookmark()
{
}

XMLSerialiserCorpusBookmark::~XMLSerialiserCorpusBookmark()
{
}

// private static
CorpusBookmark *XMLSerialiserCorpusBookmark::readBookmark(QXmlStreamReader &xml)
{
    // Check that we're really reading a bookmark
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() == xmlElementName_CorpusBookmark) {
        return 0;
    }
    CorpusBookmark *bookmark = new CorpusBookmark();
    QXmlStreamAttributes xmlAttributes = xml.attributes();
    if (xmlAttributes.hasAttribute("id"))               bookmark->setID(xmlAttributes.value("id").toString());
    if (xmlAttributes.hasAttribute("corpusID"))         bookmark->setCorpusID(xmlAttributes.value("corpusID").toString());
    if (xmlAttributes.hasAttribute("communicationID"))  bookmark->setCommunicationID(xmlAttributes.value("communicationID").toString());
    if (xmlAttributes.hasAttribute("annotationID"))     bookmark->setAnnotationID(xmlAttributes.value("annotationID").toString());
    if (xmlAttributes.hasAttribute("time"))             bookmark->setTime(RealTime::fromNanoseconds((xmlAttributes.value("time").toLongLong())));
    if (xmlAttributes.hasAttribute("name"))             bookmark->setName(xmlAttributes.value("name").toString());
    if (xmlAttributes.hasAttribute("notes"))            bookmark->setNotes(xmlAttributes.value("notes").toString());
    return bookmark;
}

// private static
bool XMLSerialiserCorpusBookmark::writeBookmark(CorpusBookmark *bookmark, QXmlStreamWriter &xml)
{
    xml.writeStartElement(xmlElementName_CorpusBookmark);
    xml.writeAttribute("corpusID", bookmark->corpusID());
    xml.writeAttribute("communicationID", bookmark->communicationID());
    xml.writeAttribute("annotationID", bookmark->annotationID());
    xml.writeAttribute("time", QString::number(bookmark->time().toNanoseconds()));
    xml.writeAttribute("name", bookmark->name());
    xml.writeAttribute("notes", bookmark->notes());
//    foreach (MetadataStructureAttribute *attribute, mstructure->attributes(CorpusObject::Type_Recording)) {
//        xml.writeAttribute(attribute->ID(), rec->property(attribute->ID()).toString());
//    }
    xml.writeEndElement(); // CorpusBookmark
    return true;
}

// static
bool XMLSerialiserCorpusBookmark::saveCorpusBookmarks(const QList<QPointer<CorpusBookmark> > &list, QXmlStreamWriter &xml)
{
    xml.writeStartElement("CorpusBookmarks");
    foreach (CorpusBookmark *bookmark, list) {
        if (!bookmark) continue;
        writeBookmark(bookmark, xml);
    }
    xml.writeEndElement(); // CorpusBookmarks
    return true;
}

// static
bool XMLSerialiserCorpusBookmark::loadCorpusBookmarks(QList<QPointer<CorpusBookmark> > &list, QXmlStreamReader &xml)
{
    list.clear();
    while (!xml.atEnd() && !xml.hasError()) {
        // If token is StartElement, we'll see if we can read it.
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "CorpusBookmarks") {
            xml.readNext();
            while (!xml.atEnd() && !xml.hasError() &&
                   !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "CorpusBookmarks")) {
                if (xml.tokenType() == QXmlStreamReader::StartElement) {
                    if (xml.name() == "CorpusBookmark") {
                        CorpusBookmark *bookmark = readBookmark(xml);
                        if (bookmark) list << bookmark;
                    }
                }
                xml.readNext();
            }
        }
        xml.readNext(); // next element
    }
    // Error handling
    if(xml.hasError()) return false;
    return true;
}

// static
bool XMLSerialiserCorpusBookmark::saveCorpusBookmarks(const QList<QPointer<CorpusBookmark> > &list, const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    bool ret = saveCorpusBookmarks(list, xml);
    xml.writeEndDocument();
    file.close();
    return ret;
}

// static
bool XMLSerialiserCorpusBookmark::loadCorpusBookmarks(QList<QPointer<CorpusBookmark> > &list, const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QXmlStreamReader xml(&file);
    bool ret = loadCorpusBookmarks(list, xml);
    xml.clear();
    file.close();
    return ret;
}

} // namespace Core
} // namespace Praaline
