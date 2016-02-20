#include <QString>
#include <QList>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>
#include <QProcess>
#include <QCoreApplication>

#include "pncore/corpus/corpus.h"
#include "pncore/corpus/corpuscommunication.h"
#include "pncore/corpus/corpusannotation.h"
#include "pncore/annotation/annotationtiergroup.h"
#include "pncore/annotation/intervaltier.h"
#include "pncore/annotation/interval.h"
#include "pncore/interfaces/praat/praattextgrid.h"

#include "pnlib/phonetiser/ExternalPhonetiser.h"
#include "easyalignbasic.h"

EasyAlignBasic::EasyAlignBasic(QObject *parent) :
    AnnotationPluginPraatScript(parent),
    m_alignerOutputFilenameSuffix("_align"), m_tiernameSelection("selection"),
    m_filterSelection("***"), m_tiernameTranscription("segment"),
    m_tiernamePhonetisation("phono"), m_preciseUtteranceBoundaries(true)
{
}

EasyAlignBasic::~EasyAlignBasic()
{
}

QString EasyAlignBasic::prepareAlignmentTextgrid(CorpusCommunication *com, QPointer<AnnotationTierGroup> tiers)
{
    QString result = QString("Preparing additional alignment textgrid(s)").arg(com->ID());
    // Find the input tiers
    IntervalTier *tier_selection = tiers->getIntervalTierByName(m_tiernameSelection);
    if (!tier_selection) {
        // continue;
        tier_selection = new IntervalTier(m_tiernameSelection, tiers->tMin(), tiers->tMax(), this);
        tier_selection->interval(0)->setText(m_filterSelection);
    }
    IntervalTier *tier_ortho = tiers->getIntervalTierByName(m_tiernameTranscription);
    // txg->getIntervalTierByIndex(0);

    if (!tier_ortho) return "no ortho";

    // Decide where to realign
    AnnotationTierGroup *txgAlign = new AnnotationTierGroup(this);
    QList<Interval *> intervalsToAlign;
    foreach (Interval *intv, tier_selection->intervals()) {
        if (intv->text() == m_filterSelection) {
            intervalsToAlign << tier_ortho->getIntervalsContainedIn(intv);
        }
    }

    IntervalTier *tier_toAlign_ortho = new IntervalTier(
                m_tiernameTranscription, tier_ortho->tMin(), tier_ortho->tMax(), intervalsToAlign);
    IntervalTier *tier_toAlign_phono = new IntervalTier(tier_toAlign_ortho);
    tier_toAlign_phono->setName(m_tiernamePhonetisation);

    QList<Interval *> intervalsWords;
    QList<Interval *> intervalsPhones;

    for (int i = 0; i < tier_toAlign_ortho->countItems(); i++) {
        Interval *intv_utteranceOrtho = tier_toAlign_ortho->interval(i);
        if (intv_utteranceOrtho->text().trimmed().isEmpty())
            continue;
        Interval *intv_utterance = tier_toAlign_phono->interval(i);
        // Format the utterance for EasyAlign...
        QString utterance = intv_utteranceOrtho->text().trimmed();
        utterance = utterance.replace("' ", "'"); // c' etait > c'etait
        intv_utteranceOrtho->setText(utterance);
        // Format for phonetizer
        QString utteranceOut = utterance;
        utteranceOut = utteranceOut.replace("/", "").replace("-", "").replace("=", "").replace("!", "");
        // noises and paraverbal (make sure it's after the _ replace block!)
        QRegExp rx("[(.*)]");
        rx.setMinimal(true);
        int s = -1;
        while ((s = rx.indexIn(utteranceOut, s+1)) >= 0) {
            QString capture = rx.cap(0);
            utteranceOut.replace(s, capture.length(), "");
            s += rx.cap(1).length();
        }
        //
        QList<Interval *> utt; utt << new Interval(intv_utteranceOrtho);
        utt.first()->setText(utteranceOut.replace("_", "").trimmed());
        QList<SpeechToken> atokens = ExternalPhonetiser::phonetiseList(utt);
        QList<QString> ls_words;
        QList<QString> ls_phonetizations;
        QString phonetized;
        foreach (SpeechToken atoken, atokens) {
            ls_words << atoken.orthographic;
            ls_phonetizations << atoken.phonetisation;
            phonetized = phonetized.append(atoken.phonetisation).append("| ");
        }
        // If the number of phonetisations is less than the number of words (should not happen)
        // add fake phonetisations to ensure no words are lost
        while (ls_words.count() > ls_phonetizations.count())
            ls_phonetizations.append("@");
        int count = qMin(ls_phonetizations.count(), ls_words.count());
        //
        RealTime w_xMin = intv_utterance->tMin();
        RealTime w_step = (intv_utterance->tMax() - intv_utterance->tMin()) / ((double) count);
        RealTime w_xMax = w_xMin;
        for (int i = 0 ; i < count; i++) {
            QString s_word = ls_words.at(i);
            QString s_phonetisation = ls_phonetizations.at(i);
            w_xMax = w_xMax + w_step; if (w_xMax > intv_utterance->tMax()) w_xMax = intv_utterance->tMax();
            intervalsWords << new Interval(w_xMin, w_xMax, s_word);
            // phonemes
            QList<QString> ls_phones = s_phonetisation.split(" ", QString::SkipEmptyParts);
            int countPhones = ls_phones.count();
            RealTime p_xMin = w_xMin;
            RealTime p_step = (w_xMax - w_xMin) / ((double) countPhones);
            RealTime p_xMax = p_xMin;
            for (int j = 0; j < countPhones; j++) {
                QString s_phone = ls_phones.at(j);
                p_xMax = p_xMax + p_step; if (p_xMax > w_xMax) p_xMax = w_xMax;
                intervalsPhones << new Interval(p_xMin, p_xMax, s_phone);
                p_xMin = p_xMax; // next phone
            }
            //
            w_xMin = w_xMax; // next word
        }
        phonetized.replace(" ", ""); phonetized.replace(" ", "");
        phonetized.replace("|", " ");
        intv_utterance->setText(phonetized.trimmed());
    }
    IntervalTier *tier_toAlign_wordsT = new IntervalTier(
                "wordsT", tier_ortho->tMin(), tier_ortho->tMax(), intervalsWords);
    IntervalTier *tier_toAlign_phonesT = new IntervalTier(
                "phonesT", tier_ortho->tMin(), tier_ortho->tMax(), intervalsPhones);

    IntervalTier *tier_toAlign_selection = new IntervalTier(tier_selection);
    txgAlign->addTier(tier_toAlign_phonesT);
    txgAlign->addTier(tier_toAlign_wordsT);
    txgAlign->addTier(tier_toAlign_phono);
    txgAlign->addTier(tier_toAlign_ortho);
    txgAlign->addTier(tier_toAlign_selection);

    QString filename = com->basePath() + "/" + com->recordings().first()->filename();
    filename = filename.replace(".wav", m_alignerOutputFilenameSuffix + ".TextGrid");
    PraatTextGrid::save(filename, txgAlign);

    return result;
}

