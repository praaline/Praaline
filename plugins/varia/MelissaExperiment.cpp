#include <QDebug>
#include <QPointer>
#include <QString>
#include <QList>
#include <QStringList>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "MelissaExperiment.h"

MelissaExperiment::MelissaExperiment()
{

}

QString mergeChocolat(const QString &subjectID, const QString &condition)
{
    QStringList list;
    list << QString("stim/chocolat_%1_A.wav").arg(condition) << QString("rep/%1_chocolat_%2_A.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_B.wav").arg(condition) << QString("rep/%1_chocolat_%2_B.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_C.wav").arg(condition) << QString("rep/%1_chocolat_%2_C.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_D.wav").arg(condition) << QString("rep/%1_chocolat_%2_D.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_E.wav").arg(condition) << QString("rep/%1_chocolat_%2_E.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_F.wav").arg(condition) << QString("rep/%1_chocolat_%2_F.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_G.wav").arg(condition) << QString("rep/%1_chocolat_%2_G.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_H.wav").arg(condition) << QString("rep/%1_chocolat_%2_H.wav").arg(subjectID).arg(condition);
    QString sox("sox ");
    foreach (QString filename, list) sox.append(filename).append(" ");
    sox.append(QString("out/%1_chocolat_%2.wav").arg(subjectID).arg(condition));
    return sox;
}

QString mergePlantation(const QString &subjectID, const QString &condition)
{
    QStringList list;
    list << QString("stim/plantation_%1_A.wav").arg(condition) << QString("rep/%1_plantation_%2_A.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_B.wav").arg(condition) << QString("rep/%1_plantation_%2_B.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_C.wav").arg(condition) << QString("rep/%1_plantation_%2_C.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_D.wav").arg(condition) << QString("rep/%1_plantation_%2_D.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_E.wav").arg(condition) << QString("rep/%1_plantation_%2_E.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_F.wav").arg(condition) << QString("rep/%1_plantation_%2_F.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_G.wav").arg(condition) << QString("rep/%1_plantation_%2_G.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_H.wav").arg(condition) << QString("rep/%1_plantation_%2_H.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_I.wav").arg(condition) << QString("rep/%1_plantation_%2_I.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_J.wav").arg(condition) << QString("rep/%1_plantation_%2_J.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_K.wav").arg(condition) << QString("rep/%1_plantation_%2_K.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_L.wav").arg(condition) << QString("rep/%1_plantation_%2_L.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_M.wav").arg(condition) << QString("rep/%1_plantation_%2_M.wav").arg(subjectID).arg(condition);
    QString sox("sox ");
    foreach (QString filename, list) sox.append(filename).append(" ");
    sox.append(QString("out/%1_plantation_%2.wav").arg(subjectID).arg(condition));
    return sox;
}

QString mergeTabouret(const QString &subjectID, const QString &condition)
{
    QStringList list;
    list << QString("stim/tabouret_%1_A.wav").arg(condition) << QString("rep/%1_tabouret_%2_A.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_B.wav").arg(condition) << QString("rep/%1_tabouret_%2_B.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_C.wav").arg(condition) << QString("rep/%1_tabouret_%2_C.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_D.wav").arg(condition) << QString("rep/%1_tabouret_%2_D.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_E.wav").arg(condition) << QString("rep/%1_tabouret_%2_E.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_F.wav").arg(condition) << QString("rep/%1_tabouret_%2_F.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_G.wav").arg(condition) << QString("rep/%1_tabouret_%2_G.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_H.wav").arg(condition) << QString("rep/%1_tabouret_%2_H.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_I.wav").arg(condition) << QString("rep/%1_tabouret_%2_I.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_J.wav").arg(condition) << QString("rep/%1_tabouret_%2_J.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_K.wav").arg(condition) << QString("rep/%1_tabouret_%2_K.wav").arg(subjectID).arg(condition);
    QString sox("sox ");
    foreach (QString filename, list) sox.append(filename).append(" ");
    sox.append(QString("out/%1_tabouret_%2.wav").arg(subjectID).arg(condition));
    return sox;
}

