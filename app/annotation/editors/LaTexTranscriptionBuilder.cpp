#include <QString>

#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "LaTexTranscriptionBuilder.h"

LaTexTranscriptionBuilder::LaTexTranscriptionBuilder()
{

}

//foreach (QString speakerID, d->currentTierGroups.keys()) {
//    IntervalTier *tierTokens = d->currentTierGroups.value(speakerID)->getIntervalTierByName(d->levelTokens);
//    if (!tierTokens) continue;
//    QList<QPair<int, int> > segments = calculateIndicesForSegmentation(tierTokens);
//    QPair<int, int> segment;
//    foreach (segment, segments) {
//        QString tikz = createTikzDependencyText(tierTokens, segment.first, segment.second);
//        d->editor->appendPlainText(tikz);
//    }
//}

QString LaTexTranscriptionBuilder::createTikzDependencyText(IntervalTier *tokens, int indexFrom, int indexTo)
{
    if (indexFrom < 0) indexFrom = 0;
    if (indexTo >= tokens->count()) indexTo = tokens->count() - 1;
    QString ret;
    ret.append("\\begin{dependency}\n");
    ret.append("\\tikzstyle{POS}=[font=\\footnotesize]\n");
    ret.append("\\begin{deptext}\n");
    QString textGloss, textTokens;
    QString sep = " \\& ";
    for (int i = indexFrom; i <= indexTo; ++i) {
        Interval *token = tokens->at(i);
        if (token->isPauseSilent()) continue;
        textGloss.append("|[POS]| ").append(token->attribute("pos_min").toString()).append(sep);
        textTokens.append(token->text()).append(sep);
    }
    if (textGloss.length() > sep.length()) textGloss.chop(sep.length());
    if (textTokens.length() > sep.length()) textTokens.chop(sep.length());
    ret.append(textGloss).append(" \\\\\n");
    ret.append(textTokens).append(" \\\\\n");
    ret.append("\\end{deptext}\n");
    ret.append("\\end{dependency}\n");
    return ret;
}


QList<QPair<int, int> > LaTexTranscriptionBuilder::calculateIndicesForSegmentation(IntervalTier *tokens)
{
    QList<QPair<int, int> > list;
    int indexFrom(0), indexTo(0);
    while (indexTo < tokens->count()) {
        bool breakHere = false;
        Interval *token = tokens->at(indexTo);
        if (token->isPauseSilent() && token->duration().toDouble() > 0.250) breakHere = true;
        if (breakHere) {
            if (indexFrom != indexTo) list << QPair<int, int>(indexFrom, indexTo);
            indexTo++;
            indexFrom = indexTo;
        } else {
            indexTo++;
        }
    }
    return list;
}
