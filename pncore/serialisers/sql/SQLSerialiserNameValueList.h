#ifndef SQLSERIALISERNAMEVALUELIST_H
#define SQLSERIALISERNAMEVALUELIST_H

#include <QString>
#include <QMap>
#include <QPointer>
#include <QSqlDatabase>
#include "SQLSerialiserBase.h"
#include "SQLSchemaProxyBase.h"
#include "structure/NameValueList.h"

namespace Praaline {
namespace Core {

class SQLSerialiserNameValueList : public SQLSerialiserBase, public SQLSchemaProxyBase
{
public:
    enum NameValueListType {
        Metadata   = 10,
        Annotation = 20
    };

    // ==========================================================================================================================
    // Name-value lists
    // ==========================================================================================================================
    static bool initialiseNameValueListSchema(QSqlDatabase &db);
    static bool upgradeNameValueListSchema(QSqlDatabase &db);
    static NameValueList *getNameValueList(const QString &listID, NameValueListType listType, QSqlDatabase &db);
    static QStringList getAllNameValueListIDs(NameValueListType listType, QSqlDatabase &db);
    static QMap<QString, QPointer<NameValueList> > getAllNameValueLists(NameValueListType listType, QSqlDatabase &db);
    static bool createNameValueList(NameValueList *newList, NameValueListType listType, QSqlDatabase &db);
    static bool updateNameValueList(NameValueList *list, NameValueListType listType, QSqlDatabase &db);
    static bool deleteNameValueList(const QString &listID, NameValueListType listType, QSqlDatabase &db);
    static bool checkNameValueListExists(const QString &listID, NameValueListType listType, QSqlDatabase &db);

private:
    static QString getTableName(const QString &listID, NameValueListType listType);

    SQLSerialiserNameValueList() {}
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERNAMEVALUELIST_H
