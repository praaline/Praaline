#ifndef DIFFINTERVALS_H
#define DIFFINTERVALS_H

#include <QString>
#include "pncore/annotation/interval.h"
#include "dtl.h"
using namespace std;
using dtl::Diff;

class CompareInterval : dtl::Compare<Interval *>
{
public:
    CompareInterval(bool compareBoundaries, const QString &attributeID_A, const QString &attributeID_B) :
        m_compareBoundaries(compareBoundaries), m_attributeID_A(attributeID_A), m_attributeID_B(attributeID_B)
    {}
    virtual ~CompareInterval() {}

    bool compareBoundaries() const { return m_compareBoundaries; }
    QString attributeID_A() const { return m_attributeID_A; }
    QString attributeID_B() const { return m_attributeID_B; }

    virtual inline bool impl (Interval *e1, Interval *e2) const {
        bool ret;
        ret = (m_compareBoundaries) ? (e1->tMin() == e2->tMin() && e1->tMax() == e2->tMax()) : true;
        if (m_attributeID_A.isEmpty() && m_attributeID_B.isEmpty())
            ret = ret && (e1->text() == e2->text());
        else if (!m_attributeID_A.isEmpty() && !m_attributeID_B.isEmpty())
            ret = ret && (e1->attribute(m_attributeID_A) == e2->attribute(m_attributeID_B));
        else if (m_attributeID_A.isEmpty())
            ret = ret && (e1->text() == e2->attribute(m_attributeID_B).toString());
        else if (m_attributeID_B.isEmpty())
            ret = ret && (e1->attribute(m_attributeID_A).toString() == e2->text());
        return ret;
    }

private:
    const bool m_compareBoundaries;
    const QString m_attributeID_A;
    const QString m_attributeID_B;
};

class DiffIntervals
{
public:
    DiffIntervals();
    ~DiffIntervals();

    static Diff<string> stringDiff(QList<Interval *> listA, QList<Interval *> listB,
                                   QString attributeID_A = QString(), QString attributeID_B = QString());
    static Diff<Interval *, vector<Interval *>, CompareInterval>
    intervalDiff(QList<Interval *> listA, QList<Interval *> listB, bool compareBoundaries = false,
                 QString attributeID_A = QString(), QString attributeID_B = QString());

    static QString diffTable(Diff<Interval *, vector<Interval *>, CompareInterval> &diff);
};

#endif // DIFFINTERVALS_H
