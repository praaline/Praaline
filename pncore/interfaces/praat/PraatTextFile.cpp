#include <QString>
#include <QTextStream>
#include <QRegularExpression>
#include "PraatTextFile.h"

namespace Praaline {
namespace Core {

void PraatTextFile::detectEncoding(QFile &file, QTextStream &stream)
{
    InterfaceTextFile::detectEncoding(file, stream);
}

QString PraatTextFile::seekPattern(QTextStream &stream, const QRegularExpression &regex)
{
    QString line = QString();
    QRegularExpressionMatch match;
    while (!match.hasMatch() && ! stream.atEnd()) {
        line += stream.readLine();
        match = regex.match(line);
    }
    if (match.hasMatch())
        return match.captured(1);
    return QString();
}

} // namespace Core
} // namespace Praaline
