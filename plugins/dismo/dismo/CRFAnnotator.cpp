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

#include <QDebug>
#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>
#include <QCoreApplication>
#include "TokenUnit.h"
#include "Token.h"
#include "TokenList.h"
#include "CRFAnnotator.h"
#include "DisMoConfiguration.h"
using namespace DisMoAnnotator;

// Public
void CRFAnnotator::tagTokenUnits(const QString &filenameModel)
{
    tagFromCRF(filenameModel, true);
}

void CRFAnnotator::tagTokens(const QString &filenameModel)
{
    tagFromCRF(filenameModel, false);
}

void CRFAnnotator::writeTrainingForTokenUnits(QTextStream &out)
{
    writeCRFFile(out, true, true);
}

void CRFAnnotator::writeTrainingForTokens(QTextStream &out)
{
    writeCRFFile(out, true, false);
}

// Private
void CRFAnnotator::tagFromCRF(const QString &filenameModel, bool isTokenUnitLevel)
{
    // Get a temporary file and write out the features
    QString filenameIn, filenameOut;
    QTemporaryFile fileIn, fileOut;

    if (fileIn.open()) {
        filenameIn = fileIn.fileName();
        qDebug() << "IN: " << filenameIn;
        QTextStream streamIn(&fileIn);
        streamIn.setCodec("UTF-8");
        streamIn.setGenerateByteOrderMark(true);
        writeCRFFile(streamIn, false, isTokenUnitLevel);
        fileIn.close();
    }

    // Touch the output file, and get its filename
    if (fileOut.open()) {
        filenameOut = fileOut.fileName();
        qDebug() << "OUT: " << filenameOut;
        fileOut.close();
    }

    // Pass to CRF decoder
    QProcess *decoder = new QProcess();
    // DIRECTORY:
    decoder->start(DisMoConfiguration::filePathCRFTest(), QStringList() <<
                   "-m" << filenameModel << "-v" << "2" << "-o" << filenameOut << filenameIn);
    if (!(decoder->waitForStarted(-1))) return;
    if (!(decoder->waitForFinished(-1))) return;
    // Read responses into token list
    if (fileOut.open()) {
        QTextStream streamOut(&fileOut);
        streamOut.setCodec("UTF-8");
        readCRFDecoding(streamOut, isTokenUnitLevel);
        fileOut.close();
    }
    decoder->deleteLater();
}

void CRFAnnotator::writeCRFFile(QTextStream &out, bool isTraining, bool isTokenUnitLevel)
{
    bool endSequence = true;
    int sequenceLength = 0;
    int maxSequenceLength = 50;
    for (int iterToken = 0; iterToken < m_tokens.count(); iterToken++) {
        sequenceLength++;
        Token *token = m_tokens[iterToken];
        if (isEOSToken(iterToken) || (token->text() == "_" && sequenceLength > maxSequenceLength)) {
            if (!endSequence) {
                out << "\n";
                endSequence = true;
                sequenceLength = 0;
            }
            continue;
        }
        else if (skipToken(iterToken)) {
            continue;
        }
        // Otherwise, print token units
        if (isTokenUnitLevel) {
            for(int iterTokenUnit = 0; iterTokenUnit < token->getTokenUnits().count(); iterTokenUnit++) {
                if (isEOSTokenUnit(iterToken, iterTokenUnit) || (token->text() == "_" && sequenceLength > maxSequenceLength)) {
                    if (!endSequence) {
                        out << "\n";
                        endSequence = true;
                        sequenceLength = 0;
                    }
                    continue;
                }
                else if (skipTokenUnit(iterToken, iterTokenUnit))
                    continue;
                endSequence = false;
                writeTokenUnitToCRF(out, iterToken, iterTokenUnit, isTraining);
            }
        } else {
            endSequence = false;
            writeTokenToCRF(out, iterToken, isTraining);
        }
    }
}

void CRFAnnotator::readCRFDecoding(QTextStream &in, bool isTokenUnitLevel)
{
    QString line;
    int sequenceLength = 0;
    int maxSequenceLength = 50;
    for (int iterToken = 0; iterToken < m_tokens.count(); iterToken++) {
        sequenceLength++;
        Token *token = m_tokens[iterToken];
        if (isEOSToken(iterToken) || (token->text() == "_" && sequenceLength > maxSequenceLength)) {
            sequenceLength = 0;
            continue;
        }
        else if (skipToken(iterToken)) {
            continue;
        }
        // Otherwise, read tokens
        if (isTokenUnitLevel) {
            for(int iterTokenUnit = 0; iterTokenUnit < token->getTokenUnits().count(); iterTokenUnit++) {
                if (isEOSTokenUnit(iterToken, iterTokenUnit) || (token->text() == "_" && sequenceLength > maxSequenceLength)) {
                    sequenceLength = 0;
                    continue;
                }
                else if (skipTokenUnit(iterToken, iterTokenUnit))
                    continue;
                // read
                line = in.readLine();
                while ((line.startsWith("# 1.") || line.startsWith("# 0.") || line.isEmpty()) && !in.atEnd())
                    line = in.readLine();
                decodeTokenUnitFromCRF(line, iterToken, iterTokenUnit);
            }
        } else {
            line = in.readLine();
            while ((line.startsWith("# 1.") || line.startsWith("# 0.") || line.isEmpty()) && !in.atEnd())
                line = in.readLine();
            decodeTokenFromCRF(line, iterToken);
        }
    }
}

QString CRFAnnotator::sanitizeString(QString input)
{
    QString ret = input;
    ret = ret.replace(" ", "_").replace("\t", "_").replace("\n", "_");
    return ret;
}
