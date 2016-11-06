#ifndef PRAATPOINTTIERFILE_H
#define PRAATPOINTTIERFILE_H

#include <QObject>
#include <QPair>
#include <QString>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include "base/RealTime.h"
#include "PraatTextFile.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT PraatPointTierFile : PraatTextFile
{
public:
    // Public static methods to read and write point tier files (e.g. pitch tier, intensity tier)
    static bool load(const QString &filename, QMap<RealTime, double> &points);
    static bool save(const QString &filename,  const QString &praatObjectClass,
                     QMap<RealTime, double> &points);

private:
    static bool readPoint(bool isShortFile, QTextStream &stream, QString &line,
                          double &timePoint, double &value);
    PraatPointTierFile() {}
};

} // namespace Core
} // namespace Praaline

#endif // PRAATPOINTTIERFILE_H
