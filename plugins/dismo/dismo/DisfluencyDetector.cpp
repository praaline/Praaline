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
#include <QVector>
#include <QTextStream>
#include "TokenUnit.h"
#include "Token.h"
#include "TokenList.h"
#include "CRFAnnotator.h"
#include "DisfluencyDetector.h"
using namespace DisMoAnnotator;

bool DisfluencyDetector::isEOSToken(int i)
{
    Token *token = m_tokens[i];
    if (token->isPause() && token->duration().toDouble() > 0.350) {
        m_previous_tokunit = "";
        return true;
    }
    return false;
}

bool DisfluencyDetector::isEOSTokenUnit(int i, int j)
{
    Token *token = m_tokens[i];
    if (token->isPause()  && token->duration().toDouble() > 0.350) {
        m_previous_tokunit = "";
        return true;
    }
    return false;
}

bool DisfluencyDetector::skipToken(int i)
{
    if (m_tokens[i]->text().isEmpty()) return true;
    return false;
}

bool DisfluencyDetector::skipTokenUnit(int i, int j)
{
    if (m_tokens[i]->getTokenUnits()[j]->text().isEmpty()) return true;
    return false;
}

void DisfluencyDetector::writeTokenToCRF(QTextStream &out, int i, bool isTraining)
{
    // Disfluencies are always found at the token unit level
}

void DisfluencyDetector::writeTokenUnitToCRF(QTextStream &out, int i, int j, bool isTraining)
{
    Token *token = m_tokens[i];
    TokenUnit *tokunit = token->getTokenUnits()[j];
    QString tok_unit_text = tokunit->text().replace(" ", "_").replace("#", "_");
    if (tok_unit_text.isEmpty()) tok_unit_text = "_";
    out << tok_unit_text << "\t";
    RealTime dur = tokunit->duration();
    int durRounded = ((dur.msec() / 20) * 20);
    out << durRounded << "\t";
    if (tokunit->isPauseShort())
        out << "SIL:b\t";
    else if (tokunit->isPauseLong())
        out << "SIL:l\t";
    else {
        if (!tokunit->getTagPOS().isEmpty() && tokunit->getTagPOS() != "0")
            out << tokunit->getTagPOS() << "\t";
        else if (!token->getTagDiscourse().isEmpty())
            out << token->getTagDiscourse() << "\t";
        else
            out << "UNK\t";
    }
    out << editDistance(m_previous_tokunit, tokunit->text());
    if (isTraining) {
        if (tokunit->getTagDisfluency().isEmpty())
            out << "\t0";
        else
            out << "\t" << tokunit->getTagDisfluency();
    }
    out << "\n";
    m_previous_tokunit = tokunit->text();
}

void DisfluencyDetector::decodeTokenFromCRF(const QString &line, int i)
{
    // Disfluencies are always found at the token unit level
}

void DisfluencyDetector::decodeTokenUnitFromCRF(const QString &line, int i, int j)
{
    Token *token = m_tokens[i];
    TokenUnit *tokunit = token->getTokenUnits()[j];
    if (tokunit->isPause()) return;
    if (tokunit->getConfidenceDisfluencyTag() == 1.0) return;
    // no disfluency
    QString reply = line.section("\t", 4, 4);
    if (reply.left(1) == "0") return;
    // else, annotate the disfluency
    tokunit->setTagDisfluency(reply.section("/", 0, 0), reply.section("/", 1, 1).toDouble());
    token->updateSimpleTokenFromTokenUnit();
}

int DisfluencyDetector::editDistance(const QString &A, const QString &B)
{
    int insert_cost = 1;
    int delete_cost = 1;
    int replace_cost = 2;

    int lenA = A.length();
    int lenB = B.length();
    QVector<int> m0(lenB + 1);
    QVector<int> m1(lenB + 1);

    for (int b = 0; b <= lenB; b++) {
        m0[b] = b * delete_cost;
    }
    for (int a = 1; a <= lenA; a++) {
        m1[0] = a * insert_cost;
        for (int b = 1; b <= lenB; b++) {
            int x = m0[b] + insert_cost;
            int y = m1[b-1] + delete_cost;
            QChar cA = A[a-1]; QChar cB = B[b-1];
            int z = m0[b-1] + ((cA == cB) ? 0 : replace_cost);
            m1[b] = qMin(qMin(x, y), z);
        }
        qSwap(m0, m1);
    }
    return m0[lenB];
}
