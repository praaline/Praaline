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
#include "Interval.h"

namespace Praaline {
namespace Core {

Interval::Interval()
{
}

Interval::Interval(const RealTime tMin, const RealTime tMax, const QString &text) :
    AnnotationElement(text), m_tMin(tMin), m_tMax(tMax)
{
}

Interval::Interval(const RealTime tMin, const RealTime tMax, const QString &text, const QHash<QString, QVariant> &attributes) :
    AnnotationElement(text, attributes), m_tMin(tMin), m_tMax(tMax)
{
}

Interval::Interval(const Interval &copy) :
    AnnotationElement(copy.m_text, copy.m_attributes), m_tMin(copy.m_tMin), m_tMax(copy.m_tMax)
{
}

Interval::Interval(const Interval *copy, bool copyAttributes)
{
    m_tMin = copy->m_tMin;
    m_tMax = copy->m_tMax;
    m_text = copy->m_text;
    if (copyAttributes) {
        foreach (QString attributeID, copy->m_attributes.keys())
            m_attributes.insert(attributeID, copy->m_attributes.value(attributeID));
    }
}

Interval::Interval(const RealTime tMin, const RealTime tMax, const Interval *copy) :
    AnnotationElement(copy->text()), m_tMin(tMin), m_tMax(tMax)
{
    foreach (QString attributeID, copy->m_attributes.keys())
        m_attributes.insert(attributeID, copy->m_attributes.value(attributeID));
}

Interval::Interval(const QList<Interval *> &intervals, const QString &separator)
{
    // Constructor that concatenates intervals into a new one
    bool first = true;
    if (intervals.count() > 0) {
        m_tMin = intervals[0]->tMin();
        m_tMax = intervals[intervals.count() - 1]->tMax();
        foreach (Interval *intv, intervals) {
            if (!first) m_text.append(separator);
            m_text.append(intv->text().trimmed());
            first = false;
        }
    }
}

QVariant Interval::attribute(const QString &name) const
{
    if (name == "tMin") return tMin().toDouble();
    if (name == "tMinNanoseconds") return tMin().toNanoseconds();
    if (name == "tMax") return tMax().toDouble();
    if (name == "tMaxNanoseconds") return tMax().toNanoseconds();
    if (name == "tCenter") return tCenter().toDouble();
    if (name == "tCenterNanoseconds") return tCenter().toNanoseconds();
    if (name == "duration") return duration().toDouble();
    if (name == "durationNanoseconds") return duration().toNanoseconds();
    return AnnotationElement::attribute(name);
}

void Interval::setAttribute(const QString &name, QVariant value) {
    if ((name == "tMin") || (name == "tMinNanoseconds") || (name == "tMax") || (name == "tMaxNanoseconds") ||
        (name == "tCenter") || (name == "tCenterNanoseconds") || (name == "duration") || (name == "durationNanoseconds"))
        return; // these are read-only properties (use tier to move intervals)
    AnnotationElement::setAttribute(name, value);
}

bool Interval::overlaps(const Interval &other, const RealTime threshold) const
{
    // default threshold is 0
    return (((m_tMax - other.m_tMin) > threshold) &&
            ((other.m_tMax - m_tMin) > threshold));
}

bool Interval::covers(const Interval &other, const RealTime threshold) const
{
    // default threshold is 0
    return (((m_tMin - other.m_tMin) <= threshold) &&
            ((other.m_tMax - m_tMax) <= threshold));
}

bool Interval::isCovered(const Interval &other, const RealTime threshold) const
{
    // default threshold is 0
    return (((other.m_tMin - m_tMin) <= threshold) &&
            ((m_tMax - other.m_tMax) <= threshold));
}

bool Interval::contains(const RealTime timePoint) const
{
    return ((m_tMin <= timePoint) && (timePoint <= m_tMax));
}

bool Interval::isPauseSilent() const
{
    return (m_text == "" || m_text == "_" || m_text == "#"); // TODO: make customizable!
}

int Interval::compare(const Interval &other) const
{
    if (other.m_tMin < m_tMin) return 1;
    if (other.m_tMin > m_tMin) return -1;
    if (other.m_tMax < m_tMax) return 1;
    if (other.m_tMax > m_tMax) return -1;
    return other.m_text.compare(m_text);
}

} // namespace Core
} // namespace Praaline
