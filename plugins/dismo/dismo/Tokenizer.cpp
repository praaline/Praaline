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
#include <QList>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QRegExp>
#include <QPointer>
#include <QDebug>

#include "annotation/Interval.h"
#include "annotation/IntervalTier.h"
#include "Token.h"
#include "TokenUnit.h"
#include "Tokenizer.h"
using namespace DisMoAnnotator;

Tokenizer::Tokenizer(QString &language, QList<TokenUnit *> &tokenUnits) :
    m_language(language), m_tokenUnits(tokenUnits)
{
    // Read tokenize together list
    // DIRECTORY:
    QString appPath = QCoreApplication::applicationDirPath();
    QFile file(appPath + "/plugins/dismo/lex/tokenizetogether_" + language + ".txt");
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QString line;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    do {
        line = stream.readLine();
        m_tokenizeTogether[line.section('\t', 0, 0)] = line.section('\t', 1, 1);
    }
    while (!stream.atEnd());
    file.close();

    // Create phonetisations hash
    m_phonetisations.insert("apr\u00E8s-", "a p R E"); // apres-midi
    m_phonetisations.insert("week-", "w i k"); // week-end
    m_phonetisations.insert("l\u00E0-", "l a"); // la-bas etc
    m_phonetisations.insert("peut-", "p 2 t"); // peut-etre
    m_phonetisations.insert("grand-", "g R a~");  // grand-pere
    m_phonetisations.insert("jusqu'", "Z y s k"); // jusqu'a
    m_phonetisations.insert("quelqu'", "k E l k"); // quelqu' un, une
    m_phonetisations.insert("quelques", "k E l k @ z"); // quelques uns, unes
    m_phonetisations.insert("rendez-", "R a~ d e"); // rendez-vous
    m_phonetisations.insert("Saint-", "s e~"); // Saint-Pierre
    m_phonetisations.insert("trente-", "t R a~ t"); // trente-...
    m_phonetisations.insert("quatre-", "k a t R @"); // quatre-vingt
    m_phonetisations.insert("vis-", "v i z"); // vis-a-vis
    m_phonetisations.insert("\u00E0-", "a"); // vis-a-vis
    m_phonetisations.insert("auto-", "o t O"); // auto-ecole
}