// static
QString MelissaExperiment::multiplex(int participantNo)
{
    QString chocolat, tabouret, plantation;
    QString subjectID = QString("S%1").arg(participantNo);
    if      (participantNo ==  1) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo ==  2) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo ==  3) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo ==  4) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo ==  5) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo ==  6) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo ==  7) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo ==  8) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo ==  9) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 10) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 11) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 12) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 13) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 14) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 15) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 16) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 17) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 18) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 19) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 20) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 21) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 22) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 23) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 24) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "C");    }

    else if (participantNo == 31) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 32) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 33) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 34) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 35) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 36) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 37) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 38) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 39) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 40) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 41) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 42) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 43) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 44) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 45) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 46) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 47) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 48) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 49) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 50) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 51) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 52) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 53) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 54) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "C");    }

    return QString("%1\n%2\n%3").arg(chocolat).arg(tabouret).arg(plantation);
}

// Prepare for the alignment of stimuli
void MelissaExperiment::prepareStimuliCorpus(QPointer<CorpusCommunication> com)
{
    Q_UNUSED(com)
    QString path = "/home/george/Dropbox/Corpus_Melissa_Lesseur";
    QFile file(path + "/soundbite_export_table_for_alignment.txt");
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return;
    QTextStream stream(&file);

    QString previousStimulusID;
    RealTime offset = RealTime::fromMilliseconds(150);
    QList<Interval *> intervals_transcription;
    QList<Interval *> intervals_phonetisation;
    do {
        QString line = stream.readLine().trimmed();
        if (line.startsWith("#")) continue;
        QStringList fields = line.split("\t");
        QString stimulusID = fields.at(0);
        if ((!previousStimulusID.isEmpty()) && (previousStimulusID != stimulusID)) {
            intervals_transcription << new Interval(offset, offset + RealTime::fromMilliseconds(150), "");
            intervals_phonetisation << new Interval(offset, offset + RealTime::fromMilliseconds(150), "");
            IntervalTier *ortho = new IntervalTier("ortho", intervals_transcription);
            IntervalTier *phono = new IntervalTier("phono", intervals_phonetisation);
            AnnotationTierGroup *txg = new AnnotationTierGroup();
            txg->addTier(phono);
            txg->addTier(ortho);
            PraatTextGrid::save("/home/george/AA/" + previousStimulusID + ".TextGrid", txg);
            delete txg;
            intervals_phonetisation.clear();
            intervals_transcription.clear();
            offset = RealTime::fromMilliseconds(150);
        }
        RealTime pause_after = RealTime::fromMilliseconds(fields.at(2).toInt());
        RealTime duration = RealTime::fromNanoseconds(fields.at(9).toLongLong());
        QString transcription = fields.at(11);
        QString phonetisation = fields.at(12);
        intervals_transcription << new Interval(offset, offset + duration, transcription);
        intervals_phonetisation << new Interval(offset, offset + duration, phonetisation);
        offset = offset + duration + pause_after;
        previousStimulusID = stimulusID;
    } while (!stream.atEnd());
    // LAST FILE
    intervals_transcription << new Interval(offset, offset + RealTime::fromMilliseconds(150), "");
    intervals_phonetisation << new Interval(offset, offset + RealTime::fromMilliseconds(150), "");
    IntervalTier *ortho = new IntervalTier("ortho", intervals_transcription);
    IntervalTier *phono = new IntervalTier("phono", intervals_phonetisation);
    AnnotationTierGroup *txg = new AnnotationTierGroup();
    txg->addTier(phono);
    txg->addTier(ortho);
    PraatTextGrid::save("/home/george/AA/" + previousStimulusID + ".TextGrid", txg);
    delete txg;

    file.close();

}

