#include <QString>
#include <QList>
#include <QPointer>
#include <QMap>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "PFCPreprocessor.h"

struct PFCPreprocessorData {
    int i;
};

PFCPreprocessor::PFCPreprocessor() : d(new PFCPreprocessorData())
{
}

PFCPreprocessor::~PFCPreprocessor()
{
    delete d;
}

QString PFCPreprocessor::prepareTranscription(QPointer<CorpusCommunication> com)
{
    if (!com) return "No Communication";
    QStringList levels; levels << "transcription";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        AnnotationTierGroup *tiers = com->repository()->annotations()->getTiers(annot->ID(), annot->ID(), levels);
        IntervalTier *transcription = tiers->getIntervalTierByName("transcription");
        if (!transcription) continue;
        foreach (Interval *intv, transcription->intervals()) {
            QString ortho = intv->text();
            QString schwa = intv->attribute("schwa").toString();
            QString liaison = intv->attribute("liaison").toString();
            ortho = ortho.replace("< ", "<").replace("<", " <").replace(" >", ">").replace(">", "> ").replace(" :", ":")
                    .replace(" )", ")").replace("( ", "(")
                    .replace(" .", ".").replace(" ,", ",")
                    .replace("  ", " ").replace("  ", " ").replace("  ", " ").trimmed();
            schwa = schwa.replace("< ", "<").replace("<", " <").replace(" >", ">").replace(">", "> ").replace(" :", ":")
                    .replace(" )", ")").replace("( ", "(")
                    .replace(" .", ".").replace(" ,", ",")
                    .replace("  ", " ").replace("  ", " ").replace("  ", " ").trimmed();
            liaison = liaison.replace("< ", "<").replace("<", " <").replace(" >", ">").replace(">", "> ").replace(" :", ":")
                    .replace(" )", ")").replace("( ", "(")
                    .replace(" .", ".").replace(" ,", ",")
                    .replace("  ", " ").replace("  ", " ").replace("  ", " ").trimmed();
            if (liaison.isEmpty()) liaison = ortho;
            if (schwa.isEmpty()) schwa = ortho;
            int o = ortho.split(" ").count();
            int s = schwa.split(" ").count();
            int l = liaison.split(" ").count();
            QString comment;
            if (o != s || o != l || s != l) comment = "check";
            intv->setText(ortho);
            intv->setAttribute("schwa", schwa);
            intv->setAttribute("liaison", liaison);
            intv->setAttribute("comment", comment);
        }
        com->repository()->annotations()->saveTier(annot->ID(), annot->ID(), transcription);
    }
    return com->ID();
}

QString PFCPreprocessor::checkSpeakers(QPointer<CorpusCommunication> com)
{
    QStringList levels; levels << "transcription";
    if (!com) return "No Communication";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        AnnotationTierGroup *tiers = com->repository()->annotations()->getTiers(annot->ID(), annot->ID(), levels);
        IntervalTier *transcription = tiers->getIntervalTierByName("transcription");
        if (!transcription) continue;
        foreach (Interval *intv, transcription->intervals()) {
            QString ortho = intv->text();
            QString schwa = intv->attribute("schwa").toString();
            QString liaison = intv->attribute("liaison").toString();
            int o = ortho.split(":").count();
            int s = schwa.split(":").count();
            int l = liaison.split(":").count();
            if (o != s || o != l || s != l) intv->setAttribute("comment", "num loc a");
            o = ortho.split("<").count();
            s = schwa.split("<").count();
            l = liaison.split("<").count();
            if (o != s || o != l || s != l) intv->setAttribute("comment", "num loc b");
        }
        com->repository()->annotations()->saveTier(annot->ID(), annot->ID(), transcription);
    }
    return com->ID();
}

