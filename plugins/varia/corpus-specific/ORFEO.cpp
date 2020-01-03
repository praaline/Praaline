#include <QDebug>
#include <QString>
#include <QList>
#include <QMap>
#include <QHash>
#include <QPointer>
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>
#include <QMutex>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/PointTier.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/SequenceTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/interfaces/tei/TEIHeader.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "pnlib/asr/phonetiser/DictionaryPhonetiser.h"
#include "pnlib/asr/htk/HTKForcedAligner.h"
#include "pnlib/asr/syllabifier/SyllabifierEasy.h"
using namespace Praaline::ASR;

#include "ORFEO.h"

struct ORFEOData {
    DictionaryPhonetiser phonetiser;
    HTKForcedAligner aligner;
    SyllabifierEasy syllabifier;
};

ORFEO::ORFEO() : d(new ORFEOData())
{
    d->phonetiser.readPhoneticDictionary("/mnt/hgfs/DATA/PFCALIGN/phonetisation/fr.dict");
    d->phonetiser.readPhonemeTranslation("/mnt/hgfs/DATA/PFCALIGN/phonetisation/sphinx_to_sampa.json");
    d->phonetiser.readAdditionalPhoneticDictionary(QDir::homePath() + "/Dropbox/CORPORA/cefc_extra_phon.txt");
}

ORFEO::~ORFEO()
{
    delete d;
}

