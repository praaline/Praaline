#include <QDebug>
#include <QString>
#include <QList>
#include <QMap>
#include <QHash>
#include <QPointer>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/PointTier.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/SequenceTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "ORFEO.h"

ORFEO::ORFEO()
{

}

QString ORFEO::readOrfeoFile(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error: No communication";
    QString filename = com->ID() + ".orfeo";
    QString path = "/mnt/hgfs/DATA/cefc-gold/data/";
    // Open ORFEO file
    QFile file(path + filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return "Error opening ORFEO file";
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    ret.append(QString("Communication ID: %1\n").arg(com->ID()));
    // Data structures
    QHash<QString, QList<Interval *> > orfeoData;
    // Process file
    RealTime last_tMax;
    QString last_speakerID;
    bool inSentence(false);
    int sentenceID(0);
    do {
        QString line = stream.readLine();
        line = line.trimmed();
        if (line.startsWith("# sent_id =")) {
            sentenceID = QString(line).remove("# sent_id = cefc-").remove(com->ID() + "-").toInt();
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
        // Fix timecodes
        RealTime tMin = RealTime::fromSeconds(tMin_s);
        RealTime tMax = RealTime::fromSeconds(tMax_s);
        if ((last_speakerID == speakerID) && (tMin - last_tMax <= RealTime::fromMilliseconds(200))) {
            tMin = last_tMax;
        }
        last_speakerID = speakerID;
        last_tMax = tMax;
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
        ret.append(QString("\tSpeaker ID: %1\t saved %2 tokens\n").arg(speakerID).arg(orfeoData.value(speakerID).count()));
    }

    return ret;
}

QString ORFEO::mapTokensToDisMo(QPointer<Praaline::Core::CorpusCommunication> com)
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
            IntervalTier *tier_orfeo = tiers->getIntervalTierByName("orfeo_token");
            if (!tier_orfeo) { ret.append("No ORFEO tier\n"); continue; }
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) { ret.append("No tok_min tier\n"); continue; }
            int i_orfeo(0), i_tokmin(0);
            while ((i_orfeo < tier_orfeo->count()) && (i_tokmin < tier_tok_min->count())) {
                Interval *orfeo = tier_orfeo->at(i_orfeo);
                Interval *tok_min = tier_tok_min->at(i_tokmin);
                if (tok_min->text() == orfeo->text()) {
                    tok_min->setAttribute("orfeo_pos", orfeo->attribute("orfeo_UPOS"));
                    tok_min->setAttribute("orfeo_token_id", orfeo->attribute("orfeo_token_id"));
                    tok_min->setAttribute("orfeo_sent_id", orfeo->attribute("orfeo_sent_id"));
                    tok_min->setAttribute("orfeo_head", orfeo->attribute("orfeo_head"));
                    tok_min->setAttribute("orfeo_deprel", orfeo->attribute("orfeo_deprel"));
                    tok_min->setAttribute("orfeo_seg", orfeo->attribute("orfeo_seg"));
                } else {
                    if (tok_min->isPauseSilent())
                        tok_min->setAttribute("orfeo_pos", "_");
                    else
                        tok_min->setAttribute("orfeo_pos", "***");
                }
                if (orfeo->tMax() > tok_min->tMax()) {
                    i_tokmin++;
                }
                else {
                    i_orfeo++;
                    i_tokmin++;
                }
            }
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_min);
            ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

QString ORFEO::createSentenceUnits(QPointer<Praaline::Core::CorpusCommunication> com)
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
            IntervalTier *tier_orfeo = tiers->getIntervalTierByName("orfeo_token");
            if (!tier_orfeo) { ret.append("No ORFEO tier\n"); continue; }
            QList<Interval *> intervals_sentences;
            RealTime tMin, tMax;
            foreach (Interval *token, tier_orfeo->intervals()) {
                token->setText(token->text().trimmed());
                if (token->attribute("orfeo_seg").toString().contains("[")) tMin = token->tMin();
                if (token->attribute("orfeo_seg").toString().contains("]")) {
                    tMax = token->tMax();
                    if (tMin > tMax)
                        qDebug() << "Error tMin-tMax";
                    else {
                        QString text = tier_orfeo->getIntervalsTextContainedIn(tMin, tMax);
                        text = text.replace("  ", " ").replace("  ", " ");
                        Interval *sentence;
                        sentence = new Interval(tMin, tMax, text);
                        sentence->setAttribute("sent_id", token->attribute("orfeo_sent_id").toInt());
                        intervals_sentences << sentence;
                    }
                }
            }
            IntervalTier *tier_sentences = new IntervalTier("orfeo_sent", intervals_sentences);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_sentences);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_orfeo);
            ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}
