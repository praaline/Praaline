#include <QDebug>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>
#include "CoNLLUReader.h"
#include "UDSentence.h"

CoNLLUReader::CoNLLUReader()
{
}

// static
bool CoNLLUReader::readCoNLLU(const QString &filename, QList<UDSentence> &sentences)
{
    // Read CoNLLU file
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    // File opened ok
    sentences.clear();
    QString line;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    int lineNumber = 1;
    int rangeFrom = 0, rangeTo = 0;
    // Current sentence
    UDSentence sent;
    QString currentSentenceID;
    int sentenceFrom = 0, sentenceTo = 0;

    do {
        line = stream.readLine().trimmed();
        if (line.startsWith("# sentid:")) {
            currentSentenceID = line.section(":", 1, 1).trimmed();
            sentenceFrom = lineNumber;
            rangeFrom = rangeTo = 0;
            continue;
        }
        else if (line.isEmpty()) {
            sentenceTo = lineNumber - 1;
            //intervals_sentences << new Interval(RealTime(sentenceFrom, 0), RealTime(sentenceTo, 0), currentSentenceID);
            rangeFrom = rangeTo = 0;
            continue;
        }
        else if (line.startsWith("#")) {
            // skip comments, reset
            rangeFrom = rangeTo = 0;
            continue;
        }
        // otherwise this line is a word / minimal word
        QStringList fields = line.split("\t");
        if (fields.count() < 10) {
            qDebug() << "Less than 10 fields in line " << lineNumber;
            rangeFrom = rangeTo = 0;
            continue;
        }
        QString id = fields.at(0);
        if (rangeFrom != 0) {
            // we are inside a range, check if finished
            if (id.toInt() == rangeTo) {
                rangeFrom = rangeTo = 0; // finished
            }
            // in any case skip this token
            continue;
        }
        // begin a new range?
        if (id.contains("-")) {
            // begin range
            rangeFrom = id.section("-", 0, 0).toInt();
            rangeTo = id.section("-", 1, 1).toInt();
        }
        UDToken token;
        token.setID(fields.at(0).toInt());
        token.setForm(fields.at(1));
        token.setLemma(fields.at(2));
        token.setUPOSTag(fields.at(3));
        token.setXPOSTag(fields.at(4));
        token.setFeatsFromString(fields.at(5));
        token.setHead(fields.at(6).toInt());
        token.setDepRel(fields.at(7));
        token.setDepsFromString(fields.at(8));
        token.setMiscFromString(fields.at(9));

        // Interval *intv = new Interval(RealTime(lineNumber - 1, 0), RealTime(lineNumber, 0), fields.at(1));

        if (lineNumber % 10000 == 0) qDebug() << lineNumber << "\t" << fields.at(1);
        ++lineNumber;
    }
    while (!stream.atEnd());
    file.close();
//    IntervalTier *tier_words = new IntervalTier("ud_words");
//    tier_words->replaceAllIntervals(intervals_words);
//    IntervalTier *tier_sentences = new IntervalTier("ud_sentences");
//    tier_sentences->replaceAllIntervals(intervals_sentences);
//    group->addTierReplacing(tier_words);
//    group->addTierReplacing(tier_sentences);
    return true;
}

