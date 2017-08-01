#include <QString>
#include <QList>
#include <QPointer>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "PhonetiserExternal.h"

PhonetiserExternal::PhonetiserExternal()
{

}

QString PhonetiserExternal::exportToPhonetiser(QPointer<CorpusCommunication> com)
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
            IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_min");
            if (!tier_tokens) continue;
            tier_tokens->mergeIdenticalAnnotations("_");
            QList<QList<Interval *> > utterances;

            QList<Interval *> utterance;
            foreach (Interval *intv, tier_tokens->intervals()) {
                if (intv->text() == "***") continue;
                if (intv->text().startsWith("(") || intv->text().endsWith(")")) continue;
                if (intv->text().trimmed() == "/" || intv->text().trimmed() == "?" || intv->text().trimmed().isEmpty()) continue;
                if (intv->text() == "_") {
                    if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
                } else {
                    utterance << intv;
                }
                intv->setText(intv->text().remove("/").remove("?"));
            }
            if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
            // Create a phonetisation file

            QString path = "/home/george/AA/";
            QFile file(path + QString("%1_%2_phono.txt").arg(annotationID).arg(speakerID));
            if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) continue;
            QTextStream out(&file);
            out.setCodec("ISO 8859-1");
            foreach (utterance, utterances) {
                QString s;
                foreach (Interval *intv, utterance) {
                    s.append(intv->text());
                    if (intv->text().endsWith("'") && intv->text() != "jusqu'" && intv->text() != "lorsqu'" && intv->text() != "parce qu'") continue;
                    s.append(" ");
                }
                out << s << ".\n";
            }
            file.close();
            ret.append(QString("phon250 phon250.fra.ini %1_%2_phono.txt %1_%2_phono_out.txt\n").arg(annotationID).arg(speakerID));
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}

QString PhonetiserExternal::importFromPhonetiser(QPointer<CorpusCommunication> com)
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
            IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_min");
            if (!tier_tokens) continue;
            tier_tokens->mergeIdenticalAnnotations("_");
            QList<QList<Interval *> > utterances;

            QList<Interval *> utterance;
            foreach (Interval *intv, tier_tokens->intervals()) {
                if (intv->text() == "***") continue;
                if (intv->text().startsWith("(") || intv->text().endsWith(")")) continue;
                if (intv->text().trimmed() == "/" || intv->text().trimmed() == "?" || intv->text().trimmed().isEmpty()) continue;
                if (intv->text() == "_") {
                    if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
                } else {
                    utterance << intv;
                }
                intv->setText(intv->text().remove("/").remove("?"));
            }
            if (!utterance.isEmpty()) { utterances << utterance; utterance.clear(); }
            // Read a phonetisation file
            QStringList phonetisations;
            QString path = "/home/george/AA/phon/";
            QFile file(path + QString("%1_%2_phono_out.txt").arg(annotationID).arg(speakerID));
            if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) continue;
            QTextStream stream(&file);


            do {
                QString line = stream.readLine().trimmed();
                if (line.endsWith("| _")) line.chop(3);
                phonetisations.append(line);
            } while (!stream.atEnd());
            file.close();

            // CHECKS
            bool checks = true;
            bool hasErrors = false;
            if (checks) {
                if (phonetisations.count() != utterances.count()) {
                    ret.append(QString("CountError %1 %2\n").arg(annotationID).arg(speakerID));
                    continue;
                }

                for (int i = 0; i < utterances.count(); ++i) {
                    utterance = utterances.at(i);
                    QString phonetisation = phonetisations.at(i);
                    // phonetisation.replace("k  | e  | s @  | k", "k e  | s @  | k");
                    QStringList phonetokens = phonetisation.split("|");
                    int j(0);
                    foreach (Interval *intv, utterance) {
                        if (intv->text().endsWith("'") && intv->text() != "jusqu'" && intv->text() != "lorsqu'" && intv->text() != "puisqu'" && intv->text() != "parce qu'") continue;
                        j++;
                    }
                    if (j != phonetokens.count()) {
                        QString s;
                        foreach (Interval *intv, utterance) s.append(intv->text()).append("|");
                        ret.append(QString("%1\t%2\t%3\t%4\t%5\n").arg(annotationID).arg(speakerID).arg(i).arg(s).arg(phonetisation));
                        hasErrors = true;
                    }
                }
            }

            if (hasErrors) continue;

            // Update phonetisaton from phonetiser output files
            foreach (Interval *intv, tier_tokens->intervals()) {
                intv->setAttribute("phonetisation", "");
                if (intv->isPauseSilent()) intv->setAttribute("phonetisation", "_");
            }

            for (int i = 0; i < utterances.count(); ++i) {
                utterance = utterances.at(i);
                QString phonetisation = phonetisations.at(i);
                QStringList phonetokens = phonetisation.split("|");
                int j(0);
                foreach (Interval *intv, utterance) {
                    if (intv->text().endsWith("'") && intv->text() != "jusqu'" && intv->text() != "lorsqu'" && intv->text() != "puisqu'" && intv->text() != "parce qu'") continue;
                    intv->setAttribute("phonetisation", QString(phonetokens.at(j)).replace(" ", ""));
                    j++;
                }
            }
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tokens);

            // Apostrophes
            for (int i = 0; i < tier_tokens->count() - 1; ++i) {
                if (!tier_tokens->at(i)->attribute("phonetisation").toString().isEmpty()) continue;
                QString t = tier_tokens->at(i)->text();
                QString p = tier_tokens->at(i+1)->attribute("phonetisation").toString();
                if      (t == "c'" && p.startsWith("s")) {
                    tier_tokens->at(i)->setAttribute("phonetisation", "s");
                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
                }
                else if (t == "d'" && p.startsWith("d")) {
                    tier_tokens->at(i)->setAttribute("phonetisation", "d");
                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
                }
                else if (t == "j'" && p.startsWith("Z")) {
                    tier_tokens->at(i)->setAttribute("phonetisation", "Z");
                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
                }
                else if (t == "l'" && p.startsWith("l")) {
                    tier_tokens->at(i)->setAttribute("phonetisation", "l");
                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
                }
                else if (t == "m'" && p.startsWith("m")) {
                    tier_tokens->at(i)->setAttribute("phonetisation", "m");
                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
                }
                else if (t == "n'" && p.startsWith("n")) {
                    tier_tokens->at(i)->setAttribute("phonetisation", "n");
                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
                }
                else if (t == "qu'" && p.startsWith("k")) {
                    tier_tokens->at(i)->setAttribute("phonetisation", "k");
                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
                }
                else if (t == "s'" && p.startsWith("s")) {
                    tier_tokens->at(i)->setAttribute("phonetisation", "s");
                    tier_tokens->at(i+1)->setAttribute("phonetisation", p.mid(1, -1));
                }
            }

            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tokens);
            ret.append(QString("Updated phonetisation for %1 %2\n").arg(annotationID).arg(speakerID));

        }
        qDeleteAll(tiersAll);
    }
    return ret;
}

