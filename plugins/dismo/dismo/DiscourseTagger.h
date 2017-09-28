#ifndef DISCOURSETAGGER_H
#define DISCOURSETAGGER_H

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
#include <QTextStream>
#include "TokenList.h"
#include "CRFAnnotator.h"

namespace DisMoAnnotator {

class DiscourseTagger : public CRFAnnotator
{
public:
    DiscourseTagger(TokenList &tokens) : CRFAnnotator(tokens) {}
protected:
    virtual bool isEOSToken(int i);
    virtual bool isEOSTokenUnit(int i, int j);
    virtual bool skipToken(int i);
    virtual bool skipTokenUnit(int i, int j);
    virtual void writeTokenToCRF(QTextStream &out, int i, bool isTraining);
    virtual void writeTokenUnitToCRF(QTextStream &out, int i, int j, bool isTraining);
    virtual void decodeTokenFromCRF(const QString &line, int i);
    virtual void decodeTokenUnitFromCRF(const QString &line, int i, int j);
};

}

#endif // DISCOURSETAGGER_H
