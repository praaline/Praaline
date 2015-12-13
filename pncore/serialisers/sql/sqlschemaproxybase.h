#ifndef SQLSCHEMAPROXYBASE_H
#define SQLSCHEMAPROXYBASE_H

#include <QString>
#include <QSqlDatabase>

class SQLSchemaProxyBase
{    
protected:
    static bool addColumnToTable(QString table, QString column, QString datatype, QSqlDatabase &db);
    static bool renameColumn(QString table, QString oldColumn, QString newColumn, QSqlDatabase &db);
    static bool renameTable(QString oldTable, QString newTable, QSqlDatabase &db);
    static bool deleteColumn(QString table, QString column, QSqlDatabase &db);
    static bool deleteTable(QString table, QSqlDatabase &db);
};

#endif // SQLSCHEMAPROXYBASE_H
