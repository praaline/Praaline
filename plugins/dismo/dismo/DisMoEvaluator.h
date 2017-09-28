#ifndef DISMOEVALUATOR_H
#define DISMOEVALUATOR_H

/*
    DisMo Annotator
    Copyright (c) 2012-2014 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "TokenList.h"

namespace DisMoAnnotator {

class DismoEvaluator
{
public:

    class ComparisonTableLine {
    public:
        bool ignore;
        QString name;
        RealTime xMin;
        RealTime xMax;
        QString tokmin;
        QString posmin_dismo;
        QString posmin_corr;
        QString tokmwu_dismo;
        QString tokmwu_corr;
        QString posmwu_dismo;
        QString posmwu_corr;
        QString discourse_dismo;
        QString discourse_corr;
        QString disfluency_dismo;
        QString disfluency_corr;
        QString boundary_dismo;
        QString boundary_corr;
        QString lemma_dismo;
        QString lex_dismo;
        double confidence_posmin_dismo;
        QString method_dismo;
    };

    DismoEvaluator() {}

    void compare(const QString &name, TokenList &TLdismo, TokenList &TLcorrect);
    QStringList writeToTable() const;
    void writeToTableFile(const QString &filename) const;

    double getPrecisionPOSMinCoarse();
    double getPrecisionPOSMinFine();
    double getPrecisionPOSMinPrecise();

    double getDisfluencyDetectionRecall();
    double getDisfluencyDetectionPrecision();
    double getDisfluencyClassificationPrecision();

    double getDiscourseDetectionPrecision();
    double getDiscourseDetectionRecall();
    double getDiscourseClassificationPrecision();

private:

    QList<ComparisonTableLine *> m_comparison;
};

}

#endif // DISMOEVALUATOR_H
