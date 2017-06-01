#ifndef INTERVAL_H
#define INTERVAL_H

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

class PRAALINE_CORE_SHARED_EXPORT Interval : public AnnotationElement
{
    friend class IntervalTier;

public:
    Interval();
    Interval(const RealTime tMin, const RealTime tMax, const QString &text);
    Interval(const RealTime tMin, const RealTime tMax, const QString &text, const QHash<QString, QVariant> &attributes);
    Interval(const Interval &copy);
    Interval(const Interval *copy, bool copyAttributes = true);
    Interval(const RealTime tMin, const RealTime tMax, const Interval *copy);
    Interval(const QList<Interval *> &intervals, const QString &separator = QString());
    virtual ~Interval() {}

    // Properties
    inline RealTime tMin() const
        { return m_tMin; }
    inline RealTime tMax() const
        { return m_tMax; }
    inline RealTime tCenter() const
        { return (m_tMin + m_tMax) / 2.0; }
    inline RealTime duration() const
        { return (m_tMax - m_tMin); }

    // Overrides
    virtual QVariant attribute(const QString &name) const override ;
    virtual void setAttribute(const QString &name, QVariant value) override;
    inline virtual ElementType elementType() const override {
        return Type_Interval;
    }

    // Methods for Intervals
    bool overlaps(const Interval &other, const RealTime threshold = RealTime(0, 0)) const;
    bool covers(const Interval &other, const RealTime threshold = RealTime(0, 0)) const;
    bool isCovered(const Interval &other, const RealTime threshold = RealTime(0, 0)) const;
    bool contains(const RealTime timePoint) const;
    bool isPauseSilent() const;
    int compare(const Interval &other) const;

protected:
    RealTime m_tMin;
    RealTime m_tMax;
};

} // namespace Core
} // namespace Praaline

Q_DECLARE_METATYPE(Praaline::Core::Interval)
Q_DECLARE_METATYPE(Praaline::Core::Interval *)

#endif // INTERVAL_H
