#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

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
#include <QHash>
// Praaline.Core
#include "annotation/IntervalTier.h"
// DisMo
#include "DictionaryEntry.h"
#include "TokenList.h"

namespace DisMoAnnotator {

class Preprocessor
{
public:
    Preprocessor(const QString &language, TokenList &tokens);
    void process(const IntervalTier *input, const IntervalTier *tierPhones = 0);
    void processTokenizedToMinimal(const IntervalTier *tier_tok_min);
    void processTokenized();
private:
    // State
    QString m_language;
    bool m_useSQLDictionary;
    QString m_filenameDictionary;       // the main dictionary
    QString m_filenameMWUDictionary;    // list of multi-word units
    QHash<QString, QString> m_dictionaryMWU;
    TokenList &m_tokens;
    // Steps
    void loadDictionaries();
    void applyLEXMIN(QList<TokenUnit *> &tokenUnits);
    void applyLEXMWU(QList<TokenUnit *> &tokenUnits);
    void applyLEXtoTokenized();
    void applyParaverbal();
    void applyPunctuation();
    void prosodicPreprocessing();
    void findMajorFrontiers();
};

}

#endif // PREPROCESSOR_H
