#ifndef CRFANNOTATOR_H
#define CRFANNOTATOR_H

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

namespace DisMoAnnotator {

class CRFAnnotator
{
public:
    CRFAnnotator(TokenList &tokens) : m_tokens(tokens) {}
    void tagTokenUnits(const QString &filenameModel);
    void tagTokens(const QString &filenameModel);
    void writeTrainingForTokenUnits(QTextStream &out);
    void writeTrainingForTokens(QTextStream &out);
protected:
    TokenList &m_tokens;
    virtual bool isEOSToken(int i) = 0;
    virtual bool isEOSTokenUnit(int i, int j) = 0;
    virtual bool skipToken(int i) = 0;
    virtual bool skipTokenUnit(int i, int j) = 0;
    virtual void writeTokenToCRF(QTextStream &out, int i, bool isTraining) = 0;
    virtual void writeTokenUnitToCRF(QTextStream &out, int i, int j, bool isTraining)  = 0;
    virtual void decodeTokenFromCRF(const QString &line, int i)  = 0;
    virtual void decodeTokenUnitFromCRF(const QString &line, int i, int j) = 0;
    QString sanitizeString(QString input);
private:
    void tagFromCRF(const QString &filenameModel, bool isTokenUnitLevel);
    void writeCRFFile(QTextStream &out, bool isTraining, bool isTokenUnitLevel);
    void readCRFDecoding(QTextStream &in, bool isTokenUnitLevel);
};

}

#endif // CRFANNOTATOR_H
