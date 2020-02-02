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

#include <QDebug>
#include <QString>
#include <QStringBuilder>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "TokenList.h"
using namespace DisMoAnnotator;

TokenList::TokenList()
{
    m_speakerID = "SPK";
}

TokenList::TokenList(const QString &speakerID)
{
    m_speakerID = speakerID;
}

QString TokenList::tokenTextToString(const QString separator) const
{
    QString ret;
    foreach (Token *token, *this) {
        ret.append(token->text() + separator);
    }
    return ret.trimmed();
}

QString TokenList::tokenTextToString(const QString separatorBefore, const QString separatorAfter) const
{
    QString ret;
    foreach (Token *token, *this) {
        ret.append(separatorBefore + token->text() + separatorAfter);
    }
    return ret.trimmed();
}

// =============================================================================================================
// Serialization to tables
// =============================================================================================================

void TokenList::readFromTable(const QStringList &input)
{
    QString line, group, fileid, s_tMin, s_tMax, s_tok_min, s_pos_min, s_tok_loc, s_pos_loc, s_disc, s_disf, s_boundary;
    QString prev_tok_loc, prev_pos_loc, prev_disc, prev_boundary;
    RealTime r_tMin, r_tMax;
    QList<TokenUnit *> units;

    prev_tok_loc = "";

    foreach(line, input) {
        group = line.section('\t', 0, 0);
        fileid = line.section('\t', 1, 1);
        s_tMin = line.section('\t', 2, 2).replace(",", ".");
        s_tMax = line.section('\t', 3, 3).replace(",", ".");
        r_tMin = RealTime::fromSeconds(s_tMin.toDouble());
        r_tMax = RealTime::fromSeconds(s_tMax.toDouble());
        s_tok_min = line.section('\t', 4, 4);
        s_pos_min = line.section('\t', 5, 5);
        s_tok_loc = line.section('\t', 6, 6);
        s_pos_loc = line.section('\t', 7, 7);
        s_disc = line.section('\t', 8, 8);
        s_disf = line.section('\t', 9, 9);
        s_boundary = line.section('\t', 10, 10);

        TokenUnit *u = new TokenUnit(r_tMin, r_tMax, s_tok_min);
        u->setTagPOS(s_pos_min, "", 1.0);
        u->setTagDisfluency(s_disf, 1.0);

        if (prev_tok_loc.isEmpty() && s_tok_loc.isEmpty()) {
            // add one
            Token *t = new Token(u);
            t->setTagDiscourse(s_disc, 1.0);
            t->setTagFrontier(s_boundary, 1.0);
            append(t);
            units.clear();
        } else {
            if (prev_tok_loc == s_tok_loc) {
                units << u;
            }
            else {
                if (!prev_tok_loc.isEmpty()) {
                    // add many - previous
                    Token *t = new Token(units, prev_tok_loc);
                    t->setTagPOS(prev_pos_loc, "", 1.0);
                    t->setTagDiscourse(prev_disc, 1.0);
                    t->setTagFrontier(prev_boundary, 1.0);
                    append(t);
                    units.clear();
                }
                if (!s_tok_loc.isEmpty()) {
                    units << u;
                } else {
                    // add one - current
                    Token *t = new Token(u);
                    t->setTagDiscourse(s_disc, 1.0);
                    t->setTagFrontier(s_boundary, 1.0);
                    append(t);
                    units.clear();
                }
            }
        }
        prev_tok_loc = s_tok_loc;
        prev_pos_loc = s_pos_loc;
        prev_disc = s_disc;
        prev_boundary = s_boundary;
    }
}

void TokenList::readFromTableFile(const QString &filename)
{
    QStringList table;
    QFile file(filename);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text ))
        return;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    do {
        table << stream.readLine();
    } while (! stream.atEnd());
    file.close();
    this->readFromTable(table);
}