QString MelissaExperiment::exportSyntacticAnnotation(QPointer<CorpusCommunication> com)
{
    QString ret;
    // Creates a transcription
    if (!com) return ret;
    bool parenthesis(false);
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        QString transcription;
        QList<Interval *> tokens = com->repository()->annotations()->getIntervals(AnnotationDatastore::Selection(annotationID, "", "tok_mwu"));
        if (tokens.isEmpty()) continue;
        QString currentSpeakerID = tokens.first()->attribute("speakerID").toString();
        transcription.append(currentSpeakerID).append("\t1\t"); // first line
        int line(1);
        for (int i = 0; i < tokens.count(); ++i) {
            Interval *token = tokens.at(i);
            if (token->isPauseSilent() && i == 0) continue;
            QString speakerID = token->attribute("speakerID").toString();
            if (currentSpeakerID != speakerID) {
                QString nextSpeakerID = (i < tokens.count() - 1) ? tokens.at(i+1)->attribute("speakerID").toString() : "";
                if (nextSpeakerID == currentSpeakerID && token->isPauseSilent()) continue;
                currentSpeakerID = speakerID;
                line++;
                transcription.append("\n").append(currentSpeakerID).append("\t").append(QString::number(line)).append("\t");
            }
            QString syntax = token->attribute("syntaxe").toString();
            if (syntax.contains("(") && !syntax.contains(")"))
                if (parenthesis) transcription.append("(");
            if (token->isPauseSilent() && token->duration().toDouble() >= 0.250)
                transcription.append("//");
            else if (token->isPauseSilent() && token->duration().toDouble() < 0.250)
                transcription.append("/");
            else
                transcription.append(token->text().trimmed());
            bool newline(false);
            if (!syntax.isEmpty() && !(syntax.contains("(") && !syntax.contains(")"))) {
                if (parenthesis) {
                    transcription.append(syntax.remove("(")).append("\n\t");
                } else {
                    transcription.append("\t").append(syntax.remove("(").remove(")"));
                    line++;
                    transcription.append("\n").append(currentSpeakerID).append("\t").append(QString::number(line)).append("\t");
                }
                newline = true;
            }
            if (!newline) transcription.append(" ");
        }
        // ret.append(com->ID()).append("\n");
        ret.append(transcription);
        // ret.append("\n\n");
    }
    return ret;
}

void MelissaExperiment::exportForEA(QPointer<CorpusCommunication> com)
{
    if (!com) return;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_mwu");
            if (!tier_tokens) continue;
            foreach (Interval *intv, tier_tokens->intervals()) {
                if (intv->text().startsWith("(") || intv->text().endsWith(")")) intv->setText("");
                intv->setText(intv->text().remove("/").remove("?"));
            }
            tier_tokens->mergeContiguousAnnotations(QStringList() << "_", " ");
            AnnotationTierGroup *txg = new AnnotationTierGroup();
            tier_tokens->setName("ortho");
            txg->addTier(tier_tokens);
            PraatTextGrid::save("/home/george/AA/" + annotationID + ".TextGrid", txg);
        }
        qDeleteAll(tiersAll);
    }
}

void MelissaExperiment::importPhonetisation(QPointer<CorpusCommunication> com)
{
    if (!com) return;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_min");
            if (!tier_tokens) continue;
            tier_tokens->mergeIdenticalAnnotations("_");
            QList<QList<Interval *> > utterances;

            QList<Interval *> utterance;
            foreach (Interval *intv, tier_tokens->intervals()) {
                if (intv->text().startsWith("(") || intv->text().endsWith(")") || intv->isPauseSilent()) {
                    if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
                } else {
                    utterance << intv;
                }
                intv->setAttribute("phonetisation", "");
                if (intv->isPauseSilent()) intv->setAttribute("phonetisation", "_");
                // intv->setText(intv->text().remove("/").remove("?"));
            }
            if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }

//            QList<Interval *> utterance;
//            foreach (Interval *intv, tier_tokens->intervals()) {
//                if (intv->text().startsWith("(") || intv->text().endsWith(")")) continue;
//                if (intv->text().trimmed() == "/" || intv->text().trimmed() == "?" || intv->text().trimmed().isEmpty()) continue;
//                if (intv->text() == "_") {
//                    if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
//                } else {
//                    utterance << intv;
//                }
//                intv->setAttribute("phonetisation", "");
//                if (intv->isPauseSilent()) intv->setAttribute("phonetisation", "_");
//                // intv->setText(intv->text().remove("/").remove("?"));
//            }
//            if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
            // open textgrid
//            AnnotationTierGroup *txg = new AnnotationTierGroup();
//            if (!PraatTextGrid::load("/home/george/AA/phonetisation/" + annotationID + ".TextGrid", txg)) {
//                qDebug() << "error with" << annotationID;
//                continue;
//            }
//            IntervalTier *tier_ortho = txg->getIntervalTierByName("ortho");
//            IntervalTier *tier_phono = txg->getIntervalTierByName("phono");
//            if (!tier_ortho || !tier_phono) continue;

