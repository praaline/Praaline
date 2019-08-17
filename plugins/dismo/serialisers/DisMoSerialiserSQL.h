#ifndef DISMOSERIALISERSQL_H
#define DISMOSERIALISERSQL_H

#include <QObject>
#include <QSqlDatabase>
#include "dismo/TokenList.h"

namespace DisMoAnnotator {

class DisMoSerialiserSql : public QObject
{
    Q_OBJECT
public:
    explicit DisMoSerialiserSql(QObject *parent = nullptr);
    static void serializeToDatabase(TokenList &TL, QSqlDatabase &db, const QString &annotationID, const QString &speakerID);

signals:

public slots:

};

}

#endif // DISMOSERIALISERSQL_H
