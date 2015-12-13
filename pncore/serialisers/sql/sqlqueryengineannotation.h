#ifndef SQLQUERYENGINEANNOTATION_H
#define SQLQUERYENGINEANNOTATION_H

#include <QSqlDatabase>
#include "structure/annotationstructure.h"
#include "query/querydefinition.h"
#include "query/queryoccurrence.h"

class SQLQueryEngineAnnotation
{
public:
    static QList<QueryOccurrencePointer *> runQuery(QueryDefinition *qdef,
                                                    AnnotationStructure *structure, QSqlDatabase &db);
    static QueryOccurrence *getOccurrence(QueryOccurrencePointer *pointer, QueryDefinition *qdef,
                                          AnnotationStructure *structure, QSqlDatabase &db,
                                          int lengthContextLeft = 10, int lengthContextRight = 10);
    static bool updateAnnotationsFromQueryOccurrences(const QList<QueryOccurrence *> &occurrences,
                                                      AnnotationStructure *structure, QSqlDatabase &db);

private:
    SQLQueryEngineAnnotation();
};

#endif // SQLQUERYENGINEANNOTATION_H
