#include "QueryFilterSequence.h"

namespace Praaline {
namespace Core {

QueryFilterSequence::QueryFilterSequence(QString annotationLevelID) :
    m_annotationLevelID(annotationLevelID)
{
}

QueryFilterSequence::~QueryFilterSequence()
{
}

QString QueryFilterSequence::annotationLevelID() const
{
    return m_annotationLevelID;
}

QStringList QueryFilterSequence::attributeIDs() const
{
    return m_conditions.keys();
}

int QueryFilterSequence::attributesCount() const
{
    return m_conditions.count();
}

int QueryFilterSequence::sequenceLength() const
{
    if (m_conditions.empty()) return 0;
    return m_conditions.first().count();
}

QueryFilterSequence::Condition QueryFilterSequence::condition(QString annotationAttributeID, int position) const
{
    if (position < 0 || position >= sequenceLength()) return Condition(NoCondition, QVariant());
    if (!m_conditions.contains(annotationAttributeID)) return Condition(NoCondition, QVariant());
    return m_conditions.value(annotationAttributeID).at(position);
}

void QueryFilterSequence::setCondition(QString annotationAttributeID, int position, Condition condition)
{
    if (position < 0 || position >= sequenceLength()) return;
    if (!m_conditions.contains(annotationAttributeID)) return;
    m_conditions[annotationAttributeID][position] = condition;
}

QString QueryFilterSequence::conditionString(QString annotationAttributeID, int position) const
{
    if (position < 0 || position >= sequenceLength()) return QString();
    if (!m_conditions.contains(annotationAttributeID)) return QString();
    Condition condition = m_conditions.value(annotationAttributeID).at(position);
    switch (condition.operand) {
    case Equals: return QString("%1").arg(condition.value.toString()); break;
    case DoesNotEqual: return QString("<> %1").arg(condition.value.toString()); break;
    case GreaterThan: return QString("> %1").arg(condition.value.toString()); break;
    case GreaterThanOrEqual: return QString(">= %1").arg(condition.value.toString()); break;
    case LessThan: return QString("< %1").arg(condition.value.toString()); break;
    case LessThanOrEqual: return QString("<= %1").arg(condition.value.toString()); break;
    case Contains: return QString("CONTAINS %1").arg(condition.value.toString()); break;
    case DoesNotContain: return QString("NOT_CONTAINS %1").arg(condition.value.toString()); break;
    case Like: return QString("LIKE %1").arg(condition.value.toString()); break;
    case NoCondition: return QString(); break;
    }
    return QString();
}

void QueryFilterSequence::setConditionString(QString annotationAttributeID, int position, QString whereClause)
{
    if (position < 0 || position >= sequenceLength()) return;
    if (!m_conditions.contains(annotationAttributeID)) return;
    Condition condition(NoCondition, QVariant());
    if      (whereClause.startsWith("<>")) { condition.operand = DoesNotEqual; condition.value = whereClause.remove(0, 2).trimmed(); }
    else if (whereClause.startsWith(">")) { condition.operand = GreaterThan; condition.value = whereClause.remove(0, 1).trimmed(); }
    else if (whereClause.startsWith("<=")) { condition.operand = GreaterThanOrEqual; condition.value = whereClause.remove(0, 2).trimmed(); }
    else if (whereClause.startsWith("<")) { condition.operand = LessThan; condition.value = whereClause.remove(0, 1).trimmed(); }
    else if (whereClause.startsWith("<=")) { condition.operand = LessThanOrEqual; condition.value = whereClause.remove(0, 2).trimmed(); }
    else if (whereClause.startsWith("CONTAINS ")) { condition.operand = Contains; condition.value = whereClause.remove(0, 9).trimmed(); }
    else if (whereClause.startsWith("NOT_CONTAINS ")) { condition.operand = DoesNotContain; condition.value = whereClause.remove(0, 13).trimmed(); }
    else if (whereClause.startsWith("LIKE ")) { condition.operand = Like; condition.value = whereClause.remove(0, 5).trimmed(); }
    else if (whereClause.startsWith("=")) { condition.operand = Equals; condition.value = whereClause.remove(0, 1).trimmed(); }
    else if (whereClause.isEmpty()) { condition.operand = NoCondition; condition.value = QVariant(); }
    else    { condition.operand = Equals; condition.value = whereClause.trimmed(); }
    m_conditions[annotationAttributeID][position] = condition;
}

void QueryFilterSequence::addAttribute(QString annotationAttributeID)
{
    if (m_conditions.contains(annotationAttributeID)) return;
    QList<Condition> conditions;
    for (int position = 0; position < sequenceLength(); ++position)
        conditions << Condition(NoCondition, QVariant());
    m_conditions.insert(annotationAttributeID, conditions);
}

void QueryFilterSequence::removeAttribute(QString annotationAttributeID)
{
    m_conditions.remove(annotationAttributeID);
}

void QueryFilterSequence::expandSequence(int length)
{
    foreach (QString annotationAttributeID, m_conditions.keys()) {
        for (int i = 1; i <= length; ++i)
            m_conditions[annotationAttributeID].append(Condition(NoCondition, QVariant()));
    }
}

void QueryFilterSequence::reduceSequence(int length)
{
    foreach (QString annotationAttributeID, m_conditions.keys()) {
        for (int i = 1; i <= length; ++i)
            m_conditions[annotationAttributeID].removeLast();
    }
}

QString QueryFilterSequence::friendlySyntax() const
{
    return QString();
}

} // namespace Core
} // namespace Praaline
