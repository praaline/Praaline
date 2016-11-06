#ifndef INTERFACETEXTFILE_H
#define INTERFACETEXTFILE_H

#include "pncore_global.h"
#include <QString>
#include <QFile>
#include <QTextStream>

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT InterfaceTextFile
{
public:
    static void setDefaultEncoding(const QString &encoding);
    static QString defaultEncoding();

protected:
    InterfaceTextFile();
    ~InterfaceTextFile();
    static void detectEncoding(QFile &file, QTextStream &stream);
    static QString m_defaultEncoding;
};

} // namespace Core
} // namespace Praaline

#endif // INTERFACETEXTFILE_H
