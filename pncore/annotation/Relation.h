#ifndef RELATION_H
#define RELATION_H

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
#include <QString>
#include <QVariant>
#include "base/RealTime.h"
#include "AnnotationElement.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT Relation : public AnnotationElement
{
    friend class RelationsTier;
public:
    Relation();
    Relation(int indexFrom, int indexTo, const QString &text);
    Relation(int indexFrom, int indexTo, const QString &text, const QHash<QString, QVariant> &attributes);
    Relation(const Relation &copy);
    virtual ~Relation() {}

    // Properties
    inline int indexFrom () const
        { return m_indexFrom; }
    inline int indexTo() const
        { return m_indexTo; }

    // Override
    virtual QVariant attribute(const QString &name) const override;
    virtual void setAttribute(const QString &name, QVariant value) override;
    inline virtual ElementType elementType() const override {
        return Type_Relation;
    }

    int compare(const Relation &other) const;

protected:
    int m_indexFrom;
    int m_indexTo;
};


} // namespace Core
} // namespace Praaline

Q_DECLARE_METATYPE(Praaline::Core::Relation)

#endif // RELATION_H
