#ifndef RELATIONTIER_H
#define RELATIONTIER_H

/*
    Praaline - Core module - Annotation
    Copyright (c) 2011-2017 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include <QList>
#include "AnnotationTier.h"
#include "Relation.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT RelationTier : public AnnotationTier
{
    Q_OBJECT
public:
    // Constructors, destructor
    RelationTier(const QString &name = QString(), QObject *parent = 0);
    RelationTier(const QString &name, const QList<Relation *> &relations, QObject *parent = 0);
    RelationTier(const RelationTier *copy, QString name = QString(), QObject *parent = 0);
    virtual ~RelationTier();

    // Implementation of AnnotationTier
    AnnotationTier::TierType tierType() const override
        { return AnnotationTier::TierType_Relations; }
    int count() const override
        { return m_relations.count(); }
    bool isEmpty() const override;
    void clear() override;
    Relation *at(int index) const override;
    Relation *first() const override;
    Relation *last() const override;
    QStringList getDistinctLabels() const override;
    QList<QVariant> getDistinctValues(const QString &attributeID) const override;
    void replace(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive) override;
    void fillEmptyWith(const QString &attributeID,const QString &filler) override;

    // Accessors for Relations
    Relation* relation(int index) const;
    QList<Relation *> relations() const;

    // Mutators for Relations
    void addRelation(Relation *relation);
    void addRelations(QList<Relation *> relations);
    void removeRelationAt(int i);

protected:
    QList<Relation *> m_relations;

private:
    static bool compareRelations(Relation *A, Relation *B);
};

} // namespace Core
} // namespace Praaline

#endif // RELATIONTIER_H
