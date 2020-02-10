#include <QDebug>
#include <QString>
#include <QList>
#include <QStringList>
#include <QPointer>
#include <QMap>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>

#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"
#include "PraalineCore/Statistics/WordAlign.h"
using namespace Praaline::Core;

#include "PFCPreprocessor.h"

struct PFCPreprocessorData {
    QStringList punctuationMarks;
    QHash<QString, QString> replaceSegments;
};

PFCPreprocessor::PFCPreprocessor() : d(new PFCPreprocessorData())
{
    d->punctuationMarks << "." << "," << "?" << "!" << "-" << ":" << "\"" << "\\";

    QFile fileReplace("/mnt/hgfs/DATA/PFCALIGN/phonetisation/replace_segments.txt");
    if ( !fileReplace.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QTextStream replacein(&fileReplace);
    replacein.setCodec("UTF-8");
    while (!replacein.atEnd()) {
        QString line = replacein.readLine();
        if (!line.contains("\t")) continue;
        while (line.contains("  ")) line = line.replace("  ", " ");
        QString text_from = line.section("\t", 0, 0);
        QString text_to   = line.section("\t", 1, 1);
        if (text_from.isEmpty()) continue;
        if (text_to.isEmpty()) continue;
        d->replaceSegments.insert(text_from, text_to);
    }
    fileReplace.close();
}

PFCPreprocessor::~PFCPreprocessor()
{
    delete d;
}

/// This function processes a collection of PFC TextGrids and ensures that there are three annotation tiers,
/// (orthographic transcription, schwa coding, liaison coding) as per the PFC protocol, and optionally additional
/// tiers for anonymisation, comments and prosody annotation. The function renames the tiers in the TextGrid
/// according to the PFC standard (transcription, schwa, liaison, anonymisation) and saves the TextGrid in
/// ISO 8859-1 encoding (Latin 1). It is also possible to have a TextGrid with only one tier (transcription)
/// in cases where the schwa and liaison annotation has not yet been performed.
///
/// The function will perform the following verifications and operations:
/// - If the file name indicates the word task (m, c, ms, ms2 and s suffix) and there is only one tier, then
///   it is assumed that this is a transcription tier and it is renamed accordingly.
/// - In all other cases, the number of tiers in the TextGrid is checked:
///   - If there are 4 tiers and the fourth tier is an anonymisation tier, it is appropriately named and kept.
///   - If there are 4 or 3 tiers, and the second/third contain the words liaison/schwa, then the first is considered to
///     be the transcription. All tiers are appropriately named.
/// In all other cases, the file is marked for manual verification and correction of its tier names.
QString PFCPreprocessor::renameTextgridTiers(const QString& directory)
{
    QString ret;
    InterfaceTextFile::setDefaultEncoding("ISO 8859-1");

    QDir dirinfo(directory);
    QFileInfoList list;
    list << dirinfo.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
    dirinfo.setNameFilters(QStringList() << "*.textgrid");
    list << dirinfo.entryInfoList();

    bool save = true;

    foreach (QFileInfo info, list) {
        if (info.isDir()) {
            ret.append(renameTextgridTiers(info.filePath()));
        }
        else {
            QString txgFilename = info.filePath();
            AnnotationTierGroup *txg = new AnnotationTierGroup();
            if (!PraatTextGrid::load(txgFilename, txg)) {
                ret.append(txgFilename).append("\tCHECK\t0\n");
            }
            else if ((txg->tiersCount() == 1) &&
                     ((txgFilename.endsWith("m.TextGrid") || txgFilename.endsWith("c.TextGrid") ||
                       txgFilename.endsWith("ms.TextGrid") || txgFilename.endsWith("ms2.TextGrid") || txgFilename.endsWith("s.TextGrid")))) {
                QString tiername_ortho = txg->tier(0)->name().trimmed();
                txg->tier(0)->setName("transcription");
                if (save) PraatTextGrid::save(txgFilename, txg);
                ret.append(txgFilename).append("\tOK\t1\t").append(tiername_ortho).append("\n");
            }
            else if ((txg->tiersCount() == 3) || (txg->tiersCount() == 4)) {
                if (txg->tiersCount() == 4 && txg->tier(3)->name() == "Anonymisation")
                    txg->tier(3)->setName("anonymisation");
                QString tiername_ortho, tiername_schwa, tiername_liaison, tiername_others;
                if (txg->tier(1)->name().toLower().contains("liaison") && txg->tier(2)->name().toLower().contains("schwa")) {
                    tiername_ortho   = txg->tier(0)->name().trimmed(); txg->tier(0)->setName("transcription");
                    tiername_liaison = txg->tier(1)->name().trimmed(); txg->tier(1)->setName("liaison");
                    tiername_schwa   = txg->tier(2)->name().trimmed(); txg->tier(2)->setName("schwa");
                    if (txg->tiersCount() == 4) tiername_others = txg->tier(3)->name();
                    if (save) PraatTextGrid::save(txgFilename, txg);
                    ret.append(txgFilename).append("\tOK\t").append(QString("%1").arg(txg->tiersCount())).append("\t")
                            .append(tiername_ortho).append("\t").append(tiername_liaison).append("\t").append(tiername_schwa).append("\t")
                            .append(tiername_others).append("\n");
                }
                else if (txg->tier(2)->name().toLower().contains("liaison") && txg->tier(1)->name().toLower().contains("schwa")) {
                    tiername_ortho   = txg->tier(0)->name().trimmed(); txg->tier(0)->setName("transcription");
                    tiername_liaison = txg->tier(2)->name().trimmed(); txg->tier(2)->setName("liaison");
                    tiername_schwa   = txg->tier(1)->name().trimmed(); txg->tier(1)->setName("schwa");
                    if (txg->tiersCount() == 4) tiername_others = txg->tier(3)->name();
                    if (save) PraatTextGrid::save(txgFilename, txg);
                    ret.append(txgFilename).append("\tOK\t").append(QString("%1").arg(txg->tiersCount())).append("\t")
                            .append(tiername_ortho).append("\t").append(tiername_liaison).append("\t").append(tiername_schwa).append("\t")
                            .append(tiername_others).append("\n");
                }
                else {
                    ret.append(txgFilename).append("\tCHECK\t").append(QString("%1").arg(txg->tiersCount()))
                            .append("\t\t\t\t").append(txg->tierNames().join(" ")).append("\n");
                }
            }
            else {
                ret.append(txgFilename).append("\tCHECK\t").append(QString("%1").arg(txg->tiersCount()))
                        .append("\t\t\t\t").append(txg->tierNames().join(" ")).append("\n");
            }
            delete txg;
        }
    }
    return ret;
}

/// Pre-Processing Step 1.1
///
/// This function is the first step in preparing the PFC transcriptions for pre-processing.
///
/// Input: a CorpusCommunication with one transcription tier, having 2 attributes (liaison and schwa).
/// The function applies basic typographic conventions on all intervals of the three tiers. It then calculates the
/// number of words separated by whitespace in each of the three tiers (text + 2 attributes), for all intervals.
///
/// If an interval does not have the exact same number of words accross the three tiers, it is marked to be checked manually.
/// This is done by setting the attribute "tocheck" to "CHECK". In these cases, the word alignment between the orthographic
/// transcription and the schwa tier is also calculated and stored in the database (it is used in the interactive editor).
///
/// If there are transcription intervals to be checked, their number is saved in the transcriptionsToCheck attribute of
/// the Communication
QString PFCPreprocessor::prepareTranscription(CorpusCommunication *com)
{
    if (!com) return "No Communication";
    QStringList levels; levels << "transcription";
    int transcriptionsToCheck(0);
    foreach (CorpusAnnotation *annot, com->annotations()) {
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
                    .replace("  ", " ").replace("  ", " ").replace("  ", " ")
                    .replace("d\u00E9clar\u00E9:", "d\u00E9clar\u00E9").trimmed();
            schwa = schwa.replace("< ", "<").replace("<", " <").replace(" >", ">").replace(">", "> ").replace(" :", ":")
                    .replace(" )", ")").replace("( ", "(")
                    .replace(" .", ".").replace(" ,", ",")
                    .replace("  ", " ").replace("  ", " ").replace("  ", " ")
                    .replace("d\u00E9clar\u00E9:", "d\u00E9clar\u00E9").trimmed();
            liaison = liaison.replace("< ", "<").replace("<", " <").replace(" >", ">").replace(">", "> ").replace(" :", ":")
                    .replace(" )", ")").replace("( ", "(")
                    .replace(" .", ".").replace(" ,", ",")
                    .replace("  ", " ").replace("  ", " ").replace("  ", " ")
                    .replace("d\u00E9clar\u00E9:", "d\u00E9clar\u00E9").trimmed();
            if (liaison.isEmpty()) liaison = ortho;
            if (schwa.isEmpty()) schwa = ortho;
            int o = ortho.split(" ").count();
            int s = schwa.split(" ").count();
            int l = liaison.split(" ").count();
            QString tocheck;
            if (o != s || o != l || s != l) {
                tocheck = "CHECK";
                transcriptionsToCheck++;
            }
            intv->setText(ortho);
            intv->setAttribute("schwa", schwa);
            intv->setAttribute("liaison", liaison);
            intv->setAttribute("tocheck", tocheck);

            WordAlign align;
            align.align(ortho.split(" "), schwa.split(" "));
            intv->setAttribute("wordalign", align.alignmentText());
            intv->setAttribute("wordalign_wer", align.WER());
        }
        com->repository()->annotations()->saveTier(annot->ID(), annot->ID(), transcription);
    }
    com->setProperty("transcriptionsToCheck", transcriptionsToCheck);
    return QString(com->ID()).append("\t").append(QString::number(transcriptionsToCheck));
}

