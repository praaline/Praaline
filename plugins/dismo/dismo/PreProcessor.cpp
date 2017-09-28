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
#include <QStringList>
#include <QList>
#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include "annotation/IntervalTier.h"
#include "TokenList.h"
#include "Tokenizer.h"
#include "DictionaryEntry.h"
#include "DictionaryQuery.h"
#include "DictionaryFST.h"
#include "DictionarySQL.h"
#include "PreProcessor.h"
using namespace DisMoAnnotator;

Preprocessor::Preprocessor(const QString &language, TokenList &tokens)
    : m_language(language), m_useSQLDictionary(true), m_tokens(tokens)
{
    // DIRECTORY:
    QString appPath = QCoreApplication::applicationDirPath();
    m_filenameDictionary = appPath + "/plugins/dismo/lex/dismodic_" + language;
    if (m_useSQLDictionary) m_filenameDictionary.append(".db"); else m_filenameDictionary.append(".fst");
    m_filenameMWUDictionary = appPath + "/plugins/dismo/lex/mwudic_" + language + ".txt";
    loadDictionaries();
}

// Public methods
void Preprocessor::process(const IntervalTier *input, const IntervalTier *tierPhones)
{
    QList<TokenUnit *> tokenUnits;
    Tokenizer T(m_language, tokenUnits);
    T.Tokenize(input, tierPhones);          // splits input to token units (minimal)
    applyLEXMIN(tokenUnits);                // apply LEX-MIN
    applyLEXMWU(tokenUnits);                // apply LEX-MWU
    applyParaverbal();
    applyPunctuation();
    foreach (Token *token, m_tokens) {
        // This will set the unambiguous DISFLUENCY and DISCOURSE tags to the relevant tokens,
        // based on dictionary entries (if any).
        token->updateUnambiguous();
    }
    prosodicPreprocessing();
    findMajorFrontiers();
}

void Preprocessor::processTokenizedToMinimal(const IntervalTier *tier_tok_min)
{
    QList<TokenUnit *> tokenUnits;
    foreach (Interval *intv, tier_tok_min->intervals()) {
        QString t = intv->text().trimmed();
        if (t.isEmpty()) t = "_";
        tokenUnits << new TokenUnit(intv->tMin(), intv->tMax(), t, intv->attribute("punctuation_after").toString());
    }
    applyLEXMIN(tokenUnits);                // apply LEX-MIN
    applyLEXMWU(tokenUnits);                // apply LEX-MWU
    applyParaverbal();
    applyPunctuation();
    foreach (Token *token, m_tokens) {
        // This will set the unambiguous DISFLUENCY and DISCOURSE tags to the relevant tokens,
        // based on dictionary entries (if any).
        token->updateUnambiguous();
    }
    prosodicPreprocessing();
    findMajorFrontiers();
}

void Preprocessor::processTokenized()
{
    QList<TokenUnit *> tokenUnits;
    foreach (Token *token, m_tokens) {
        tokenUnits << token->getTokenUnits();
    }
    applyLEXtoTokenized();      // apply LEX-MIN, but do not change MWU tokenisation
    applyParaverbal();
    applyPunctuation();
    foreach (Token *token, m_tokens) {
        // This will set the unambiguous DISFLUENCY and DISCOURSE tags to the relevant tokens,
        // based on dictionary entries (if any).
        token->updateUnambiguous();
    }
    prosodicPreprocessing();
    findMajorFrontiers();
}

// =====================================================================================================================

