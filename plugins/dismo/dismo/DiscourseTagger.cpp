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
#include "DiscourseTagger.h"
using namespace DisMoAnnotator;

bool DiscourseTagger::isEOSToken(int i)
{
    if (m_tokens[i]->getTagFrontier() == "MAJ") return true;
    return false;
}

bool DiscourseTagger::isEOSTokenUnit(int i, int j)
{
    return false;
}

bool DiscourseTagger::skipToken(int i)
{
    Token *token = m_tokens[i];
    if (token->isPause() || (!token->getTagDisfluency().isEmpty() && token->getTagDisfluency() != "0"))
        return true;
    return false;
}

bool DiscourseTagger::skipTokenUnit(int i, int j)
{
    TokenUnit *tokunit = m_tokens[i]->getTokenUnits()[j];
    if (tokunit->isPause() || (!tokunit->getTagDisfluency().isEmpty() && tokunit->getTagDisfluency() != "0"))
        return true;
    return false;
}

void DiscourseTagger::writeTokenToCRF(QTextStream &out, int i, bool isTraining)
{
    Token *token = m_tokens[i];
    out << token->text().replace(" ", "_").replace("#", "_") << "\t";
    RealTime dur = token->duration();
    int durRounded = ((dur.msec() / 20) * 20);
    out << durRounded << "\t";
    if (token->isPauseShort())
        out << "SIL:b";
    else if (token->isPauseLong())
        out << "SIL:l";
    else {
        if (!token->getTagPOS().isEmpty() && token->getTagPOS() != "0")
            out << token->getTagPOS();
        else
            out << "0";
    }
    if (isTraining) {
        if (token->getTagDiscourse().isEmpty())
            out << "\t0";
        else
            out << "\t" << token->getTagDiscourse();
    }
    out << "\n";
}

void DiscourseTagger::writeTokenUnitToCRF(QTextStream &out, int i, int j, bool isTraining)
{
    // Discourse tags are always annotated at the token level
}

void DiscourseTagger::decodeTokenFromCRF(const QString &line, int i)
{
    Token *token = m_tokens[i];
    if (token->isPause()) return;
    if (token->getConfidenceDiscourseTag() == 1.0) return;
    // no discourse tag
    QString reply = line.section("\t", 3, 3);
    if (reply.left(1) == "0")
        token->setTagDiscourse("", reply.section("/", 1, 1).toDouble());
    else
        token->setTagDiscourse(reply.section("/", 0, 0), reply.section("/", 1, 1).toDouble());
}

void DiscourseTagger::decodeTokenUnitFromCRF(const QString &line, int i, int j)
{
    // Discourse tags are always annotated at the token level
}

