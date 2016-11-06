#include <QString>
#include <QList>
#include <QPointer>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QHash>
#include <QDebug>
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "CorpusImporter.h"

CorpusImporter::CorpusImporter()
{

}

bool CorpusImporter::readPerceo(const QString &filename, QMap<QString, QPointer<AnnotationTierGroup> > &tiers)
{
    QHash<QString, QList<Interval *> > words_collector;

    // Read Perceo corpus file
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    QString line;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    int intervalNo = 1;
    QString currentSpeakerID;

    do {
        line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;
        QString form = line.section("\t", 0, 0);
        QString pos = line.section("\t", 1, 1);
        QString lemma = line.section("\t", 2, 2);
        if (pos == "LOC") {
            currentSpeakerID = form;
            continue;
        }
        if (form == "O.K.") form = "OK";
        if (pos == "TRC" && form.endsWith("-")) {
            form.chop(1);
            form.append("/");
            lemma.chop(1);
            lemma.append("/");
        }
        Interval *intv = new Interval(RealTime(intervalNo - 1, 0), RealTime(intervalNo, 0), form);
        intv->setAttribute("pos_perceo", pos);
        intv->setAttribute("lemma_perceo", lemma);
        words_collector[currentSpeakerID].append(intv);
        ++intervalNo;
    }
    while (!stream.atEnd());
    file.close();
    foreach (QString speakerID, words_collector.keys()) {
        QPointer<AnnotationTierGroup> tiersForSpeaker = new AnnotationTierGroup();
        IntervalTier *tier_words = new IntervalTier("words");
        tier_words->replaceAllIntervals(words_collector[speakerID]);
        tiersForSpeaker->addTierReplacing(tier_words);
        tiers.insert(speakerID, tiersForSpeaker);
    }
    return true;
}
