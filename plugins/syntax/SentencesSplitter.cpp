#include <QDebug>
#include <QString>
#include <QPointer>
#include <QList>
#include <QMap>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/SequenceTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "SentencesSplitter.h"

struct SentencesSplitterData {
    // Sentences {AnnotationID, SpeakerID, sentences}
    QMap<QString, QMap<QString, QStringList> > sentences;
};

SentencesSplitter::SentencesSplitter() :
    d(new SentencesSplitterData())
{
}

SentencesSplitter::~SentencesSplitter()
{
    delete d;
}

QString SentencesSplitter::exportSentences(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return ret;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;

            IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");
            if (!tier_tok_mwu) continue;
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;

            ret.append("\n\n# ").append(annotationID).append("\t").append(speakerID).append("\n");
            foreach (Interval *tok_mwu, tier_tok_mwu->intervals()) {
                if (tok_mwu->isPauseSilent()) continue;
                if (tok_mwu->text() == "euh") continue;
                if (tok_mwu->attribute("sentence_break") == "*") {
                    ret.append(tok_mwu->text()).append("\n");
                } else {
                    ret.append(tok_mwu->text()).append(" ");
                }
            }
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}

QString SentencesSplitter::readBreaksFile(const QString &filename)
{
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        return QString("Error reading file %1").arg(filename);
    QString line;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    d->sentences.clear();
    QString annotationID, speakerID;
    do {
        line = stream.readLine().trimmed();
        if (line.startsWith("#") && line.contains("\t")) {
            annotationID = line.section("\t", 0, 0).remove("#").trimmed();
            speakerID = line.section("\t", 1, 1).trimmed();
        }
        else if (!line.isEmpty()) {
            if (!d->sentences.contains(annotationID))
                d->sentences.insert(annotationID, QMap<QString, QStringList>());
            if (!d->sentences[annotationID].contains(speakerID))
                d->sentences[annotationID].insert(speakerID, QStringList());
            d->sentences[annotationID][speakerID].append(line.trimmed());
        }
    } while (!stream.atEnd());
    file.close();
    return QString("Read sentences file");
}

QString SentencesSplitter::importBreaks(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return ret;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        if (!d->sentences.contains(annotationID)) {
            ret.append(QString("%1\t\t\tskipped (annotation ID not present)\n").arg(annotationID));
            continue;
        }
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            if (!d->sentences[annotationID].contains(speakerID)) {
                ret.append(QString("%1\t%2\t\tskipped (speaker ID not present)\n").arg(annotationID).arg(speakerID));
                continue;
            }

            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");
            if (!tier_tok_mwu) continue;

            QStringList sentences = d->sentences[annotationID][speakerID];
            int i_sent(0);
            QString accumulator;
            foreach (Interval *tok_mwu, tier_tok_mwu->intervals()) {
                if (tok_mwu->isPauseSilent()) continue;
                if (tok_mwu->text() == "euh") continue;

                if (tok_mwu->attribute("sentence_break").toString() == "*")
                    tok_mwu->setAttribute("sentence_break", "");
                accumulator.append(tok_mwu->text()).append(" ");
                if (accumulator.trimmed().length() >= sentences.at(i_sent).trimmed().length()) {
                    if (    (accumulator.trimmed().right(1) != sentences.at(i_sent).trimmed().right(1)) ||
                            (accumulator.trimmed().left(1)  != sentences.at(i_sent).trimmed().left(1))) {
                        qDebug() << "Error detected" << annotationID << speakerID <<
                                    accumulator << sentences.at(i_sent);
                    }

                    tok_mwu->setAttribute("sentence_break", "*");
                    accumulator.clear();
                    i_sent++;
                }
                if (i_sent >= sentences.count()) break;
            }
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_mwu);
            ret.append(QString("%1\t%2\t%3\tsentences").arg(annotationID).arg(speakerID).arg(i_sent)).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

QString SentencesSplitter::createSentenceTier(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return ret;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");
            if (!tier_tok_mwu) continue;

            QList<Interval *> sentence_intervals;
            bool insideSentence(false);
            RealTime start;
            int i_sent(0);
            foreach (Interval *tok_mwu, tier_tok_mwu->intervals()) {
                if (tok_mwu->isPauseSilent()) continue;
                if (!insideSentence) {
                    start = tok_mwu->tMin();
                    insideSentence = true;
                }
                if (tok_mwu->attribute("sentence_break") == "*") {
                    QString text = tier_tok_mwu->getIntervalsTextContainedIn(start, tok_mwu->tMax());
                    // text = text.replace("_", "").replace("  ", " ").replace("  ", " "); // remove silent pause mark
                    sentence_intervals << new Interval(start, tok_mwu->tMax(), text);
                    i_sent++;
                    insideSentence = false;
                }
            }
            IntervalTier *tier_sentence = new IntervalTier("sentence", sentence_intervals);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_sentence);
            ret.append(QString("%1\t%2\t%3\tsentences").arg(annotationID).arg(speakerID).arg(i_sent)).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}
