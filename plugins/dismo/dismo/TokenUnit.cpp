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
#include "PraalineCore/Annotation/Interval.h"
#include "TokenUnit.h"
using namespace DisMoAnnotator;

TokenUnit::TokenUnit(const RealTime &tMin, const RealTime &tMax, const QString &text, const QString &punctuation) :
    Interval(tMin, tMax, text), m_punctuation(punctuation)
{
    m_lemma = m_tagPOS = m_tagPOSext = m_tagDisfluency = "";
    m_confidencePOS = 0.0; m_confidenceDisfluency = 0.0;
    m_matchtypePOS = "";
    m_intonation = "";
}
TokenUnit::TokenUnit(const Interval *interval) :
    Interval(interval->tMin(), interval->tMax(), interval->text())
{
    m_lemma = m_tagPOS = m_tagPOSext = m_tagDisfluency = "";
    m_confidencePOS = 0.0; m_confidenceDisfluency = 0.0;
    m_matchtypePOS = "";
    m_intonation = "";
}
TokenUnit::TokenUnit(const TokenUnit *tokenUnit) :
    Interval(tokenUnit->tMin(), tokenUnit->tMax(), tokenUnit->text())
{
    m_lemma = tokenUnit->m_lemma;
    m_tagPOS = tokenUnit->m_tagPOS;
    m_tagPOSext = tokenUnit->m_tagPOSext;
    m_tagDisfluency = tokenUnit->m_tagDisfluency;
    m_confidencePOS = tokenUnit->m_confidencePOS;
    m_confidenceDisfluency = tokenUnit->m_confidenceDisfluency;
    m_matchtypePOS = tokenUnit->m_matchtypePOS;
    m_intonation = tokenUnit->m_intonation;
}

// Prosodic features

bool TokenUnit::isPauseShort() const
{
    return (m_tagDisfluency == "SIL:b");
}

bool TokenUnit::isPauseLong() const
{
    return (m_tagDisfluency == "SIL:l");
}

bool TokenUnit::isDisfluency() const
{
    return (m_tagDisfluency == "HESI" || m_tagDisfluency == "AMO" ||
            m_tagDisfluency == "REP-B"  || m_tagDisfluency == "REP-I" ||
            m_tagDisfluency == "CORR-B" || m_tagDisfluency == "CORR-I");
}

void TokenUnit::setPause()
{
    m_possiblePOStags.clear();
    if (duration().toDouble() > 0.250) {
        m_tagDisfluency = "SIL:l";
        m_possiblePOStags << DictionaryEntry(m_text, "_", "", "SIL:l", "SIL:l");
    }
    else {
        m_tagDisfluency = "SIL:b";
        m_possiblePOStags << DictionaryEntry(m_text, "_", "", "SIL:b", "SIL:b");
    }
    updateUnambiguous();
}

// Morpho-syntactic features

void TokenUnit::addPossibleTag(const QString &dictionaryEntryText)
{
    QStringList entries = QString(dictionaryEntryText).split("#", QString::KeepEmptyParts);
    QString Lemma, POS, POSext, Discourse, Disfluency;
    if (entries.count() >= 1) Lemma = entries.at(0);
    if (Lemma.length() == 0) Lemma = m_text;
    if (entries.count() >= 2) POS = entries.at(1);
    if (entries.count() >= 3) POSext = entries.at(2);
    if (entries.count() >= 4) Disfluency = entries.at(3);
    if (entries.count() >= 5) Discourse = entries.at(4);
    m_possiblePOStags << DictionaryEntry(Lemma, POS, POSext, Discourse, Disfluency);
}

void TokenUnit::addPossibleTag(const DictionaryEntry &dictionaryEntry)
{
    m_possiblePOStags << dictionaryEntry;
}

void TokenUnit::updateUnambiguous()
{
    if ((m_possiblePOStags.count() == 1) && (m_matchtypePOS != "MWU")) {
        m_lemma = m_possiblePOStags[0].Lemma;
        m_tagPOS = m_possiblePOStags[0].POS;
        m_tagPOSext = m_possiblePOStags[0].POSExt;
        m_tagDisfluency = m_possiblePOStags[0].Disfluency;
        m_confidencePOS = 1.0;
        m_confidenceDisfluency = 1.0;
        m_matchtypePOS = "UNAMB";
    }
}

