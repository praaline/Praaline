#ifndef SQLSCHEMAPROXYBASE_H
#define SQLSCHEMAPROXYBASE_H

#include <QString>
#include <QSqlDatabase>
#include "base/DataType.h"

namespace Praaline {
namespace Core {

class SQLSchemaProxyBase
{    
protected:
    static bool addColumnToTable(QString table, QString column, DataType datatype, QSqlDatabase &db);
    static bool renameColumn(QString table, QString oldColumn, QString newColumn, QSqlDatabase &db);
    static bool renameTable(QString oldTable, QString newTable, QSqlDatabase &db);
    static bool deleteColumn(QString table, QString column, QSqlDatabase &db);
    static bool deleteTable(QString table, QSqlDatabase &db);
    static bool retypeColumn(QString tableName, QString columnName, DataType oldDataType, DataType newDataType, QSqlDatabase &db);
};

} // namespace Core
} // namespace Praaline

#endif // SQLSCHEMAPROXYBASE_H
