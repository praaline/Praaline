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
#include <QCoreApplication>
#include "annotation/AnnotationTierGroup.h"
#include "PreProcessor.h"
#include "BoundaryDetector.h"
#include "DisfluencyDetector.h"
#include "DictionaryFST.h"
#include "POSTagger.h"
#include "PostProcessor.h"
#include "Tokenizer.h"
#include "TokenList.h"
#include "DisMoAnnotator.h"
using namespace DisMoAnnotator;

DismoAnnotator::DismoAnnotator(const QString &language)
    : TL(), PREPRO(language, TL), POSTAG(TL), DISFLDET(TL), DISCTAG(TL), BOUNDET(TL), POSTPRO(language, TL)
{
    // default model filenames
    // DIRECTORY:
    QString appPath = QCoreApplication::applicationDirPath();
    m_modelFilename_POSMin = appPath + "/plugins/dismo/models/posmin_" + language + ".model";
    m_modelFilename_Disfluency = appPath + "/plugins/dismo/models/disfluency_" + language + ".model";
    m_modelFilename_Discourse = appPath + "/plugins/dismo/models/discourse_" + language + ".model";
    m_modelFilename_POSFin = appPath + "/plugins/dismo/models/posfin_" + language + ".model";
}

DismoAnnotator::~DismoAnnotator()
{
    qDeleteAll(TL);
}

void DismoAnnotator::setModelPOSMin(const QString &modelFilename)
{
    m_modelFilename_POSMin = modelFilename;
}
void DismoAnnotator::setModelDisfluency(const QString &modelFilename)
{
    m_modelFilename_Disfluency = modelFilename;
}
void DismoAnnotator::setModelDiscourse(const QString &modelFilename)
{
    m_modelFilename_Discourse = modelFilename;
}
void DismoAnnotator::setModelPOSFin(const QString &modelFilename)
{
    m_modelFilename_POSFin = modelFilename;
}

bool DismoAnnotator::annotate(IntervalTier *tier_input,
                              IntervalTier *tier_tok_min, IntervalTier *tier_tok_mwu,
                              IntervalTier *tier_phones)
{
    if (!tier_input || !tier_tok_min || !tier_tok_mwu) return false;

    qDeleteAll(TL);
    TL.clear();
    PREPRO.process(tier_input, tier_phones);
    POSTAG.tagTokenUnits(m_modelFilename_POSMin);
    DISFLDET.tagTokenUnits(m_modelFilename_Disfluency);
    DISCTAG.tagTokens(m_modelFilename_Discourse);
    BOUNDET.process();
    POSTAG.tagTokens(m_modelFilename_POSFin);
    POSTPRO.process(true); // will merge tokens

    TL.writeToTiers(tier_tok_min, 0, tier_tok_mwu, 0, 0, 0, 0);
    return true;
}

bool DismoAnnotator::annotateTokenized(IntervalTier *tier_tok_min, IntervalTier *tier_tok_mwu,
                                       const QHash<QString, QString> &attributes)
{
    if (!tier_tok_min || !tier_tok_mwu) return false;
    qDeleteAll(TL);
    TL.clear();
    foreach (Interval *intv_tok_mwu, tier_tok_mwu->intervals()) {
        QList<Interval *> intvs_tok_min = tier_tok_min->getIntervalsContainedIn(intv_tok_mwu);
        QList<TokenUnit *> tokenUnits;
        foreach (Interval *intv, intvs_tok_min) tokenUnits << new TokenUnit(intv);
        Token *tok = new Token(tokenUnits, intv_tok_mwu->text());
        TL << tok;
    }
    PREPRO.processTokenized();
    POSTAG.tagTokenUnits(m_modelFilename_POSMin);
    DISFLDET.tagTokenUnits(m_modelFilename_Disfluency);
    DISCTAG.tagTokens(m_modelFilename_Discourse);
    BOUNDET.process();
    POSTAG.tagTokens(m_modelFilename_POSFin);
    POSTPRO.process(false); // will not merge tokens

    TL.updateTokenizedTiers(tier_tok_min, tier_tok_mwu, attributes);
    return true;
}

bool DismoAnnotator::annotateTokenizedToMinimal(IntervalTier *tier_tok_min, IntervalTier *tier_tok_mwu,
                                                const QHash<QString, QString> &attributes)
{
    if (!tier_tok_min) return false;
    qDeleteAll(TL);
    TL.clear();
    PREPRO.processTokenizedToMinimal(tier_tok_min);
    POSTAG.tagTokenUnits(m_modelFilename_POSMin);
    DISFLDET.tagTokenUnits(m_modelFilename_Disfluency);
    DISCTAG.tagTokens(m_modelFilename_Discourse);
    BOUNDET.process();
    POSTAG.tagTokens(m_modelFilename_POSFin);
    POSTPRO.process(true); // will merge tokens

    TL.updateTokenizedTiers(tier_tok_min, tier_tok_mwu, attributes, true);
    return true;
}