QString ORFEO::readOrfeoFile(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    if (com->recordingsCount() == 0) return "Error: Communication does not contain recorings";
    QString filename = QString(com->recordings().first()->filePath()).replace(".wav", ".orfeo");
    // Open ORFEO file
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return "Error opening ORFEO file";
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    ret.append(QString("Communication ID: %1\n").arg(com->ID()));
    // Data structures
    QHash<QString, QList<Interval *> > orfeoData;
    QHash<QString, RealTime> last_tMax; // per speakerID
    QString last_speakerID;
    bool inSentence(false);
    int sentenceID(0);
    // Process file
    do {
        QString line = stream.readLine();
        line = line.trimmed();
        if (line.startsWith("# sent_id =")) {
            sentenceID = QString(line).split("-").last().toInt();
            inSentence = false;
        }
        if (line.startsWith("#")) continue;
        if (line.isEmpty()) {
            inSentence = false;
            continue;
        }
        QStringList fields = line.split("\t");
        if (fields.count() < 13) {
            qDebug() << "Error: line with less than 13 fields";
        }
        // Read ORFEO token
        int ID = fields.at(0).toInt();
        QString form = QString(fields.at(1)).replace("#", " ");
        QString lemma = fields.at(2);
        QString UPOSTag = fields.at(3);
        QString XPOSTag = fields.at(4);
        QString feats = fields.at(5);
        int head = fields.at(6).toInt();
        QString deprel = fields.at(7);
        QString deps = fields.at(8);
        QString misc = fields.at(9);
        double tMin_s = fields.at(10).toDouble();
        double tMax_s = fields.at(11).toDouble();
        QString speakerID = fields.at(12);
        RealTime tMin = RealTime::fromSeconds(tMin_s);
        RealTime tMax = RealTime::fromSeconds(tMax_s);
        // Fix timecodes (ORFEO files contain errors)
        bool tokenHasTimeCodeError(false);
        if ((tMax <= tMin) || (tMin <= last_tMax[speakerID])) {
            tMin = last_tMax[speakerID];
            tMax = tMin + RealTime::fromMilliseconds(20);
            tokenHasTimeCodeError = true;
        }
        // Remove fake micro-pauses
        if (tMin - last_tMax[speakerID] <= RealTime::fromMilliseconds(200)) {
            tMin = last_tMax[speakerID];
        }
        // Update state
        last_speakerID = speakerID;
        last_tMax[speakerID] = tMax;
        // Transcription conventions: use / for false starts (easier for DisMo)
        if (form.endsWith("-~")) form = form.replace("-~", "/");
        if (form.endsWith("~"))  form = form.replace("~", "/");
        form = form.replace(" /", "/");
        // Create interval for ORFEO token
        Interval *intv = new Interval(tMin, tMax, form);
        intv->setAttribute("orfeo_token_id", ID);
        intv->setAttribute("orfeo_lemma", lemma);
        intv->setAttribute("orfeo_UPOS", UPOSTag);
        intv->setAttribute("orfeo_XPOS", XPOSTag);
        intv->setAttribute("orfeo_feats", feats);
        intv->setAttribute("orfeo_head", head);
        intv->setAttribute("orfeo_deprel", deprel);
        intv->setAttribute("orfeo_deps", deps);
        intv->setAttribute("orfeo_misc", misc);
        if (tokenHasTimeCodeError) intv->setAttribute("orfeo_timecode_error", true);
        if (!inSentence) {
            intv->setAttribute("orfeo_seg", "[");
            if (orfeoData.contains(speakerID)) {
                if (orfeoData[speakerID][orfeoData[speakerID].count() - 1]->attribute("orfeo_seg").toString() == "[") {
                    orfeoData[speakerID][orfeoData[speakerID].count() - 1]->setAttribute("orfeo_seg", "[]");
                } else {
                    orfeoData[speakerID][orfeoData[speakerID].count() - 1]->setAttribute("orfeo_seg", "]");
                }
            }
            inSentence = true;
        }
        intv->setAttribute("orfeo_sent_id", sentenceID);
        // Include token in the list of tokens of the appropriate speaker
        orfeoData[speakerID].append(intv);
    } while (!stream.atEnd());
    file.close();
    // Create tiers and save them into the database
    AnnotationTierGroup *txg = new AnnotationTierGroup();
    foreach (QString speakerID, orfeoData.keys()) {
        // Finish up segmentation
        if (orfeoData[speakerID][orfeoData[speakerID].count() - 1]->attribute("orfeo_seg").toString() == "[") {
            orfeoData[speakerID][orfeoData[speakerID].count() - 1]->setAttribute("orfeo_seg", "[]");
        } else {
            orfeoData[speakerID][orfeoData[speakerID].count() - 1]->setAttribute("orfeo_seg", "]");
        }
        // Create and save tier
        IntervalTier *tier = new IntervalTier("orfeo_token", orfeoData.value(speakerID));
        com->repository()->annotations()->saveTier(com->ID(), speakerID, tier);
        // Add tier to TextGrid (in addition to Praaline's database)
        tier->setName(speakerID);
        txg->addTier(tier);
        ret.append(QString("\tSpeaker ID: %1\t saved %2 tokens\n").arg(speakerID).arg(orfeoData.value(speakerID).count()));
    }
    PraatTextGrid::save(QString(com->recordings().first()->filePath()).replace(".wav", ".TextGrid"), txg);
    delete txg;
    return ret.trimmed();
}

#include "pncore/diff/DiffIntervals.h"

