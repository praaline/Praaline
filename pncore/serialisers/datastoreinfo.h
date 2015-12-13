#ifndef DATASTOREINFO_H
#define DATASTOREINFO_H

#include "pncore_global.h"
#include <QString>

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
};

#endif // DATASTOREINFO_H