// Private methods - preprocessing steps
void Preprocessor::loadDictionaries()
{
    // MWU Dictionary
    QString line;
    QFile fileMWU(m_filenameMWUDictionary);
    if ( !fileMWU.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QTextStream streamMWU(&fileMWU);
    streamMWU.setCodec("UTF-8");
    streamMWU.setGenerateByteOrderMark(true);
    do {
        line = streamMWU.readLine();
        m_dictionaryMWU[line.section('#', 0, 0)] = line.section('#', 1);
    }
    while (!streamMWU.atEnd());
    fileMWU.close();
}

void Preprocessor::applyLEXtoTokenized()
{
    // Apply MWU LEX
    foreach(Token *token, m_tokens) {
        if (token->isSimple()) continue;
        QString join = token->text().replace("'", "'$").replace(" ", "$").replace("-", "$").replace("$$", "$");
        if (m_dictionaryMWU.contains(join)) {
            token->setText(m_dictionaryMWU[join].section("#", 0, 0));
            token->updateTokenUnitsFromMWUToken(m_dictionaryMWU[join].section("#", 1));
        }
    }
    // Apply simple LEX
    QHash<QString, QStringList> myTokenUnits;
    foreach(Token *token, m_tokens) {
        foreach (TokenUnit *tokenUnit, token->getTokenUnits()) {
            QString lookup = tokenUnit->text();
            if (lookup.length() > 1 && lookup.endsWith("-"))
                lookup.chop(1); // ex: existe- # t-il : remove - from existe
            if (lookup.contains("=")) lookup = lookup.replace("=", "");
            myTokenUnits.insert(lookup, QStringList());
        }
    }
    if (m_useSQLDictionary) {
        DictionarySQL dic(m_filenameDictionary);
        dic.lookup(myTokenUnits);
    }
    else {
        DictionaryFST dic(m_filenameDictionary);
        dic.lookup(myTokenUnits);
    }
    foreach(Token *token, m_tokens) {
        foreach (TokenUnit *tokenUnit, token->getTokenUnits()) {
            QString lookup = tokenUnit->text();
            if (lookup.length() > 1 && lookup.endsWith("-"))
                lookup.chop(1); // ex: existe- # t-il : remove - from existe
            if (lookup.contains("=")) lookup = lookup.replace("=", "");
            if (myTokenUnits.contains(lookup) || myTokenUnits.contains(lookup.toLower())) {
                foreach (QString dictionaryEntryText, myTokenUnits[lookup]) {
                    tokenUnit->addPossibleTag(dictionaryEntryText);
                }
                foreach (QString dictionaryEntryText, myTokenUnits[lookup.toLower()]) {
                    tokenUnit->addPossibleTag(dictionaryEntryText);
                }
                tokenUnit->updateUnambiguous();
            }
        }
        if (!token->isSimple())
            token->updateAmbiguityClassFromTag();
        else
            token->updateSimpleTokenFromTokenUnit();
    }
}


// Application of LEX-MIN on TOK-MIN (updateTags : default is true)
void Preprocessor::applyLEXMIN(QList<TokenUnit *> &tokenUnits)
{
    QHash<QString, QStringList> myTokenUnits;
    foreach (TokenUnit *tokenUnit, tokenUnits) {
        QString lookup = tokenUnit->text();
        if (lookup.length() > 1 && lookup.endsWith("-"))
            lookup.chop(1); // ex: existe- # t-il : remove - from existe
        if (lookup.contains("=")) lookup = lookup.replace("=", "");
        myTokenUnits.insert(lookup, QStringList());
    }
    if (m_useSQLDictionary) {
        DictionarySQL dic(m_filenameDictionary);
        dic.lookup(myTokenUnits);
    }
    else {
        DictionaryFST dic(m_filenameDictionary);
        dic.lookup(myTokenUnits);
    }
    foreach (TokenUnit *tokenUnit, tokenUnits) {
        QString lookup = tokenUnit->text();
        if (lookup.length() > 1 && lookup.endsWith("-"))
            lookup.chop(1); // ex: existe- # t-il : remove - from existe
        if (lookup.contains("=")) lookup = lookup.replace("=", "");
        if (myTokenUnits.contains(lookup) || myTokenUnits.contains(lookup.toLower())) {
            foreach (QString dictionaryEntryText, myTokenUnits[lookup]) {
                tokenUnit->addPossibleTag(dictionaryEntryText);
            }
            foreach (QString dictionaryEntryText, myTokenUnits[lookup.toLower()]) {
                tokenUnit->addPossibleTag(dictionaryEntryText);
            }
            tokenUnit->updateUnambiguous();
        }
    }
}

// Application of LEX-MWU and creation of TokenList
void Preprocessor::applyLEXMWU(QList<TokenUnit *> &tokenUnits)
{
    // This function creates the token list. It also applies the multi-word unit lexical resources.
    QString join;
    int i, j, k, max;
    i = 0;
    while (i < tokenUnits.count()) {
        if (tokenUnits[i]->isPauseSilent()) {
            m_tokens << new Token(tokenUnits[i]);
            i++;
        }
        else {
            max = i + 7;
            if (max >= tokenUnits.count()) max = tokenUnits.count() - 1;
            for (j = max; j >= i + 1; j--) {
                join = tokenUnits[i]->text();
                for (k = i + 1; k <= j; k++)
                    join.append("$").append(tokenUnits[k]->text());
                if (m_dictionaryMWU.contains(join)) {
                    Token *tok = new Token(tokenUnits.mid(i, j - i + 1), m_dictionaryMWU[join].section("#", 0, 0));
                    tok->updateTokenUnitsFromMWUToken(m_dictionaryMWU[join].section("#", 1));
                    m_tokens << tok;
                    break;
                }
            }
            if (i == j) {
                Token *tok = new Token(tokenUnits[i]); // simple
                tok->updateSimpleTokenFromTokenUnit();
                m_tokens << tok;
                i++;
            }
            else {
                i = j + 1;
            }
        }
    }
    foreach (Token *token, m_tokens) {
        token->updateSimpleTokenFromTokenUnit();
    }
}

// After having a list of tokens

void Preprocessor::applyParaverbal()
{
    foreach (Token *token, m_tokens) {
        // if (token->text() == "(rires)" || token->text() == "(souffle)") {
        if (token->text().startsWith("(") || token->text().endsWith(")")) {
            token->setTagPOS("0", "", 1.0);
            token->setTagDisfluency("PARA", 1.0);
            token->setTagDiscourse("PARA", 1.0);
            token->updateUnambiguous();
        }
    }
}

void Preprocessor::applyPunctuation()
{
    foreach (Token *token, m_tokens) {
        QString t = token->text().trimmed();
        if (t == "." || t == "," || t == "?" || t == "!" || t == "..." || t == "\u2026" ||
            t == ";" || t == ":" || t == "@" || t == "#" || t == "$" ||
            t == "\\" || t == "/" || t == "(" || t == ")" ||
            t == "\u201C" || t == "\u201D" || t == "\u2018" || t == "\u2019" ||
            t == ":-)" || t == ";-)" || t == ":-(" || t == ":-/")
        {
            token->setTagPOS("PNC", "", 1.0);
            token->setTagDisfluency("", 1.0);
            token->setTagDiscourse("PNC", 1.0);
            token->updateUnambiguous();
        }
    }
}

void Preprocessor::prosodicPreprocessing()
{
    foreach (Token *token, m_tokens) {
        if (token->text() == "_" || token->text() == "#") {
            token->setPause();
        }
        if (token->text().endsWith("/")) {
            token->setTagDisfluency("FST", 1.0); // VALIBEL convention!
        }
    }
}

void Preprocessor::findMajorFrontiers()
{
    foreach (Token *token, m_tokens) {
        if (token->isPauseLong()) {
            token->setTagFrontier("MAJ", 1.0);
        }
    }
}
