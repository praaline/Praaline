#ifndef POINT_H
#define POINT_H

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

#include "pncore_global.h"
#include <QString>
#include "base/RealTime.h"
#include "annotationelement.h"

class PRAALINE_CORE_SHARED_EXPORT Point : public AnnotationElement
{
public:
    Point();
    Point(const RealTime &time, const QString &text = QString());
    Point(const RealTime &time, const QString &text, const QHash<QString, QVariant> &attributes);
    Point(const Point &copy);
    Point(const Point *copy);
    virtual ~Point();

    friend class PointTier;
};

#endif // POINT_H
