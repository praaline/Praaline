/*
    Praaline - Annotation module
    Copyright (c) 2011-2016 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "AnnotationElement.h"
#include "Sequence.h"

namespace Praaline {
namespace Core {

Sequence::Sequence()
{
}

Sequence::Sequence(int indexFrom, int indexTo, const QString &text) :
    AnnotationElement(text), m_indexFrom(indexFrom), m_indexTo(indexTo)
{
}

Sequence::Sequence(int indexFrom, int indexTo, const QString &text, const QHash<QString, QVariant> &attributes) :
    AnnotationElement(text, attributes), m_indexFrom(indexFrom), m_indexTo(indexTo)
{
}

Sequence::Sequence(const Sequence &copy) :
    AnnotationElement(copy.m_text, copy.m_attributes), m_indexFrom(copy.m_indexFrom), m_indexTo(copy.m_indexTo)
{
}

QVariant Sequence::attribute(const QString &name) const
{
    if (name == "indexFrom") return indexFrom();
    if (name == "indexTo")   return indexTo();
    if (name == "length")    return length();
    return AnnotationElement::attribute(name);
}

void Sequence::setAttribute(const QString &name, QVariant value) {
    if ((name == "indexFrom") || (name == "indexTo") || (name == "length")) return;
    AnnotationElement::setAttribute(name, value);
}

int Sequence::compare(const Sequence &other) const
{
    if (other.m_indexFrom < m_indexFrom) return 1;
    if (other.m_indexFrom > m_indexFrom) return -1;
    if (other.m_indexTo < m_indexTo) return 1;
    if (other.m_indexTo > m_indexTo) return -1;
    return other.m_text.compare(m_text);
}

} // namespace Core
} // namespace Praaline
