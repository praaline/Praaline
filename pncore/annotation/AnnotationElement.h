#ifndef ANNOTATIONELEMENT_H
#define ANNOTATIONELEMENT_H

/*
    Praaline - Annotation module
    Copyright (c) 2011-2015 George Christodoulides

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
#include <QHash>
#include "base/RealTime.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT AnnotationElement
{
public:
    explicit AnnotationElement();
    AnnotationElement(const QString &text);
    AnnotationElement(const QString &text, const QHash<QString, QVariant> &attributes);
    AnnotationElement(const AnnotationElement &other);
    inline virtual ~AnnotationElement() {}
    // Note: Requirements for QVariant are a public default constructor, a public copy constructor, and a public destructor

    enum ElementType {
        Type_Element,
        Type_Point,
        Type_Interval,
        Type_Sequence,
        Type_Relation
    };
    inline virtual ElementType elementType() const {
        return Type_Element;
    }

    inline virtual QString text() const {
        return m_text;
    }
    inline virtual void setText(const QString &text) {
        m_text = text;
    }
    inline virtual void appendText(const QString &appendedText) {
        m_text.append(appendedText);
    }
    inline virtual void replaceText(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive) {
        m_text.replace(before, after, cs);
    }
    inline virtual void replaceAttributeText(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive) {
        if (!m_attributes.contains(attributeID)) return;
        m_attributes[attributeID] = m_attributes[attributeID].toString().replace(before, after, cs);
    }

    inline virtual QVariant attribute(const QString &name) const {
        if (name == "text") return text();
        return m_attributes.value(name, QVariant());
    }
    inline virtual void setAttribute(const QString &name, QVariant value){
        if (name == "text")
            setText(value.toString());
        else
            m_attributes.insert(name, value);
    }
    inline virtual const QVariantHash &attributes() const {
        return m_attributes;
    }

protected:
    QString m_text;
    QVariantHash m_attributes;

    friend class AnnotationTier;
};

} // namespace Core
} // namespace Praaline

Q_DECLARE_METATYPE(Praaline::Core::AnnotationElement)

#endif // ANNOTATIONELEMENT_H