QString ORFEO::mapTokensToDisMo(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_orfeo = tiers->getIntervalTierByName("orfeo_token");
            if (!tier_orfeo) { ret.append("No ORFEO tier\n"); continue; }
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) { ret.append("No tok_min tier\n"); continue; }

            tier_orfeo->fillEmptyWith("", "_");

            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                tok_min->setAttribute("orfeo_token_id", "");
                tok_min->setAttribute("orfeo_lemma", "");
                tok_min->setAttribute("orfeo_UPOS", "");
                tok_min->setAttribute("orfeo_XPOS", "");
                tok_min->setAttribute("orfeo_feats", "");
                tok_min->setAttribute("orfeo_head", "");
                tok_min->setAttribute("orfeo_deprel", "");
                tok_min->setAttribute("orfeo_deps", "");
                tok_min->setAttribute("orfeo_misc", "");
                tok_min->setAttribute("orfeo_sent_id", "");
                tok_min->setAttribute("orfeo_seg", "");
                tok_min->setAttribute("orfeo_tMin", "");
                tok_min->setAttribute("orfeo_tMax", "");
                tok_min->setAttribute("orfeo_timecode_error", "");
            }

            dtl::Ses<Interval *>::sesElemVec sesSequence = DiffIntervals::intervalDiff(
                        tier_orfeo->intervals(), tier_tok_min->intervals(), false, "", "").getSes().getSequence();
            dtl::Ses<Interval *>::sesElem elem;

            // Update tok_min from orfeo_token
            int i_orfeo(0), i_tokmin(0);
            bool firstADD(true);
            foreach (elem, sesSequence) {
                if (i_orfeo >= tier_orfeo->count()) break;
                if (i_tokmin >= tier_tok_min->count()) break;
                Interval *orfeo = tier_orfeo->at(i_orfeo);
                Interval *tok_min = tier_tok_min->at(i_tokmin);
                if ((elem.second.type == dtl::SES_COMMON) || ((elem.second.type == dtl::SES_ADD) && firstADD)) {
                    if (!tok_min->isPauseSilent()) {
                        tok_min->setAttribute("orfeo_token_id", orfeo->attribute("orfeo_token_id"));
                        tok_min->setAttribute("orfeo_lemma", orfeo->attribute("orfeo_lemma"));
                        tok_min->setAttribute("orfeo_UPOS", orfeo->attribute("orfeo_UPOS"));
                        tok_min->setAttribute("orfeo_XPOS", orfeo->attribute("orfeo_XPOS"));
                        tok_min->setAttribute("orfeo_feats", orfeo->attribute("orfeo_feats"));
                        tok_min->setAttribute("orfeo_head", orfeo->attribute("orfeo_head"));
                        tok_min->setAttribute("orfeo_deprel", orfeo->attribute("orfeo_deprel"));
                        tok_min->setAttribute("orfeo_deps", orfeo->attribute("orfeo_deps"));
                        tok_min->setAttribute("orfeo_misc", orfeo->attribute("orfeo_misc"));
                        tok_min->setAttribute("orfeo_sent_id", orfeo->attribute("orfeo_sent_id"));
                        tok_min->setAttribute("orfeo_seg", "");
                        // Segmentation needs special treatment
                        if (tok_min->text() != orfeo->text()) {
                            // Open segmentation in ORFEO token split into multiple DisMo tokens
                            if (orfeo->attribute("orfeo_seg").toString().contains("[")) {
                                tok_min->setAttribute("orfeo_seg", "[");
                            }
                        } else {
                            // Normal copy (ORFEO token == DisMo token)
                            tok_min->setAttribute("orfeo_seg", orfeo->attribute("orfeo_seg"));
                        }
                        tok_min->setAttribute("orfeo_tMin", QString("%1").arg(orfeo->tMin().toNanoseconds()));
                        tok_min->setAttribute("orfeo_tMax", QString("%1").arg(orfeo->tMax().toNanoseconds()));
                        tok_min->setAttribute("orfeo_timecode_error", orfeo->attribute("orfeo_timecode_error"));
                    } else {
                        tok_min->setAttribute("orfeo_UPOS", "_");
                    }
                    // advance
                    if (elem.second.type == dtl::SES_COMMON) {
                        i_orfeo++;
                        i_tokmin++;
                        firstADD = true;
                    }
                    else if (elem.second.type == dtl::SES_ADD) {
                        i_tokmin++;
                        firstADD = false;
                    }
                }
                else if ((elem.second.type == dtl::SES_ADD) && (!firstADD)) {
                    if (tok_min->isPauseSilent()) {
                        tok_min->setAttribute("orfeo_UPOS", "_");
                    } else {
                        tok_min->setAttribute("orfeo_UPOS", "***");
                        tok_min->setAttribute("orfeo_sent_id", orfeo->attribute("orfeo_sent_id"));
                        // Close segmentation in ORFEO token split into multiple DisMo tokens
                        if (orfeo->text().endsWith(tok_min->text())) {
                            if (orfeo->attribute("orfeo_seg").toString().contains("]"))
                                tok_min->setAttribute("orfeo_seg", "]");
                        }
                    }
                    i_tokmin++;
                }
                else if (elem.second.type == dtl::SES_DELETE) {
                    i_orfeo++;
                }
            }

            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_min);
            ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

