#ifndef HELPER_COLUMNSERVICE_H
#define HELPER_COLUMNSERVICE_H

#include "QSqlMigrator/QSqlMigratorConfig.h"

#include "QSqlMigrator/Structure/Column.h"
#include "QSqlMigrator/Structure/Index.h"

#include <QString>
#include <QHash>

namespace QSqlMigrator {
namespace Helper {

/*!
 * \brief interface for a service that can convert column and index definitions to proper SQL statements
 */
class QSQLMIGRATOR_DLL_EXPORT ColumnService
{
public:
    virtual ~ColumnService() {}

    virtual QString generateColumnDefinitionSql(const Structure::Column &column) const = 0;
    virtual QString generateColumnsDefinitionSql(const QList<Structure::Column> &columnList) const = 0;

    virtual QString generateIndexColumnDefinitionSql(const Structure::Index::Column& column) const = 0;
    virtual QString generateIndexColumnsDefinitionSql(const Structure::Index::ColumnList &columns) const = 0;
};

} // namespace Helper
} // namespace QSqlMigrator

#endif // COLUMNSERVICE_H
