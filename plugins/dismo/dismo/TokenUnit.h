#ifndef TOKENUNIT_H
#define TOKENUNIT_H

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
// Praaline.Core
#include "PraalineCore/Annotation/Interval.h"
using namespace Praaline::Core;
// DisMo
#include "DictionaryEntry.h"

namespace DisMoAnnotator {

class TokenUnit : public Interval
{
public:
    TokenUnit(const RealTime &xMin, const RealTime &xMax, const QString &text, const QString &punctuation = QString());
    TokenUnit(const Interval *interval);
    TokenUnit(const TokenUnit *tokenUnit);

    QString punctuation() const
        { return m_punctuation; }

    QString getTagPOS() const
        { return m_tagPOS; }
    QString getTagPOSext() const
        { return m_tagPOSext; }
    QString getLemma() const
        { return m_lemma; }
    QString getTagDisfluency() const
        { return m_tagDisfluency; }

    double getConfidencePOS() const
        { return m_confidencePOS; }
    double getConfidenceDisfluencyTag() const
        { return m_confidenceDisfluency; }

    QString getMatchTypePOS() const
        { return m_matchtypePOS; }

    QList<DictionaryEntry> getAllPossiblePOStags() const
        { return m_possiblePOStags; }

    virtual void setLemma(const QString lemma)
        { m_lemma = lemma; }
    virtual void setTagPOS(const QString POS, const QString POSext, const double confidencePOS)
        { m_tagPOS = POS; m_tagPOSext = POSext; m_confidencePOS = confidencePOS; }
    virtual void setTagDisfluency(const QString &tagDisfluency, const double confidenceDisfluency)
        { m_tagDisfluency = tagDisfluency; m_confidenceDisfluency = confidenceDisfluency; }

    void renameTagPOS(const QString &oldTag, const QString &newTag)
        { if (m_tagPOS == oldTag) m_tagPOS = newTag; }
    void renameTagDisfluency(const QString &oldTag, const QString &newTag)
        { if (m_tagDisfluency == oldTag) m_tagDisfluency = newTag; }

    bool isTagged() const
        { return (m_tagPOS.count() > 0); }
    bool isUnambiguous() const
        { return ((m_possiblePOStags.count() == 1) || (m_matchtypePOS == "MWU")); }

    // Prosodic features
    bool isPauseShort() const;
    bool isPauseLong() const;
    bool isPause() const
        { return isPauseShort() || isPauseLong(); }
    bool isDisfluency() const;
    virtual void setPause();

    // Morphosyntactic features
    virtual void addPossibleTag(const QString &dictionaryEntryText);
    virtual void addPossibleTag(const DictionaryEntry &dictionaryEntry);
    virtual void updateUnambiguous();
    QString getAmbiguityClass() const;
    QString getAmbiguityClassDetailed() const;

    virtual void setTagFromMWU(const QString &tagPOS);
    virtual void setTagsFromCRF(const QString &crfFileLine);

    Interval *toIntervalToken() const;
    Interval *toIntervalPOS() const;
    Interval *toIntervalDisfluency() const;

protected:
    QList<DictionaryEntry> m_possiblePOStags;
    QString m_punctuation;
    QString m_lemma;
    QString m_tagPOS;
    QString m_tagPOSext;
    double m_confidencePOS;
    QString m_matchtypePOS;
    QString m_tagDisfluency;
    double m_confidenceDisfluency;
    QString m_intonation;
};

}

#endif // TOKENUNIT_H