void mergeInsideParentheses(QString &input) {
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

QString formatSegment(QString input)
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

QString PFCPreprocessor::separateSpeakers(QPointer<CorpusCommunication> com)
{
    QStringList levels; levels << "transcription";
    if (!com) return "No Communication";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        AnnotationTierGroup *tiers = com->repository()->annotations()->getTiers(annot->ID(), annot->ID(), levels);
        IntervalTier *transcription = tiers->getIntervalTierByName("transcription");
        if (!transcription) continue;
        IntervalTier *segmentMain = new IntervalTier("segment", transcription->tMin(), transcription->tMax());
        segmentMain->copyIntervalsFrom(transcription, false);
        IntervalTier *segment = new IntervalTier("segment", transcription->tMin(), transcription->tMax());
        segment->copyIntervalsFrom(transcription, false);

        int iseg = 0; int isegmain = 0;
        foreach (Interval *intv, transcription->intervals()) {
            QString ortho = intv->text();
            QString schwa = intv->attribute("schwa").toString();
            QString liaison = intv->attribute("liaison").toString();
            mergeInsideParentheses(ortho);
            mergeInsideParentheses(schwa);
            mergeInsideParentheses(liaison);
            ortho = ortho.replace(" <", " |<").replace("> ", ">| ");
            schwa = schwa.replace(" <", " |<").replace("> ", ">| ");
            liaison = liaison.replace(" <", " |<").replace("> ", ">| ");
            if (schwa.split("|").count() != ortho.split("|").count()) schwa = ortho;
            if (liaison.split("|").count() != ortho.split("|").count()) liaison = ortho;
            QStringList splitOrtho = ortho.split("|");
            QStringList splitSchwa = schwa.split("|");
            QStringList splitLiaison = liaison.split("|");
            int count = splitOrtho.count();
            QList<int> lengths;
            for (int i = 0; i < count; ++i) lengths << splitOrtho.at(i).length();
            QList<Interval *> intervals = segment->splitToProportions(iseg, lengths);
            if (intervals.count() == 0) intervals << segment->interval(iseg);
            QString mainOrtho, mainLiaison, mainSchwa;
            for (int i = 0; i < count; ++i) {
                if (splitOrtho.at(i).startsWith("<")) {
                    // embedded - find speaker
                    intervals[i]->setAttribute("overlap", true);
                    QString seg_ortho = formatSegment(QString(splitOrtho.at(i)).replace("<", "").replace(">", ""));
                    QString seg_schwa = formatSegment(QString(splitSchwa.at(i)).replace("<", "").replace(">", ""));
                    QString seg_liaison = formatSegment(QString(splitLiaison.at(i)).replace("<", "").replace(">", ""));
                    intervals[i]->setText(seg_ortho);
                    intervals[i]->setAttribute("schwa", seg_schwa);
                    intervals[i]->setAttribute("liaison", seg_liaison);
                } else {
                    mainOrtho.append(splitOrtho.at(i));
                    mainSchwa.append(splitSchwa.at(i));
                    mainLiaison.append(splitLiaison.at(i));
                }
            }
            segmentMain->interval(isegmain)->setText(formatSegment(mainOrtho));
            segmentMain->interval(isegmain)->setAttribute("schwa", formatSegment(mainSchwa));
            segmentMain->interval(isegmain)->setAttribute("liaison", formatSegment(mainLiaison));
            isegmain++;
            iseg = iseg + count;
        }
        segment->mergeIdenticalAnnotations("");
        segmentMain->mergeIdenticalAnnotations("");

        QHash<QString, QList<Interval *> > lists;
        foreach (Interval *seg, segment->intervals()) {
            if (seg->text().isEmpty()) continue;
            QString spk = "X";
            QString s = seg->text();
            s = s.replace(":", ": ").section(" ", 0, 0);;
            if (s.endsWith(":")) {
                s.chop(1); spk = s;
                QString text = seg->text();
                seg->setText(text.remove(0, spk.length() + 1).trimmed());
                QString schwa = seg->attribute("schwa").toString();
                if (schwa.startsWith(spk + ":")) seg->setAttribute("schwa", schwa.remove(0, spk.length() + 1).trimmed());
                QString liaison = seg->attribute("liaison").toString();
                if (liaison.startsWith(spk + ":")) seg->setAttribute("liaison", liaison.remove(0, spk.length() + 1).trimmed());
            }
            lists[spk].append(seg);
        }
        foreach (Interval *seg, segmentMain->intervals()) {
            if (seg->text().isEmpty()) continue;
            QString spk = "X";
            QString s = seg->text();
            s = s.replace(":", ": ").section(" ", 0, 0);;
            if (s.endsWith(":")) {
                s.chop(1); spk = s;
                seg->setText(seg->text().remove(0, spk.length() + 1).trimmed());
                QString schwa = seg->attribute("schwa").toString();
                if (schwa.startsWith(spk + ":")) seg->setAttribute("schwa", schwa.remove(0, spk.length() + 1).trimmed());
                QString liaison = seg->attribute("liaison").toString();
                if (liaison.startsWith(spk + ":")) seg->setAttribute("liaison", liaison.remove(0, spk.length() + 1).trimmed());
            }
            lists[spk].append(seg);
        }

        QMap<QString, AnnotationTier *> tiersAll;
        foreach (QString spk, lists.keys()) {
            IntervalTier *t = new IntervalTier("segment", lists[spk], transcription->tMin(), transcription->tMax());
            foreach (Interval *i, t->intervals()) {
                if (i->text().isEmpty()) { i->setText("_"); i->setAttribute("schwa", "_"); i->setAttribute("liaison", "_"); }
            }
            tiersAll.insert(annot->ID() + "_" + spk, t);
        }

        foreach (QString spk, tiersAll.keys()) {
            com->repository()->annotations()->saveTier(annot->ID(), spk, tiersAll.value(spk));
        }

        delete segment;
        delete segmentMain;
        qDeleteAll(tiersAll);
    }
    return com->ID();
}

