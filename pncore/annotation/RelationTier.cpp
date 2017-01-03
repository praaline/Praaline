#include <QString>
#include <QTextStream>
#include "RelationTier.h"
#include "IntervalTier.h"
#include "AnnotationTier.h"

namespace Praaline {
namespace Core {

// ==========================================================================================================
// Constructors - destructor
// ==========================================================================================================
RelationTier::RelationTier(const QString &name, QObject *parent) :
    AnnotationTier(parent)
{
    m_name = name;
}

RelationTier::RelationTier(const QString &name, const QList<Relation *> &relations, QObject *parent) :
    AnnotationTier(parent)
{
    m_name = name;
    m_relations = relations;
    if (m_relations.count() == 0) return;
    qSort(m_relations.begin(), m_relations.end(), RelationTier::compareRelations);
}

RelationTier::RelationTier(const RelationTier *copy, QString name, QObject *parent) :
    AnnotationTier(parent)
{
    if (!copy) return;
    m_name = (name.isEmpty()) ? copy->name() : name;
    // deep copy of Relations
    foreach (Relation *relation, copy->relations()) {
        m_relations << new Relation(*relation);
    }
}

RelationTier::~RelationTier()
{
    qDeleteAll(m_relations);
}

// ==============================================================================================================================
// Implementation of AnnotationTier
// ==============================================================================================================================

bool RelationTier::isEmpty() const
{
    return m_relations.isEmpty();
}

void RelationTier::clear()
{
    qDeleteAll(m_relations);
    m_relations.clear();
}

Relation *RelationTier::at(int index) const
{
    return m_relations.at(index);
}

Relation *RelationTier::first() const
{
    if (isEmpty()) return Q_NULLPTR;
    return m_relations.first();
}

Relation *RelationTier::last() const
{
    if (isEmpty()) return Q_NULLPTR;
    return m_relations.last();
}

QList<QString> RelationTier::getDistinctTextLabels() const
{
    QList<QString> ret;
    foreach (Relation *relation, m_relations) {
        if (!ret.contains(relation->text())) ret << relation->text();
    }
    return ret;
}

QList<QVariant> RelationTier::getDistinctAttributeValues(const QString &attributeID) const
{
    QList<QVariant> ret;
    foreach (Relation *relation, m_relations) {
        if (!ret.contains(relation->attribute(attributeID))) ret.append(relation->attribute(attributeID));
    }
    return ret;
}

void RelationTier::replaceTextLabels(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    foreach (Relation *relation, m_relations) {
        relation->replaceText(before, after, cs);
    }
}

void RelationTier::fillEmptyTextLabelsWith(const QString &filler)
{
    foreach (Relation *relation, m_relations) {
        if (relation->text().isEmpty()) relation->setText(filler);
    }
}


void RelationTier::replaceAttributeText(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    foreach (Relation *relation, m_relations) {
        relation->replaceAttributeText(attributeID, before, after, cs);
    }
}

void RelationTier::fillEmptyAttributeTextWith(const QString &attributeID,const QString &filler)
{
    foreach (Relation *relation, m_relations) {
        if (relation->attribute(attributeID).toString().isEmpty())
            relation->setAttribute(attributeID, filler);
    }
}

// ==============================================================================================================================
// Accessors and mutators for Relations
// ==============================================================================================================================

Relation *RelationTier::relation(int index) const
{
    return m_relations.value(index);
}

QList<Relation *> RelationTier::relations() const
{
    return m_relations;
}

bool RelationTier::compareRelations(Relation *A, Relation *B) {
    if (A->indexFrom() == B->indexFrom())
        return (A->indexTo() < B->indexTo());
    // else
    return (A->indexFrom() < B->indexFrom());
}

void RelationTier::addRelation(Relation *relation)
{
    m_relations << relation;
    qSort(m_relations.begin(), m_relations.end(), RelationTier::compareRelations);
}

void RelationTier::addRelations(QList<Relation *> relations)
{
    m_relations << relations;
    qSort(m_relations.begin(), m_relations.end(), RelationTier::compareRelations);
}

void RelationTier::removeRelationAt(int i)
{
    m_relations.removeAt(i);
}

} // namespace Core
} // namespace Praaline
