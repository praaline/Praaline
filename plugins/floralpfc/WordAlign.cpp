#include <QString>
#include <QList>
#include "WordAlign.h"

struct WordAlignData {
    WordAlignData() :
        ins_pen(0), del_pen(0), sub_pen(0), matches(0)
    {}

    QList<QList<int> > matrixAlign;
    QList<QList<int> > matrixBacktrack;
    QList<WordAlign::AlignmentItem> alignment;
    int ins_pen, del_pen, sub_pen, matches;
    QList<QString> wordsReference;
    QList<QString> wordsHypothesis;

    static const int INS = 1;
    static const int DEL = 2;
    static const int MATCH = 4;
    static const int SUB = 3;
};

WordAlign::WordAlign() :
    d(new WordAlignData)
{
}

WordAlign::~WordAlign()
{
    delete d;
}

void WordAlign::initialise()
{
    d->ins_pen = 0; d->del_pen = 0; d->sub_pen = 0; d->matches = 0;
    d->alignment.clear();
    d->matrixAlign.clear();
    d->matrixBacktrack.clear();
    for (int i = 0; i < d->wordsReference.count() + 1; ++i) {
        d->matrixAlign << QList<int>();
        d->matrixBacktrack << QList<int>();
        for (int j = 0; j < d->wordsHypothesis.count() + 1; ++j) {
            d->matrixAlign[i].append(0);
            d->matrixBacktrack[i].append(0);
        }
    }
    for (int j = 0; j < d->wordsHypothesis.count() + 1; ++j){
        d->matrixAlign[0][j] = j;
        d->matrixBacktrack[0][j] = d->INS;
    }
    for (int i = 0; i < d->wordsReference.count() + 1; ++i) {
        d->matrixAlign[i][0] = i;
        d->matrixBacktrack[i][0] = d->DEL;
    }
}

void WordAlign::align(QList<QString> wordsReference, QList<QString> wordsHypothesis)
{
    d->wordsReference = wordsReference;
    d->wordsHypothesis = wordsHypothesis;
    initialise();
    int cost(0);
    for (int i = 1; i < d->wordsReference.count() + 1; ++i) {
        for (int j = 1; j < d->wordsHypothesis.count() + 1; ++j) {
            if (wordsReference[i - 1] != wordsHypothesis[j - 1])
                cost = 1;
            else
                cost = 0;
            int ins = d->matrixAlign[i][j - 1] + 1;
            int dels = d->matrixAlign[i - 1][j] + 1;
            int substs = d->matrixAlign[i - 1][j - 1] + cost;
            int m = qMin(qMin(ins, dels), substs);
            d->matrixAlign[i][j] = m;

            if (m == substs)
                d->matrixBacktrack[i][j] = d->MATCH + cost;
            else if (m == ins)
                d->matrixBacktrack[i][j] = d->INS;
            else if (m == dels)
                d->matrixBacktrack[i][j] = d->DEL;
        }
    }
    backtrace();
}

void WordAlign::backtrace()
{
    int i = d->wordsReference.count();
    int j = d->wordsHypothesis.count();

    d->ins_pen = 0; d->del_pen = 0; d->sub_pen = 0; d->matches = 0;
    d->alignment.clear();
    WordAlign::AlignmentItem item;
    while (!((i ==0) && (j == 0))) {
        int pointer = d->matrixBacktrack[i][j];
        if (pointer == d->INS) {
            item.ref = ""; item.hyp = d->wordsHypothesis[j - 1]; item.op = "INS";
            d->alignment << item;
            d->ins_pen++;
            j--;
        }
        else if (pointer == d->DEL) {
            item.ref = d->wordsReference[i - 1]; item.hyp = ""; item.op = "DEL";
            d->alignment << item;
            d->del_pen++;
            i--;
        }
        else if (pointer == d->MATCH) {
            item.ref = d->wordsReference[i - 1]; item.hyp = d->wordsHypothesis[j - 1]; item.op = "===";
            d->alignment << item;
            d->matches++;
            j--; i--;
        }
        else {
            item.ref = d->wordsReference[i - 1]; item.hyp = d->wordsHypothesis[j - 1]; item.op = "SUB";
            d->alignment << item;
            d->sub_pen++;
            j--; i--;
        }
    }
}

QList<WordAlign::AlignmentItem> WordAlign::alignment() const
{
    return d->alignment;
}

double WordAlign::WER() const
{
    return ((double)(d->ins_pen + d->del_pen + d->sub_pen)) / ((double)d->wordsReference.count()) * 100.0;
}

int WordAlign::insertionsCount() const
{
    return d->ins_pen;
}

int WordAlign::deletionsCount() const
{
    return d->del_pen;
}

int WordAlign::substitutionsCount() const
{
    return d->sub_pen;
}

int WordAlign::matchesCount() const
{
    return d->matches;
}

int WordAlign::wordsReferenceCount() const
{
    return d->wordsReference.count();
}

int WordAlign::wordsHypothesisCount() const
{
    return d->wordsHypothesis.count();
}

QString WordAlign::alignmentText() const
{
    QString lineRef, lineHyp, lineOp;
    for (int i = d->alignment.count() - 1; i >= 0; --i) {
        WordAlign::AlignmentItem item = d->alignment.at(i);
        int width = qMax(qMax(item.ref.length(), item.hyp.length()), item.op.length()) + 1;
        lineRef = lineRef.append(item.ref.leftJustified(width, ' '));
        lineHyp = lineHyp.append(item.hyp.leftJustified(width, ' '));
        lineOp  = lineOp.append(item.op.leftJustified(width, ' '));
    }
    return QString("%1\n%2\n%3").arg(lineRef).arg(lineHyp).arg(lineOp);
}

QString WordAlign::alignmentTextOriginal(const QString &format) const
{
    QString ret;
    for (int i = d->alignment.count() - 1; i >= 0; --i) {
        WordAlign::AlignmentItem item = d->alignment.at(i);
        if      (item.op == "DEL") {
            ret.append("\\st{").append(item.ref).append("} ");
        }
        else if (item.op == "SUB") {
            ret.append("\\textcolor{magenta}{").append(item.ref).append("} ");
        }
        else if (item.op != "INS") {
            ret.append(item.ref).append(" ");
        }
    }
    return ret.trimmed();
}

QString WordAlign::alignmentTextModified(const QString &format) const
{
    QString ret;
    for (int i = d->alignment.count() - 1; i >= 0; --i) {
        WordAlign::AlignmentItem item = d->alignment.at(i);
        if      (item.op == "INS") {
            ret.append("\\textcolor{blue}{").append(item.hyp).append("} ");
        }
        else if (item.op == "SUB") {
            ret.append("\\textcolor{magenta}{").append(item.hyp).append("} ");
        }
        else if (item.op != "DEL") {
            ret.append(item.hyp).append(" ");
        }
    }
    return ret.trimmed();
}