void EasyAlignBasic::runEasyAlign(CorpusCommunication *com)
{
    QString appPath = QCoreApplication::applicationDirPath();
    QString script = appPath + "/plugins/easyalign/align_sound.praat";
    QStringList scriptArguments;
    QString filenameSound = com->basePath() + "/" + com->recordings().first()->filename();
    QString filenameTextgrid = QString(filenameSound).replace(".wav", ".TextGrid");
    filenameTextgrid = filenameTextgrid.replace(".TextGrid", m_alignerOutputFilenameSuffix + ".TextGrid");
    QString preciseEndpoint = (m_preciseUtteranceBoundaries) ? "1" : "0";
    QString considerStar = "1";
    QString allowElision = "1";

    scriptArguments << filenameSound << filenameTextgrid <<
                       m_tiernameTranscription << m_tiernamePhonetisation << "1" << "fra" << "}-';(),.?" <<
                       preciseEndpoint << considerStar << allowElision << "0" << "0" << "0";
    executePraatScript(script, scriptArguments);
}

QString EasyAlignBasic::postAlignment(CorpusCommunication *com, QPointer<AnnotationTierGroup> tiers, bool fakeAlignment)
{
    QString result = "Post-processing EasyAlign results";

    AnnotationTierGroup *txgAlign = new AnnotationTierGroup(this);
    QString filename = com->basePath() + "/" + com->recordings().first()->filename();
    filename = filename.replace(".wav", m_alignerOutputFilenameSuffix + ".TextGrid");
    PraatTextGrid::load(filename, txgAlign);

    // Find the input tiers
    IntervalTier *tier_selection = txgAlign->getIntervalTierByName(m_tiernameSelection); if (!tier_selection) return "tier not found 1";
    IntervalTier *tier_transcription = txgAlign->getIntervalTierByName(m_tiernameTranscription); if (!tier_transcription) return "tier not found 2";
    IntervalTier *tier_words = txgAlign->getIntervalTierByName("words"); if (!tier_words) return "tier not found 3";
    IntervalTier *tier_phones = txgAlign->getIntervalTierByName("phones"); if (!tier_phones) return "tier not found 4";
    IntervalTier *tier_wordsT = txgAlign->getIntervalTierByName("wordsT"); if (!tier_wordsT) return "tier not found 5";
    IntervalTier *tier_phonesT = txgAlign->getIntervalTierByName("phonesT"); if (!tier_phonesT) return "tier not found 6";

    IntervalTier *tier_wordsOrig = tiers->getIntervalTierByName("tok_min");
    QList<Interval *> list_new_phones;
    QList<Interval *> list_new_words;

    foreach (Interval *segment, tier_transcription->intervals()) {
        if (segment->text() == "_") continue;
        Interval *selection = tier_selection->intervalAtTime(segment->tCenter());
        if (selection->text() == m_filterSelection) {
            QList<Interval *> words = tier_words->getIntervalsContainedIn(segment);
            if (    (words.count() == 0) ||
                    (words.count() == 1 && (words.at(0)->text() == "_")) ||
                    (words.count() == 1 && (words.at(0)->text().trimmed().isEmpty()))   ) {
                list_new_words << tier_wordsT->getIntervalsContainedIn(segment);
                list_new_phones << tier_phonesT->getIntervalsContainedIn(segment);
            }
            else {
                list_new_words << words;
                list_new_phones << tier_phones->getIntervalsContainedIn(segment);
            }
        }
        else {
            if (tier_wordsOrig)
                list_new_words << tier_wordsOrig->getIntervalsContainedIn(segment);
        }
    }
    IntervalTier *tier_phonesN = new IntervalTier("phones", txgAlign->tMin(), txgAlign->tMax(), list_new_phones);
    IntervalTier *tier_wordsN = new IntervalTier("words", txgAlign->tMin(), txgAlign->tMax(), list_new_words);
    txgAlign->removeTierByName("phones"); txgAlign->removeTierByName("phonesT");
    txgAlign->removeTierByName("words"); txgAlign->removeTierByName("wordsT");
    txgAlign->removeTierByName("phono");
    txgAlign->insertTier(0, tier_phonesN);
    txgAlign->insertTier(1, tier_wordsN);
    PraatTextGrid::save(filename, txgAlign);
    return result;
}