//            IntervalTier *tier_ortho = tiers->getIntervalTierByName("transcription");
//            IntervalTier *tier_phono = new IntervalTier(tier_ortho);
//            if (!tier_ortho || !tier_phono) continue;
//            foreach (Interval *intv, tier_phono->intervals()) intv->setText(intv->attribute("phonetisation").toString());

            // Check phonetisation
//            for (int i = 0; i < tier_ortho->count() && i < tier_phono->count(); ++i) {
//                if (tier_ortho->at(i)->text().replace("' ", "'").replace("jusqu'", "jusqu' ").replace("lorsqu'", "lorsqu' ").split(" ").count()
//                        != tier_phono->at(i)->text().split(" ").count()) {
//                    qDebug() << annotationID << i
//                             << tier_ortho->at(i)->text() << " <> " << tier_phono->at(i)->text();
//                }
//            }



            // Check phonetisation relative to tok_min
//            int i(0);
//            foreach (Interval *phono, tier_phono->intervals()) {
//                if (phono->isPauseSilent()) continue;
//                if (i >= utterances.count()) {
//                    qDebug() << annotationID << "Count mismatch";
//                    break;
//                }
//                utterance = utterances.at(i);
//                QString s;
//                foreach (Interval *intv, utterance) {
//                    s.append(intv->text().trimmed());
//                    if (!intv->text().endsWith("'") || intv->text() == "jusqu'" || intv->text() == "lorsqu'" || intv->text() == "puisqu'") s.append(" ");
//                }
//                s = s.trimmed();
//                if (s.split(" ").count() != phono->text().split(" ").count())
//                    qDebug() << annotationID << i << s << "<>" << phono->text();
//                i++;
//            }
            // Update phonetisaton
//            int i(0);
//            foreach (Interval *phono, tier_phono->intervals()) {
//                if (phono->isPauseSilent()) continue;
//                utterance = utterances.at(i);
//                QStringList phonetisations = phono->text().split(" ");
//                int j(0);
//                foreach (Interval *intv, utterance) {
//                    if (intv->text().endsWith("'") && intv->text() != "jusqu'" && intv->text() != "lorsqu'" && intv->text() != "puisqu'" && intv->text() != "parce qu'") continue;
//                    intv->setAttribute("phonetisation", phonetisations.at(j));
//                    j++;
//                }
//                i++;
//            }
//            qDebug() << annotationID;
//            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tokens);

//            // Apostrophes
//            for (int i = 0; i < tier_tokens->count() - 1; ++i) {
//                if (!tier_tokens->at(i)->attribute("phonetisation").toString().isEmpty()) continue;
//                QString t = tier_tokens->at(i)->text();
//                QString p = tier_tokens->at(i+1)->attribute("phonetisation").toString();
//                if      (t == "c'" && p.startsWith("s")) {
//                    tier_tokens->at(i)->setAttribute("phonetisation", "s");
//                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
//                }
//                else if (t == "d'" && p.startsWith("d")) {
//                    tier_tokens->at(i)->setAttribute("phonetisation", "d");
//                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
//                }
//                else if (t == "j'" && p.startsWith("Z")) {
//                    tier_tokens->at(i)->setAttribute("phonetisation", "Z");
//                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
//                }
//                else if (t == "l'" && p.startsWith("l")) {
//                    tier_tokens->at(i)->setAttribute("phonetisation", "l");
//                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
//                }
//                else if (t == "m'" && p.startsWith("m")) {
//                    tier_tokens->at(i)->setAttribute("phonetisation", "m");
//                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
//                }
//                else if (t == "n'" && p.startsWith("n")) {
//                    tier_tokens->at(i)->setAttribute("phonetisation", "n");
//                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
//                }
//                else if (t == "qu'" && p.startsWith("k")) {
//                    tier_tokens->at(i)->setAttribute("phonetisation", "k");
//                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
//                }
//                else if (t == "s'" && p.startsWith("s")) {
//                    tier_tokens->at(i)->setAttribute("phonetisation", "s");
//                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
//                }
//            }
//            qDebug() << annotationID;
//            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tokens);

