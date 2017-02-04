#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "datastore/CorpusRepository.h"
#include "SQLSerialiserAnnotation.h"
#include "SQLQueryEngineAnnotation.h"

namespace Praaline {
namespace Core {

SQLQueryEngineAnnotation::SQLQueryEngineAnnotation()
{
}

QSqlQuery getSqlQueryForSequence(const QueryFilterSequence &qsequence, QSqlDatabase &db)
{
    QSqlQuery qsql(db);
    // MAIN CLAUSE
    QString queryText = QString("SELECT com.corpusID, annot.communicationID, t1.annotationID, t1.speakerID, "
                                "t1.intervalNo AS intervalNoMin, t1.tMin, "
                                "t%1.intervalNo AS intervalNoMax, t%1.tMax FROM %2 t1 "
                                "INNER JOIN annotation annot ON annot.annotationID=t1.annotationID "
                                "INNER JOIN communication com ON com.communicationID=annot.communicationID ")
            .arg(qsequence.sequenceLength()).arg(qsequence.annotationLevelID());
    for (int i = 2; i <= qsequence.sequenceLength(); ++i) {
        queryText.append(QString(" INNER JOIN %1 t%2 ON t1.annotationID=t%2.annotationID AND t1.speakerID=t%2.speakerID AND "
                                 " t%2.intervalNo=t%3.intervalNo+1 ").arg(qsequence.annotationLevelID()).arg(i).arg(i-1));
    }
    // WHERE CLAUSE
    queryText.append(" WHERE 1=1 ");
    for (int i = 1; i <= qsequence.sequenceLength(); ++i) {
        foreach (QString attributeID, qsequence.attributeIDs()) {
            QueryFilterSequence::Condition condition = qsequence.condition(attributeID, i-1);
            if (condition.operand == QueryFilterSequence::NoCondition) continue;
            QString sqlOperand;
            switch (condition.operand) {
                case QueryFilterSequence::Equals:             sqlOperand = "=";     break;
                case QueryFilterSequence::DoesNotEqual:       sqlOperand = "<>";    break;
                case QueryFilterSequence::GreaterThan:        sqlOperand = ">";     break;
                case QueryFilterSequence::GreaterThanOrEqual: sqlOperand = ">=";    break;
                case QueryFilterSequence::LessThan:           sqlOperand = "<";     break;
                case QueryFilterSequence::LessThanOrEqual:    sqlOperand = "<=";    break;
                case QueryFilterSequence::Contains:           sqlOperand = "LIKE";  break;
                case QueryFilterSequence::DoesNotContain:     sqlOperand = "NOT LIKE"; break;
                case QueryFilterSequence::Like:               sqlOperand = "LIKE";  break;
                case QueryFilterSequence::NoCondition:        break;
            }
            if (attributeID == qsequence.annotationLevelID()) attributeID = "xText";
            queryText.append(QString(" AND t%1.%2 %3 :crit_%2_%1 ").arg(i).arg(attributeID).arg(sqlOperand));
        }
    }
    qDebug() << queryText;
    // CRITERIA
    qsql.setForwardOnly(true);
    qsql.prepare(queryText);
    for (int i = 1; i <= qsequence.sequenceLength(); ++i) {
        foreach (QString attributeID, qsequence.attributeIDs()) {
            QueryFilterSequence::Condition condition = qsequence.condition(attributeID, i-1);
            if (condition.operand == QueryFilterSequence::NoCondition) continue;
            QString attrID = (attributeID == qsequence.annotationLevelID()) ? "xText" : attributeID;
            if ((condition.operand == QueryFilterSequence::Contains) || (condition.operand == QueryFilterSequence::DoesNotContain)) {
                qsql.bindValue(QString(":crit_%1_%2").arg(attrID).arg(i), "%" + condition.value.toString() + "%");
            }
            else {
                qsql.bindValue(QString(":crit_%1_%2").arg(attrID).arg(i), condition.value);
            }
        }
    }
    return qsql;
}

QList<QueryOccurrencePointer *> SQLQueryEngineAnnotation::runQuery(QueryDefinition *qdef, AnnotationStructure *structure, QSqlDatabase &db)
{
    Q_UNUSED(structure)
    QList<QueryOccurrencePointer *> pointers;

    if (!qdef) return pointers;
    if (qdef->filterGroups.count() == 0) return pointers;
    QueryFilterGroup qfiltergroup = qdef->filterGroups.first();
    foreach (QueryFilterSequence qsequence, qfiltergroup.filterSequences) {
        QSqlQuery sql = getSqlQueryForSequence(qsequence, db);
        sql.exec();
        if (sql.lastError().isValid()) {
            qDebug() << sql.lastError();
            continue;
        }
        while (sql.next()) {
            QueryOccurrencePointer *pointer = new QueryOccurrencePointer();
            pointer->corpusID = sql.value(0).toString();
            pointer->communicationID = sql.value(1).toString();
            pointer->annotationID = sql.value(2).toString();
            pointer->speakerID = sql.value(3).toString();
            pointer->annotationLevelID = qsequence.annotationLevelID();
            pointer->intervalNoMin = sql.value(4).toLongLong();
            pointer->tMin_nsec = sql.value(5).toLongLong();
            pointer->intervalNoMax = sql.value(6).toLongLong();
            pointer->tMax_nsec = sql.value(7).toLongLong();
            pointers << pointer;
        }
    }
    return pointers;
}

QueryOccurrence *SQLQueryEngineAnnotation::getOccurrence(QueryOccurrencePointer *pointer, QueryDefinition *qdef,
                                                         AnnotationStructure *structure, QSqlDatabase &db,
                                                         int lengthContextLeft, int lengthContextRight)
{
    if (!structure) return 0;
    // Levels to include in results
    QStringList levelIDs;
    QPair<QString, QString> resultLA;
    foreach (resultLA, qdef->resultLevelsAttributes) {
        if (structure->hasLevel(resultLA.first))
            levelIDs << resultLA.first;
    }
    if (levelIDs.isEmpty()) return 0;
    // Results
    QMap<QString, QList<QueryOccurrence::ResultInterval> > resultIntervals;
    QList<QueryOccurrence::ResultInterval> resultIntervalsForLevel;
    QList<Interval *> intervals;
    // Get context for occurrence level
    int intervalNoMin = pointer->intervalNoMin - lengthContextLeft; if (intervalNoMin < 1) intervalNoMin = 1;
    int intervalNoMax = pointer->intervalNoMax + lengthContextRight;

    intervals = SQLSerialiserAnnotation::getIntervals(
                AnnotationDatastore::Selection(pointer->annotationID, pointer->speakerID, levelIDs.first(), intervalNoMin, intervalNoMax),
                structure, db);
    for (int i = 0; i < intervals.count(); ++i) {
        int intervalNo = intervalNoMin + i;
        QueryOccurrence::ResultInterval::Type type;
        if      (intervalNo < pointer->intervalNoMin) type = QueryOccurrence::ResultInterval::LeftContext;
        else if (intervalNo > pointer->intervalNoMax) type = QueryOccurrence::ResultInterval::RightContext;
        else    type = QueryOccurrence::ResultInterval::Target;
        resultIntervalsForLevel << QueryOccurrence::ResultInterval(type, pointer->speakerID, intervalNo, intervals.at(i));
    }
    resultIntervals.insert(levelIDs.first(), resultIntervalsForLevel);
    // Get the intervals of other levels within the same context span
    RealTime tMinContext, tMaxContext;
    if (intervals.count() > 0) {
        tMinContext = intervals.first()->tMin();
        tMaxContext = intervals.last()->tMax();
        foreach (QString levelID, levelIDs) {
            if (levelID == levelIDs.first()) continue;  // this is already covered
            QStringList attributeIDs;                   // for this level
            // Which attributes to load for this level?
            foreach (resultLA, qdef->resultLevelsAttributes) {
                if (resultLA.first == levelID) attributeIDs << resultLA.second;
            }
            // Get intervals for this level from the database
            QList<Interval *> intervalsForLevel = SQLSerialiserAnnotation::getIntervals(
                        AnnotationDatastore::Selection(pointer->annotationID, pointer->speakerID, levelID, attributeIDs, tMinContext, tMaxContext),
                        structure, db);
            QList<QueryOccurrence::ResultInterval> resultIntervalsForExtraLevel;
            int intervalNo = 0;
            foreach (Interval *intv, intervalsForLevel) {
                QueryOccurrence::ResultInterval::Type type;
                if      (intv->tMax() < RealTime::fromNanoseconds(pointer->tMin_nsec)) type = QueryOccurrence::ResultInterval::LeftContext;
                else if (intv->tMin() > RealTime::fromNanoseconds(pointer->tMax_nsec)) type = QueryOccurrence::ResultInterval::RightContext;
                else    type = QueryOccurrence::ResultInterval::Target;
                resultIntervalsForExtraLevel << QueryOccurrence::ResultInterval(type, pointer->speakerID, intervalNo, intv);
                intervalNo++;
            }
            resultIntervals.insert(levelID, resultIntervalsForExtraLevel);
        }
    }
    // Return the result
    return new QueryOccurrence(pointer->corpusID, pointer->communicationID, pointer->annotationID, resultIntervals);
}

bool SQLQueryEngineAnnotation::updateAnnotationsFromQueryOccurrences(const QList<QueryOccurrence *> &occurrences, AnnotationStructure *structure, QSqlDatabase &db)
{
    Q_UNUSED(occurrences)
    Q_UNUSED(structure)
    Q_UNUSED(db)
    return false;
}

} // namespace Core
} // namespace Praaline