// Phones provides a phonetic transcription, which by default is assumed to be unavailable.
void Tokenizer::Tokenize(const IntervalTier *tierInput, const IntervalTier *tierPhones)
{
    QList<Interval *> splitIntervals;
    QString input;

    m_tokenUnits.clear();

    QPointer<IntervalTier> tierCopy = new IntervalTier(tierInput->name(), tierInput->tMin(), tierInput->tMax());
    tierCopy->copyIntervalsFrom(tierInput);
    tierCopy->fillEmptyWith("", "_");
    tierCopy->mergeIdenticalAnnotations("_");

    foreach(Interval *intvInput, tierCopy->intervals()) {
        // Get input
        input = intvInput->text().trimmed();
        if (input.length() == 0)
            continue; // an empty interval

        if (tierInput->name() == "text") {
            input.replace("-", " - ");
            input.replace("_", " _ ");
            input.replace(",", " , ");
            input.replace(".", " . ");
            // ellipsis
            input.replace("...", " ... ");
            input.replace("\u2026", " \u2026 ");
            // guillemets
            input.replace("\u201C", " \u201C ");
            input.replace("\u201D", " \u201D ");
            input.replace("\"", " \" ");
            input.replace("(", " ( ");
            input.replace(")", " ) ");
            input.replace(":", " : ");
            input.replace(";", " ; ");
            input.replace("!", " ! ");
            input.replace("?", " ? ");
            input.replace("#", " # ");
            input.replace("$", " $ ");
            input.replace("\\", " \\ ");
            input.replace("/", " / ");
            // Apostrophes
            input = input.replace("'", "' ");
            input = input.replace("\u2019", "' ");
            // group
            input.replace(". . .", " ... ");
            input.replace(": - )", " :-) ");
            input.replace(": - (", " :-( ");
            input.replace(": - /", " :-/ ");
            input.replace("; - )", " ;-) ");
            for (int i = 0; i < 5; i++) { input.replace("  ", " "); }
            // return dashes
            input.replace(" - ", "-");
        }
        else {
            // false start convention
//             input = input.replace("- ", "/ ");
//             input.replace(QRegExp("-$"), "/ ");
             input = input.replace("~ ", "/ ");
             input.replace(QRegExp("~$"), "/ ");

            // Valibel:
//            input = input.replace("|-", "").replace("-|", "");

            // CID:
            // input = input.replace("+", "").replace("#", "_");

            // characters to eliminate
            // TODO ADD CHARACTERS AND MAKE THIS CUSTOMISABLE
            input = input.replace("\n", " ").replace("\t", " ");
            input = input.replace(",", "").replace("\u201C", "").replace("\u201D", "").replace("\"", "");
            // characters that separate token units
            // 30/3/2013: _ can separate units with some aligners (en_ce_moment) but it's a pause when by itself
            if (input != "_")
                input = input.replace("_", " ");

            // noises and paraverbal (make sure it's after the _ replace block!)
            QRegExp rx("\\((.*)\\)");
            rx.setMinimal(true);
            int s = -1;
            while ((s = rx.indexIn(input, s+1)) >= 0) {
                QString capture = rx.cap(0);
                input.replace(s, capture.length(), capture.replace(" ", "_"));
                s += rx.cap(1).length();
            }

            // characters that separate token units and we need to keep
            input = input.replace(".", ". ");
            // Apostrophes
            input = input.replace("'", "' ");
            input = input.replace("\u2019", "' ");
            // English concatenated forms
            if (m_language == "en") {
                input.replace(QRegExp("' d$"), " 'd");      // I'd
                input.replace(QRegExp("' m$"), " 'm");      // I'm
                input.replace(QRegExp("' s$"), " 's");      // he's
                input.replace(QRegExp("' ll$"), " 'll");    // he'll
                input.replace(QRegExp("' re$"), " 're");    // we're
                input.replace(QRegExp("' ve$"), " 've");    // we've
                input.replace(QRegExp("n' t$"), " n't");    // couldn't => could n't
                // forms like can't should be in the tokenize-together list
                // posessive apostrophe:
                input.replace(QRegExp("s' $"), "s ' ");    // hopefully not a greengrocers' one
                input.replace(QRegExp("z' $"), "z ' ");    // Grosz' jokes
            }
        }
        // Dashes
        if (input.startsWith("-"))
            input.remove(0, 1); // if a token begins with '-', the next rule would create a lonely dash token
        input = input.replace("-", "- ");

        // break everything at spaces
        QStringList split = input.split(" ", QString::SkipEmptyParts);

        // If this interval has to be separated, use phonetic transcription (if available) or heuristics
        // to find token boundaries (tMin, tMax).
        splitIntervals.append(separateToken(intvInput, split, tierPhones));
    }

    // Tokenize together
    QString join;
    int i, j, k, max;
    i = 0;
    while (i < splitIntervals.count()) {
        if (splitIntervals[i]->isPauseSilent()) {
            TokenUnit *tu = new TokenUnit(splitIntervals[i]);
            m_tokenUnits << tu;
            // m_tokens << new Token(tu);
            i++;
        }
        else {
            max = i + 3;
            if (max >= splitIntervals.count()) max = splitIntervals.count() - 1;

            for (j = max; j >= i + 1; j--) {
                join = splitIntervals[i]->text();
                for (k = i + 1; k <= j; k++)
                    join.append("#").append(splitIntervals[k]->text());
                if (m_tokenizeTogether.contains(join)) {
                    m_tokenUnits << new TokenUnit(splitIntervals[i]->tMin(),
                                                  splitIntervals[j]->tMax(),
                                                  m_tokenizeTogether[join]);
                    break;
                }
            }
            if (i == j) {
                TokenUnit *tu = new TokenUnit(splitIntervals[i]);
                if (tu->text().length() > 1 && tu->text().endsWith("-")) {
                    QString corr = tu->text();
                    corr.chop(1);
                    tu->setText(corr);
                }
                m_tokenUnits << new Token(tu);
                i++;
            }
            else {
                i = j + 1;
            }
        }
    }
    qDeleteAll(splitIntervals);
}

int matchPhonemes(int start, QList<Interval *> phones, QList<QString> phonetisation) {
    int i = start; int j = 0;
    while (i < phones.count() && j < phonetisation.count()) {
        if (phones.at(i)->text() != phonetisation.at(j))
            break;
        i++; j++;
    }
    return i-1;
}