bool PFCPreprocessor::tryFixingSpeakers(Praaline::Core::Interval *intv)
{
    int o = intv->text().split(":").count() - 1;
    int s = intv->attribute("schwa").toString().split(":").count() - 1;
    int l = intv->attribute("liaison").toString().split(":").count() - 1;
    if (o != s || o != l || s != l) return false;
    if (o != 3) return false;

    QStringList ortho = intv->text().split(" ", QString::SkipEmptyParts);
    QStringList schwa = intv->attribute("schwa").toString().split(" ");
    QStringList liaison = intv->attribute("liaison").toString().split(" ");
    if (ortho.isEmpty() || schwa.isEmpty() || liaison.isEmpty()) return false;
    if (!ortho.first().endsWith(":") || !schwa.first().endsWith(":") || !liaison.first().endsWith(":")) return false;
    QString mainSpeaker = ortho.first();
    if ((schwa.first() != mainSpeaker) || (liaison.first() != mainSpeaker)) return false;

    bool inside(false);
    for (int i = 1; i < ortho.count(); ++i) {
        if (ortho.at(i) == mainSpeaker + ":") {
            ortho[i].clear();
        }
        else if (!inside && ortho.at(i).endsWith(":") && !ortho.at(i).startsWith("<")) {
            ortho[i].prepend("<"); inside = true;
        }
        else if (inside && ortho.at(i).endsWith(":") && !ortho.at(i - 1).endsWith(">")) {
            ortho[i-1].append(">"); inside = false;
        }
    }
    if (inside) { ortho[ortho.count() - 1].append(">"); inside = false; }
    for (int i = 1; i < schwa.count(); ++i) {
        if (schwa.at(i) == mainSpeaker + ":") {
            schwa[i].clear();
        }
        else if (!inside && schwa.at(i).endsWith(":") && !schwa.at(i).startsWith("<")) {
            schwa[i].prepend("<"); inside = true;
        }
        else if (inside && schwa.at(i).endsWith(":") && !schwa.at(i - 1).endsWith(">")) {
            schwa[i-1].append(">"); inside = false;
        }
    }
    if (inside) { schwa[schwa.count() - 1].append(">"); inside = false; }
    for (int i = 1; i < liaison.count(); ++i) {
        if (liaison.at(i) == mainSpeaker + ":") {
            liaison[i].clear();
        }
        else if (!inside && liaison.at(i).endsWith(":") && !liaison.at(i).startsWith("<")) {
            liaison[i].prepend("<"); inside = true;
        }
        else if (inside && liaison.at(i).endsWith(":") && !liaison.at(i - 1).endsWith(">")) {
            liaison[i-1].append(">"); inside = false;
        }
    }
    if (inside) { liaison[liaison.count() - 1].append(">"); inside = false; }
    intv->setText(ortho.join(" "));
    intv->setAttribute("schwa", schwa.join(" "));
    intv->setAttribute("liaison", liaison.join(" "));
    return true;
}

