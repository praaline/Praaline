#ifndef CRFANNOTATOR_H
#define CRFANNOTATOR_H

/*
    Praaline
    Copyright (c) 2012 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include <QObject>
#include <QString>
#include <QList>
#include <QTextStream>

class CRFAnnotator
{
public:
    CRFAnnotator();
    virtual ~CRFAnnotator();
    virtual void annotate(const QString &filenameModel);
    virtual void writeTraining(QTextStream &out);

protected:
    int m_numberOfUnits;

    virtual bool isEndOfSequence(int i) = 0;
    virtual bool skipUnit(int i) = 0;
    virtual void writeUnitToCRF(QTextStream &out, int i, bool isTraining) = 0;
    virtual void decodeUnitFromCRF(const QString &line, int i)  = 0;

    void annotateFromCRF(const QString &filenameModel);
    void writeCRFFile(QTextStream &out, bool isTraining);
    void readCRFDecoding(QTextStream &in);
};

#endif // CRFANNOTATOR_H