QStringList TokenList::writeToTable() const
{
    QString separator = "\t";
    QString line = "";
    QStringList ret;
    foreach(Token *tok, *this) {
        foreach(TokenUnit *tu, tok->getTokenUnits()) {
            line = QString(tu->tMin().toString().c_str()).append(separator);    // tMin
            line.append(tu->tMax().toString().c_str()).append(separator);       // tMax
            line.append(tu->getAmbiguityClass()).append(separator);             // ambiguity class
            line.append(tu->text()).append(separator);                          // tok-min
            line.append(tu->getTagPOS()).append(separator);                     // pos-min
            if (tok->isMWU()) {
                line.append(tok->text()).append(separator);                     // tok-mwu
                line.append(tok->getTagPOS()).append(separator);                // pos-mwu
            }
            else {
                line.append(separator).append(separator);
            }
            line.append(tok->getTagDiscourse()).append(separator);              // discourse
            line.append(tu->getTagDisfluency()).append(separator);              // disfluence
            line.append(tok->getTagFrontier()).append(separator);               // boundary
            // extra information
            line.append(tu->getTagPOSext()).append(separator);                  // pos-min-ext
            line.append(tu->getLemma()).append(separator);                      // lemma
            line.append(QString::number(
                tu->getConfidencePOS())).append(separator);                     // pos-min confidence
            line.append(tu->getMatchTypePOS());                                 // pos-min match type
            ret << line;
        }
    }
    return ret;
}

void TokenList::writeToTableFile(const QString &filename) const
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

// =============================================================================================================
// Serialization to textgrids
// =============================================================================================================

void TokenList::readFromTextGrid(const AnnotationTierGroup &tiers)
{
    IntervalTier *tierTokMin = tiers.getIntervalTierByName("dismo-tok-min");
    if (tierTokMin == 0) {
        tierTokMin = new IntervalTier("dismo-tok-min", tiers.tMin(), tiers.tMax());
    }
    IntervalTier *tierPOSMin = tiers.getIntervalTierByName("dismo-pos-min");
    if (tierPOSMin == 0) {
        tierPOSMin = new IntervalTier("dismo-pos-min", tiers.tMin(), tiers.tMax());
    }
    IntervalTier *tierTokMWU = tiers.getIntervalTierByName("dismo-tok-mwu");
    if (tierTokMWU == 0) {
        tierTokMWU = new IntervalTier("dismo-tok-mwu", tiers.tMin(), tiers.tMax());
    }
    IntervalTier *tierPOSMWU = tiers.getIntervalTierByName("dismo-pos-mwu");
    if (tierPOSMWU == 0) {
        tierPOSMWU = new IntervalTier("dismo-pos-mwu", tiers.tMin(), tiers.tMax());
    }
    IntervalTier *tierDiscourse = tiers.getIntervalTierByName("dismo-discourse");
    if (tierDiscourse == 0) {
        tierDiscourse = new IntervalTier("dismo-discourse", tiers.tMin(), tiers.tMax());
    }
    IntervalTier *tierDisfluency = tiers.getIntervalTierByName("dismo-disfluency");
    if (tierDisfluency == 0) {
        tierDisfluency = new IntervalTier("dismo-disfluency", tiers.tMin(), tiers.tMax());
    }
    IntervalTier *tierFrontiers = tiers.getIntervalTierByName("dismo-boundary");
    if (tierFrontiers == 0) {
        tierFrontiers = new IntervalTier("dismo-boundary", tiers.tMin(), tiers.tMax());
    }
    readFromTiers(tierTokMin, tierPOSMin, tierTokMWU, tierPOSMWU, tierDiscourse, tierDisfluency, tierFrontiers);
}


void TokenList::readFromTiers(const IntervalTier *tierTokMin, const IntervalTier *tierPOSMin,
                              const IntervalTier *tierTokMWU, const IntervalTier *tierPOSMWU,
                              const IntervalTier *tierDiscourse, const IntervalTier *tierDisfluency,
                              const IntervalTier *tierBoundary)
{
    QList<TokenUnit *> units;
    QList<Interval *> found;
    QString textMWU = "";
    foreach(Interval *intvToken, tierTokMWU->intervals()) {
        foreach(Interval *intvTokMin, tierTokMin->getIntervalsContainedIn(intvToken)) {
            TokenUnit *tu = new TokenUnit(intvTokMin);
            found = tierPOSMin->getIntervalsContainedIn(intvTokMin);
            if (!found.isEmpty())
                tu->setTagPOS(found[0]->text(), "", 1.0);
            found = tierDisfluency->getIntervalsContainedIn(intvTokMin);
            if (!found.isEmpty())
                tu->setTagDisfluency(found[0]->text(), 1.0);
            units << tu;
        }
        textMWU = "";
        found = tierTokMWU->getIntervalsContainedIn(intvToken);
        if (!found.isEmpty())
            textMWU = found[0]->text();
        Token *token;
        if (textMWU == "" && !units.isEmpty()) {
            token = new Token(units[0]);                        // not a MWU
        } else {
            token = new Token(units, textMWU);                  // tok-mwu
            found = tierPOSMWU->getIntervalsContainedIn(intvToken);
            if (!found.isEmpty())
                token->setTagPOS(found[0]->text(), "", 1.0);    // pos-mwu
        }
        found = tierDiscourse->getIntervalsContainedIn(intvToken);
        if (!found.isEmpty())
            token->setTagDiscourse(found[0]->text(), 1.0);
        found = tierBoundary->getIntervalsContainedIn(intvToken);
        if (!found.isEmpty())
            token->setTagFrontier(found[0]->text(), 1.0);
        append(token);
        units.clear();
    }
}