/// Pre-Processing Step 1.2
/// This function checks whether the same number of speakers is indicated in all three transcription tiers.
/// If it is not the case, then it sets the "tocheck" attribute of the interval to "num loc".
/// If the interval is a single-speaker interval, it is marked accordingly in the "single_speaker" attribute.
QString PFCPreprocessor::checkSpeakers(CorpusCommunication *com)
{
    QStringList levels; levels << "transcription";
    if (!com) return "No Communication";
    int transcriptionsToCheck(0);
    foreach (CorpusAnnotation *annot, com->annotations()) {
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
            // Reset single-speaker indicator
            intv->setAttribute("single_speaker", "");
            // Check for single speaker
            if ((o == s) && (o == l) && (o == 2)) {
                // Single speaker intervals are formatted like that: LOC: text text ...
                intv->setAttribute("single_speaker", ortho.split(":").at(0));
            }
            // Check if error in the number of speakers given by the ":" symbol
            if (o != s || o != l || s != l) intv->setAttribute("tocheck", intv->attribute("tocheck").toString() + " num loc a");
            // Check for embedded speakers
            o = ortho.split("<").count() + ortho.split(">").count();
            s = schwa.split("<").count() + schwa.split(">").count();
            l = liaison.split("<").count() + liaison.split(">").count();
            if (o != s || o != l || s != l) {
                // tryFixingSpeakers(intv);
                // o = ortho.split("<").count() + ortho.split(">").count();
                // s = schwa.split("<").count() + schwa.split(">").count();
                // l = liaison.split("<").count() + liaison.split(">").count();
                // if (o != s || o != l || s != l) {
                    intv->setAttribute("tocheck", intv->attribute("tocheck").toString() + " num loc b");
                // }
            }
            if (!intv->attribute("tocheck").toString().isEmpty()) transcriptionsToCheck++;
        }
        com->repository()->annotations()->saveTier(annot->ID(), annot->ID(), transcription);
    }
    com->setProperty("transcriptionsToCheck", transcriptionsToCheck);
    return QString(com->ID()).append("\t").append(QString::number(transcriptionsToCheck));
}

