#ifndef QUERYFILTERSEQUENCE_H
#define QUERYFILTERSEQUENCE_H

#include "pncore_global.h"
#include <QString>
#include <QList>
#include <QVariant>

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT QueryFilterSequence
{
public:
    QueryFilterSequence(QString annotationLevelID);
    ~QueryFilterSequence();

    enum Operand {
        NoCondition,
        Equals,
        DoesNotEqual,
        GreaterThan,
        GreaterThanOrEqual,
        LessThan,
        LessThanOrEqual,
        Contains,
        DoesNotContain,
        Like
    };

    struct Condition {
        Condition(Operand operand, QVariant value) : operand(operand), value(value)
        {}
        Operand operand;
        QVariant value;
    };

    QString annotationLevelID() const;
    QStringList attributeIDs() const;
    int attributesCount() const;
    int sequenceLength() const;
    Condition condition(QString annotationAttributeID, int position) const;
    void setCondition(QString annotationAttributeID, int position, Condition condition);
    QString conditionString(QString annotationAttributeID, int position) const;
    void setConditionString(QString annotationAttributeID, int position, QString whereClause);
    void addAttribute(QString annotationAttributeID);
    void removeAttribute(QString annotationAttributeID);
    void expandSequence(int length = 1);
    void reduceSequence(int length = 1);
    QString friendlySyntax() const;

private:
    QString m_annotationLevelID;
    QMap<QString, QList<Condition> > m_conditions;
};

} // namespace Core
} // namespace Praaline

#endif // QUERYFILTERSEQUENCE_H
