#ifndef FILEDATASTORE_H
#define FILEDATASTORE_H

/*
    Praaline - Core module - Datastores
    Copyright (c) 2011-2017 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "pncore_global.h"
#include <QObject>

namespace Praaline {
namespace Core {

class Corpus;
class CorpusObjectInfo;

class PRAALINE_CORE_SHARED_EXPORT FileDatastore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString basePath READ basePath WRITE setBasePath)

public:
    explicit FileDatastore(QObject *parent = 0);
    virtual ~FileDatastore() {}

    QString basePath() const { return m_basePath; }
    void setBasePath(const QString &path);

    QString getRelativeToBasePath (const QString &absoluteFilePath) const;

protected:
    QString m_basePath;
};

} // namespace Core
} // namespace Praaline

#endif // FILEDATASTORE_H
