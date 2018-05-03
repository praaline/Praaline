#include <QDebug>
#include <QPointer>
#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QMapIterator>
#include <QFile>
#include <QTextStream>
#include <math.h>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/statistics/Measures.h"
#include "pncore/statistics/StatisticalSummary.h"
using namespace Praaline::Core;

#include "PhonogenreDiscourseMarkers.h"

PhonogenreDiscourseMarkers::PhonogenreDiscourseMarkers()
{

}

// static
QString PhonogenreDiscourseMarkers::readBackAnnotations(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    if (!com->corpus()) return ret;
    QString path = QDir::homePath() + "/Dropbox/CORPORA/Phonogenre/DiscourseMarkerAnnotation/";
    QStringList filenames; filenames << "phonogenre_et.txt";
    foreach (QString filename, filenames) {
        QString line;
        QFile file(path + filename);
        if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
            ret.append("Error reading file ").append(filename); continue;
        }
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        do {
            line = stream.readLine();
            if (line.startsWith("#")) continue;
            QStringList fields = line.split("\t");
            QString annotationID = fields.at(2);
            QString speakerID = fields.at(3);
            IntervalTier *tier_tok_min = qobject_cast<IntervalTier *>(com->corpus()->repository()->annotations()->getTier(annotationID, speakerID, "tok_min"));
            if (!tier_tok_min) { ret.append("Error ").append(annotationID).append(" ").append(speakerID).append("\n"); continue; }
            int intervalNo = fields.at(6).toInt();
            if  (intervalNo <= 0 || intervalNo > tier_tok_min->count()) {
                ret.append("Error ").append(annotationID).append(" ").append(speakerID).append("\n");
                continue;
            }
            Interval *tok_min = tier_tok_min->interval(intervalNo - 1);
            QString text = fields.at(8);
            if (tok_min->text() != text) {
                ret.append("Error ").append(annotationID).append(" ").append(speakerID).append("\n");
                continue;
            }
            tok_min->setAttribute("DM", fields.at(10));
            tok_min->setAttribute("Non_DM", fields.at(11));
            com->corpus()->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_min);
            delete tier_tok_min;
        } while (!stream.atEnd());
        file.close();
        ret.append("Read filename ").append(filename).append("\n");
    }
    return ret;
}

// Gets the syllable index of: the target syllable (1st, 2nd...), of the left context syllable or of the
// right context syllable
int getSyllIndex(Interval *token, IntervalTier *tier_syll, int left, int target, int right)
{
    if (!token) return -1;
    if (!tier_syll) return -1;
    QPair<int, int> targetSyllIndices = tier_syll->getIntervalIndexesOverlappingWith(token);
    int syllIndex(0); int contextSyllCount(0);
    if (target > 0) {
        for (int syllIndex = targetSyllIndices.first; syllIndex <= targetSyllIndices.second; ++syllIndex) {
            if ((syllIndex < 0) || (syllIndex >= tier_syll->count())) return -1;
            if (contextSyllCount + 1 == target) return syllIndex;
            contextSyllCount++;
        }
    }
    else if (left > 0) {
        syllIndex = targetSyllIndices.first - 1;
        while ((syllIndex >= 0) && (contextSyllCount <= left)) {
            Interval *syll = tier_syll->at(syllIndex);
            if (!syll->isPauseSilent()) {
                if (contextSyllCount + 1 == left) return syllIndex;
                contextSyllCount++;
            }
            syllIndex--;
        }
    }
    else if (right > 0) {
        syllIndex = targetSyllIndices.second + 1;
        while ((syllIndex < tier_syll->count()) && (contextSyllCount <= right)) {
            Interval *syll = tier_syll->at(syllIndex);
            if (!syll->isPauseSilent()) {
                if (contextSyllCount + 1 == right) return syllIndex;
                contextSyllCount++;
            }
            syllIndex++;
        }
    }
    return -1;
}

