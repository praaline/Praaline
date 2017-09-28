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
#include "annotation/Interval.h"
#include "TokenUnit.h"
#include "Token.h"
using namespace DisMoAnnotator;

Token::Token(TokenUnit *tokenUnit)
    : TokenUnit((Interval *)tokenUnit)
{
    // tMin, tMax,
    m_lemma = tokenUnit->getLemma();
    m_tagPOS = tokenUnit->getTagPOS();
    m_tagPOSext = tokenUnit->getTagPOSext();
    m_possiblePOStags = tokenUnit->getAllPossiblePOStags();
    m_confidencePOS = tokenUnit->getConfidencePOS();
    //
    m_tokenUnits << tokenUnit;
    m_tagDiscourse = m_tagDisfluency = m_tagFrontier = "";
    m_confidenceDiscourse = m_confidenceDisfluency = m_confidenceFrontier = 0.0;
}

Token::Token(const QList<TokenUnit *> &listTokMin, const QString &textMWU)
    : TokenUnit(RealTime(0,0), RealTime(0,0), textMWU)
{
    if (listTokMin.count() > 0) {
        m_tMin = listTokMin[0]->tMin();
        m_tMax = listTokMin[listTokMin.count() - 1]->tMax();
    }
    //
    m_lemma = m_tagPOS = m_tagPOSext = "";
    m_confidencePOS = 0.0;
    //
    m_tokenUnits << listTokMin;
    m_tagDiscourse = m_tagDisfluency = m_tagFrontier = "";
    m_confidenceDiscourse = m_confidenceDisfluency = m_confidenceFrontier = 0.0;
}

Token::~Token()
{
    qDeleteAll(m_tokenUnits);
}

QList<Interval *> Token::toIntervalsTokMin() const
{
    QList<Interval *> ret;
    foreach(TokenUnit *tokMin, m_tokenUnits) {
        ret << tokMin->toIntervalToken();
    }
    return ret;
}

QList<Interval *> Token::toIntervalsPOSMin() const
{
    QList<Interval *> ret;
    foreach(TokenUnit *tokMin, m_tokenUnits) {
        ret << tokMin->toIntervalPOS();
    }
    return ret;
}

QList<Interval *> Token::toIntervalsDisfluency() const
{
    QList<Interval *> ret;
    foreach(TokenUnit *tokMin, m_tokenUnits) {
        ret << tokMin->toIntervalDisfluency();
    }
    return ret;
}

Interval *Token::toIntervalDiscourse() const
{
    return new Interval(m_tMin, m_tMax, m_tagDiscourse);
}

Interval *Token::toIntervalFrontier() const
{
    return new Interval(m_tMin, m_tMax, m_tagFrontier);
}

// Overriden mutators from TokenUnit. When the token is Simple (only one TokenUnit), we synchronize
// the tags.
void Token::setLemma(const QString lemma)
{
    if (isSimple()) m_tokenUnits[0]->setLemma(lemma);
    TokenUnit::setLemma(lemma);  // call base
}

void Token::setTagPOS(const QString POS, const QString POSext, const double confidencePOS)
{
    if (isSimple()) m_tokenUnits[0]->setTagPOS(POS, POSext, confidencePOS);
    TokenUnit::setTagPOS(POS, POSext, confidencePOS);
}

void Token::setTagDisfluency(const QString &tagDisfluency, const double confidenceDisfluency)
{
    if (isSimple()) m_tokenUnits[0]->setTagDisfluency(tagDisfluency, confidenceDisfluency);
    TokenUnit::setTagDisfluency(tagDisfluency, confidenceDisfluency);
}

void Token::setPause()
{
    if (isSimple()) m_tokenUnits[0]->setPause();
    TokenUnit::setPause();
}

void Token::addPossibleTag(const QString &dictionaryEntryText)
{
    if (isSimple()) m_tokenUnits[0]->addPossibleTag(dictionaryEntryText);
    TokenUnit::addPossibleTag(dictionaryEntryText);
}

void Token::addPossibleTag(const DictionaryEntry &dictionaryEntry)
{
    if (isSimple()) m_tokenUnits[0]->addPossibleTag(dictionaryEntry);
    TokenUnit::addPossibleTag(dictionaryEntry);
}

void Token::updateUnambiguous()
{
    if (isSimple()) {
        m_tokenUnits[0]->updateUnambiguous();
        // update discoure tag, if available (unambiguous MDs).
        if (m_tokenUnits[0]->getAllPossiblePOStags().count() == 1)
            setTagDiscourse(m_tokenUnits[0]->getAllPossiblePOStags()[0].Discourse, 1.0);
    }
    TokenUnit::updateUnambiguous();
}

void Token::setTagsFromCRF(const QString &crfFileLine)
{
    if (isSimple()) m_tokenUnits[0]->setTagsFromCRF(crfFileLine);
    TokenUnit::setTagsFromCRF(crfFileLine);
}

void Token::updateSimpleTokenFromTokenUnit()
{
    if (!isSimple()) return;
    m_lemma = m_tokenUnits[0]->getLemma();
    m_tagPOS = m_tokenUnits[0]->getTagPOS();
    m_tagPOSext = m_tokenUnits[0]->getTagPOSext();
    m_confidencePOS = m_tokenUnits[0]->getConfidencePOS();
    m_tagDisfluency = m_tokenUnits[0]->getTagDisfluency();
    m_confidenceDisfluency = m_tokenUnits[0]->getConfidenceDisfluencyTag();
    m_possiblePOStags = m_tokenUnits[0]->getAllPossiblePOStags();
    // m_intonation
}

void Token::updateTokenUnitsFromMWUToken(const QString &MWUEntry)
{
    if (isSimple()) return;
    QString Lemma = MWUEntry.section("#", 0, 0);
    if (Lemma.length() == 0) Lemma = m_text;
    m_lemma = Lemma;
    m_tagPOS = MWUEntry.section("#", 1, 1);
    m_tagPOSext = MWUEntry.section("#", 2, 2);
    m_tagDiscourse = MWUEntry.section("#", 3, 3);
    for (int i = 0; i < m_tokenUnits.count(); i++) {
        m_tokenUnits[i]->setTagFromMWU(MWUEntry.section("#", 4, 4).section("$", i, i));
    }
    updateAmbiguityClassFromTag();
}

void Token::updateAmbiguityClassFromTag()
{
    m_possiblePOStags.clear();
    m_possiblePOStags << DictionaryEntry(m_lemma, m_tagPOS, m_tagPOSext);
    m_confidencePOS = 1.0;
}

