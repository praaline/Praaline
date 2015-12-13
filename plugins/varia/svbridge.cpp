#include <QString>
#include <QFile>
#include <QTextStream>
#include "svbridge.h"

SVBridge::SVBridge()
{
}

SVBridge::~SVBridge()
{
}

// static
bool SVBridge::saveSVTimeInstantsLayer(const QString &filename, unsigned int sampleRate, IntervalTier *tier, const QString &attribute)
{

    QFile file(filename);
    if ( !file.open( QIODevice::ReadWrite | QIODevice::Text ) ) return false;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<!DOCTYPE sonic-visualiser>\n";
    out << "<sv>\n";
    out << "  <data>\n";
    out << QString("    <model id=\"1\" name=\"\" sampleRate=\"%1\" start=\"%2\" end=\"%3\" type=\"sparse\" dimensions=\"1\" resolution=\"1\" notifyOnAdd=\"true\" dataset=\"1\" />")
           .arg(sampleRate).arg(RealTime::realTime2Frame(tier->tMin(), sampleRate)).arg(RealTime::realTime2Frame(tier->tMax(), sampleRate)) << "\n";
    out << "    <dataset id=\"1\" dimensions=\"1\">\n";
    foreach (Interval *intv, tier->intervals()) {
        QString text = (attribute.isEmpty()) ? intv->text() : intv->attribute(attribute).toString();
        out << QString("      <point frame=\"%1\" label=\"%2\" />\n").arg(RealTime::realTime2Frame(intv->tMin(), sampleRate)).arg(text);
    }
    out << "    </dataset>\n";
    out << "  </data>\n";
    out << "  <display>\n";
    out << QString("    <layer id=\"1\" type=\"timeinstants\" name=\"%1\" model=\"1\"  plotStyle=\"1\" colourName=\"Orange\" colour=\"#ff9632\" darkBackground=\"false\" />\n")
           .arg(tier->name());
    out << "  </display>\n";
    out << "</sv>\n";
    file.close();
    return true;
}
