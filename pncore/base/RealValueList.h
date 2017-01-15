#ifndef REALVALUELIST_H
#define REALVALUELIST_H

/*
    Praaline - Core module - Base
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
#include <QList>

class PRAALINE_CORE_SHARED_EXPORT  RealValueList : public QList<double>
{
public:
    RealValueList();

    void recalculate();
    void append(const double &x);

    inline RealValueList &operator+=(const double &x)
        { RealValueList::append(x); return *this; }
    inline RealValueList &operator<< (const double &x)
        { RealValueList::append(x); return *this; }

    double mean() const;
    double stddev() const;
    double variance() const;
    double sum() const;

    double zscore(int i) const;
    double zscore_calculate(double x) const;

private:
    double m_mean;
    double m_M2;
    double m_sum;
};

#endif // REALVALUELIST_H
