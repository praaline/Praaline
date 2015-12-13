#include <QSqlDatabase>
#include <QSqlQuery>
#include "sqlschemaproxybase.h"


bool SQLSchemaProxyBase::addColumnToTable(QString table, QString column, QString datatype, QSqlDatabase &db)
{
    QString query = QString("ALTER TABLE %1 ADD %2 %3").arg(table).arg(column).arg(datatype);
    QSqlQuery q(db);
    q.prepare(query);
    return q.exec();
}

bool SQLSchemaProxyBase::renameColumn(QString table, QString oldColumn, QString newColumn, QSqlDatabase &db)
{
    QString query = QString("ALTER TABLE %1 RENAME COLUMN %2 TO %3").arg(table).arg(oldColumn).arg(newColumn);
    QSqlQuery q(db);
    q.prepare(query);
    return q.exec();
}

bool SQLSchemaProxyBase::renameTable(QString oldTable, QString newTable, QSqlDatabase &db)
{
    QString query = QString("ALTER TABLE %1 RENAME TO %2").arg(oldTable).arg(newTable);
    QSqlQuery q(db);
    q.prepare(query);
    return q.exec();
}

bool SQLSchemaProxyBase::deleteColumn(QString table, QString column, QSqlDatabase &db)
{
    QString query = QString("ALTER TABLE %1 DROP COLUMN %2").arg(table).arg(column);
    QSqlQuery q(db);
    q.prepare(query);
    return q.exec();
}

bool SQLSchemaProxyBase::deleteTable(QString table, QSqlDatabase &db)
{
    QString query = QString("DROP TABLE %1").arg(table);
    QSqlQuery q(db);
    q.prepare(query);
    return q.exec();
}