//            int i(0);
//            foreach (Interval *ortho, tier_ortho->intervals()) {
//                if (ortho->isPauseSilent()) continue;
//                utterance = utterances.at(i);
//                // Update
//                QString s;
//                foreach (Interval *intv, utterance) {
//                    s.append(intv->text().toLower().trimmed().remove("/").remove("?")).append(" ");
//                }
//                s = s.trimmed();
//                if (ortho->text() != s) qDebug() << annotationID << i << ortho->text() << " <> " << s;
//                i++;
//                if (i >= utterances.count()) break;
//            }

//            foreach (utterance, utterances) {
//                QString s;
//                foreach (Interval *intv, utterance) {
//                    s.append(intv->text().remove("/").remove("?")).append(" ");
//                }
//                qDebug() << s;
//            }

            //
            // tier_tokens->setName("ortho");
            // txg->addTier(tier_tokens);
            // PraatTextGrid::save();
        }
        qDeleteAll(tiersAll);
    }
}

QString MelissaExperiment::splitResponses(QPointer<CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_response = tiers->getIntervalTierByName("response");
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");
            IntervalTier *tier_transcription = tiers->getIntervalTierByName("transcription");
            if (!tier_response || !tier_tok_min || !tier_tok_mwu || !tier_transcription) {
                ret.append(QString("Not all tiers present in %1").arg(annotationID));
                continue;
            }

            for (int i = 0; i < tier_response->count() - 1; ++i) {
                RealTime t = tier_response->interval(i)->tMax();
                Interval *tok_min = tier_tok_min->intervalAtTime(t);
                Interval *tok_mwu = tier_tok_mwu->intervalAtTime(t);
                Interval *transcription = tier_transcription->intervalAtTime(t);


//                QString tok_min_t = tok_min->text();
//                QString tok_mwu_t = tok_mwu->text();
//                QString transcr_t = transcription->text();
//                if ((tok_min_t != tok_mwu_t) || (transcr_t.right(tok_min_t.length()) != tok_min_t))
//                    ret.append(QString("%1\t Error on interval \t%2 time \t%3 %4 %5\n")
//                               .arg(annotationID).arg(i).arg(t.toDouble()).arg(tok_min_t).arg(transcr_t));

            }
            // if (ret.isEmpty()) ret.append(QString("OK %1\n").arg(annotationID));
        }
        qDeleteAll(tiersAll);
    }
    if (ret.endsWith("\n")) ret.chop(1);
    return ret;
}

QString MelissaExperiment::exportForAlignment(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tokens = new IntervalTier(tiers->getIntervalTierByName("tok_min"));
            if (!tier_tokens) continue;
            IntervalTier *tier_tokmin = tiers->getIntervalTierByName("tok_min");
            IntervalTier *tier_tokmwu = tiers->getIntervalTierByName("tok_mwu");
            IntervalTier *tier_transcription = tiers->getIntervalTierByName("transcription");
            IntervalTier *tier_response = tiers->getIntervalTierByName("response");

            QList<QList<Interval *> > utterances;

            QList<Interval *> utterance;
            foreach (Interval *intv, tier_tokens->intervals()) {
                if (intv->isPauseSilent()) {
                    if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
                } else if (intv->text().startsWith("(") || intv->text().endsWith(")")) {
                    if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
                    intv->setText("%");
                    intv->setAttribute("phonetisation", "%");
                    utterance << intv;
                    if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
                } else {
                    utterance << intv;
                }
            }
            if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }

            QList<Interval *> utt_o, utt_p;
            foreach (utterance, utterances) {
                QString o, p;
                foreach (Interval *intv, utterance) {
                    o.append(intv->text()).append(" ");
                    p.append(intv->attribute("phonetisation").toString()).append(" ");
                }
                utt_o << new Interval(utterance.first()->tMin(), utterance.last()->tMax(), o);
                utt_p << new Interval(utterance.first()->tMin(), utterance.last()->tMax(), p);
            }
            IntervalTier *tier_ortho = new IntervalTier("ortho", utt_o);
            tier_ortho->fillEmptyWith("", "_");
            IntervalTier *tier_phono = new IntervalTier("phono", utt_p);
            tier_phono->fillEmptyWith("", "_");

            AnnotationTierGroup *txg = new AnnotationTierGroup();
            txg->addTier(tier_phono);
            txg->addTier(tier_ortho);
            txg->addTier(tier_tokmin);
            txg->addTier(tier_tokmwu);
            txg->addTier(tier_transcription);
            txg->addTier(tier_response);

            PraatTextGrid::save("/home/george/AA/align/" + annotationID + "_" + speakerID + ".TextGrid", txg);
            ret.append(QString("OK %1").arg(annotationID));
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}