QString ORFEO::phonetise(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) { ret.append("No tok_min tier\n"); continue; }
            // Phonetise each token
            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                if (tok_min->isPauseSilent()) {
                    tok_min->setAttribute("phonetisation", "_");
                    continue;
                }
                QString word = tok_min->text().toLower();
                if (word.contains(" ")) word.replace(" ", "_");
                QString phonetisation = d->phonetiser.phonetiseToken(word);
                if (phonetisation.isEmpty()) phonetisation = "@";
                tok_min->setAttribute("phonetisation", phonetisation);
            }
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_min);
            ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

QString ORFEO::phonetiseOOV()
{
    d->phonetiser.writeTokensOutOfVocabulary(QDir::homePath() + "/Dropbox/CORPORA/cefc_extra_phon_in.txt");
    return "OK";
}

QString ORFEO::createUtterances(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_utterance = tiers->getIntervalTierByName("tok_min");
            if (!tier_utterance) { ret.append("No tok_min tier\n"); continue; }
            tier_utterance->mergeContiguousAnnotations(QStringList() << "_", " ");
            tier_utterance->setName("utterance");
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_utterance);
            ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}


QString ORFEO::align(Praaline::Core::CorpusCommunication *com)
{
    static QMutex mutex;
    QElapsedTimer timer;
    QString ret;
    if (!com) return "Error: No communication";
    if (com->recordingsCount() == 0) return "Error: Communication does not contain recorings";

    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        mutex.lock();
        tiersAll = com->repository()->annotations()->
                getTiersAllSpeakers(com->ID(), QStringList() << "utterance" << "tok_min" << "phone" << "syll");
        mutex.unlock();
        int numberOfUtterances(0);
        timer.start();
        // Praat textgrid for the results
        foreach (QString speakerID, tiersAll.keys()) {
            // Check if already aligned
            QString filenameTextGrid = QString(com->recordings().first()->filePath()).replace(".wav", "_align_%1.TextGrid").arg(speakerID);
            if (QFile::exists(filenameTextGrid)) continue;

            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_utterance = tiers->getIntervalTierByName("utterance");
            if (!tier_utterance) continue;
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;
            IntervalTier *tier_phone = tiers->getIntervalTierByName("phone");
            if (!tier_phone) tier_phone = new IntervalTier("phone", tier_utterance->tMin(), tier_utterance->tMax());
            IntervalTier *tier_syll = tiersAll.value(speakerID)->getIntervalTierByName("syll");
            if (!tier_syll) tier_syll = new IntervalTier("syll", tier_utterance->tMin(), tier_utterance->tMax());

            // Adjust utterance boundaries for alignment
            for (int i = 1; i < tier_utterance->count() - 1; ++i) {
                if (tier_utterance->at(i)->isPauseSilent()) continue;

                RealTime delta = RealTime::fromMilliseconds(200);
                if (tier_utterance->at(i - 1)->duration() < delta) delta = tier_utterance->at(i - 1)->duration() / 2;
                tier_utterance->moveBoundary(i, tier_utterance->at(i)->tMin() - delta);
                delta = RealTime::fromMilliseconds(200);
                if (tier_utterance->at(i + 1)->duration() < delta) delta = tier_utterance->at(i + 1)->duration() / 2;
                tier_utterance->moveBoundary(i+1, tier_utterance->at(i)->tMax() + delta);
            }

            int indexUtterance = tier_utterance->count() - 1;
            while (indexUtterance >= 0) {
                if (tier_utterance->at(indexUtterance)->isPauseSilent()) { indexUtterance--; continue; }
                QString alignerOutput;
                bool result(false);
                result = d->aligner.alignUtterance(com->recordings().first()->filePath(), tier_utterance, indexUtterance,
                                                   tier_tok_min, tier_phone, alignerOutput);
                // if (!result) qDebug() << indexUtterance << result << tier_utterance->at(indexUtterance)->text() << alignerOutput;
                qDebug() << annotationID << speakerID << indexUtterance;
                indexUtterance--;
                numberOfUtterances++;
            }
            tier_utterance->fillEmptyWith("", "_"); tier_utterance->mergeIdenticalAnnotations("_");
            tier_tok_min->fillEmptyWith("", "_");   tier_tok_min->mergeIdenticalAnnotations("_");
            tier_phone->fillEmptyWith("", "_");     tier_phone->mergeIdenticalAnnotations("_");
            // Syllabify
            d->syllabifier.syllabify(tier_phone, tier_syll, tier_utterance->tMin(), tier_utterance->tMax());
            tier_syll->fillEmptyWith("", "_");  tier_syll->mergeIdenticalAnnotations("_");
            // Save to database
            mutex.lock();
            com->corpus()->repository()->annotations()->saveTier(annotationID, speakerID, tier_phone);
            com->corpus()->repository()->annotations()->saveTier(annotationID, speakerID, tier_syll);
            com->corpus()->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_min);
            // com->corpus()->repository()->annotations()->saveTier(annotationID, speakerID, tier_utterance);
            mutex.unlock();
            // Save Praat TextGrid
            AnnotationTierGroup *txg = new AnnotationTierGroup();
            txg->addTier(tier_phone);
            txg->addTier(tier_syll);
            txg->addTier(tier_tok_min);
            txg->addTier(tier_utterance);
            PraatTextGrid::save(filenameTextGrid, txg);
            delete txg;
        }
        // User output
        double timeForAnnotation = (static_cast<double>(timer.elapsed()) / 1000.0); // in seconds
        double speed = timeForAnnotation / com->recordings().first()->duration().toDouble();
        double utterancesPerSecond = static_cast<double>(numberOfUtterances) / timeForAnnotation;
        ret.append(QString("%1\t").arg(annotationID));
        ret.append(QString("Utt:\t%1\t").arg(numberOfUtterances));
        ret.append(QString("Time:\t%1\t").arg(timeForAnnotation));
        ret.append(QString("Speed (xRT):\t%1\t").arg(speed));
        ret.append(QString("Utt/sec:\t%1\n").arg(utterancesPerSecond));
        // qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

