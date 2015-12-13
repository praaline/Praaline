#ifndef PRAATTEXTFILE_H
#define PRAATTEXTFILE_H

#include "pncore_global.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include "interfaces/interfacetextfile.h"

class PRAALINE_CORE_SHARED_EXPORT PraatTextFile : InterfaceTextFile
{
protected:
    PraatTextFile() {}
    virtual ~PraatTextFile() {}

    static void detectEncoding(QFile &file, QTextStream &stream);
    static QString seekPattern(QTextStream &stream, const QRegularExpression &regex);
};

#endif // PRAATTEXTFILE_H