QString MelissaExperiment::reimportAlignment(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            AnnotationTierGroup *txg = new AnnotationTierGroup();
            PraatTextGrid::load("/home/george/AA/align_corr/" + annotationID + "_" + speakerID + "_corr.TextGrid", txg);

            IntervalTier *tier_transcription_aligned = txg->getIntervalTierByName("transcription");
            IntervalTier *tier_words_aligned = txg->getIntervalTierByName("words");
            IntervalTier *tier_syll = txg->getIntervalTierByName("syll");
            IntervalTier *tier_phone = txg->getIntervalTierByName("phones");
            tier_phone->setName("phone");

            IntervalTier *tier_transcription = tiers->getIntervalTierByName("transcription");
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");

            // Checks
            if (tier_transcription->count() != tier_transcription_aligned->count()) {
                ret.append(QString("Count mismatch %1 transcription").arg(annotationID));
                continue;
            }
            if (tier_words_aligned->count() != tier_tok_min->count()) {
                ret.append(QString("Count mismatch %1 tokens: file %2 database %3").arg(annotationID)
                           .arg(tier_words_aligned->count()).arg(tier_tok_min->count()));
                continue;
            }

            QList<bool> takeBoundary;
            // Correspondance
            int i(0), j(0);
            while ((i < tier_tok_min->count()) && (j < tier_tok_mwu->count())) {
                Interval *tok_min = tier_tok_min->at(i);
                Interval *tok_mwu = tier_tok_mwu->at(j);
                while (tok_min->tMax() < tok_mwu->tMax())  {
                    takeBoundary << false;
                    i++;
                    tok_min = tier_tok_min->at(i);
                }
                takeBoundary << true;
                i++; j++;
            }

            // Realign
            QList<RealTime> newBoundaries;
            // Realign transcription
            newBoundaries.clear(); newBoundaries << RealTime();
            newBoundaries.append(tier_transcription_aligned->timesMax());
            tier_transcription->realignIntervals(0, newBoundaries);
            // Realign tok_min
            newBoundaries.clear(); newBoundaries << RealTime();
            newBoundaries.append(tier_words_aligned->timesMax());
            tier_tok_min->realignIntervals(0, newBoundaries);
            // Realign tok_mwu
            newBoundaries.clear(); newBoundaries << RealTime();
            for (int i = 0; i < takeBoundary.count(); ++i) {
                RealTime b = tier_words_aligned->timesMax().at(i);
                if (takeBoundary.at(i)) newBoundaries << b;
            }
            tier_tok_mwu->realignIntervals(0, newBoundaries);

            tier_phone->fillEmptyWith("", "_");
            tier_syll->fillEmptyWith("", "_");

            AnnotationTierGroup *test = new AnnotationTierGroup();
            test->addTier(tier_phone);
            test->addTier(tier_syll);
            test->addTier(tier_words_aligned);
            test->addTier(tier_tok_min);
            test->addTier(tier_tok_mwu);
            test->addTier(tier_transcription);
            PraatTextGrid::save("/home/george/AA/align_corr/" + annotationID + "_test.TextGrid", test);

            bool approved = false;
            if (approved) {
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_phone);
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_syll);
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_min);
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_mwu);
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_transcription);
            }
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}

#include "ForcedAlignerDummy.h"
#include "SyllabifierEasy.h"

