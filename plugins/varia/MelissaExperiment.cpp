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
                if (intv->text().startsWith("(") || intv->text().endsWith(")")) continue;
                if (intv->text().trimmed() == "/" || intv->text().trimmed() == "?" || intv->text().trimmed().isEmpty()) continue;
                if (intv->text() == "_") {
                    if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
                } else {
                    utterance << intv;
                }
                intv->setAttribute("phonetisation", "");
                if (intv->isPauseSilent()) intv->setAttribute("phonetisation", "_");
                // intv->setText(intv->text().remove("/").remove("?"));
            }
            if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
            // open textgrid
            AnnotationTierGroup *txg = new AnnotationTierGroup();
            if (!PraatTextGrid::load("/home/george/AA/phonetisation/" + annotationID + ".TextGrid", txg)) {
                qDebug() << "error with" << annotationID;
                continue;
            }
            IntervalTier *tier_ortho = txg->getIntervalTierByName("ortho");
            IntervalTier *tier_phono = txg->getIntervalTierByName("phono");
            if (!tier_ortho || !tier_phono) continue;

            // Check phonetisation
//            for (int i = 0; i < tier_ortho->count() && i < tier_phono->count(); ++i) {
//                if (tier_ortho->at(i)->text().replace("' ", "'").replace("jusqu'", "jusqu' ").replace("lorsqu'", "lorsqu' ").split(" ").count() != tier_phono->at(i)->text().split(" ").count()) {
//                    qDebug() << annotationID << i
//                             << tier_ortho->at(i)->text() << " <> " << tier_phono->at(i)->text();
//                }
//            }
            // Check phonetisation relative to tok_min
//            int i(0);
//            foreach (Interval *phono, tier_phono->intervals()) {
//                if (phono->isPauseSilent()) continue;
//                utterance = utterances.at(i);
//                QString s;
//                foreach (Interval *intv, utterance) {
//                    s.append(intv->text().trimmed());
//                    if (!intv->text().endsWith("'") || intv->text() == "jusqu'" || intv->text() == "lorsqu'") s.append(" ");
//                }
//                s = s.trimmed();
//                if (s.split(" ").count() != phono->text().split(" ").count())
//                    qDebug() << annotationID << i << s << "<>" << phono->text();
//                i++;
//            }
//            // Update phonetisaton from textgrids
//            int i(0);
//            foreach (Interval *phono, tier_phono->intervals()) {
//                if (phono->isPauseSilent()) continue;
//                utterance = utterances.at(i);
//                QStringList phonetisations = phono->text().split(" ");
//                int j(0);
//                foreach (Interval *intv, utterance) {
//                    if (intv->text().endsWith("'") && intv->text() != "jusqu'" && intv->text() != "lorsqu'" && intv->text() != "parce qu'") continue;
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
