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
#include "annotation/IntervalTier.h"
#include "Token.h"
#include "TokenList.h"
#include "PostProcessor.h"
using namespace DisMoAnnotator;

void Postprocessor::process(bool canMergeTokens)
{
    specialPOStags();
    if (canMergeTokens) mergeSpecialTokens();
    auxiliaryVerbs();
// Should be rewritten after updating new repetitions detector    fixPOSofRepetitions();
    tagRename();
    finalCoherence();
}

void Postprocessor::auxiliaryVerbs()
{

    // etre ou avoir + VER:ppas ==> etre ou avoir auxiliaire
    // etre/VER:pres + (possibly ADV*) + ADJ:adj ==> etre/VER:pres:pred

    int i = 0, j = 0, k = 0;
    while (i < m_tokens.count()) {
        j = i + 1; k = i + 2;
        if (j >= m_tokens.count()) j = i;
        if (k >= m_tokens.count()) k = j;
        if ((m_tokens[i]->getLemma() == "avoir" || m_tokens[i]->getLemma() == "\u00EAtre") &&
            (m_tokens[j]->getTagPOS() == "VER:ppas") && (i != j)) {
            if (!m_tokens[i]->getTagPOS().endsWith(":aux"))
                m_tokens[i]->setTagPOS(m_tokens[i]->getTagPOS() + ":aux",
                                       m_tokens[i]->getTagPOSext(),
                                       m_tokens[i]->getConfidencePOS());
        }
        if (m_usePredicate) {
            if ((m_tokens[i]->getLemma() == "\u00EAtre") &&
                (  ((m_tokens[j]->getTagPOS() == "ADJ") && (i != j))  ||
                   ((m_tokens[j]->getTagPOS().startsWith("ADV")) &&
                    (m_tokens[k]->getTagPOS().startsWith("ADJ")) &&
                    (i != j) && (j != k))  )) {
                if (!m_tokens[i]->getTagPOS().endsWith(":pred")) {
                    QString newTag = m_tokens[i]->getTagPOS() + ":pred";
                    newTag = newTag.replace(":aux:pred", ":pred");
                    m_tokens[i]->setTagPOS(newTag,
                                           m_tokens[i]->getTagPOSext(),
                                           m_tokens[i]->getConfidencePOS());

                }
            }
        }
        i++;
    }
}

void Postprocessor::mergeSpecialTokens()
{
    int i = 0, j = 0;
    while (i < m_tokens.count()) {
        QString mwuPOStag = "", mwuPOStagext;
        j = i + 1;
        if (j < m_tokens.count() - 1 && m_tokens[i]->text() == "Saint" && m_tokens[j]->getTagPOS().left(3) == "NOM") {
            mwuPOStag = "NOM:pro";
            mwuPOStagext = m_tokens[j]->getTagPOSext();
            j++;
        }
        else if (m_tokens[i]->getTagPOS().startsWith("NUM")) {
            while (j < m_tokens.count() && m_tokens[j]->getTagPOS().startsWith("NUM")) {
                mwuPOStag = m_tokens[j]->getTagPOS();
                mwuPOStagext = m_tokens[j]->getTagPOSext();
                j++;
            }
        }
        else if (m_tokens[i]->getTagPOS() == "NOM:pro") {
            while (j < m_tokens.count() && m_tokens[j]->getTagPOS() == "NOM:pro") {
                j++;
            }
        }
        if (j > i + 1) {
            m_tokens.mergeTokens(i, j - 1);
            m_tokens[i]->setText(m_tokens[i]->text().replace("- ", "-"));
            if (!mwuPOStag.isEmpty()) m_tokens[i]->setTagPOS(mwuPOStag, mwuPOStagext, 1.0);
            i = j;
        }
        else
            i++;
    }
}

