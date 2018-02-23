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
#include <QProcess>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>
#include <QCoreApplication>
#include "DictionaryQuery.h"
#include "DictionaryFST.h"
#include "DisMoConfiguration.h"
using namespace DisMoAnnotator;

DictionaryFST::DictionaryFST(const QString &filename)
    : m_filenameFST(filename)
{
}

void DictionaryFST::lookup(QHash<QString, QStringList> &table)
{
    QString filenameIn, filenameOut;
    QTemporaryFile fileIn, fileOut;

    // Nothing to do if the hashtable is empty
    if (table.isEmpty())
        return;

    // Write all input words in a file, each word on its own line
    if (fileIn.open()) {
        filenameIn = fileIn.fileName();
        QTextStream streamIn(&fileIn);
        streamIn.setCodec("UTF-8");
        streamIn.setGenerateByteOrderMark(false);
        foreach (QString search, table.keys()) {
            streamIn << search << "\n";
            streamIn << search.toLower() << "\n";
        }
        fileIn.close();
    }

    // Touch the output file, and get its filename
    if (fileOut.open()) {
        filenameOut = fileOut.fileName();
        fileOut.close();
    }
    // Pass the tokens trough the lookup function of HFST
    QProcess *lookup = new QProcess();
    // DIRECTORY:
    lookup->start(DisMoConfiguration::resourcesPath() + "/lex/hfst-lookup.exe", QStringList() <<
                  "-I" << filenameIn << "-o" << filenameOut << "-i" << m_filenameFST);
    if (!(lookup->waitForStarted(-1))) return;
    if (!(lookup->waitForFinished(-1))) return;
    // Read FST output into tags
    if (fileOut.open()) {
        QTextStream streamOut(&fileOut);
        streamOut.setCodec("UTF-8");
        QString line, word, entry;
        do {
            line = streamOut.readLine();
            if (line == "") continue;
            word = line.section("\t", 0, 0).replace("_", " ");
            entry = line.section("\t", 1, 1).replace("_", " ");
            if (entry == word + "+?")
                table.remove(word);
            else
                table[word].append(entry);
        } while (! streamOut.atEnd());
        fileOut.close();
    }
    lookup->deleteLater();
}