QString MelissaExperiment::preprocessAlignment(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        foreach (QString speakerID, com->repository()->annotations()->getSpeakersInAnnotation(annotationID))  {
            AnnotationTierGroup *txg = new AnnotationTierGroup();
            PraatTextGrid::load("/home/george/AA/align_auto_orig/" + annotationID + "_" + speakerID + ".TextGrid", txg);

            IntervalTier *tier_phone = txg->getIntervalTierByName("phones");
            IntervalTier *tier_syll  = txg->getIntervalTierByName("syll");
            IntervalTier *tier_words = txg->getIntervalTierByName("words");
            IntervalTier *tier_phono = txg->getIntervalTierByName("phono");
            IntervalTier *tier_ortho = txg->getIntervalTierByName("ortho");

            ForcedAlignerDummy f;
            f.setTierPhones(tier_phone);
            f.setTierTokens(tier_words);

            int unalignedTotal(0), unalignedAligned(0);
            for (int i = 0; i < tier_ortho->count(); ++i) {
                QString ortho = tier_ortho->at(i)->text().trimmed().replace("parce qu", "parce_qu");
                QString phono = tier_phono->at(i)->text().trimmed().replace("*", "");
                if (ortho == "_" || ortho.isEmpty() || phono == "_" || phono.isEmpty())  continue;
                if (ortho == "%") continue;
                if (ortho == "ps" || ortho == "ts" || ortho == "tl" || ortho == "q/" || ortho == "pr") continue;
                QList<Interval *> words = tier_words->getIntervalsContainedIn(tier_ortho->at(i));
                if (words.isEmpty()) {
                    ret.append(QString("ERROR\t%1\t%2\t%3\n").arg(annotationID).arg(tier_ortho->at(i)->tMin().toDouble()).arg("NULL"));
                    continue;
                }
                if (words.count() > 1) continue;
                if (words.at(0)->text() != "_") continue;
                if (ortho.split(" ").count() != phono.split(" ").count()) {
                    ret.append(QString("ERROR\t%1\t%2\t%3 <> %4\n").arg(annotationID).arg(words.at(0)->tMin().toDouble()).arg(ortho).arg(phono));
                    continue;
                }
                QPair<int, int> phoneIndices = tier_phone->getIntervalIndexesContainedIn(tier_ortho->at(i));
                QPair<int, int> tokenIndices = tier_words->getIntervalIndexesContainedIn(tier_ortho->at(i));
                if ((phoneIndices.first != phoneIndices.second) || (tokenIndices.first != tokenIndices.second)) {
                    ret.append(QString("ERROR\t%1\t%2\t%3\n").arg(annotationID).arg(words.at(0)->tMin().toDouble()).arg(ortho));
                    continue;
                }
                unalignedTotal++;
                // Align
                if (f.processUnalignedSegment(phoneIndices.first, tokenIndices.first, ortho.split(" "), phono.split(" "))) {
                    // ret.append(QString("ALIGN\t%1\t%2\t%3\n").arg(annotationID).arg(words.at(0)->tMin().toDouble()).arg(ortho));
                    unalignedAligned++;
                }
                else {
                    ret.append(QString("ERROR\t%1\t%2\t%3\n").arg(annotationID).arg(words.at(0)->tMin().toDouble()).arg(ortho));
                }
            }
            ret.append(QString("%1\tTotal unaligned:%2 Aligned: %3\n").arg(annotationID).arg(unalignedTotal).arg(unalignedAligned));

            IntervalTier *tier_syll_ours = SyllabifierEasy::syllabify(tier_phone);
            tier_syll_ours->setName("syll");
            txg->insertTierReplacing(1, tier_syll_ours);
            PraatTextGrid::save("/home/george/AA/align_auto/" + annotationID + "_" + speakerID + ".TextGrid", txg);
        }
    }
    if (ret.endsWith("\n")) ret.chop(1);
    return ret;
}