// Default tierNamePrefix = "dismo-"
void TokenList::writeToTextGrid(AnnotationTierGroup &tiers, const QString &tierNamePrefix,
                                bool outputTokMin, bool outputPOSMin, bool outputTokMWU, bool outputPOSMWU,
                                bool outputDiscourse, bool outputDisfluency, bool outputBoundary,
                                int tierIndex) const
{
    QString tierNameTokMin = tierNamePrefix + "tok-min";
    QString tierNamePOSMin = tierNamePrefix + "pos-min";
    QString tierNameTokMWU = tierNamePrefix + "tok-mwu";
    QString tierNamePOSMWU = tierNamePrefix + "pos-mwu";
    QString tierNameDiscourse = tierNamePrefix + "discourse";
    QString tierNameDisfluency = tierNamePrefix + "disfluency";
    QString tierNameBoundary = tierNamePrefix + "boundary";

    IntervalTier *tierTokMin = 0; IntervalTier *tierPOSMin = 0;
    IntervalTier *tierTokMWU = 0; IntervalTier *tierPOSMWU = 0;
    IntervalTier *tierDiscourse = 0; IntervalTier *tierDisfluency = 0;
    IntervalTier *tierBoundary = 0;

    // If not told where to insert DisMo's tiers, add them to the bottom of the textgrid.
    if (tierIndex < 0) tierIndex = tiers.tiersCount();

    if (outputTokMin) {
        tierTokMin = tiers.getIntervalTierByName(tierNameTokMin);
        if (tierTokMin == 0) {
            tierTokMin = new IntervalTier(tierNameTokMin, tiers.tMin(), tiers.tMax());
            tiers.insertTier(tierIndex, tierTokMin);
            tierIndex++;
        }
    }
    if (outputPOSMin) {
        tierPOSMin = tiers.getIntervalTierByName(tierNamePOSMin);
        if (tierPOSMin == 0) {
            tierPOSMin = new IntervalTier(tierNamePOSMin, tiers.tMin(), tiers.tMax());
            tiers.insertTier(tierIndex, tierPOSMin);
            tierIndex++;
        }
    }
    if (outputDisfluency) {
        tierDisfluency = tiers.getIntervalTierByName(tierNameDisfluency);
        if (tierDisfluency == 0) {
            tierDisfluency = new IntervalTier(tierNameDisfluency, tiers.tMin(), tiers.tMax());
            tiers.insertTier(tierIndex, tierDisfluency);
            tierIndex++;
        }
    }
    if (outputTokMWU) {
        tierTokMWU = tiers.getIntervalTierByName(tierNameTokMWU);
        if (tierTokMWU == 0) {
            tierTokMWU = new IntervalTier(tierNameTokMWU, tiers.tMin(), tiers.tMax());
            tiers.insertTier(tierIndex, tierTokMWU);
            tierIndex++;
        }
    }
    if (outputPOSMWU) {
        tierPOSMWU = tiers.getIntervalTierByName(tierNamePOSMWU);
        if (tierPOSMWU == 0) {
            tierPOSMWU = new IntervalTier(tierNamePOSMWU, tiers.tMin(), tiers.tMax());
            tiers.insertTier(tierIndex, tierPOSMWU);
            tierIndex++;
        }
    }
    if (outputDiscourse) {
        tierDiscourse = tiers.getIntervalTierByName(tierNameDiscourse);
        if (tierDiscourse == 0) {
            tierDiscourse = new IntervalTier(tierNameDiscourse, tiers.tMin(), tiers.tMax());
            tiers.insertTier(tierIndex, tierDiscourse);
            tierIndex++;
        }
    }
    if (outputBoundary) {
        tierBoundary = tiers.getIntervalTierByName(tierNameBoundary);
        if (tierBoundary == 0) {
            tierBoundary = new IntervalTier(tierNameBoundary, tiers.tMin(), tiers.tMax());
            tiers.insertTier(tierIndex, tierBoundary);
            tierIndex++;
        }
    }
    writeToTiers(tierTokMin, tierPOSMin, tierTokMWU, tierPOSMWU, tierDiscourse, tierDisfluency, tierBoundary);
}