QString TokenUnit::getAmbiguityClass() const
{
    QList<QString> list;
    QString ret = "";
    foreach(DictionaryEntry entry, m_possiblePOStags) {
        // get only the left three characters = main POS category
        if (!list.contains(entry.POS.left(3)))
            list << entry.POS.left(3);
    }
    if (list.isEmpty())
        return "UNK";
    foreach(QString tag, list)
        ret.append(tag).append("+");
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

QString TokenUnit::getAmbiguityClassDetailed() const
{
    QList<QString> list;
    QString ret = "";
    foreach(DictionaryEntry entry, m_possiblePOStags) {
        // get entire possible POS tag
        if (!list.contains(entry.POS))
            list << entry.POS;
    }
    if (list.isEmpty())
        return "UNK";
    foreach(QString tag, list)
        ret.append(tag).append("+");
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

void TokenUnit::setTagFromMWU(const QString &tagPOS)
{
    m_tagPOS = tagPOS;
    m_confidencePOS = 1.0;
    m_matchtypePOS = "MWU";
    foreach(DictionaryEntry entry, m_possiblePOStags) {
        if (tagPOS.left(7) == entry.POS.left(7)) {
            // exact match, therefore we get ext and lemma
            m_tagPOSext = entry.POSExt;
            m_lemma = entry.Lemma;
            return;
        }
    }
    if (m_lemma.isEmpty()) {
        m_tagPOSext = "";
        m_lemma = m_text;
    }
}

void TokenUnit::setTagsFromCRF(const QString &crfFileLine)
{
    if (isUnambiguous())
        return;
    // try exact match with the first CRF tag, checking entire tag
    for (int i = 0; i < 1; i++) {
        QString pos = crfFileLine.section("\t", i, i).section("/", 0, 0);
        double conf = crfFileLine.section("\t", i, i).section("/", 1, 1).toDouble();
        foreach(DictionaryEntry entry, m_possiblePOStags) {
            if (pos == entry.POS) {
                // exact match
                m_tagPOS = pos;
                m_tagPOSext = entry.POSExt;
                m_lemma = entry.Lemma;
                m_confidencePOS = conf;
                m_matchtypePOS = "EXACT" + QString::number(i);
                return;
            }
        }
    }
    // try exact match with the first or second CRF tag ignoring subtag (:aux, :pred, :entatif)
    for (int i = 0; i < 2; i++) {
        QString pos = crfFileLine.section("\t", i, i).section("/", 0, 0);
        double conf = crfFileLine.section("\t", i, i).section("/", 1, 1).toDouble();
        foreach(DictionaryEntry entry, m_possiblePOStags) {
            QString crfPOSNoSubcat = pos.section(":", 0, 1);
            QString entryPOSNoSubcat = entry.POS.section(":", 0, 1);
            if (crfPOSNoSubcat == entryPOSNoSubcat) {
                // exact match
                m_tagPOS = entry.POS;
                m_tagPOSext = entry.POSExt;
                m_lemma = entry.Lemma;
                m_confidencePOS = conf;
                m_matchtypePOS = "EXACTCAT" + QString::number(i);
                return;
            }
        }
    }
    // try approximate match with the first three CRF tags
    for (int i = 0; i < 3; i++) {
        QString pos = crfFileLine.section("\t", i, i).section("/", 0, 0);
        double conf = crfFileLine.section("\t", i, i).section("/", 1, 1).toDouble();
        foreach(DictionaryEntry entry, m_possiblePOStags) {
            if (pos.left(3) == entry.POS.left(3)) {
                // approximate match
                m_tagPOS = entry.POS;
                m_tagPOSext = entry.POSExt;
                m_lemma = entry.Lemma;
                m_confidencePOS = conf;
                m_matchtypePOS = "APPROX" + QString::number(i) + "_" + pos;
                return;
            }
        }
    }
    // No chance...
    if (m_possiblePOStags.count() > 0) {
        DictionaryEntry entry = m_possiblePOStags[0];
        m_tagPOS = entry.POS;
        m_tagPOSext = entry.POSExt;
        m_lemma = entry.Lemma;
        m_confidencePOS = crfFileLine.section("\t", 0, 0).section("/", 1, 1).toDouble();
        m_matchtypePOS = "CONFL-LEX";
    }
    else {
        m_tagPOS = crfFileLine.section("\t", 0, 0).section("/", 0, 0);
        m_tagPOSext = "";
        m_lemma = text();
        m_confidencePOS = crfFileLine.section("\t", 0, 0).section("/", 1, 1).toDouble();
        m_matchtypePOS = "CONFL-CRF";
    }
}

Interval *TokenUnit::toIntervalToken() const
{
    return new Interval(m_tMin, m_tMax, m_text);
}

Interval *TokenUnit::toIntervalPOS() const
{
    return new Interval(m_tMin, m_tMax, m_tagPOS);
}

Interval *TokenUnit::toIntervalDisfluency() const
{
    return new Interval(m_tMin, m_tMax, m_tagDisfluency);
}