void mergeInsideParentheses(QString &input, const QString &open = "\\(", const QString &close = "\\)") {
    // (make sure it's after the _ replace block!)
    // noises and paraverbal: \\((.*)\\)
    // phonetisation:         \\[(.*)\\]
    QRegExp rx(QString("%1(.*)%2").arg(open).arg(close));
    rx.setMinimal(true);
    int s = -1;
    while ((s = rx.indexIn(input, s+1)) >= 0) {
        QString capture = rx.cap(0);
        input.replace(s, capture.length(), capture.replace(" ", "_").replace("'", "'_").replace("-", "-_"));
        s += rx.cap(1).length();
    }
}

QString PFCPreprocessor::formatSegment(const QString &input)
{
    QString ret = input;
    if (ret.startsWith("'")) { ret = ret.remove(0, 1).prepend("\""); }
    if (ret.endsWith("'"))   { ret.chop(1); ret = ret.append("\""); }
    if (ret.startsWith("/")) { ret = ret.remove(0, 1); }
    ret = ret.replace("''", "'").replace(" /", "/").replace("-/", "/").replace("'/", "/");
    ret = ret.replace(" '", " \"").replace("' ", "\" ").replace("'.", "\".").replace("', ", "\", ").replace("\" ,", "\",").replace("\" .", "\".");
    ret = ret.replace("/", "/ ").replace("/ ,", "/,").replace("/ .", "/.").replace("/ )", "/)");
    while (ret.contains("  ")) ret = ret.replace("  ", " ");
    ret = ret.replace(" .", ".").replace(" ,", ",").replace(" ?", "?").replace(" !", "!");
    ret = ret.replace(".(", ". (").replace(",(", ", (").replace("?(", "? (").replace("!(", "! (");
    ret = ret.replace("(", " (").replace(")", ") ");
    while (ret.contains("  ")) ret = ret.replace("  ", " ");
    ret = ret.replace(" _", "_").replace("_ ", "_");
    return ret.trimmed();
}