void TokenList::writeToTiers(IntervalTier *tierTokMin, IntervalTier *tierPOSMin,
                             IntervalTier *tierTokMWU, IntervalTier *tierPOSMWU,
                             IntervalTier *tierDiscourse, IntervalTier *tierDisfluency,
                             IntervalTier *tierBoundary) const
{
    if (tierTokMin) tierTokMin->clear();
    if (tierPOSMin) tierPOSMin->clear();
    if (tierTokMWU) tierTokMWU->clear();
    if (tierPOSMWU) tierPOSMWU->clear();
    if (tierDiscourse) tierDiscourse->clear();
    if (tierDisfluency) tierDisfluency->clear();
    if (tierBoundary) tierBoundary->clear();
    QList<Interval *> intervals_tokmin; QList<Interval *> intervals_posmin;
    QList<Interval *> intervals_tokmwu; QList<Interval *> intervals_posmwu;
    QList<Interval *> intervals_disfluency; QList<Interval *> intervals_discourse;
    QList<Interval *> intervals_boundary;
    foreach(Token *tok, *this) {
        foreach (TokenUnit *tu, tok->getTokenUnits()) {
            Interval *i_tok_min = tu->toIntervalToken();
            i_tok_min->setAttribute("pos_min", tu->getTagPOS());
            i_tok_min->setAttribute("pos_ext_min", tu->getTagPOSext());
            i_tok_min->setAttribute("lemma_min", tu->getLemma());
            i_tok_min->setAttribute("disfluency", tu->getTagDisfluency());
            i_tok_min->setAttribute("dismo_confidence", tu->getConfidencePOS());
            i_tok_min->setAttribute("dismo_method", tu->getMatchTypePOS());
            if (tok->isMWU())
                i_tok_min->setAttribute("part_of_mwu", true);
            else
                i_tok_min->setAttribute("part_of_mwu", false);
            intervals_tokmin.append(i_tok_min);
            intervals_posmin.append(tu->toIntervalPOS());
            intervals_disfluency.append(tu->toIntervalDisfluency());
        }
        Interval *i_tok_mwu = tok->toIntervalToken();
        i_tok_mwu->setAttribute("pos_mwu", tok->getTagPOS());
        i_tok_mwu->setAttribute("pos_ext_mwu", tok->getTagPOSext());
        i_tok_mwu->setAttribute("lemma_mwu", tok->getLemma());
        i_tok_mwu->setAttribute("discourse", tok->getTagDiscourse());
        i_tok_mwu->setAttribute("dismo_confidence", tok->getConfidencePOS());
        i_tok_mwu->setAttribute("dismo_method", tok->getMatchTypePOS());
        intervals_tokmwu.append(i_tok_mwu);
        intervals_posmwu.append(tok->toIntervalPOS());
        intervals_discourse.append(tok->toIntervalDiscourse());
        intervals_boundary.append(tok->toIntervalFrontier());
    }
    if (tierTokMin) tierTokMin->replaceAllIntervals(intervals_tokmin); else qDeleteAll(intervals_tokmin);
    if (tierPOSMin) tierPOSMin->replaceAllIntervals(intervals_posmin); else qDeleteAll(intervals_posmin);
    if (tierTokMWU) tierTokMWU->replaceAllIntervals(intervals_tokmwu); else qDeleteAll(intervals_tokmwu);
    if (tierPOSMWU) tierPOSMWU->replaceAllIntervals(intervals_posmwu); else qDeleteAll(intervals_posmwu);
    if (tierDiscourse) tierDiscourse->replaceAllIntervals(intervals_discourse); else qDeleteAll(intervals_discourse);
    if (tierDisfluency) tierDisfluency->replaceAllIntervals(intervals_disfluency); else qDeleteAll(intervals_disfluency);
    if (tierBoundary) tierBoundary->replaceAllIntervals(intervals_boundary); else qDeleteAll(intervals_boundary);
}

