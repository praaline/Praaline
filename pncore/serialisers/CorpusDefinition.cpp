#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "DatastoreInfo.h"
#include "CorpusDefinition.h"

namespace Praaline {
namespace Core {

CorpusDefinition::CorpusDefinition() :
    basePath(""), baseMediaPath("")
{
}

CorpusDefinition::~CorpusDefinition()
{
}

QString datastoreType(DatastoreInfo info)
{
    if (info.type == DatastoreInfo::XML) return "xml";
    else if (info.type == DatastoreInfo::SQL) return "sql";
    else if (info.type == DatastoreInfo::Files) return "files";
    return "";
}

bool CorpusDefinition::save(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("PraalineCorpusDefinition");
    xml.writeAttribute("ID", corpusID);
    xml.writeAttribute("name", corpusName);
    xml.writeStartElement("MetadataDatastore");
    xml.writeAttribute("type", datastoreType(datastoreMetadata));
    xml.writeAttribute("driver", datastoreMetadata.driver);
    xml.writeAttribute("hostname", datastoreMetadata.hostname);
    if (datastoreMetadata.driver == "QSQLITE") {
        QFileInfo finfo(datastoreMetadata.datasource);
        xml.writeAttribute("datasource", finfo.fileName());
    } else {
        xml.writeAttribute("datasource", datastoreMetadata.datasource);
    }
    xml.writeAttribute("username", datastoreMetadata.username);
    xml.writeAttribute("usepassword", (datastoreMetadata.usePassword) ? "yes" : "no");
    xml.writeEndElement(); // MetadataDatastore
    xml.writeStartElement("AnnotationsDatastore");
    xml.writeAttribute("type", datastoreType(datastoreAnnotations));
    xml.writeAttribute("driver", datastoreAnnotations.driver);
    xml.writeAttribute("hostname", datastoreAnnotations.hostname);
    if (datastoreAnnotations.driver == "QSQLITE") {
        QFileInfo finfo(datastoreAnnotations.datasource);
        xml.writeAttribute("datasource", finfo.fileName());
    } else {
        xml.writeAttribute("datasource", datastoreMetadata.datasource);
    }
    xml.writeAttribute("username", datastoreAnnotations.username);
    xml.writeAttribute("usepassword", (datastoreAnnotations.usePassword) ? "yes" : "no");
    xml.writeEndElement(); // AnnotationsDatastore
    xml.writeStartElement("MediaDatastore");
    QString relativeBaseMediaPath = QDir(basePath).relativeFilePath(baseMediaPath);
    xml.writeAttribute("baseMediaPath", relativeBaseMediaPath);
    xml.writeEndElement(); // MediaDatastore
    xml.writeEndElement(); // PraalineCorpusDefinition
    xml.writeEndDocument();
    file.close();
    this->filenameDefinition = filename;
    return true;
}

void readDatastoreInfo(QXmlStreamReader &xml, DatastoreInfo &info)
{
    if (xml.attributes().hasAttribute("type")) {
        QString type = xml.attributes().value("type").toString();
        if (type == "xml") info.type = DatastoreInfo::XML;
        else if (type == "sql") info.type = DatastoreInfo::SQL;
        else if (type == "files") info.type = DatastoreInfo::Files;
    }
    if (xml.attributes().hasAttribute("driver"))     info.driver = xml.attributes().value("driver").toString();
    if (xml.attributes().hasAttribute("hostname"))   info.hostname = xml.attributes().value("hostname").toString();
    if (xml.attributes().hasAttribute("datasource")) info.datasource = xml.attributes().value("datasource").toString();
    if (xml.attributes().hasAttribute("username"))   info.username = xml.attributes().value("username").toString();
    if (xml.attributes().hasAttribute("usepassword")) {
        if (xml.attributes().value("usepassword").toString() == "yes")
            info.usePassword = true;
        else
            info.usePassword = false;
    }
}

bool CorpusDefinition::load(const QString &filename)
{
    QFile file(filename);
    QFileInfo finfo(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        // If token is StartElement, we'll see if we can read it.
        if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "PraalineCorpusDefinition") {
            if (xml.attributes().hasAttribute("ID")) corpusID = xml.attributes().value("ID").toString();
            if (xml.attributes().hasAttribute("name")) corpusName = xml.attributes().value("name").toString();
            xml.readNext();
            while (!xml.atEnd() && !xml.hasError() &&
                   !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "PraalineCorpusDefinition")) {
                if (xml.tokenType() == QXmlStreamReader::StartElement) {
                    if (xml.name() == "MetadataDatastore") {
                        readDatastoreInfo(xml, datastoreMetadata);
                    }
                    else if (xml.name() == "AnnotationsDatastore") {
                        readDatastoreInfo(xml, datastoreAnnotations);
                    }
                    else if (xml.name() == "MediaDatastore") {
                        if (xml.attributes().hasAttribute("baseMediaPath"))
                            this->baseMediaPath = xml.attributes().value("baseMediaPath").toString();
                    }
                }
                xml.readNext();
            }
        }
        xml.readNext(); // next element
    }
    // Error handling
    if(xml.hasError()) {
        file.close();
        return false;
    }
    // Removes any device() or data from the reader and resets its internal state to the initial state.
    xml.clear();
    file.close();
    this->filenameDefinition = filename;
    this->basePath = finfo.canonicalPath();
    // Media path is relative to corpus definition file
    if (this->baseMediaPath.isEmpty())
        this->baseMediaPath = this->basePath;
    else
        this->baseMediaPath = this->baseMediaPath.replace(QRegularExpression("^./"), this->basePath + "/");
    // Adjust to open correctly SQLite databases
    if (this->datastoreMetadata.driver == "QSQLITE")
        this->datastoreMetadata.datasource = this->basePath + "/" + this->datastoreMetadata.datasource;
    if (this->datastoreAnnotations.driver == "QSQLITE")
        this->datastoreAnnotations.datasource = this->basePath + "/" + this->datastoreAnnotations.datasource;

    return true;
}

} // namespace Core
} // namespace Praaline
