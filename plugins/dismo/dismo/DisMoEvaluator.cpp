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

#include <QString>
#include <QList>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include "PraalineCore/Base/RealTime.h"
#include "TokenUnit.h"
#include "Token.h"
#include "TokenList.h"
#include "DisMoEvaluator.h"
using namespace DisMoAnnotator;

void DismoEvaluator::compare(const QString &name, TokenList &TLdismo, TokenList &TLcorrect)
{
    m_comparison.clear();
    foreach (Token *tok, TLcorrect) {
        foreach (TokenUnit *tu, tok->getTokenUnits()) {
            ComparisonTableLine *line = new ComparisonTableLine();
            if (tu->isPauseSilent()) line->ignore = true;
            line->name = name;
            line->xMin = tu->tMin();
            line->xMax = tu->tMax();
            line->tokmin = tu->text();
            line->posmin_corr = tu->getTagPOS();
            if (tok->isMWU()) {
                line->tokmwu_corr = tok->text();
                line->posmwu_corr = tok->getTagPOS();
            }
            line->discourse_corr = tok->getTagDiscourse();
            line->disfluency_corr = tu->getTagDisfluency();
            line->boundary_corr = tok->getTagFrontier();
            m_comparison.append(line);
        }
    }
    int i = 0;
    foreach (Token *tok, TLdismo) {
        foreach (TokenUnit *tu, tok->getTokenUnits()) {
            ComparisonTableLine *line = m_comparison.at(i);
            line->posmin_dismo = tu->getTagPOS();
            if (tok->isMWU()) {
                line->tokmwu_dismo = tok->text();
                line->posmwu_dismo = tok->getTagPOS();
            }
            line->discourse_dismo = tok->getTagDiscourse();
            line->disfluency_dismo = tu->getTagDisfluency();
            line->boundary_dismo = tok->getTagFrontier();
            line->lemma_dismo = tu->getLemma();
            line->lex_dismo = tu->getAmbiguityClass();
            line->confidence_posmin_dismo = tu->getConfidencePOS();
            line->method_dismo = tu->getMatchTypePOS();
            i++;
        }
    }
}

QStringList DismoEvaluator::writeToTable() const
{
    QString separator = "\t";
    QString line = "";
    QStringList ret;
    foreach (ComparisonTableLine *rec, m_comparison) {
        if (rec->ignore) continue;
        line = rec->name.append(separator);
        line.append(rec->xMin.toString().c_str()).append(separator);         // xMin
        line.append(rec->xMax.toString().c_str()).append(separator);         // xMax
        line.append(rec->tokmin).append(separator);
        line.append(rec->posmin_dismo).append(separator);
        line.append(rec->posmin_corr).append(separator);
        line.append(rec->tokmwu_dismo).append(separator);
        line.append(rec->tokmwu_corr).append(separator);
        line.append(rec->posmwu_dismo).append(separator);
        line.append(rec->posmwu_corr).append(separator);
        line.append(rec->discourse_dismo).append(separator);
        line.append(rec->discourse_corr).append(separator);
        line.append(rec->disfluency_dismo).append(separator);
        line.append(rec->disfluency_corr).append(separator);
        line.append(rec->boundary_dismo).append(separator);
        line.append(rec->boundary_corr).append(separator);
        line.append(rec->lemma_dismo).append(separator);
        line.append(rec->lex_dismo).append(separator);
        line.append(QString::number(rec->confidence_posmin_dismo)).append(separator);
        line.append(rec->method_dismo).append(separator);
        ret << line;
    }
    return ret;
}

void DismoEvaluator::writeToTableFile(const QString &filename) const
{
    // Write to table
    QFile fileOut(filename);
    if (! fileOut.open(QFile::WriteOnly | QFile::Text)) return;
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    QFileInfo fileInfoOut(fileOut);
    foreach(QString line, writeToTable()) {
        out << fileInfoOut.baseName() << "\t" << line << "\n";
    }
}

double DismoEvaluator::getPrecisionPOSMinCoarse()
{
    int incorrect = 0; int total = 0;
    foreach (ComparisonTableLine *rec, m_comparison) {
        if (rec->ignore) continue;
        total++;
        if (rec->posmin_corr.left(3) != rec->posmin_dismo.left(3))
            incorrect++;
    }
    return ((double)(total - incorrect)) / ((double)total);
}

double DismoEvaluator::getPrecisionPOSMinFine()
{
    int incorrect = 0; int total = 0;
    foreach (ComparisonTableLine *rec, m_comparison) {
        if (rec->ignore) continue;
        total++;
        QStringList split_corr = rec->posmin_corr.split(":");
        QStringList split_dismo = rec->posmin_dismo.split(":");
        split_corr << ""; split_dismo << "";
        if ((split_corr[0] != split_dismo[0]) || (split_corr[1] != split_dismo[1]))
            incorrect++;
    }
    return ((double)(total - incorrect)) / ((double)total);
}