void TokenList::updateTokenizedTiers(IntervalTier *tierTokMin, IntervalTier *tierTokMWU,
                                     const QHash<QString, QString> &attributes, bool keepOnlyMin)
{
    if (!tierTokMWU || !tierTokMin) return;
    if (!keepOnlyMin) {
        if ((tierTokMWU->count() != this->count())) {
            qDebug() << "Error - mismatch number of tokens in tok-mwu and token list";
            return;
        }
    }
    int i = 0; int j = 0;
    for (i = 0; i < this->count(); ++i) {
        Token *tok = this->at(i);
        foreach (TokenUnit *tu, tok->getTokenUnits()) j++;
    }
    if (tierTokMin->count() != j) {
        qDebug() << "Error - mismatch number of tokens in tok-min and token list";
        int iterTU = 0;
        for (int iterTok = 0; iterTok < this->count(); ++iterTok) {
            Token *tok = this->at(iterTok);
            foreach (TokenUnit *tu, tok->getTokenUnits()) {
                qDebug() << iterTok << "\t" << iterTU << "\t" << tu->text() << "\t" << tu->getTagPOS();
                iterTU++;
            }
        }
        return;
    }
    j = 0;
    QList<Interval *> list_intervals_tok_mwu;
    for (i = 0; i < this->count(); ++i) {
        Token *tok = this->at(i);
        Interval *i_tok_mwu = 0;
        if (keepOnlyMin) {
            i_tok_mwu = tok->toIntervalToken();
        } else {
            i_tok_mwu = tierTokMWU->interval(i);
        }
        i_tok_mwu->setAttribute(attributes.value("pos_mwu", "pos_mwu"), tok->getTagPOS());
        i_tok_mwu->setAttribute(attributes.value("pos_ext_mwu", "pos_ext_mwu"), tok->getTagPOSext());
        i_tok_mwu->setAttribute(attributes.value("lemma_mwu", "lemma_mwu"), tok->getLemma());
        i_tok_mwu->setAttribute(attributes.value("discourse", "discourse"), tok->getTagDiscourse());
        i_tok_mwu->setAttribute(attributes.value("dismo_confidence", "dismo_confidence"), tok->getConfidencePOS());
        i_tok_mwu->setAttribute(attributes.value("dismo_method", "dismo_method"), tok->getMatchTypePOS());
        list_intervals_tok_mwu << i_tok_mwu;
        foreach (TokenUnit *tu, tok->getTokenUnits()) {
            Interval *i_tok_min = tierTokMin->interval(j);
            i_tok_min->setAttribute(attributes.value("pos_min", "pos_min"), tu->getTagPOS());
            i_tok_min->setAttribute(attributes.value("pos_ext_min", "pos_ext_min"), tu->getTagPOSext());
            i_tok_min->setAttribute(attributes.value("lemma_min", "lemma_min"), tu->getLemma());
            i_tok_min->setAttribute(attributes.value("disfluency", "disfluency"), tu->getTagDisfluency());
            i_tok_min->setAttribute(attributes.value("dismo_confidence", "dismo_confidence"), tu->getConfidencePOS());
            i_tok_min->setAttribute(attributes.value("dismo_method", "dismo_method"), tu->getMatchTypePOS());
            if (tok->isMWU())
                i_tok_min->setAttribute(attributes.value("part_of_mwu", "part_of_mwu"), true);
            else
                i_tok_min->setAttribute(attributes.value("part_of_mwu", "part_of_mwu"), false);
            j++;
        }
    }
    if (keepOnlyMin) {
        tierTokMWU->replaceAllIntervals(list_intervals_tok_mwu);
        tierTokMWU->mergeIdenticalAnnotations("_");
    }
}

// Merge tokens

void TokenList::mergeTokens(int from, int to)
{
    if ((from < 0) || (to > count()) || (from >= to)) return;
    QList<TokenUnit *> all;
    QString joined;
    for (int i = from; i <= to; i++) {
        foreach(TokenUnit *unit, at(i)->getTokenUnits()) {
            all << new TokenUnit(unit);
            joined.append(unit->text());
            joined.append(" ");
        }
    }
    Token *newToken = new Token(all, joined.trimmed());
    newToken->setTagPOS(at(from)->getTagPOS(), at(from)->getTagPOSext(), 1.0);
    newToken->setTagDiscourse(at(from)->getTagDiscourse(), at(from)->getConfidenceDiscourseTag());
    newToken->setTagFrontier(at(from)->getTagFrontier(), at(from)->getConfidenceFrontierTag());
    for (int i = from; i <= to; i++) {
        removeAt(from);
    }
    insert(from, newToken);
}
