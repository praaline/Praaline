#ifndef REALVALUELIST_H
#define REALVALUELIST_H

#include <QList>
#include "pncore_global.h"

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