void EasyAlignBasic::runSyllabify(CorpusCommunication *com)
{
    QString appPath = QCoreApplication::applicationDirPath();
    QString script = appPath + "/plugins/easyalign/syllabify2.praat";
    QStringList scriptArguments;
    QString filenameTextgrid = com->basePath() + "/" + com->recordings().first()->filename();
    filenameTextgrid = filenameTextgrid.replace(".wav", m_alignerOutputFilenameSuffix + ".TextGrid");
    scriptArguments << filenameTextgrid;
    executePraatScript(script, scriptArguments);
}

QString EasyAlignBasic::mergeFiles(CorpusCommunication *com)
{
    QString result = "Merging files on " + com->ID();
//    foreach (CorpusAnnotation *annot, com->annotations()) {
//        AnnotationObject *obj = annot->getAnnotation();
//        if (!obj || obj->fileType() != AnnotationObject::FileType_TierGroup) continue;
//        AnnotationTierGroup *txg = qobject_cast<AnnotationTierGroup *>(obj);
//        if (!txg) continue;

//        AnnotationTierGroup *txgMergeSource = new AnnotationTierGroup(this);
//        QString filename = com->basePath() + "/" + annot->filename();
//        filename = filename.replace(".TextGrid", m_alignerOutputFilenameSuffix + ".TextGrid");
//        PraatTextGrid::load(filename, txgMergeSource);

//        // Merge driver
//        IntervalTier *tier_selection = txg->getIntervalTierByName("selection");
//        // Original tiers
//        QStringList original_tier_names;
//        original_tier_names << txg->tierNames(); // all

//        QHash<QString, IntervalTier *> original_tiers;
//        QHash<QString, QList<Interval *> > merged_interval_lists;
//        foreach (QString original_tier_name, original_tier_names) {
//            IntervalTier *tier = txg->getIntervalTierByName(original_tier_name);
//            if (!tier) return QString("Tier not found in merge destination file: %1").arg(original_tier_name);
//            original_tiers.insert(original_tier_name, tier);
//            merged_interval_lists.insert(original_tier_name, QList<Interval *>());
//        }
//        // Tiers to merge into
//        QStringList mergesrc_tier_names;
//        mergesrc_tier_names << "phones" << "syll" << "words" << "ortho" << "tok-min" << "pos-min" << "disfluency" << "tok-mwu" << "pos-mwu" << "discourse";
//        QHash<QString, IntervalTier *> mergesrc_tiers;
//        foreach (QString mergesrc_tier_name, mergesrc_tier_names) {
//            IntervalTier *tier = txgMergeSource->getIntervalTierByName(mergesrc_tier_name);
//            if (!tier) return QString("Tier not found in merge source file: %1").arg(mergesrc_tier_name);
//            mergesrc_tiers.insert(mergesrc_tier_name, tier);
//        }

//        foreach (Interval *selection, tier_selection->intervals()) {
//            if (selection->text() == "***") {
//                foreach (QString tier_name, original_tier_names) {
//                    if (mergesrc_tiers.contains(tier_name)) {
//                        merged_interval_lists[tier_name].append(mergesrc_tiers.value(tier_name)->getIntervalsContainedIn(selection));
//                    }
//                    else {
//                        if (tier_name == "prom" || tier_name == "clash") {
//                            foreach (Interval *syll, mergesrc_tiers.value("syll")->getIntervalsContainedIn(selection)) {
//                                merged_interval_lists[tier_name].append(
//                                            new Interval(syll->xMin(), syll->xMax(), ""));
//                            }
//                        }
//                        else
//                            merged_interval_lists[tier_name].append(original_tiers.value(tier_name)->getIntervalsContainedIn(selection));
//                    }
//                }
//            }
//            else {
//                foreach (QString tier_name, original_tier_names) {
//                    merged_interval_lists[tier_name].append(original_tiers.value(tier_name)->getIntervalsContainedIn(selection));
//                }
//            }
//        }

//        AnnotationTierGroup *txgProcessed = new AnnotationTierGroup(this);
//        QString filenameProcessed = com->basePath() + "/" + annot->filename();
//        filenameProcessed = filenameProcessed.replace(".TextGrid", "_processed.TextGrid");
//        foreach (QString tier_name, original_tier_names) {
//            txgProcessed->addTier(new IntervalTier(tier_name, txg->xMin(), txg->xMax(), merged_interval_lists.value(tier_name)));
//        }
//        // txgProcessed->addTier(tier_selection);
//        PraatTextGrid::save(filenameProcessed, txgProcessed);
//    }
    return result;
}

QString EasyAlignBasic::quickScript(CorpusCommunication *com)
{
    QString ret;
//    foreach (CorpusAnnotation *annot, com->annotations()) {
//        AnnotationObject *obj = annot->getAnnotation();
//        if (!obj || obj->fileType() != AnnotationObject::FileType_TierGroup) continue;
//        AnnotationTierGroup *txg = qobject_cast<AnnotationTierGroup *>(obj);
//        if (!txg) continue;

//        IntervalTier *tier_syll = txg->getIntervalTierByIndex(1);
//        IntervalTier *tier_ortho = txg->getIntervalTierByIndex(4);
//        foreach (Interval *ortho, tier_ortho->intervals()) {
//            if (ortho->text() == "_" || ortho->text() == "#")
//                continue;
//            ret.append(annot->ID()).append("\t");
//            ret.append(QString::number(ortho->duration().toDouble())).append("\t");
//            QList<Interval *> sylls = tier_syll->getIntervalsContainedIn(ortho);
//            ret.append(QString::number(sylls.count())).append("\t");
//            ret.append(ortho->text());
//            ret.append("\n");
//        }
//    }
    return ret;
}
