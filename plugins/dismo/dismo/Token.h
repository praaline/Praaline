#ifndef TOKEN_H
#define TOKEN_H

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
// Praaline.Core
#include "PraalineCore/Base/RealTime.h"
#include "PraalineCore/Annotation/Interval.h"
// DisMo
#include "TokenUnit.h"

namespace DisMoAnnotator {

class Token : public TokenUnit
{
public:
    Token(TokenUnit *tokenUnit);
    Token(const QList<TokenUnit *> &listTokMin, const QString &textMWU);
    ~Token();    

    QString getTagDiscourse() const
        { return m_tagDiscourse; }
    QString getTagFrontier() const
        { return m_tagFrontier; }

    double getConfidenceDiscourseTag() const
        { return m_confidenceDiscourse; }
    double getConfidenceFrontierTag() const
        { return m_confidenceFrontier; }

    // override mutators from TokenUnit
    void setLemma(const QString lemma);
    void setTagPOS(const QString POS, const QString POSext, const double confidencePOS);
    void setTagDisfluency(const QString &tagDisfluency, const double confidenceDisfluency);
    void setPause();
    void addPossibleTag(const QString &dictionaryEntryText);
    void addPossibleTag(const DictionaryEntry &dictionaryEntry);
    void updateUnambiguous();
    void setTagsFromCRF(const QString &crfFileLine);

    void updateSimpleTokenFromTokenUnit();
    void updateTokenUnitsFromMWUToken(const QString &MWUEntry);
    void updateAmbiguityClassFromTag();

    // these mutators apply only to Token objects
    void setTagDiscourse(const QString &tagDiscourse, const double confidenceDiscourse)
        { m_tagDiscourse = tagDiscourse; m_confidenceDiscourse = confidenceDiscourse; }
    void setTagFrontier(const QString &tagFrontier, const double confidenceFrontier)
        { m_tagFrontier = tagFrontier; m_confidenceFrontier = confidenceFrontier; }

    bool isSimple() const
        { return (m_tokenUnits.count() == 1); }
    bool isMWU() const
        { return (m_tokenUnits.count() > 1); }

    QList<TokenUnit *> getTokenUnits() const
        { return m_tokenUnits; }

    QList<Interval *>toIntervalsTokMin() const;
    QList<Interval *>toIntervalsPOSMin() const;
    QList<Interval *>toIntervalsDisfluency() const;
    Interval *toIntervalDiscourse() const;
    Interval *toIntervalFrontier() const;

private:
    QList<TokenUnit *>m_tokenUnits;
    QString m_tagDiscourse;
    QString m_tagFrontier;
    double m_confidenceDiscourse;
    double m_confidenceFrontier;
};

}

#endif // TOKEN_H