QList<Interval *> Tokenizer::separateToken(Interval *intvInput, QStringList &split,
                                           const IntervalTier *tierPhones)
{
    Interval *intvPrevious = 0;
    QList<Interval *> splitIntervals;
    QList<Interval *> phones;

    if (split.count() == 1) {
        splitIntervals << new Interval(intvInput->tMin(), intvInput->tMax(), split[0]);
        return splitIntervals;
    }

    if (tierPhones) phones = tierPhones->getIntervalsContainedIn(intvInput);
    if ((tierPhones == 0) || (phones.count() == 0) || (split.count() >= phones.count())) {
        // Use approximation algorithm when:
        // - no phonetic transcription available, or
        // - there are more tokens than phones to align with.
        RealTime d = intvInput->duration();
        int sum = 0;
        foreach (QString s, split) {
            sum += s.length();
        }
        if (sum > 0)
            d = d / sum;
        int part = 0;
        RealTime tMin = intvInput->tMin();
        foreach(QString s, split) {
            if (s.isEmpty()) continue;
            RealTime tMax = tMin + d * s.length();
            if (part == (split.count() - 1)) tMax = intvInput->tMax();
            Interval *intv = new Interval(tMin, tMax, s);
            part++;
            splitIntervals << intv;
            intvPrevious = intv;
            tMin = tMax;
        }
    }
    else {
        QPointer<IntervalTier> temp = new IntervalTier("temp", intvInput->tMin(), intvInput->tMax());
        QList<int> proportions;
        foreach(QString s, split) {
            if (s.isEmpty()) continue;
            proportions << s.length();
        }
        temp->splitToProportions(0, proportions);
        temp->fixBoundariesBasedOnTier(tierPhones);
        int i = 0;
        foreach (QString s, split) {
            if (s.isEmpty()) continue;
            Interval *tempIntv = temp->interval(i);
            if (tempIntv) {
                splitIntervals << new Interval(tempIntv->tMin(), tempIntv->tMax(), s);
            }
            ++i;
        }
    }
    return splitIntervals;
}


// Previous, bad algorithm
//int part = 0;
//int phoneIter = 0;
//int totalLength = intvInput->text().length();
//RealTime tMin = intvInput->tMin();
//RealTime tMax = phones.at(0)->tMax();
//foreach(QString s, split) {
//    if (s.isEmpty()) continue;
//    if (part == split.count() - 1) {
//        // Last token unit
//        tMax = intvInput->tMax();
//    }
//    else if (part == 0 && s.trimmed().length() == 2 && s.trimmed().endsWith("'")) {
//        // First token unit, single-letter clitic (s', t', n' etc).
//        tMax = phones.at(0)->tMax();
//        phoneIter++;
//    } else if (m_phonetisations.contains(s)) {
//        // Try to match phonemes
//        QList<QString> phonetisation = m_phonetisations.value(s).split(" ", QString::SkipEmptyParts);
//        int match = matchPhonemes(phoneIter, phones, phonetisation);
//        if (match > phoneIter) {
//            tMax = phones.at(match)->tMax();
//            phoneIter = match;
//            if (phoneIter >= phones.count()) phoneIter = phones.count() - 1;
//        } else {
//            tMax = phones.at(phoneIter)->tMax();
//            phoneIter += (int)((double)s.length() / (double)totalLength);
//            if (phoneIter >= phones.count()) phoneIter = phones.count() - 1;
//        }
//    }
//    else {
//        // As a last resort, align proportionally to the number of characters
//        tMax = phones.at(phoneIter)->tMax();
//        phoneIter += (int)((double)s.length() / (double)totalLength);
//        if (phoneIter >= phones.count()) phoneIter = phones.count() - 1;
//    }
//    // This is the last chance to avoid a zero-duration interval
//    if (tMin == tMax) {
//        if (part == split.count() - 1) {
//            // last interval => cannot move tMax, must move previous
//            RealTime diff = intvPrevious->duration() * 0.10;
//            intvPrevious = new Interval(intvPrevious->tMin(), intvPrevious->tMax() - diff,
//                                        intvPrevious->text());
//            splitIntervals.removeLast();
//            splitIntervals << intvPrevious;
//            tMin = intvPrevious->tMax();
//        }
//        else {
//            tMax = tMin + RealTime::fromNanoseconds(10);
//        }
//    }

//    // Add interval to the list of tokenised intervals
//    Interval *intv = new Interval(tMin, tMax, s);
//    part++;
//    splitIntervals << intv;
//    intvPrevious = intv;
//    tMin = intv->tMax();
//}
