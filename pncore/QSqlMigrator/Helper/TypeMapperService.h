#ifndef HELPER_TYPEMAPPERSERVICE_H
#define HELPER_TYPEMAPPERSERVICE_H

#include "QSqlMigrator/QSqlMigratorConfig.h"

#include "QSqlMigrator/Structure/SqlType.h"

#include <QString>
#include <QHash>

namespace QSqlMigrator {
namespace Helper {

/*!
 * \brief interface for a mapping basic types to proper sql types
 */
class QSQLMIGRATOR_DLL_EXPORT TypeMapperService
{
public:
    virtual ~TypeMapperService() {}

    virtual QString map(const Structure::SqlType& type) const = 0;
};

} // namespace Helper
} // namespace QSqlMigrator

#endif // TYPEMAPPERSERVICE_H