QString PFCPreprocessor::separateSpeakers(CorpusCommunication *com)
{
    QStringList levels; levels << "transcription";
    if (!com) return "No Communication";
    foreach (CorpusAnnotation *annot, com->annotations()) {
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
            mergeInsideParentheses(ortho);      mergeInsideParentheses(ortho, "\\[", "\\]");
            mergeInsideParentheses(schwa);      mergeInsideParentheses(schwa, "\\[", "\\]");
            mergeInsideParentheses(liaison);    mergeInsideParentheses(liaison , "\\[", "\\]");
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

        // The main speaker ID from the sample's coding 11aal1 -> AL1
        QString mainSpeakerID = annot->ID().mid(3, 3).toUpper();

        QHash<QString, QList<Interval *> > lists;
        foreach (Interval *seg, segment->intervals()) {
            if (seg->text().isEmpty()) continue;
            QString spk = mainSpeakerID;
            QString s = seg->text();
            s = s.replace(":", ": ").section(" ", 0, 0);
            if (s.endsWith(":")) {
                s.chop(1); spk = s.toUpper();
                if (QString(s).append("1") == mainSpeakerID) spk = mainSpeakerID; // AL short-hand for AL1
                if (QString(s).append("2") == mainSpeakerID) spk = mainSpeakerID; // AL short-hand for AL2
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
            QString spk = mainSpeakerID;
            QString s = seg->text();
            s = s.replace(":", ": ").section(" ", 0, 0);
            if (s.endsWith(":")) {
                s.chop(1); spk = s.toUpper();
                if (QString(s).append("1") == mainSpeakerID) spk = mainSpeakerID; // AL short-hand for AL1
                if (QString(s).append("2") == mainSpeakerID) spk = mainSpeakerID; // AL short-hand for AL2
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
            QString speakerID = annot->ID().left(6) + "_" + spk;
            tiersAll.insert(speakerID, t);
        }

        foreach (QString speakerID, tiersAll.keys()) {
            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tiersAll.value(speakerID));
        }

        delete segment;
        delete segmentMain;
        // qDeleteAll(tiersAll);
    }
    return com->ID();
}

QList<QString> PFCPreprocessor::splitToken(const QString &input)
{
    QString text = input;
    // Initial
    text = text.replace("'", "' ").replace("' _", "'_");
    text = text.replace("\u2019", "' ").replace("' _", "'_");
    // Numbers not transcribed
    QList<QString> acc;
    foreach (QString tok, text.trimmed().split(" ", QString::SkipEmptyParts)) {
        if (d->replaceSegments.contains(tok)) {
            acc << d->replaceSegments.value(tok);
        } else {
            acc << tok;
        }
    }
    text = acc.join(" ");
    // Dashes
    if (text.startsWith("-"))
        text.remove(0, 1); // if a token begins with '-', the next rule would create a lonely dash token
    text = text.replace("-", "- ").replace("- _", "-_");
    // Keep-together words
    text = text.replace("aujourd' hui", "aujourd'hui");
    text = text.replace("t- il", "t-il");
    text = text.replace("t- elle", "t-elle");
    text = text.replace("t- on", "t-on");

    // Break everything at spaces
    QList<QString> ret;
    QList<QString> result = text.trimmed().split(" ", QString::SkipEmptyParts);
    int i = 0;
    while (i < result.count()) {
        if (result.at(i) == "parce" || result.at(i) == "Parce") {
            // parce que et parce qu'
            if ((i + 1 < result.count()) && (result.at(i+1) == "que" || result.at(i+1) == "qu'")) {
                ret << QString("%1 %2").arg(result.at(i)).arg(result.at(i+1));
                i++;
            } else ret << result.at(i);
        }
        else if (result.at(i) == "/" || result.at(i) == "-") {
            // Lone symbols, despite our best efforts
            if (i + 1 < result.count()) {
                ret << QString("%1 %2").arg(result.at(i)).arg(result.at(i+1));
                i++;
            } else ret << result.at(i);
        }
        else {
            ret << result.at(i);
        }
        i++;
    }
    return ret;
}

QString PFCPreprocessor::tokenise(CorpusCommunication *com)
{
    if (!com) return "No Communication";
    foreach (CorpusAnnotation *annot, com->annotations()) {
        SpeakerAnnotationTierGroupMap tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "segment");
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
                // Whenever there is a count mismatch, we drop the schwa and liaison coding; better than creating
                // inconsistent intervals. These cases should have been caught earlier in the preprocessor.
                int count = splitOrtho.count();
                if (count != splitSchwa.count()) splitSchwa = splitOrtho;
                if (count != splitLiaison.count()) splitLiaison = splitOrtho;
                // Move
                // Create intervals proportional to the length of each token in characters
                QList<int> lengths;
                for (int i = 0; i < count; ++i) lengths << splitOrtho.at(i).length();
                QList<Interval *> intervals_tok_min = tier_tok_min->splitToProportions(itok, lengths);
                if (intervals_tok_min.count() == 0) {
                    intervals_tok_min << tier_tok_min->interval(itok);
                }
                // Insert the tokenised data (token-schwa-liaison) into the corresponding interval
                for (int i = 0; i < count; ++i) {
                    intervals_tok_min[i]->setText(splitOrtho.at(i));
                    intervals_tok_min[i]->setAttribute("schwa", splitSchwa.at(i));
                    intervals_tok_min[i]->setAttribute("liaison", splitLiaison.at(i));
                }
                itok = itok + count;
            }
            // Fill empty intervals with the pause symbol and merge contiguous pauses
            tier_tok_min->fillEmptyWith("", "_");
            tier_tok_min->mergeIdenticalAnnotations("_");
            // Now fill pauses with the pause symbol for the schwa and liaison attributes
            tier_tok_min->fillEmptyWith("schwa", "_");
            tier_tok_min->fillEmptyWith("liaison", "_");
            // Move non-speech comments to the previous token. Anything left on the tok_min tier must be actual
            // articulated speech (something that can be phonetised).
            for (int i = tier_tok_min->count() - 1; i >= 1; --i) {
                QString token = tier_tok_min->at(i)->text();
                if (!token.startsWith("(")) continue;
                if (!token.endsWith(")")) continue;
                // Exceptions: phonetisable paraverbals
                if (token == "(X)" || token == "(XX)" || token == "(XXX)") continue;
                QString prev_schwa = tier_tok_min->at(i - 1)->attribute("schwa").toString();
                QString prev_liaison = tier_tok_min->at(i - 1)->attribute("liaison").toString();
                tier_tok_min->at(i)->setText("");
                Interval *intv = tier_tok_min->merge(i - 1, i);
                intv->setAttribute("schwa", prev_schwa);
                intv->setAttribute("liaison", prev_liaison);
                intv->setAttribute("event", token);
            }
            // Save the created tok_min tier into the database
            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
            delete tier_tok_min;
        }
        qDeleteAll(tiersAll);
    }
    return com->ID();
}

