#include <QDebug>
#include <QString>
#include <QMap>
#include <QList>
#include <QFile>
#include <QTextStream>
#include "pncore/corpus/corpus.h"
#include "pncore/annotation/annotationtiergroup.h"
#include "pncore/annotation/intervaltier.h"
#include "valibelprocessor.h"

ValibelProcessor::ValibelProcessor()
{

}

ValibelProcessor::~ValibelProcessor()
{

}

void ValibelProcessor::importValibelFile(Corpus *corpus, const QString &filename)
{
    QString line;
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    QString header = stream.readLine();
    if (header.startsWith("<") && header.endsWith(">")) {
        header = header.remove(0, 1);
        header.chop(1);
    } else return;
    CorpusCommunication *com = new CorpusCommunication(header);
    CorpusAnnotation *annot = new CorpusAnnotation(header);
    com->addAnnotation(annot);
    QMap<QString, QList<Interval *> > segmentIntervals;
    QString mainSpeakerID, embeddedSpeakerID;
    QString mainSegment, embeddedSegment;

    int intervalNo = 0;
    RealTime tMin;
    do {
        line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (line.startsWith("(")) {
            // shouldn't happen but it does
            mainSegment.append(line).append(" ");
        }
        else {
            // inside transcription
            mainSpeakerID = line.section(" ", 0, 0);
            QString content = line.section(" ", 1, -1);
            QStringList tokens = content.split(" ");
            int i = 0;
            while (i < tokens.count()) {
                QString tok = tokens.at(i);
                if (tok.startsWith("<") && tok.endsWith(">")) {
                    if (!embeddedSpeakerID.isEmpty()) {
                        RealTime tMax = RealTime::fromNanoseconds(intervalNo * 1000000 + i * 100);
                        Interval *intvEmbeddedSegment = new Interval(tMin, tMax, embeddedSegment);
                        intvEmbeddedSegment->setAttribute("embedded", true);
                        segmentIntervals[embeddedSpeakerID].append(intvEmbeddedSegment);
                        embeddedSpeakerID.clear();
                        embeddedSegment.clear();
                    }
                    // start new embedded
                    embeddedSpeakerID = QString(tok).remove(0, 1);
                    embeddedSpeakerID.chop(1);
                    tMin = RealTime::fromNanoseconds(intervalNo * 1000000 + (i + 1) * 100);
                }
                else if (!embeddedSpeakerID.isEmpty() && tok == "-|") {
                    RealTime tMax = RealTime::fromNanoseconds(intervalNo * 1000000 + i * 100);
                    Interval *intvEmbeddedSegment = new Interval(tMin, tMax, embeddedSegment);
                    intvEmbeddedSegment->setAttribute("embedded", true);
                    segmentIntervals[embeddedSpeakerID].append(intvEmbeddedSegment);
                    embeddedSpeakerID.clear();
                    embeddedSegment.clear();
                    mainSegment.append("-| ");
                }
                else if (!embeddedSpeakerID.isEmpty()) {
                    embeddedSegment.append(tok).append(" ");
                }
                else {
                    mainSegment.append(tok).append(" ");
                }
                i++;
            }
            Interval *intvMainSegment = new Interval(RealTime::fromNanoseconds(intervalNo * 1000000),
                                                     RealTime::fromNanoseconds((intervalNo + 1) * 1000000),
                                                     mainSegment);
            intvMainSegment->setAttribute("embedded", false);
            segmentIntervals[mainSpeakerID].append(intvMainSegment);
            mainSegment.clear();
            intervalNo++;
        }
    }
    while (!stream.atEnd());
    file.close();
    corpus->addCommunication(com);
    foreach (QString speakerID, segmentIntervals.keys()) {
        RealTime tend = segmentIntervals[speakerID].last()->tMax();
        IntervalTier *tier = new IntervalTier("segment", RealTime(), tend, segmentIntervals[speakerID]);
        tier->fillEmptyAnnotationsWith("_");
        corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier);
    }
}

// private
void ValibelProcessor::mergeInsideParentheses(QString &input) {
    // noises and paraverbal (make sure it's after the _ replace block!)
    QRegExp rx("\\((.*)\\)");
    rx.setMinimal(true);
    int s = -1;
    while ((s = rx.indexIn(input, s+1)) >= 0) {
        QString capture = rx.cap(0);
        input.replace(s, capture.length(), capture.replace(" ", "_"));
        s += rx.cap(1).length();
    }
}

// private
QString ValibelProcessor::formatSegment(QString input)
{
    QString ret = input;
    if (input.startsWith("'")) { ret = ret.remove(0, 1).prepend("\""); }
    if (input.endsWith("'")) { ret.chop(1); ret = ret.append("\""); }
    ret = ret.replace(" '", " \"").replace("' ", "\" ").replace("'.", "\".").replace("', ", "\", ").replace("\" ,", "\",").replace("\" .", "\".");
    ret = ret.replace("/", "/ ").replace("/ ,", "/,").replace("/ .", "/.").replace("/ )", "/)");
    ret = ret.replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ");
    ret = ret.replace(" .", ".").replace(" ,", ",").replace(" ?", "?").replace(" !", "!");
    ret = ret.replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ");
    return ret.trimmed();
}

