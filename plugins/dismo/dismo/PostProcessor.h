#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

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
// Praaline.Core
#include "PraalineCore/Annotation/IntervalTier.h"
// DisMo
#include "TokenList.h"

namespace DisMoAnnotator {

class Postprocessor
{
public:
    Postprocessor(const QString &language, TokenList &tokens) :
        m_language(language), m_tokens(tokens), m_usePredicate(false), m_useAdverbCategories(true) {}
    void process(bool canMergeTokens = true);
    QStringList createInputFST();
private:
    void mergeSpecialTokens();
    void auxiliaryVerbs();
    void specialPOStags();
    void fixPOSofRepetitions();
    void tagRename();
    void finalCoherence();
    // state
    QString m_language;
    TokenList &m_tokens;
    // parameters
    bool m_usePredicate;
    bool m_useAdverbCategories;
};

}

#endif // POSTPROCESSOR_H