QString PhonogenreDiscourseMarkers::statistics(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    if (!com->corpus()) return ret;

    QStringList discourseMarkers; discourseMarkers << "alors" << "et";
    // Output
    QString path = QDir::homePath() + "/Dropbox/CORPORA/Phonogenre/DiscourseMarkerAnnotation/";
    QFile file(path + "prosodic_dms.txt");
    if ( !file.open( QIODevice::ReadWrite | QIODevice::Text ) ) return "Error writing output file";
    QTextStream out(&file);
    out.setCodec("UTF-8");
    QString sep = "\t";

    // Header
    out << "Genre" << sep << "AnnotationID" << sep << "SpeakerID" << sep << "Token" << sep;
    out << "Category" << sep << "SubCategory" << sep;
    out << "T1_duration" << sep << "T2_duration" << sep << "T1_f0_mean" << sep << "T2_f0_mean" << sep;
    out << "T1_intersyllab" << sep << "T2_intersyllab" << sep << "R1_intersyllab" << sep;
    out << "L3_durRel30" << sep << "L2_durRel30" << sep << "L1_durRel30" << sep;
    out << "T1_durRel30" << sep << "T2_durRel30" << sep;
    out << "R1_durRel30" << sep << "R2_durRel30" << sep << "R3_durRel30" << sep;
    out << "L1_prom" << sep << "T1_prom" << sep << "T2_prom" << sep << "R1_prom" << sep;
    out << "L1_boundary" << sep << "T1_boundary" << sep << "T2_boundary" << sep << "R1_boundary" << sep;
    out << "L3_tone" << sep << "L3_tonemvt" << sep << "L2_tone" << sep << "L2_tonemvt" << sep << "L1_tone" << sep << "L1_tonemvt" << sep;
    out << "T1_tone" << sep << "T1_tonemvt" << sep << "T2_tone" << sep << "T2_tonemvt" << sep;
    out << "R1_tone" << sep << "R1_tonemvt" << sep << "R2_tone" << sep << "R2_tonemvt" << sep << "R3_tone" << sep << "R3_tonemvt" << sep;
    out << "LocalSpeechRateBeforeMD" << sep << "LocalSpeechRateAfterMD" << sep << "PauseBeforeMD" << sep << "PauseAfterMD" << sep;
    out << "L5" << sep << "L4" << sep << "L3" << sep << "L2" << sep << "L1" << sep << "T1" << sep << "T2" << sep;
    out << "R1" << sep << "R2" << sep << "R3" << sep << "R4" << sep << "R5" << "\n";


    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    QPointer<Corpus> corpus = com->corpus();
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_min");
                if (!tier_tokens) continue;
                IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
                if (!tier_syll) continue;
                // Interpolate syllable pitch values
                for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
                    Interval *syll = tier_syll->interval(isyll);
                    // Smoothing for non-stylised syllables
                    if (!syll->isPauseSilent() && syll->attribute("f0_min").toInt() == 0) {
                        syll->setAttribute("f0_min", Measures::mean(tier_syll, "f0_min", isyll, 4, 4, true, "f0_min"));
                        syll->setAttribute("f0_max", Measures::mean(tier_syll, "f0_max", isyll, 4, 4, true, "f0_min"));
                        syll->setAttribute("f0_mean", Measures::mean(tier_syll, "f0_mean", isyll, 4, 4, true, "f0_min"));
                        syll->setAttribute("int_peak", Measures::mean(tier_syll, "int_peak", isyll, 4, 4, true, "f0_min"));
                    }
                }
                // For each token
                for (int itok = 0; itok < tier_tokens->count(); ++itok) {
                    Interval *token = tier_tokens->interval(itok);
                    if (!discourseMarkers.contains(token->text())) continue;
                    out << com->ID().left(3) << sep; // genre
                    out << annotationID << sep << speakerID << sep;
                    out << token->text() << sep;    // token
                    out << token->attribute("DM").toString() << sep;  // category DM or CON
                    out << QString("%1 %2").arg(token->attribute("DM").toString()).arg(token->attribute("Non_DM").toString()).trimmed() << sep; // subcategory
                    // Syllables
                    Interval *L5(0); Interval *L4(0); Interval *L3(0); Interval *L2(0); Interval *L1(0);
                    Interval *T1(0); Interval *T2(0);
                    Interval *R1(0); Interval *R2(0); Interval *R3(0); Interval *R4(0); Interval *R5(0);
                    int indexL5 = getSyllIndex(token, tier_syll, 5, 0, 0); if (indexL5 >= 0) L5 = tier_syll->interval(indexL5);
                    int indexL4 = getSyllIndex(token, tier_syll, 4, 0, 0); if (indexL4 >= 0) L4 = tier_syll->interval(indexL4);
                    int indexL3 = getSyllIndex(token, tier_syll, 3, 0, 0); if (indexL3 >= 0) L3 = tier_syll->interval(indexL3);
                    int indexL2 = getSyllIndex(token, tier_syll, 2, 0, 0); if (indexL2 >= 0) L2 = tier_syll->interval(indexL2);
                    int indexL1 = getSyllIndex(token, tier_syll, 1, 0, 0); if (indexL1 >= 0) L1 = tier_syll->interval(indexL1);
                    int indexT1 = getSyllIndex(token, tier_syll, 0, 1, 0); if (indexT1 >= 0) T1 = tier_syll->interval(indexT1);
                    int indexT2 = getSyllIndex(token, tier_syll, 0, 2, 0); if (indexT2 >= 0) T2 = tier_syll->interval(indexT2);
                    int indexR1 = getSyllIndex(token, tier_syll, 0, 0, 1); if (indexR1 >= 0) R1 = tier_syll->interval(indexR1);
                    int indexR2 = getSyllIndex(token, tier_syll, 0, 0, 2); if (indexR2 >= 0) R2 = tier_syll->interval(indexR2);
                    int indexR3 = getSyllIndex(token, tier_syll, 0, 0, 3); if (indexR3 >= 0) R3 = tier_syll->interval(indexR3);
                    int indexR4 = getSyllIndex(token, tier_syll, 0, 0, 4); if (indexR4 >= 0) R4 = tier_syll->interval(indexR4);
                    int indexR5 = getSyllIndex(token, tier_syll, 0, 0, 5); if (indexR5 >= 0) R5 = tier_syll->interval(indexR5);
                    // Syllable measures
                    // stylised?
                    bool T1stylised = (T1) ? (T1->attribute("nucl_t1").toDouble() > 0.0) : false;
                    bool T2stylised = (T2) ? (T2->attribute("nucl_t1").toDouble() > 0.0) : false;
                    bool R1stylised = (R1) ? (R1->attribute("nucl_t1").toDouble() > 0.0) : false;
                    // duration of the MD
                    if (T1) out << T1->duration().toDouble() << "\t"; else out << "NA\t";
                    if (T2) out << T2->duration().toDouble() << "\t"; else out << "NA\t";
                    // pitch of the MD
                    if (T1stylised) out << T1->attribute("f0_mean").toDouble() << "\t"; else out << "NA\t";
                    if (T2stylised) out << T2->attribute("f0_mean").toDouble() << "\t"; else out << "NA\t";
                    // intersyllabic movement S1-->T1 and T1/T2-->R1
                    if (T1stylised) out << T1->attribute("intersyllab").toDouble() << "\t"; else out << "NA\t";
                    if (T2stylised) out << T2->attribute("intersyllab").toDouble() << "\t"; else out << "NA\t";
                    if (R1stylised) out << R1->attribute("intersyllab").toDouble() << "\t"; else out << "NA\t";
                    // relative duration (searching for lengthenings)
                    if (L3) out <<  Measures::relative(tier_syll, "duration", indexL3, 3, 0, true, "", false) << "\t"; else out << "NA\t";
                    if (L2) out <<  Measures::relative(tier_syll, "duration", indexL2, 3, 0, true, "", false) << "\t"; else out << "NA\t";
                    if (L1) out <<  Measures::relative(tier_syll, "duration", indexL1, 3, 0, true, "", false) << "\t"; else out << "NA\t";
                    if (T1) out <<  Measures::relative(tier_syll, "duration", indexT1, 3, 0, true, "", false) << "\t"; else out << "NA\t";
                    if (T2) out <<  Measures::relative(tier_syll, "duration", indexT2, 3, 0, true, "", false) << "\t"; else out << "NA\t";
                    if (R1) out <<  Measures::relative(tier_syll, "duration", indexR1, 3, 0, true, "", false) << "\t"; else out << "NA\t";
                    if (R1) out <<  Measures::relative(tier_syll, "duration", indexR2, 3, 0, true, "", false) << "\t"; else out << "NA\t";
                    if (R1) out <<  Measures::relative(tier_syll, "duration", indexR3, 3, 0, true, "", false) << "\t"    ; else out << "NA\t";
                    // prominence
                    if (L1) out << L1->attribute("promise_pos").toString() << "\t"; else out << "NA\t";
                    if (T1) out << T1->attribute("promise_pos").toString() << "\t"; else out << "NA\t";
                    if (T2) out << T2->attribute("promise_pos").toString() << "\t"; else out << "NA\t";
                    if (R1) out << R1->attribute("promise_pos").toString() << "\t"; else out << "NA\t";
                    // boundaries
                    if (L1) out << L1->attribute("promise_boundary").toString() << "\t"; else out << "NA\t";
                    if (T1) out << T1->attribute("promise_boundary").toString() << "\t"; else out << "NA\t";
                    if (T2) out << T2->attribute("promise_boundary").toString() << "\t"; else out << "NA\t";
                    if (R1) out << R1->attribute("promise_boundary").toString() << "\t"; else out << "NA\t";
                    // tonal annotation
                    if (L3) out << L3->attribute("tonal_label").toString() << "\t"; else out << "NA\t";
                    if (L3) out << L3->attribute("tonal_movement").toString() << "\t"; else out << "NA\t";
                    if (L2) out << L2->attribute("tonal_label").toString() << "\t"; else out << "NA\t";
                    if (L2) out << L2->attribute("tonal_movement").toString() << "\t"; else out << "NA\t";
                    if (L1) out << L1->attribute("tonal_label").toString() << "\t"; else out << "NA\t";
                    if (L1) out << L1->attribute("tonal_movement").toString() << "\t"; else out << "NA\t";
                    if (T1) out << T1->attribute("tonal_label").toString() << "\t"; else out << "NA\t";
                    if (T1) out << T1->attribute("tonal_movement").toString() << "\t"; else out << "NA\t";
                    if (T2) out << T2->attribute("tonal_label").toString() << "\t"; else out << "NA\t";
                    if (T2) out << T2->attribute("tonal_movement").toString() << "\t"; else out << "NA\t";
                    if (R1) out << R1->attribute("tonal_label").toString() << "\t"; else out << "NA\t";
                    if (R1) out << R1->attribute("tonal_movement").toString() << "\t"; else out << "NA\t";
                    if (R2) out << R2->attribute("tonal_label").toString() << "\t"; else out << "NA\t";
                    if (R2) out << R2->attribute("tonal_movement").toString() << "\t"; else out << "NA\t";
                    if (R3) out << R3->attribute("tonal_label").toString() << "\t"; else out << "NA\t";
                    if (R3) out << R3->attribute("tonal_movement").toString() << "\t"; else out << "NA\t";
                    // speech rate - pauses
                    // Calculate local speech rate based on L1...L5 and R1...R5;
                    if (L1 && L5) {
                        double localSpeechRateS1 = 5.0 / (L1->tMax() - L5->tMin()).toDouble();
                        out << localSpeechRateS1 << sep;
                    } else out << "NA" << sep;
                    if (R1 && R5) {
                        double localSpeechRateS2 = 5.0 / (R5->tMax() - R1->tMin()).toDouble();
                        out << localSpeechRateS2 << sep;
                    } else out << "NA" << sep;
                    // pause before
                    if (itok > 1 && tier_tokens->interval(itok - 1)->isPauseSilent())
                        out << QString::number(tier_tokens->interval(itok - 1)->duration().toDouble()) << sep;
                    else
                        out << "0" << sep;
                    // pause after
                    if (itok < tier_tokens->count() - 1 && tier_tokens->interval(itok + 1)->isPauseSilent())
                        out << QString::number(tier_tokens->interval(itok + 1)->duration().toDouble()) << sep;
                    else
                        out << "0" << sep;
                    // syllable text
                    if (L5) out << L5->text() << sep; else out <<  "#" << sep;
                    if (L4) out << L4->text() << sep; else out <<  "#" << sep;
                    if (L3) out << L3->text() << sep; else out <<  "#" << sep;
                    if (L2) out << L2->text() << sep; else out <<  "#" << sep;
                    if (L1) out << L1->text() << sep; else out <<  "#" << sep;
                    if (T1) out << T1->text() << sep; else out <<  "#" << sep;
                    if (T2) out << T2->text() << sep; else out <<  "#" << sep;
                    if (R1) out << R1->text() << sep; else out <<  "#" << sep;
                    if (R2) out << R2->text() << sep; else out <<  "#" << sep;
                    if (R3) out << R3->text() << sep; else out <<  "#" << sep;
                    if (R4) out << R4->text() << sep; else out <<  "#" << sep;
                    if (R5) out << R5->text() << "\n"; else out <<  "#" << "\n";
                }
            }
            qDeleteAll(tiersAll);
        }
        ret.append(com->ID()).append(" ");
    }
    file.close();

    return ret;
}
