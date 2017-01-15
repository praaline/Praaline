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
    AnnotationElement(text), m_time(time)
{
}

Point::Point(const RealTime &time, const QString &text, const QHash<QString, QVariant> &attributes) :
    AnnotationElement(text, attributes), m_time(time)
{
}

Point::Point(const Point &copy) :
    AnnotationElement(copy.m_text, copy.m_attributes), m_time(copy.m_time)
{
}

Point::Point(const Point *copy)
{
    m_time = copy->m_time;
    m_text = copy->m_text;
    m_attributes = copy->m_attributes;
}

Point::~Point()
{
}

QVariant Point::attribute(const QString &name) const
{
    if (name == "time") return time().toDouble();
    if (name == "timeNanoseconds") return time().toNanoseconds();
    return AnnotationElement::attribute(name);
}

void Point::setAttribute(const QString &name, QVariant value) {
    if ((name == "time") || (name == "timeNanoseconds")) return;
    AnnotationElement::setAttribute(name, value);
}

} // namespace Core
} // namespace Praaline
