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
#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>
#include "TokenUnit.h"
#include "Token.h"
#include "TokenList.h"
#include "POSTagger.h"
using namespace DisMoAnnotator;

bool POSTagger::isEOSToken(int i)
{
    if (m_tokens[i]->getTagFrontier() == "MAJ") return true;
    return false;
}

bool POSTagger::isEOSTokenUnit(int i, int j)
{
    return false;
}

bool POSTagger::skipToken(int i)
{
    Token *token = m_tokens[i];
    if (token->isPause() || (!token->getTagDisfluency().isEmpty() && token->getTagDisfluency() != "0"))
        return true;
    return false;
}

bool POSTagger::skipTokenUnit(int i, int j)
{
    TokenUnit *tokunit = m_tokens[i]->getTokenUnits()[j];
    if (tokunit->isPause() || (!tokunit->getTagDisfluency().isEmpty() && tokunit->getTagDisfluency() != "0") || (tokunit->getTagPOS() == "PNC"))
        return true;
    return false;
}


void POSTagger::writeTokenToCRF(QTextStream &out, int i, bool isTraining)
{
    Token *token = m_tokens[i];
    QString corr = sanitizeString(token->text());
    if (corr.length() > 1 && corr.endsWith("-")) corr.chop(1);
    if (corr.contains("=")) corr = corr.replace("=", ""); // Lenghtening convention
    if (corr.isEmpty()) corr = "_";
    out << corr << "\t";
    RealTime dur = token->duration();
    int durRounded = ((dur.msec() / 20) * 20);
    out << durRounded << "\t";
    if (token->getAmbiguityClass().length() == 0)
        out << "0" << "\t";
    else
        out << token->getAmbiguityClass() << "\t";
    if (isTraining) {
        if (!token->getTagPOS().isEmpty())
            out << "\t" << sanitizeString(token->getTagPOS());    // give the correct answer (training file)
        else if (!token->getTagDiscourse().isEmpty())
            out << "\t" << sanitizeString(token->getTagDiscourse());
        else if (!token->getTagDisfluency().isEmpty())
            out << "\t" << sanitizeString(token->getTagDisfluency());
        else
            out << "\tUNK";
    }
    out << "\n";
}

void POSTagger::writeTokenUnitToCRF(QTextStream &out, int i, int j, bool isTraining)
{
    // feature selection
    bool duration = false; bool ambiguity3 = true; bool ambiguity = false; bool mwu = true;
    bool outputCoarsePOSTag = true;
    // bool duration = false; bool ambiguity3 = false; bool ambiguity = true; bool mwu = false;
    // token unit
    Token *token = m_tokens[i];
    TokenUnit *tokunit = m_tokens[i]->getTokenUnits()[j];
    QString corr = sanitizeString(tokunit->text());
    if (corr.length() > 1 && corr.endsWith("-")) corr.chop(1);
    if (corr.contains("=")) corr = corr.replace("=", ""); // Lenghtening convention
    if (corr.isEmpty()) corr = "_";
    out << corr << "\t";
    // Duration
    if (duration) {
        RealTime dur = tokunit->duration();
        int durRounded = ((dur.msec() / 20) * 20);
        out << durRounded << "\t";
    }
    // Ambiguity class
    if (ambiguity3) {
        if (tokunit->getAmbiguityClass().length() == 0)
            out << "0" << "\t";
        else
            out << tokunit->getAmbiguityClass() << "\t";

    }
    // Ambiguity class detailed
    if (ambiguity) {
        if (tokunit->getAmbiguityClassDetailed().length() == 0)
            out << "0" << "\t";
        else
            out << tokunit->getAmbiguityClassDetailed() << "\t";
    }
    // MWU token
    if (mwu) {
        QString m = sanitizeString(token->text());
        if (m.isEmpty()) m = "_";
        out << m;
    }
    // POS tag
    if (isTraining) {
        QString tag = "UNK";
        if (!tokunit->getTagPOS().isEmpty())
            tag = sanitizeString(tokunit->getTagPOS());    // give the correct answer (training file)
        else if (!token->getTagDiscourse().isEmpty())
            tag = sanitizeString(token->getTagDiscourse());
        else if (!tokunit->getTagDisfluency().isEmpty())
            tag = sanitizeString(token->getTagDisfluency());
        // Output full tag or coarse tag
        if (outputCoarsePOSTag)
            out << "\t" << tag.left(3);
        else
            out << "\t" << tag;
    }
    out << "\n";
}

void POSTagger::decodeTokenFromCRF(const QString &line, int i)
{
    Token *token = m_tokens[i];
    token->setTagsFromCRF(line.section("\t", 3));
}

void POSTagger::decodeTokenUnitFromCRF(const QString &line, int i, int j)
{
    Token *token = m_tokens[i];
    TokenUnit *tokunit = m_tokens[i]->getTokenUnits()[j];
    tokunit->setTagsFromCRF(line.section("\t", 4));
    token->updateSimpleTokenFromTokenUnit();
}