double DismoEvaluator::getPrecisionPOSMinPrecise()
{
    int incorrect = 0; int total = 0;
    foreach (ComparisonTableLine *rec, m_comparison) {
        if (rec->ignore) continue;
        total++;
        if (rec->posmin_corr != rec->posmin_dismo)
            incorrect++;
    }
    return ((double)(total - incorrect)) / ((double)total);
}

double DismoEvaluator::getDisfluencyDetectionRecall()
{
    int tp = 0; int fp = 0; int tn = 0; int fn = 0;
    foreach (ComparisonTableLine *rec, m_comparison) {
        if (rec->ignore) continue;
        bool dismoIsDisfluency = !(rec->disfluency_dismo.isEmpty());
        bool corrIsDisfluency = !(rec->disfluency_corr.isEmpty());
        if (dismoIsDisfluency && corrIsDisfluency) tp++;
        else if (!dismoIsDisfluency && !corrIsDisfluency) tn++;
        else if (dismoIsDisfluency && !corrIsDisfluency) fp++;
        else if (!dismoIsDisfluency && corrIsDisfluency) fn++;
    }
    return (double)tp / ((double)tp + (double)fn);
}

double DismoEvaluator::getDisfluencyDetectionPrecision()
{
    int tp = 0; int fp = 0; int tn = 0; int fn = 0;
    foreach (ComparisonTableLine *rec, m_comparison) {
        if (rec->ignore) continue;
        bool dismoIsDisfluency = !(rec->disfluency_dismo.isEmpty());
        bool corrIsDisfluency = !(rec->disfluency_corr.isEmpty());
        if (dismoIsDisfluency && corrIsDisfluency) tp++;
        else if (!dismoIsDisfluency && !corrIsDisfluency) tn++;
        else if (dismoIsDisfluency && !corrIsDisfluency) fp++;
        else if (!dismoIsDisfluency && corrIsDisfluency) fn++;
    }
    return (double)tp / ((double)tp + (double)fp);
}

double DismoEvaluator::getDisfluencyClassificationPrecision()
{
    int incorrect = 0; int total = 0;

    foreach (ComparisonTableLine *rec, m_comparison) {
        if (rec->ignore) continue;
        if (!rec->disfluency_dismo.isEmpty()) {
            total++;
            if (rec->disfluency_dismo != rec->disfluency_corr)
                incorrect++;
        }
    }
    return ((double)(total - incorrect)) / ((double)total);
}

double DismoEvaluator::getDiscourseDetectionPrecision()
{
    int tp = 0; int fp = 0; int tn = 0; int fn = 0;
    foreach (ComparisonTableLine *rec, m_comparison) {
        if (rec->ignore) continue;
        bool dismoIsDiscourse = !(rec->discourse_dismo.isEmpty());
        bool corrIsDiscourse = !(rec->discourse_corr.isEmpty());
        if (dismoIsDiscourse && corrIsDiscourse) tp++;
        else if (!dismoIsDiscourse && !corrIsDiscourse) tn++;
        else if (dismoIsDiscourse && !corrIsDiscourse) fp++;
        else if (!dismoIsDiscourse && corrIsDiscourse) fn++;
    }
    return (double)tp / ((double)tp + (double)fp);
}

double DismoEvaluator::getDiscourseDetectionRecall()
{
    int tp = 0; int fp = 0; int tn = 0; int fn = 0;
    foreach (ComparisonTableLine *rec, m_comparison) {
        if (rec->ignore) continue;
        bool dismoIsDiscourse = !(rec->discourse_dismo.isEmpty());
        bool corrIsDiscourse = !(rec->discourse_corr.isEmpty());
        if (dismoIsDiscourse && corrIsDiscourse) tp++;
        else if (!dismoIsDiscourse && !corrIsDiscourse) tn++;
        else if (dismoIsDiscourse && !corrIsDiscourse) fp++;
        else if (!dismoIsDiscourse && corrIsDiscourse) fn++;
    }
    return (double)tp / ((double)tp + (double)fn);
}

double DismoEvaluator::getDiscourseClassificationPrecision()
{
    int incorrect = 0; int total = 0;
    foreach (ComparisonTableLine *rec, m_comparison) {
        if (rec->ignore) continue;
        if (!rec->discourse_dismo.isEmpty()) {
            total++;
            if (rec->discourse_dismo != rec->discourse_corr)
                incorrect++;
        }
    }
    return ((double)(total - incorrect)) / ((double)total);
}