QString ORFEO::createSentenceUnits(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) { ret.append("No tok_min tier\n"); continue; }
            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
            if (!tier_syll) { ret.append("No syll tier\n"); continue; }

            QList<Interval *> intervals_sentences;
            RealTime tMin, tMax;
            int start(-1);
            for (int i = 0; i < tier_tok_min->count(); ++i) {
                Interval *token = tier_tok_min->at(i);
                if (token->isPauseSilent()) continue;
                if (token->text().startsWith("(") || token->text().endsWith(")")) continue;
                // otherwise
                if (start < 0) start = i;
                int current_sent_id(tier_tok_min->at(i)->attribute("orfeo_sent_id").toInt());
                int next_sent_id(current_sent_id);
                int j = i + 1;
                while (j < tier_tok_min->count()) {
                    if (tier_tok_min->at(j)->attribute("orfeo_sent_id").toInt() != 0) {
                        next_sent_id = tier_tok_min->at(j)->attribute("orfeo_sent_id").toInt();
                        break;
                    }
                    j++;
                }
                if ((i == tier_tok_min->count() - 1) || (current_sent_id != next_sent_id)) {
                    tMin = tier_tok_min->at(start)->tMin();
                    tMax = tier_tok_min->at(i)->tMax();
                    QString text = tier_tok_min->getIntervalsTextContainedIn(tMin, tMax);
                    text = text.replace("  ", " ").replace("  ", " ");
                    Interval *sentence_unit(new Interval(tMin, tMax, text));
                    sentence_unit->setAttribute("orfeo_sent_id", token->attribute("orfeo_sent_id").toInt());
                    // count syllables inside sentence
                    int countSyll(0), countPauses(0);
                    foreach (Interval *syll, tier_syll->getIntervalsOverlappingWith(sentence_unit)) {
                        if (syll->isPauseSilent()) countPauses++; else countSyll++;
                    }
                    sentence_unit->setAttribute("count_sil_pauses", countPauses);
                    sentence_unit->setAttribute("count_syllables", countSyll);
                    // Add sentence
                    intervals_sentences << sentence_unit;
                    // Next
                    start = -1;
                }
            }
            IntervalTier *tier_sentences = new IntervalTier("sentence_unit", intervals_sentences);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_sentences);
            ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

