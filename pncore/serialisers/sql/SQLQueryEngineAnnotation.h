#ifndef SQLQUERYENGINEANNOTATION_H
#define SQLQUERYENGINEANNOTATION_H

#include <QSqlDatabase>
#include "structure/AnnotationStructure.h"
#include "query/QueryDefinition.h"
#include "query/QueryOccurrence.h"

namespace Praaline {
namespace Core {

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

} // namespace Core
} // namespace Praaline

#endif // SQLQUERYENGINEANNOTATION_H