bool PFCPreprocessor::startsWithPunctuation(const QString &text)
{
    if (text.isEmpty()) return false;
    QString c = text.left(1);
    if (d->punctuationMarks.contains(c)) return true;
    return false;
}

bool PFCPreprocessor::endsWithPunctuation(const QString &text)
{
    if (text.isEmpty()) return false;
    QString c = text.right(1);
    if (d->punctuationMarks.contains(c)) return true;
    return false;
}

bool PFCPreprocessor::isAcronym(const QString &text)
{
    QRegularExpression re(QString("^([A-Za-z]\\.)+$"));
    return re.match(text).hasMatch();
}

QString PFCPreprocessor::tokmin_punctuation(CorpusCommunication *com)
{
    if (!com) return "No Communication";
    foreach (CorpusAnnotation *annot, com->annotations()) {
        SpeakerAnnotationTierGroupMap tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "tok_min");
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;
            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                QString t = tok_min->text().trimmed();
                QString s = tok_min->attribute("schwa").toString();
                QString l = tok_min->attribute("liaison").toString();
                // Remove all punctuation marks. Keep track of the marks removed (based on the token's text)
                QString punctuation_before, punctuation_after;
                while (startsWithPunctuation(t))                { punctuation_before.append(t.left(1));  t = t.remove(0, 1); }
                while (endsWithPunctuation(t) && !isAcronym(t)) { punctuation_after.prepend(t.right(1)); t.chop(1);          }
                while (startsWithPunctuation(s))                { s = s.remove(0, 1); }
                while (endsWithPunctuation(s) && !isAcronym(s)) { s.chop(1); }
                while (startsWithPunctuation(l))                { l = l.remove(0, 1); }
                while (endsWithPunctuation(l) && !isAcronym(l)) { l.chop(1); }
                // Update the token
                tok_min->setText(t);
                tok_min->setAttribute("schwa", s);
                tok_min->setAttribute("liaison", l);
                tok_min->setAttribute("punctuation_before", punctuation_before);
                tok_min->setAttribute("punctuation_after", punctuation_after);
            }
            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
        }
        qDeleteAll(tiersAll);
    }
    return com->ID();
}

QString PFCPreprocessor::liaisonCoding(CorpusCommunication *com)
{
    if (!com) return "No Communication";
    foreach (CorpusAnnotation *annot, com->annotations()) {
        SpeakerAnnotationTierGroupMap tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "tok_min");
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

QString PFCPreprocessor::checkCharacterSet(CorpusCommunication *com)
{
    QString ret;
    QRegularExpression regex("^[a-zA-Z\u00C9\u00E9\u00C0\u00E0\u00C8\u00E8\u00D9\u00F9\u00C2\u00E2\u00CA\u00EA\u00CE\u00EE\u00D4\u00F4\u00DB\u00FB\u00CB\u00EB\u00CF\u00EF\u00DC\u00FC\u0178\u00FF\u00C7\u00E7\u0152\u0153\u00C6\u00E6]");
    regex.optimize();
    if (!com) return "No Communication";
    foreach (CorpusAnnotation *annot, com->annotations()) {
        SpeakerAnnotationTierGroupMap tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "tok_min");
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;
            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                if (!regex.match(tok_min->text()).hasMatch()) {
                    ret.append(com->ID()).append("\t").append(tok_min->text()).append("\n");
                }
            }
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}

