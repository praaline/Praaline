/*
    Praaline - Annotation module
    Copyright (c) 2011-12 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include <QString>
#include "base/RealTime.h"
#include "Point.h"

namespace Praaline {
namespace Core {

Point::Point()
{
}

Point::Point(const RealTime &time, const QString &text) :
    AnnotationElement(time, text)
{
}

Point::Point(const RealTime &time, const QString &text, const QHash<QString, QVariant> &attributes) :
    AnnotationElement(time, text, attributes)
{
}

Point::Point(const Point &copy) :
    AnnotationElement(copy.m_time, copy.m_text, copy.m_attributes)
{
}

Point::Point(const Point *copy)
{
    m_time = copy->m_time;
    m_text = copy->m_text;
    foreach (QString attributeID, copy->m_attributes.keys())
        m_attributes.insert(attributeID, copy->m_attributes.value(attributeID));
}

Point::~Point()
{
}

} // namespace Core
} // namespace Praaline