void Postprocessor::specialPOStags()
{
    foreach(Token *tok, m_tokens) {
        foreach(TokenUnit *tu, tok->getTokenUnits()) {
            if (tu->getTagDisfluency() == "FST")
                tu->setTagPOS("0", "", 1);
            if (tu->isPause())
                tu->setTagPOS("_", "", 1);
        }
        if (tok->getTagPOS().startsWith("CON"))
            tok->setTagDiscourse("CON", 1.0);
        if (tok->getTagDiscourse() == "PNC")
            tok->setTagPOS("PNC", "", 1.0);

        QRegExp re("\\d*");  // a digit (\d), zero or more times (*)
        if (re.exactMatch(tok->text())) {
            tok->setTagPOS("NUM:crd", "", 1.0);
        }
    }
}

void Postprocessor::fixPOSofRepetitions()
{
    int i = 0, j = 0;
    while (i < m_tokens.count()) {
        if (m_tokens[i]->getTagDisfluency().startsWith("REP")) {
            j = i + 1;
            while ( (j < m_tokens.count()) &&
                    (m_tokens[i]->text() == m_tokens[j]->text()) &&
                    (m_tokens[j]->getTagDisfluency().startsWith("REP"))) {
                j++;
            }
            if (m_tokens[i]->text() == m_tokens[j]->text()) {
                for (int k = i; k < j; k++) {
                    m_tokens[k]->setTagPOS(m_tokens[j]->getTagPOS(),
                                           m_tokens[j]->getTagPOSext(),
                                           m_tokens[j]->getConfidencePOS());
                }
            }
        }
        i++;
    }
}

void Postprocessor::tagRename()
{
    foreach(Token *tok, m_tokens) {
//        foreach(TokenUnit *tu, tok->getTokenUnits()) {
//        }
        tok->renameTagPOS("NUM:ord:det", "NUM:ord:adj");
        if (!m_useAdverbCategories) {
            tok->renameTagPOS("ADV:deg", "ADV");
            tok->renameTagPOS("ADV:comp", "ADV");
        }
        if (!m_usePredicate) {
            if (tok->getTagPOS().endsWith(":pred")) {
                QString s = tok->getTagPOS(); s.chop(5);
                tok->setTagPOS(s, tok->getTagPOSext(), tok->getConfidencePOS());
            }
        }
    }
}

void Postprocessor::finalCoherence()
{
    foreach(Token *tok, m_tokens) {
        if (tok->getTokenUnits().count() == 1) {
            TokenUnit *tu = tok->getTokenUnits()[0];
            if (tok->getTagPOS().isEmpty())
                tok->setTagPOS(tu->getTagPOS(), tu->getTagPOSext(), tu->getConfidencePOS());
            if (tu->getTagPOS() != tok->getTagPOS())
                tu->setTagPOS(tok->getTagPOS(), tok->getTagPOSext(), tok->getConfidencePOS());
            // Lengthening convention
            if (tu->text().contains("=")) {
                QString disf = tu->getTagDisfluency();
                double cdisf = tu->getConfidenceDisfluencyTag();
                if (disf.isEmpty()) { disf = "LEN"; cdisf = 1.0; } else disf = disf + "+LEN";
                tu->setTagDisfluency(disf, cdisf);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

QStringList Postprocessor::createInputFST()
{
    QString line;
    QStringList fstDef;
    QString separator = "@";
    int i = 0;
    foreach(Token *token, m_tokens) {
        foreach(TokenUnit *tokunit, token->getTokenUnits()) {
            line = QString::number(i).append(" ").append(QString::number(i+1)).append(" ");
            line.append(tokunit->text().replace(" ", "_")).append(separator);
            line.append(tokunit->getTagPOS()).append(separator);
            line.append(token->text().replace(" ", "_")).append(separator);
            line.append(token->getTagPOS()).append(separator);
            line.append(token->getTagDiscourse()).append(separator);
            line.append(tokunit->getTagDisfluency());
            fstDef << line;
            i++;
        }
    }
    return fstDef;
}