QString MelissaExperiment::processPausesInsertedByAligner(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            AnnotationTierGroup *txg = new AnnotationTierGroup();
            PraatTextGrid::load("/home/george/AA/align_auto/" + annotationID + "_" + speakerID + ".TextGrid", txg);

            IntervalTier *tier_phone = txg->getIntervalTierByName("phones");
            IntervalTier *tier_syll  = txg->getIntervalTierByName("syll");
            IntervalTier *tier_words = txg->getIntervalTierByName("words");

            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");

            int iword(0), itokmin(0), itokmwu(0);
            while (iword < tier_words->count()) {
                Interval *word = tier_words->interval(iword);
                Interval *tok_min = tier_tok_min->interval(itokmin);
                QString w = word->text().trimmed();
                if (w.isEmpty()) w = "_";
                QString t = tok_min->text().trimmed();
                w = w.replace("/", "").replace("?", "").replace("parce_qu", "parce qu").trimmed();
                t = t.replace("/", "").replace("?", "").trimmed();
                if (t.startsWith("(") || t.endsWith(")")) t = "_";
                if (t == "ps" && w == "_") t = "_";
                if (t == "ts" && w == "_") t = "_";
                if (w == t) {
                    itokmin++;
                }
                else if (w == "_") {
                    bool added(false);
                    Interval *tok_min_split = tier_tok_min->interval(itokmin - 1);
                    // Decide whether to add this pause
                    if (word->duration().toDouble() < 0.090) {
                        added = false;
                    } else {
                        QList<Interval *> tok_mwus = tier_tok_mwu->getIntervalsContainedIn(tok_min_split);
                        if (tok_mwus.count() == 1 && tok_mwus.at(0)->tMin() == tok_min_split->tMin() && tok_mwus.at(0)->tMax() == tok_min_split->tMax()) {
                            added = true;
                        }
                        else
                            added = false;
                    }
                    if (!added) {
                        QPair<int, int> phones = tier_phone->getIntervalIndexesContainedIn(word);
                        QPair<int, int> sylls = tier_syll->getIntervalIndexesContainedIn(word);
                        tier_phone->at(phones.first)->setText("");
                        tier_syll->at(sylls.first)->setText("");
                        word->setText("");
                        if (!tier_words->interval(iword - 1)->isPauseSilent()) {
                            tier_phone->merge(phones.first - 1, phones.first);
                            tier_syll->merge(sylls.first - 1, sylls.first);
                            tier_words->merge(iword - 1, iword);
                        }
                        else {
                            tier_phone->merge(phones.first, phones.first + 1);
                            tier_syll->merge(sylls.first, sylls.first + 1);
                            tier_words->merge(iword, iword + 1);
                        }
                        iword--;
                        ret.append(QString("DEL\t%1\t%2\t%3\t%4\t%5\n").arg(annotationID).arg(word->tMin().toDouble())
                                   .arg(word->text()).arg(tok_min->text()).arg(word->duration().toDouble()));
                    }
                    else {
                        QPair<int, int> indexMwu = tier_tok_mwu->getIntervalIndexesContainedIn(tok_min_split);
                        QList<Interval *> splitMin = tier_tok_min->splitToEqual(itokmin - 1, 2);
                        splitMin.at(1)->setText("_");
                        splitMin.at(1)->setAttribute("pos_min", "_");
                        splitMin.at(1)->setAttribute("disfluency", "SIL");
                        QList<Interval *> splitMwu = tier_tok_mwu->splitToEqual(indexMwu.first, 2);
                        splitMwu.at(1)->setText("_");
                        splitMwu.at(1)->setAttribute("pos_mwu", "_");
                        ret.append(QString("ADD\t%1\t%2\t%3\t%4\t%5\n").arg(annotationID).arg(word->tMin().toDouble())
                                   .arg(word->text()).arg(tok_min->text()).arg(word->duration().toDouble()));
                        itokmin++;
                    }
                }
                else {
                    ret.append(QString("ERR\t%1\t%2\t%3\t%4\n").arg(annotationID).arg(word->tMin().toDouble()).arg(word->text()).arg(tok_min->text()));
                    break;
                }
                iword++;
            }
            // Correct last boundary
            if (!tier_words->last()->isPauseSilent() && tier_tok_min->last()->isPauseSilent()) {
                RealTime t = tier_tok_min->last()->tMin();
                tier_phone->split(tier_phone->count() - 1, t);
                tier_syll->split(tier_syll->count() - 1, t);
                tier_words->split(tier_words->count() - 1, t);
            }

            txg->insertTierReplacing(txg->getTierIndexByName("tok_min"), tier_tok_min);
            txg->insertTierReplacing(txg->getTierIndexByName("tok_mwu"), tier_tok_mwu);
            // PraatTextGrid::save("/home/george/AA/align_auto/" + annotationID + "_" + speakerID + "_corr.TextGrid", txg);

            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_min);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_mwu);
        }
        qDeleteAll(tiersAll);
    }
    if (ret.endsWith("\n")) ret.chop(1);
    return ret;
}






