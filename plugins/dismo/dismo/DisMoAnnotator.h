#ifndef DISMOANNOTATOR_H
#define DISMOANNOTATOR_H

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

// Praaline.Core
#include <QHash>
#include "annotation/AnnotationTierGroup.h"
// DisMo
#include "PreProcessor.h"
#include "BoundaryDetector.h"
#include "DisfluencyDetector.h"
#include "DictionaryFST.h"
#include "POSTagger.h"
#include "DiscourseTagger.h"
#include "PostProcessor.h"
#include "Tokenizer.h"
#include "TokenList.h"

namespace DisMoAnnotator {

class DismoAnnotator
{
public:
    DismoAnnotator(const QString &language);
    ~DismoAnnotator();

    bool annotate(IntervalTier *tier_input, IntervalTier *tier_tok_min, IntervalTier *tier_tok_mwu, IntervalTier *tier_phones = 0);
    bool annotateTokenized(IntervalTier *tier_tok_min, IntervalTier *tier_tok_mwu,
                           const QHash<QString, QString> &attributes = QHash<QString, QString>());
    bool annotateTokenizedToMinimal(IntervalTier *tier_tok_min, IntervalTier *tier_tok_mwu,
                                    const QHash<QString, QString> &attributes = QHash<QString, QString>());

    // Methods to use different models than the supplied ones (e.g. for evaluating the system)
    void setModelPOSMin(const QString &modelFilename);
    void setModelDisfluency(const QString &modelFilename);
    void setModelDiscourse(const QString &modelFilename);
    void setModelPOSFin(const QString &modelFilename);

    // The token lists are publicly accessible
    TokenList TL;

private:
    Preprocessor PREPRO;
    POSTagger POSTAG;
    DisfluencyDetector DISFLDET;
    DiscourseTagger DISCTAG;
    BoundaryDetector BOUNDET;
    Postprocessor POSTPRO;

    // Model filenames
    QString m_modelFilename_POSMin;
    QString m_modelFilename_Disfluency;
    QString m_modelFilename_Discourse;
    QString m_modelFilename_POSFin;
};

}

#endif // DISMOANNOTATOR_H
