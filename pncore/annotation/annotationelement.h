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


class PRAALINE_CORE_SHARED_EXPORT AnnotationElement
{
public:
    AnnotationElement();
    AnnotationElement(const RealTime &time, const QString &text = QString());
    AnnotationElement(const RealTime &time, const QString &text, const QHash<QString, QVariant> &attributes);
    inline virtual ~AnnotationElement() {}

    inline virtual RealTime time() const {
        return m_time;
    }
    inline QString text() const {
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

    inline QVariant attribute(const QString &name) const {
        if (name == "time") return time().toDouble();
        if (name == "text") return text();
        return m_attributes.value(name, QVariant());
    }
    inline virtual void setAttribute(const QString &name, QVariant value){
        m_attributes.insert(name, value);
    }
    inline const QHash<QString, QVariant> &attributes() const {
        return m_attributes;
    }

protected:
    RealTime m_time;
    QString m_text;
    QHash<QString, QVariant> m_attributes;

    friend class AnnotationTier;
};

#endif // ANNOTATIONELEMENT_H
