#ifndef DATASTOREINFO_H
#define DATASTOREINFO_H

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
#include <QString>

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT DatastoreInfo
{
public:
    enum Type {
        SQL,
        XML,
        Files
    };

    DatastoreInfo() : usePassword(false) {}
    DatastoreInfo(Type type, QString driver, QString hostname, QString datasource, QString username, QString password) :
        type(type), driver(driver), hostname(hostname), datasource(datasource), username(username), password(password)
    {
        if (!password.isEmpty()) usePassword = true; else usePassword = false;
    }
    ~DatastoreInfo() {}

    Type type;
    QString driver;
    QString hostname;
    QString datasource; // database name (on server) or filename
    QString username;
    QString password;
    bool usePassword;

    QString typeToString() const {
        if      (type == DatastoreInfo::XML)   return "xml";
        else if (type == DatastoreInfo::SQL)   return "sql";
        else if (type == DatastoreInfo::Files) return "files";
        return QString();
    }
    void setTypeFromString(const QString &typeString) {
        if      (typeString == "xml")   type = DatastoreInfo::XML;
        else if (typeString == "sql")   type = DatastoreInfo::SQL;
        else if (typeString == "files") type = DatastoreInfo::Files;
    }

};

} // namespace Core
} // namespace Praaline

#endif // DATASTOREINFO_H
