#ifndef PROSODICBOUNDARIES_H
#define PROSODICBOUNDARIES_H

#include <QString>
#include <QPair>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

class ProsodicBoundaries
{
public:

    static QPair<int, int> windowNoPause(IntervalTier *tier_syll, int i, int windowLeft, int windowRight);
    static bool mean(double &mean, IntervalTier *tier_syll, QString attributeName, int i, int windowLeft, int windowRight,
                     bool checkStylized);
    static double relative(IntervalTier *tier_syll, QString attributeName, int i, int windowLeft, int windowRight,
                           bool checkStylized, bool logarithmic);
    static bool isLexical(Interval *token);
    static QString categorise_CLI_INT_LEX(Interval *token);

    static QList<QString> analyseBoundaryListToStrings(Corpus *corpus, const QString &annotID,
                                                       QList<int> syllIndices, QStringList additionalAttributeIDs);
    static void analyseBoundaryList(QTextStream &out, Corpus *corpus, const QString &annotID,
                                    QList<int> syllIndices, QStringList additionalAttributeIDs);
    static QList<QString> analyseCorpusSampleToStrings(Corpus *corpus, const QString &annotID);
    static void analyseCorpusSample(QTextStream &out, Corpus *corpus, const QString &annotID);

private:
    ProsodicBoundaries() {}
    static QStringList POS_CLI;
    static QStringList POS_INT;
    static QStringList POS_LEX;
};

#endif // PROSODICBOUNDARIES_H
