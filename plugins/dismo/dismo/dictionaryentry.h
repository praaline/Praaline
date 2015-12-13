#ifndef DICTIONARYENTRY_H
#define DICTIONARYENTRY_H

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

namespace DisMoAnnotator {

class DictionaryEntry
{
public:
    DictionaryEntry() :
        Lemma(QString()), POS(QString()), POSExt(QString()), Discourse(QString()), Disfluency(QString()), MWUPOS(QString()) {}

    DictionaryEntry(const QString &lemma, const QString &pos, const QString &posext,
                    const QString &discourse = "", const QString &disfluency = "",
                    const QString &mwupos = "") :
        Lemma(lemma), POS(pos), POSExt(posext), Discourse(discourse), Disfluency(disfluency), MWUPOS(mwupos) {}

    QString Lemma;
    QString POS;
    QString POSExt;
    QString Discourse;
    QString Disfluency;
    QString MWUPOS;
};

}

#endif // DICTIONARYENTRY_H
