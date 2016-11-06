#include <QString>
#include <QByteArray>
#include <QTextStream>
#include "InterfaceTextFile.h"

namespace Praaline {
namespace Core {

// QString InterfaceTextFile::m_defaultEncoding = "UTF-8";
// or
QString InterfaceTextFile::m_defaultEncoding = "ISO 8859-1";

InterfaceTextFile::InterfaceTextFile()
{
}

InterfaceTextFile::~InterfaceTextFile()
{
}

void InterfaceTextFile::setDefaultEncoding(const QString &encoding)
{
    m_defaultEncoding = encoding;
}

QString InterfaceTextFile::defaultEncoding()
{
    return m_defaultEncoding;
}

void InterfaceTextFile::detectEncoding(QFile &file, QTextStream &stream)
{
    // Encoding detection
    QByteArray fileHeaderHex = file.peek(4).toHex();
    char *fileHeaderChar = fileHeaderHex.data();
    QString fileBOM;
    for(int i = 0; i < fileHeaderHex.size(); i++) {
        fileBOM += fileHeaderChar[i];
    }
    stream.setAutoDetectUnicode(true);
    if(fileBOM.left(6) == "efbbbf") {
        stream.setCodec("UTF-8");
    }
    else if(fileBOM.left(4) == "feff") {
        stream.setCodec("UTF-16BE");
    }
    else if(fileBOM.left(4) == "fffe") {
        stream.setCodec("UTF-16LE");
    }
    else if(fileBOM.left(8) == "0000feff") {
        stream.setCodec("UTF-32BE");
    }
    else if(fileBOM.left(8) == "fffe0000") {
        stream.setCodec("UTF-32LE");
    }
    else {
        stream.setCodec(m_defaultEncoding.toLatin1().constData());
    }
}

} // namespace Core
} // namespace Praaline