QList<QString> ValibelProcessor::splitToken(QString input)
{
    input = input.replace("'", "' ");
    input = input.replace("\u2019", "' ");
    // Dashes
    if (input.startsWith("-"))
        input.remove(0, 1); // if a token begins with '-', the next rule would create a lonely dash token
    input = input.replace("-", "- ");
    //
    input = input.replace("aujourd' hui", "aujourd'hui");
    if (input.startsWith("t-")) {
        input = input.replace("t- il", "t-il");
        input = input.replace("t- elle", "t-elle");
        input = input.replace("t- on", "t-on");
    }

    // break everything at spaces
    QList<QString> ret;
    QList<QString> result = input.split(" ", QString::SkipEmptyParts);
    int i = 0;
    while (i < result.count()) {
        if (result.at(i).startsWith("parce", Qt::CaseInsensitive)) {
            if (i + 1 < result.count()) {
                ret << QString("%1 %2").arg(result.at(i)).arg(result.at(i+1));
                i++;
            } else ret << result.at(i);
        } else {
            ret << result.at(i);
        }
        i++;
    }
    return ret;
}

void ValibelProcessor::tokenise(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "segment");
            foreach (QString speakerID, tiersAll.keys()) {
                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                IntervalTier *tier_segment = tiers->getIntervalTierByName("segment");
                if (!tier_segment) continue;
                IntervalTier *tier_tok_min = new IntervalTier("tok_min", tier_segment->tMin(), tier_segment->tMax());
                tier_tok_min->copyIntervalsFrom(tier_segment, false);

                int itok = 0;
                foreach (Interval *segment, tier_segment->intervals()) {
                    QString ortho = segment->text();
                    mergeInsideParentheses(ortho);
                    ortho = formatSegment(ortho);
                    ortho = ortho.replace("|-", "").replace("-|", "");

                    QList<QString> splitOrtho = splitToken(ortho);
                    int count = splitOrtho.count();
                    QList<int> lengths;
                    for (int i = 0; i < count; ++i) lengths << splitOrtho.at(i).length();
                    if (segment->duration().toDouble() > 1.0) lengths << 2;
                    QList<Interval *> intervals_tok_min = tier_tok_min->splitToProportions(itok, lengths);
                    if (intervals_tok_min.count() == 0) intervals_tok_min << tier_tok_min->interval(itok);
                    for (int i = 0; i < count; ++i) {
                        intervals_tok_min[i]->setText(splitOrtho.at(i));
                    }
                    itok = itok + count;
                    if (segment->duration().toDouble() > 1.0) itok = itok + 1;
                }
                tier_tok_min->fillEmptyAnnotationsWith("_");

                // Punctuation
                foreach (Interval *tok_min, tier_tok_min->intervals()) {
                    QString t = tok_min->text();
                    if (t.endsWith(",")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", ","); }
                    if (t.endsWith(".")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", "."); }
                    if (t.endsWith("?")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", "?"); }
                    if (t.endsWith("!")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", "!"); }
                    if (t.endsWith("-")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", "-"); }
                    if (t.endsWith(":")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", ":"); }
                    if (t.startsWith("\"")) { t = t.remove(0, 1); tok_min->setText(t); tok_min->setAttribute("punctuation_before", "\""); }
                    if (t.endsWith("\"")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", "\""); }
                }

                foreach (Interval *intv, tier_tok_min->intervals()) {
                    if (intv->text() == "/") intv->setText("_");
                }
                tier_tok_min->mergeIdenticalAnnotations("_");

                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
                delete tier_tok_min;
            }
            qDeleteAll(tiersAll);
        }
        qDebug() << com->ID();
    }
}

void ValibelProcessor::tokmin_punctuation(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "tok_min");
            foreach (QString speakerID, tiersAll.keys()) {
                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
                if (!tier_tok_min) continue;
                foreach (Interval *tok_min, tier_tok_min->intervals()) {
                    QString t = tok_min->text();
                    QString s = tok_min->attribute("schwa").toString();
                    QString l = tok_min->attribute("liaison").toString();
                    if (t.endsWith(",")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", ","); }
                    if (t.endsWith(".")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", "."); }
                    if (t.endsWith("?")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", "?"); }
                    if (t.endsWith("!")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", "!"); }
                    if (t.endsWith("-")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", "-"); }
                    if (t.endsWith(":")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", ":"); }
                    if (t.startsWith("\"")) { t = t.remove(0, 1); tok_min->setText(t); tok_min->setAttribute("punctuation_before", "\""); }
                    if (t.endsWith("\"")) { t.chop(1); tok_min->setText(t); tok_min->setAttribute("punctuation_after", "\""); }

                    if (s.endsWith(",") || s.endsWith(".") || s.endsWith("?") || s.endsWith("!") || s.endsWith("-") || s.endsWith(":") || s.endsWith("\""))
                    { s.chop(1); tok_min->setAttribute("schwa", s); }
                    if (s.startsWith("\"")) { s = s.remove(0, 1); tok_min->setAttribute("schwa", s); }

                    if (l.endsWith(",") || l.endsWith(".") || l.endsWith("?") || l.endsWith("!") || l.endsWith("-") || l.endsWith(":") || l.endsWith("\""))
                    { l.chop(1); tok_min->setAttribute("liaison", l); }
                    if (l.startsWith("\"")) { l = l.remove(0, 1); tok_min->setAttribute("liaison", l); }
                }
                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
            }
            qDeleteAll(tiersAll);
        }
        qDebug() << com->ID();
    }
}


void ValibelProcessor::pauses(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "tok_min");
            foreach (QString speakerID, tiersAll.keys()) {
                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
                if (!tier_tok_min) continue;
                foreach (Interval *intv, tier_tok_min->intervals()) {
                    if (intv->text() == "/") intv->setText("_");
                }
                tier_tok_min->mergeIdenticalAnnotations("_");
                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
            }
            qDeleteAll(tiersAll);
        }
        qDebug() << com->ID();
    }
}

