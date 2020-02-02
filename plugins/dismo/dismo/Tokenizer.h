#ifndef TOKENIZER_H
#define TOKENIZER_H

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
#include <QHash>
// Praaline.Core
#include "PraalineCore/Annotation/Interval.h"
#include "PraalineCore/Annotation/IntervalTier.h"
// DisMo
#include "Token.h"
#include "TokenUnit.h"
#include "TokenList.h"

namespace DisMoAnnotator {

class Tokenizer
{
public:
    Tokenizer(QString &language, QList<TokenUnit *> &tokenUnits);
    void Tokenize(const IntervalTier *tierInput, const IntervalTier *phones = 0);
private:
    // Helper method
    QList<Interval *> separateToken(Interval *input, QStringList &split, const IntervalTier *tierPhones);

    // Dictionaries
    QHash<QString, QString> m_tokenizeTogether;
    QHash<QString, QString> m_phonetisations;
    int m_maxLookAhead;

    // State
    QString m_language;
    QList<TokenUnit *> &m_tokenUnits;
};

}

#endif // TOKENIZER_H