QList<QString> splitToken(QString input)
{
    input = input.replace("'", "' ");
    input = input.replace("\u2019", "' ");
    // Dashes
    if (input.startsWith("-"))
        input.remove(0, 1); // if a token begins with '-', the next rule would create a lonely dash token
    input = input.replace("-", "- ");
    //
    input = input.replace("aujourd' hui", "aujourd'hui");
    input = input.replace("t- il", "t-il");
    input = input.replace("t- elle", "t-elle");
    input = input.replace("t- on", "t-on");

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

QString PFCPreprocessor::tokenise(QPointer<CorpusCommunication> com)
{
    if (!com) return "No Communication";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "segment");
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            IntervalTier *tier_segment = tiers->getIntervalTierByName("segment");
            if (!tier_segment) continue;
            IntervalTier *tier_tok_min = new IntervalTier("tok_min", tier_segment->tMin(), tier_segment->tMax());
            tier_tok_min->copyIntervalsFrom(tier_segment, false);

            int itok = 0;
            foreach (Interval *segment, tier_segment->intervals()) {
                QString ortho = segment->text();
                QString schwa = segment->attribute("schwa").toString();
                QString liaison = segment->attribute("liaison").toString();
                QList<QString> splitOrtho = splitToken(ortho);
                QList<QString> splitSchwa = splitToken(schwa);
                QList<QString> splitLiaison = splitToken(liaison);
                int count = splitOrtho.count();
                if (count != splitSchwa.count()) splitSchwa = splitOrtho;
                if (count != splitLiaison.count()) splitLiaison = splitOrtho;
                QList<int> lengths;
                for (int i = 0; i < count; ++i) lengths << splitOrtho.at(i).length();
                if (segment->duration().toDouble() > 1.0) lengths << 2;
                QList<Interval *> intervals_tok_min = tier_tok_min->splitToProportions(itok, lengths);
                if (intervals_tok_min.count() == 0) intervals_tok_min << tier_tok_min->interval(itok);
                for (int i = 0; i < count; ++i) {
                    intervals_tok_min[i]->setText(splitOrtho.at(i));
                    intervals_tok_min[i]->setAttribute("schwa", splitSchwa.at(i));
                    intervals_tok_min[i]->setAttribute("liaison", splitLiaison.at(i));
                }
                itok = itok + count;
                if (segment->duration().toDouble() > 1.0) itok = itok + 1;
            }
            tier_tok_min->fillEmptyWith("", "_");
            tier_tok_min->mergeIdenticalAnnotations("_");

            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
            delete tier_tok_min;
        }
        qDeleteAll(tiersAll);
    }
    return com->ID();
}

QString PFCPreprocessor::tokmin_punctuation(QPointer<CorpusCommunication> com)
{
    if (!com) return "No Communication";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "tok_min");
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
            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
        }
        qDeleteAll(tiersAll);
    }
    return com->ID();
}

QString PFCPreprocessor::liaisonCoding(QPointer<CorpusCommunication> com)
{
    if (!com) return "No Communication";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "tok_min");
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;
            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                QString t = tok_min->text();
                if (t == "_") {
                    tok_min->setAttribute("liaison", "_");
                    continue;
                }
                QString l = tok_min->attribute("liaison").toString();
                if (l.startsWith(t) && l.length() > t.length() && !l.endsWith(".")) {
                    QString code = l.remove(0, t.length());
                    int nsyll = code.mid(0, 1).toInt();
                    int realisation = (code.length() > 1) ? code.mid(1, 1).toInt() : 0;
                    QString consonant = (code.length() > 2) ? code.mid(2, 1) : "";
                    QString context = (code.length() > 3) ? code.mid(3, -1) : "";
                    tok_min->setAttribute("liaison_nsyll", nsyll);
                    tok_min->setAttribute("liaison_realisation", realisation);
                    tok_min->setAttribute("liaison_consonant", consonant);
                    tok_min->setAttribute("liaison_context", context);
                    // qDebug() << annot->ID() << speakerID << tok_min->tMin().toDouble() << t << l;
                }
            }
            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
        }
        qDeleteAll(tiersAll);
    }
    return com->ID();
}

