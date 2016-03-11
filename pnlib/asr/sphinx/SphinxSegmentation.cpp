#include <QObject>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/corpus.h"
#include "pncore/annotation/interval.h"
#include "pncore/annotation/intervaltier.h"

#include "SphinxSegmentation.h"

SphinxSegmentation::SphinxSegmentation(QObject *parent) : QObject(parent)
{

}

SphinxSegmentation::~SphinxSegmentation()
{

}

bool SphinxSegmentation::readSegmentationHypothesisFile(const QString &filename, QHash<QString, QList<Interval *> > &utterances)
{
    // Format: u S s T t A a L l sf wa wl wd sf wa sl wd ... nf
    //         0 1 2 3 4 5 6 7 8 9  10 11 12 13
    // u: the utterance ID
    // s: an acoustic score scaling done during acoustic likelihood computation (However, this field is 0 in the s3.X decoder output.)
    // t: the total score for this hypothesis
    // a: the total acoustic score for this hypothesis
    // l: the total language model score for this hypothesis
    // followed by groups of 4 fields
    //   sf: Start frame for the word (its end frame is just before the start frame of the next word)
    //   wa: Acoustic score for the word
    //   wl: LM score for the word
    //   wd: The word string itself.

    QString utteranceID, stotalScore, stotalAcousticScore, stotalLMScore;
    QString sf, wa, wl, wd, nf;
    QStringList fields;
    QList<Interval *> intvlist_words;

    QFile fileSeg(filename);
    if ( !fileSeg.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    QTextStream seg(&fileSeg);
    seg.setCodec("UTF-8");
    while (!seg.atEnd()) {
        QString line = seg.readLine();
        fields = line.split(" ");
        if (fields.count() < 9) continue;
        utteranceID = fields.at(0);
        stotalScore = fields.at(4);
        stotalAcousticScore = fields.at(6);
        stotalLMScore = fields.at(8);
        int numberOfWords = (fields.count() - 9) / 4;
        for (int i = 0; i < numberOfWords; ++i) {
            sf = fields.at(i * 4 + 9);
            wa = fields.at(i * 4 + 10);
            wl = fields.at(i * 4 + 11);
            wd = fields.at(i * 4 + 12);
            nf = fields.at(i * 4 + 13);
            long long start_msec = sf.toLongLong() * 10;
            long long end_msec = nf.toLongLong() * 10;
            Interval *intv = new Interval(RealTime::fromMilliseconds(start_msec), RealTime::fromMilliseconds(end_msec), wd);
            intv->setAttribute("acoustic_score", QVariant(wa.toLongLong()));
            intv->setAttribute("lm_score", QVariant(wl.toLongLong()));
            intvlist_words << intv;
        }
        if (intvlist_words.count() == 0) continue;
        utterances.insert(utteranceID, intvlist_words);
        intvlist_words.clear();
    }
    fileSeg.close();
    return true;
}


