#include <QString>
#include <QRegularExpression>

#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "SyllabifierEasy.h"

SyllabifierEasy::SyllabifierEasy()
{

}

int SyllabifierEasy::getCategory(IntervalTier *tier_syll, int index, int offset)
{
    int i = index + offset; // 0: current, -1: previous, -2: pre-previous
    if ((i < 0) || (i >= tier_syll->count())) return 0;
    QString syll = tier_syll->interval(i)->text();
    return phoneCategory(syll);
}

int SyllabifierEasy::phoneCategory(const QString &phone)
{
    QStringList cat6; cat6 << "a" << "A" << "e" << "E" << "o" << "O" << "i" << "u" << "y" << "@" << "2" << "6" << "9";
    QStringList cat2; cat2 << "B" << "G" << "p" << "t" << "k" << "b" << "d" << "g" << "f" << "v" << "c";
    QStringList cat3; cat3 << "T" << "D" << "s" << "S" << "z" << "Z" << "m" << "n" << "N" << "J";
    QStringList cat4; cat4 << "l" << "L" << "r" << "R" << "X" << "x" << "h" << "4";
    QStringList cat5; cat5 << "j" << "w" << "H";

    if (phone.isEmpty() || phone == "sil" || phone == "_" || phone == "-" || phone == "*" || phone.indexOf(QRegularExpression("^\[.*\]$")) >= 0)
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

