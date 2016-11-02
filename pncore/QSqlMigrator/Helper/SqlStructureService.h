#ifndef HELPER_SQLSTRUCTURESERVICE_H
#define HELPER_SQLSTRUCTURESERVICE_H

#include "QSqlMigrator/QSqlMigratorConfig.h"

#include <QString>
#include <QSqlDatabase>

namespace QSqlMigrator {


namespace Structure {
class Table;
class Index;
} // namespace Structure

namespace Helper {

/*!
 * \brief interface for a service that extracts existing structures from a sql database
 */
class QSQLMIGRATOR_DLL_EXPORT SqlStructureService
{
public:
    virtual ~SqlStructureService() {}

    //! \return the definition of an existing table in the databse
    virtual Structure::Table getTableDefinition(const QString &tableName,
                                                QSqlDatabase database) const = 0;

    //! \return the definition of an existing index in the database for a table
    virtual Structure::Index getIndexDefinition(const QString &indexName,
                                                const QString &tableName,
                                                QSqlDatabase database) const = 0;
};

} // namespace Helper
} // namespace QSqlMigrator

#endif // HELPER_SQLSTRUCTURESERVICE_H