QString ORFEO::createMajorProsodicUnits(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) { ret.append("No tok_min tier\n"); continue; }
            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
            if (!tier_syll) { ret.append("No syll tier\n"); continue; }

            QList<Interval *> intervals_prosodic_units;
            RealTime tMin, tMax;
            int start(-1);
            for (int i = 0; i < tier_tok_min->count(); ++i) {
                Interval *token = tier_tok_min->at(i);
                if (token->isPauseSilent()) continue;
                if (token->text().startsWith("(") || token->text().endsWith(")")) continue;
                if (token->text() == "ts" || token->text() == "ps") continue;
                // otherwise
                if (start < 0) start = i;
                // check to see if end of a sequence
                QList<Interval *> syllables = tier_syll->getIntervalsOverlappingWith(token);
                if (syllables.isEmpty()) continue;
                Interval *lastSyll = syllables.last();
                if (lastSyll->attribute("promise_boundary").toString() == "B3") {
                    tMin = tier_tok_min->at(start)->tMin();
                    tMax = tier_tok_min->at(i)->tMax();
                    QString text = tier_tok_min->getIntervalsTextContainedIn(tMin, tMax);
                    text = text.replace("  ", " ").replace("  ", " ");
                    Interval *prosodic_unit(new Interval(tMin, tMax, text));
                    // count syllables inside prosodic unit
                    int countSyll(0), countPauses(0);
                    foreach (Interval *syll, tier_syll->getIntervalsOverlappingWith(prosodic_unit)) {
                        if (syll->isPauseSilent()) countPauses++; else countSyll++;
                    }
                    prosodic_unit->setAttribute("count_sil_pauses", countPauses);
                    prosodic_unit->setAttribute("count_syllables", countSyll);
                    // Add prosodic unit
                    intervals_prosodic_units << prosodic_unit;
                    // Next
                    start = -1;
                }
            }

            IntervalTier *tier_prosodic_units = new IntervalTier("prosodic_unit", intervals_prosodic_units);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_prosodic_units);
            ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

#include "IntervalTierCombinations.h"

QString ORFEO::createCombinedUnits(Praaline::Core::CorpusCommunication *com)
{
    IntervalTierCombinations combine;
    combine.setIntervalsLevelA("prosodic_unit");
    combine.setIntervalsLevelB("sentence_unit");
    combine.setIntervalsLevelCombined("prosody_syntax");
    return combine.combineIntervalTiers(com);
}




QString ORFEO::readMetadata(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    if (com->recordingsCount() == 0) return "Error: Communication does not contain recorings";
    QString filename = QString(com->recordings().first()->filePath()).replace(".wav", ".xml");
    // Origin corpus
    QString corpusOrigin = QString(filename).remove("/" + com->ID() + ".xml").split("/").last();
    com->setProperty("corpus_origin", corpusOrigin);
    TEIHeader tei;
    if (TEIHeader::load(filename, tei)) {
        foreach (TEIHeader::CatRef catRef, tei.profileDesc.textClass) {
            QString attributeID = catRef.corresp; if (attributeID == "type") attributeID = "com_type";
            com->setProperty(attributeID, catRef.target);
        }
        ret.append(QString("%1").arg(com->ID()));
    }
    return ret;
}

QString ORFEO::updateSoundFiles(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    if (com->recordingsCount() == 0) return "Error: Communication does not contain recorings";
    QString filename = com->recordings().first()->filePath();
    QString filename_processed = QString(filename).replace(".wav", "_processed.wav");
    if (QFile::exists(filename_processed)) {
        QFile::remove(filename);
        QFile::rename(filename_processed, filename);
        ret.append(QString("%1 -> %2").arg(filename_processed).arg(filename));
    }
    return ret;
}
