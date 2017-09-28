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

#include <QList>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include "interfaces/praat/PraatTextGrid.h"
#include "annotation/AnnotationTierGroup.h"
#include "TokenList.h"
#include "PreProcessor.h"
#include "POSTagger.h"
#include "DisfluencyDetector.h"
#include "DiscourseTagger.h"
#include "DisMoTrainer.h"
using namespace DisMoAnnotator;

DismoTrainer::DismoTrainer(const QString modelsPath) {
    m_modelsPath = modelsPath;
    if (modelsPath.isEmpty()) {
        m_modelsPath = QCoreApplication::applicationDirPath();
    }
}

DismoTrainer::~DismoTrainer() {
    qDeleteAll(m_tokenLists);
}

// load training corpus
void DismoTrainer::loadTableFiles(const QStringList fileNames)
{
    QStringList table;
    // A new training corpus
    qDeleteAll(m_tokenLists);
    m_tokenLists.clear();
    // Read and decode files
    foreach (QString fileName, fileNames) {
        QFile file(fileName);
        if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            break;   // continue with next file
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        do {
            table << stream.readLine();
        } while (! stream.atEnd());
        file.close();
        TokenList *TL = new TokenList();
        TL->readFromTable(table);
        m_tokenLists << TL;
    }
}

void DismoTrainer::loadTextgrids(const QStringList fileNames)
{
    // A new training corpus
    qDeleteAll(m_tokenLists);
    m_tokenLists.clear();
    // Read and decode files
    foreach (QString fileName, fileNames) {
        AnnotationTierGroup txg;
        PraatTextGrid::load(fileName, &txg);
        if (txg.tiersCount() == 0)
            break;   // continue with next file
        TokenList *TL = new TokenList();
        TL->readFromTextGrid(txg);
        m_tokenLists << TL;
    }
}

// private
void DismoTrainer::outputTrainingFile(const QString &modelname, const int indexTL, QTextStream &out)
{
    // Get token list
    TokenList *TL = m_tokenLists.at(indexTL);
    if (TL == 0) return;
    // The input is already tokenized
    Preprocessor PRE("FR", *TL);
    PRE.processTokenized();
    // Create the model training files
    if (modelname == "posmin") {
        POSTagger POS(*TL);
        POS.writeTrainingForTokenUnits(out);
    }
    if (modelname == "disfluency") {
        DisfluencyDetector DIS(*TL);
        DIS.writeTrainingForTokenUnits(out);
    }
    if (modelname == "discourse") {
        DiscourseTagger DISC(*TL);
        DISC.writeTrainingForTokens(out);
    }
    if (modelname == "posfin") {
        POSTagger POS(*TL);
        POS.writeTrainingForTokens(out);
    }
    if (modelname == "mwulist") {
        foreach (Token *tok, *TL) {
            if (!tok->isSimple()) {
                QString tokMWUparts = "";
                foreach(TokenUnit *u, tok->getTokenUnits()) {
                    tokMWUparts.append(u->text()).append("$");
                    tokMWUparts.replace("-$", "$");
                }
                if (tokMWUparts.endsWith("$")) tokMWUparts.chop(1);
                out << tokMWUparts << "\t";
                out << tok->text() << "\t";
                out << tok->getTagPOS() << "\t";
                out << tok->getTagPOSext() << "\t";
                out << tok->getTagDiscourse() << "\t";
                QString tokMWUpartsPOS = "";
                foreach(TokenUnit *u, tok->getTokenUnits()) {
                    tokMWUpartsPOS.append(u->getTagPOS()).append("$");
                }
                if (tokMWUpartsPOS.endsWith("$")) tokMWUpartsPOS.chop(1);
                out << tokMWUpartsPOS << "\n";
            }
        }
    }
    if (modelname == "corrdict") {
        QSet<QString> corrections;
        foreach (Token *tok, *TL) {
            foreach (TokenUnit *tu, tok->getTokenUnits()) {
                QString correctTag = tu->getTagPOS().replace(":aux", "").replace(":pred", "");
                bool isOK = false;
                foreach (DictionaryEntry dic, tu->getAllPossiblePOStags()) {
                    if (dic.POS == correctTag) {
                        isOK = true;
                    }
                }
                if (isOK == false) {
                    QString correction = tu->text();
                    correction.append("\t").append(correctTag).append("\t").append(tu->getTagPOSext());
                    corrections << correction;
                }
            }
        }
        foreach(QString correction, corrections) {
            out << correction << "\n";
        }
    }
}

// private
void DismoTrainer::prepareTrainingFiles(const QString &filenameBase, const QString &modelname)
{
    QString filename = filenameBase + modelname;
    if (modelname == "mwulist")
        filename.append(".txt");
    else
        filename.append("train.txt");
    QFile fileTrain(filename);
    if (! fileTrain.open(QFile::WriteOnly | QFile::Text)) {
        return;
    }
    QTextStream outTrain(&fileTrain);
    outTrain.setCodec("UTF-8");
    outTrain.setGenerateByteOrderMark(false);
    for (int i = 0; i < m_tokenLists.count(); i++) {
        outputTrainingFile(modelname, i, outTrain);
    }
    fileTrain.close();
}

// public
void DismoTrainer::train(const QString &filenameBase, const DismoTrainerModel model)
{
    if (model == DismoTrainer::POSMin || model == DismoTrainer::All) {
        prepareTrainingFiles(filenameBase, "posmin");
    }
    if (model == DismoTrainer::Disfluency || model == DismoTrainer::All) {
        prepareTrainingFiles(filenameBase, "disfluency");
    }
    if (model == DismoTrainer::Discourse || model == DismoTrainer::All) {
        prepareTrainingFiles(filenameBase, "discourse");
    }
    if (model == DismoTrainer::POSFin || model == DismoTrainer::All) {
        prepareTrainingFiles(filenameBase, "posfin");
    }
    if (model == DismoTrainer::MWUList || model == DismoTrainer::All) {
        prepareTrainingFiles(filenameBase, "mwulist");
    }
    if (model == DismoTrainer::CorrDict|| model == DismoTrainer::All) {
        prepareTrainingFiles(filenameBase, "corrdict");
    }
}

