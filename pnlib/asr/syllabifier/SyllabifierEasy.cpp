#include <QDebug>
#include <QString>
#include <QRegularExpression>

#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "SyllabifierEasy.h"

namespace Praaline {
namespace ASR {

SyllabifierEasy::SyllabifierEasy()
{
}

IntervalTier *SyllabifierEasy::SyllabifierEasy::createSyllableTier(IntervalTier *tier_phone)
{
    IntervalTier *tier_syll = new IntervalTier("syll", tier_phone->tMin(), tier_phone->tMax());
    bool result = syllabify(tier_phone, tier_syll, tier_phone->tMin(), tier_phone->tMax());
    if (!result) {
        delete tier_syll;
        return nullptr;
    }
    return tier_syll;
}

bool SyllabifierEasy::syllabify(IntervalTier *tier_phone, IntervalTier *tier_syll, RealTime from, RealTime to)
{
    if (!tier_phone) return false;
    if (!tier_syll) return false;
    if (from > to) return false;
    QList<Interval *> phones;
    foreach (Interval *intv, tier_phone->getIntervalsContainedIn(from, to))
        phones << intv->clone();
    if (phones.isEmpty()) return false;
    if (!tier_syll->patchIntervals(phones, from, to)) {
        qDeleteAll(phones);
        return false;
    }
    QPair<int, int> indices = tier_syll->getIntervalIndexesContainedIn(from, to);
    if ((indices.first < 0) || (indices.second < 0)) return false;

    int current = indices.second;
    int nncat(0), ncat(0);
    int ccat = getCategory(tier_syll, current, 0);
    int pcat = getCategory(tier_syll, current, -1);
    int ppcat = getCategory(tier_syll, current, -2);
    // For debug purposes
    // QString labelpp = (current - 2 >= 0) ? tier_syll->at(current - 2)->text() : "";
    // QString labelp  = (current - 1 >= 0) ? tier_syll->at(current - 1)->text() : "";
    // QString label   = (current >= 0) ? tier_syll->at(current)->text() : "";
    // QString labeln, labelnn;

    while (current > indices.first) {
        // For debug purposes
        // RealTime t = tier_syll->at(current)->tMin();
        // qDebug() << t.sec << labelpp << labelp << "." << label << labeln << labelnn
        //          << ppcat   << pcat   << "." << ccat  << ncat   << nncat   << "==>";
        if (ccat == 6) {
            if ((pcat == 2) || (pcat == 3) || (pcat == 4) || (pcat == 5))
                delcurrent(tier_syll, current, "6A");
        }
        else if (ccat == 5) {
            if (pcat == 2) {
                if (ncat == 6)
                    delcurrent(tier_syll, current, "5A");
            }
            else if (pcat == 3) {
                if      (ncat == 3)                                     // mO~tan-j-nO~
                    delcurrent(tier_syll, current, "5B");
                else if (ncat == 6)                                     // profes-j-onel
                    delcurrent(tier_syll, current, "5C");
                else if (ncat == 4)                                     // i n.j l  @ 6 3 5 4 6 (désigne)
                    delcurrent(tier_syll, current, "5D");
                else if (ncat == 1)                                     // i n.j _  l 6 3 5 1 4 (désigne)
                    delcurrent(tier_syll, current, "5E");
            }
            else if (pcat == 4) {
                if (ncat == 6)                                          // mOr-j-En
                    delcurrent(tier_syll, current, "5F");
            }
            else if (pcat == 6) {
                if ((ncat == 5) || (ncat == 4) || (ncat == 3) || (ncat == 2) || (ncat == 1))
                    delcurrent(tier_syll, current, "5G");
            }
        }
        else if (ccat == 4) {
            if (pcat == 2) {
                if ((ncat == 1) || (ncat == 5) || (ncat == 6))          // egza~p-l-sil at-R-wa
                    delcurrent(tier_syll, current, "4A");
                else if ((ncat == 2) && (nncat == 6) && (ppcat == 6))   // va~d-Rdabor (ameline)
                    delcurrent(tier_syll, current, "4B");
                else if ((ncat == 2) && (nncat == 6) && (ppcat == 3))   // (mini)st.Rp(uR in ministre pour)
                    delcurrent(tier_syll, current, "4C");
                else if ((ncat == 2) && (nncat == 4) && (ppcat == 6))   // o t.R p  R 6 2 4 2 4 (autres) ==>
                    delcurrent(tier_syll, current, "4D");
                else if ((ncat == 3) && (ppcat == 6))                   // i v R s  @ 6 2 4 3 (livre) ==>
                    delcurrent(tier_syll, current, "4E");
                else if ((ncat == 3) && (ppcat == 3))                   // (mini)st.Rn(@sEs) ministre ne cesse
                    delcurrent(tier_syll, current, "4F");
                else if (ncat == 4)                                     // a b.l l  E 6 2 4 4 6 (véritable) ==>
                    delcurrent(tier_syll, current, "4G");
            }
            else if (pcat == 3) {
                // if (ncat == 3)          // mO~tan-j-nO~
                //    delcurrent(tier_syll, current);
                // else if (ncat == 6)     // profes-j-onel
                //    delcurrent(tier_syll, current);
            }
            else if ((pcat == 4) || (pcat == 5)) {
                if ((ncat == 2) || (ncat == 1))
                    delcurrent(tier_syll, current, "4H");
            }
            else if (pcat == 6) {
                if ((ncat == 4) || (ncat == 3) || (ncat == 2) || (ncat == 1))   // 1=e-R-sil was ncat = 5 or mor-j-en
                    delcurrent(tier_syll, current, "4I");
            }
        }
        else if (ccat == 3) {
            if (pcat == 6) {
                if ((ncat == 4) || (ncat == 3) || (ncat == 2) || (ncat == 1))
                    delcurrent(tier_syll, current, "3A");
                else if ((ncat == 5) && ((nncat == 3) || (nncat == 4) || (nncat == 1)))     // mo~ta-n-jno~py (ameline)
                    // z i.n j  l 3 6 3 5 4 (désigne)
                    // z i.n j  _ 3 6 3 5 1 (désigne)
                    delcurrent(tier_syll, current, "3B");
            }
            else if (pcat == 4) {
                if ((ncat == 1) || (ncat == 2) || (ncat == 3))          // al-m-sil  aR-m-dujE wouteR-snu
                    delcurrent(tier_syll, current, "3C");
                else if ((ncat == 6) && (ppcat == 1))                   // _l.ministre
                    delcurrent(tier_syll, current, "3D");
            }
            else if (pcat == 3) {
                // if ((ncat == 2) && (nncat == 6) && (ppcat == 6))        // e~pas.stupide
                // {}
                // else
                if ((ncat == 1) || (ncat == 2))                         // a-m-s?  domEn-s-kjabl
                    delcurrent(tier_syll, current, "3E");
                else if (ppcat == 1)                                    // sil Z-m@, sil-m-s@ (lucchini 275.35)
                    delcurrent(tier_syll, current, "3F");
            }
            else if (pcat == 2) {
                if (ppcat == 1)                                         // _psk@ lucchini 31.04
                    delcurrent(tier_syll, current, "3G");
                else if (ppcat == 4)                                    // _doublure d-son  vet'ment wouters 133s
                    delcurrent(tier_syll, current, "3H");
                else if ((ppcat == 6) && (ncat == 3))                   // E k.s s  e 6 2 3 3 6 (circonflexe)
                    delcurrent(tier_syll, current, "3I");
                else if ((ppcat == 6) && (ncat == 1))                   // 0 i k.s _  6 2 3 1 0 (s) ==>  "ekonomiks"
                    delcurrent(tier_syll, current, "3J");
                else if (ppcat == 5)                                    // j d.Z E  s 5 2 3 6 3 (Digest) ==>
                    delcurrent(tier_syll, current, "3K");
            }
        }
        else if (ccat == 2) {
            if (pcat == 6) {
                if ((ncat == 1) || (ncat == 2))     // or (ncat == 3)   sporti-f-sil kara-k-tEr ak-sjo~
                    delcurrent(tier_syll, current, "2A");
                else if (ncat == 3) {
                    if ((nncat == 2) || (nncat == 6) || (nncat == 5) || (nncat == 3))   // l E.k s  s 4 6 2 3 3 (circonflexe)
                                                                                        // e-k-sperja~s toute-cette ak-sjo~
                                                                                        // 0 m i.k s _ 3 6 2 3 1 (konomiks) ==>
                        delcurrent(tier_syll, current, "2B");
                    else if (nncat == 1)
                        delcurrent(tier_syll, current, "2C");
                }
                else if (ncat == 4) {
                    if ((nncat == 1) || (nncat == 2) || (nncat == 3) || (nncat == 4))   // va~-dRdabor (ameline)
                                                                                        // egza~-p-l-sil
                                                                                        // l i.v R  s 4 6 2 4 3 (livre) ==>
                                                                                        // t a.b l  l 2 6 2 4 4 (véritable) ==>
                        delcurrent(tier_syll, current, "2D");
                }
            }
            else if (pcat == 3) {
                if ((ncat == 2) || (ncat == 1))                          // pis-t-purdenuvodebuSe  pis-k
                    delcurrent(tier_syll, current, "2E");
                else if ((ncat == 4) && (nncat == 1))                   // minis.tR_
                    delcurrent(tier_syll, current, "2F");
                else if ((ncat == 4) && ((nncat == 3) || (nncat == 2) || (nncat == 4))) // minis.tRn(@...)  minis.tRp(uR) minis.tR(la)
                    delcurrent(tier_syll, current, "2G");
                else if ((ncat == 6) && (ppcat == 3))                   // pass.tu
                    delcurrent(tier_syll, current, "2H");
                else if ((ppcat == 2) || (ppcat == 1))                  // eks-p-erja~s  _s-ki
                    delcurrent(tier_syll, current, "2I");
            }
            else if (pcat == 4) {
                if (ncat == 2)                                          // savwajar-d-d@lamorjEn
                    delcurrent(tier_syll, current, "2J");
                else if (ncat == 1)                                     // E R.b _  m 6 4 2 1 3 (verbe)
                    delcurrent(tier_syll, current, "2K");
                else if ((ncat == 4) && (nncat == 1))                   // 9 R.t R  _ 6 4 2 4 1 (meutre) ==>
                    delcurrent(tier_syll, current, "2L");
                else if (ppcat == 1)                                    // _l.vi
                    delcurrent(tier_syll, current, "2M");
            }
            else if (pcat == 2) {
                if (ppcat == 1)                                         // _p-t
                    delcurrent(tier_syll, current, "2N");
            }
        }
        else if (ccat == 1) {
            // sil
        }
        // zap: continue by moving one phone back
        current = current - 1;
        nncat = ncat;
        ncat = ccat;
        ccat = pcat;
        pcat = ppcat;
        ppcat = getCategory(tier_syll, current, -2);
        // For debug purposes
        // labelnn = labeln;
        // labeln = label;
        // label = labelp;
        // labelp = labelpp;
        // labelpp = (current - 2 >= 0) ? tier_syll->at(current - 2)->text() : "";
    } // while loop
    return tier_syll;
}

void SyllabifierEasy::delcurrent(Praaline::Core::IntervalTier *tier_syll, int index, const QString &ruleName)
{
    Q_UNUSED(ruleName)
    tier_syll->merge(index - 1, index);
    // qDebug() << "DEL" << ruleName;
}

int SyllabifierEasy::getCategory(IntervalTier *tier_syll, int index, int offset)
{
    int i = index + offset; // 0: current, -1: previous, -2: pre-previous
    if ((i < 0) || (i >= tier_syll->count())) return 0;
    QString syll = tier_syll->interval(i)->text().trimmed();
    return phoneCategory(syll);
}

int SyllabifierEasy::phoneCategory(const QString &phone)
{
    QStringList cat6; cat6 << "a" << "A" << "e" << "E" << "o" << "O" << "i" << "u" << "y" << "@" << "2" << "6" << "9";
    QStringList cat2; cat2 << "B" << "G" << "p" << "t" << "k" << "b" << "d" << "g" << "f" << "v" << "c";
    QStringList cat3; cat3 << "T" << "D" << "s" << "S" << "z" << "Z" << "m" << "n" << "N" << "J";
    QStringList cat4; cat4 << "l" << "L" << "r" << "R" << "X" << "x" << "h" << "4";
    QStringList cat5; cat5 << "j" << "w" << "H";

    if (phone.isEmpty() || phone == "sil" || phone == "_" || phone == "-" || phone == "*" || phone.indexOf(QRegularExpression("^\\[.*\\]$")) >= 0)
        return 1;
    else {
        QString p = phone.left(1);
        if      (cat6.contains(p)) return 6;
        else if (cat2.contains(p)) return 2;
        else if (cat3.contains(p)) return 3;
        else if (phone == "jj")    return 3;
        else if (cat4.contains(p)) return 4;
        else if (cat5.contains(p)) return 5;
    }
    return 0;
}

} // namespace ASR
} // namespace Praaline
