#include "DisMoSerialiserTiers.h"
using namespace DisMoAnnotator;

DisMoSerialiserTiers::DisMoSerialiserTiers(QObject *parent) :
    QObject(parent)
{
}

void DisMoSerialiserTiers::serializeToTiers(TokenList &TL, IntervalTier *tierMin, IntervalTier *tierMWU)
{
    if (!tierMin) return;
    if (!tierMWU) return;
    tierMin->clear();
    tierMWU->clear();
    QList<Interval *> list_tok_mwu, list_tok_min;
    foreach (Token *tl_tok_mwu, TL) {
        Interval *i_tok_mwu = new Interval(tl_tok_mwu->tMin(), tl_tok_mwu->tMax(), tl_tok_mwu->text());
        i_tok_mwu->setAttribute("pos-mwu", tl_tok_mwu->getTagPOS());
        i_tok_mwu->setAttribute("pos-ext-mwu", tl_tok_mwu->getTagPOSext());
        i_tok_mwu->setAttribute("lemma-mwu", tl_tok_mwu->getLemma());
        i_tok_mwu->setAttribute("discourse", tl_tok_mwu->getTagDiscourse());
        i_tok_mwu->setAttribute("pos-confidence", tl_tok_mwu->getConfidencePOS());
        i_tok_mwu->setAttribute("pos-matchtype", tl_tok_mwu->getMatchTypePOS());
        foreach (TokenUnit *tl_tok_min, tl_tok_mwu->getTokenUnits()) {
            Interval *i_tok_min = new Interval(tl_tok_min->tMin(), tl_tok_min->tMax(), tl_tok_min->text());
            i_tok_min->setAttribute("pos-min", tl_tok_min->getTagPOS());
            i_tok_min->setAttribute("pos-ext-min", tl_tok_min->getTagPOSext());
            i_tok_min->setAttribute("lemma-min", tl_tok_min->getLemma());
            i_tok_min->setAttribute("disfluency", tl_tok_min->getTagDisfluency());
            i_tok_min->setAttribute("pos-confidence", tl_tok_min->getConfidencePOS());
            i_tok_min->setAttribute("pos-matchtype", tl_tok_min->getMatchTypePOS());
            list_tok_min << i_tok_min;
        }
        list_tok_mwu << i_tok_mwu;
    }
    tierMin->replaceAllIntervals(list_tok_min);
    tierMWU->replaceAllIntervals(list_tok_mwu);
}
