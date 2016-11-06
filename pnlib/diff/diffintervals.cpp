#include <QString>
#include <QList>
#include <QVector>
#include "pncore/annotation/Interval.h"
#include "dtl.h"
using namespace std;
using namespace Praaline::Core;
using dtl::Diff;

#include "diffintervals.h"

DiffIntervals::DiffIntervals()
{
}

DiffIntervals::~DiffIntervals()
{
}

Diff<string> DiffIntervals::stringDiff(QList<Interval *> listA, QList<Interval *> listB,
                                       QString attributeID_A, QString attributeID_B)
{
    vector<string> sequenceA;
    vector<string> sequenceB;
    foreach (Interval *intvA, listA) {
        string A = (attributeID_A.isEmpty()) ? intvA->text().toStdString()
                                             : intvA->attribute(attributeID_A).toString().toStdString();
        sequenceA.push_back(A);
    }
    foreach (Interval *intvB, listB) {
        string B = (attributeID_B.isEmpty()) ? intvB->text().toStdString()
                                             : intvB->attribute(attributeID_B).toString().toStdString();
        sequenceA.push_back(B);
    }
    Diff<string> d(sequenceA, sequenceB);
    d.compose();
    return d;
}

Diff<Interval *, vector<Interval *>, CompareInterval>
DiffIntervals::intervalDiff(QList<Interval *> listA, QList<Interval *> listB, bool compareBoundaries,
                            QString attributeID_A, QString attributeID_B)
{
    vector<Interval *> sequenceA = listA.toVector().toStdVector();
    vector<Interval *> sequenceB = listB.toVector().toStdVector();
    CompareInterval comparator(compareBoundaries, attributeID_A, attributeID_B);
    Diff<Interval *, vector<Interval *>, CompareInterval> d(sequenceA, sequenceB, false, comparator);
    d.compose();
    return d;
}

QString DiffIntervals::diffTable(Diff<Interval *, vector<Interval *>, CompareInterval> &diff)
{
    QString ret;
    dtl::Ses<Interval *> ses = diff.getSes();
    dtl::Ses<Interval *>::sesElemVec sesElemVec = ses.getSequence();
    for (int i = 0; i < sesElemVec.size(); ++i) {
        dtl::edit_t editType = sesElemVec[i].second.type;
        Interval *intv = sesElemVec[i].first;
        if (editType == dtl::SES_COMMON)
            ret.append("=\t").append(intv->text()).append("\t").append(intv->text()).append("\n");
        else if (editType == dtl::SES_DELETE)
            ret.append("-\t").append(intv->text()).append("\t").append("").append("\n");
        else
            ret.append("+\t").append("").append("\t").append(intv->text()).append("\n");
    }
    return ret;
}
