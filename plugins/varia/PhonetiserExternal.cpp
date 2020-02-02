#include <QString>
#include <QList>
#include <QPointer>
#include <QTextStream>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "PhonetiserExternal.h"

struct PhonetiserExternalData {
    QString levelTokens;
    QString attributePhonetisationOfTokens;
    QString outputFilesPath;
    QStringList phonemes;
    QRegExp regexMatchPhoneme;
    QHash<QString, QString> citationForms;
};

PhonetiserExternal::PhonetiserExternal() :
    d(new PhonetiserExternalData())
{
    d->levelTokens = "tok_min";
    d->attributePhonetisationOfTokens = "phonetisation";
    d->outputFilesPath = "/home/george/AA/";

    // The order is important. Start with the longest phonemes.
    d->phonemes << "9~" << "a~" << "e~" << "o~"
                << "2" << "9" << "A" << "@" << "E" << "H" << "O" << "R" << "S" << "Z"
                << "a" << "b" << "d" << "e" << "f" << "g" << "i" << "j" << "k" << "l"
                << "m" << "n" << "o" << "p" << "s" << "t" << "u" << "v" << "w" << "y" << "z";
    QString regex;
    foreach (QString phoneme, d->phonemes)
        regex = regex.append(QString("%1\\*|%1|").arg(phoneme));
    if (!regex.isEmpty()) regex.chop(1);
    d->regexMatchPhoneme = QRegExp(regex);
}

PhonetiserExternal::~PhonetiserExternal()
{
    delete d;
}

QString PhonetiserExternal::outputFilesPath() const
{
    return d->outputFilesPath;
}

void PhonetiserExternal::setOutputFilesPath(const QString &path)
{
    d->outputFilesPath = path;
}

QString PhonetiserExternal::attributePhonetisationOfTokens() const
{
    return d->attributePhonetisationOfTokens;
}

void PhonetiserExternal::setAttributePhonetisationOfTokens(const QString &attributeID)
{
    d->attributePhonetisationOfTokens = attributeID;
}

QString PhonetiserExternal::exportToPhonetiser(CorpusCommunication *com)
{
    QString ret;
    if (!com) return ret;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->levelTokens);
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
            QFile file(d->outputFilesPath + QString("%1_%2_phono.txt").arg(annotationID).arg(speakerID));
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

QString PhonetiserExternal::importFromPhonetiser(CorpusCommunication *com, bool fromTranscriptionTier)
{
    QString ret;
    if (!com) return ret;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->levelTokens);
            if (!tier_tokens) continue;
            tier_tokens->mergeIdenticalAnnotations("_");

            // Create list of utterances
            QList<QList<Interval *> > utterances;
            QList<Interval *> utterance;


            if (fromTranscriptionTier) {
                IntervalTier *tier_transcription = tiers->getIntervalTierByName("transcription");
                if (!tier_transcription) continue;
                foreach (Interval *transcriptionUnit, tier_transcription->intervals()) {
                    if (transcriptionUnit->isPauseSilent()) continue;
                    foreach (Interval *intv, tier_tokens->getIntervalsContainedIn(transcriptionUnit)) {
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
                }
            } else {
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
            }


            // Get phonetisations
            QStringList phonetisations;
            if (fromTranscriptionTier) {
                IntervalTier *tier_transcription = tiers->getIntervalTierByName("transcription");
                if (!tier_transcription) continue;
                foreach (Interval *intv, tier_transcription->intervals()) {
                    if (intv->isPauseSilent()) continue;
                    QString t = intv->attribute("phonetisation").toString().trimmed();
                    if (t.isEmpty() || t == "_") continue;
                    if (t.endsWith("| _")) t.chop(3);
                    phonetisations << t; // QString(t).replace("  ", " ").replace(" ", "|"); ???
                }
            }
            else {
                // Read a phonetisation file
                QFile file(d->outputFilesPath + QString("%1_%2_phono_out.txt").arg(annotationID).arg(speakerID));
                if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) continue;
                QTextStream stream(&file);
                do {
                    QString line = stream.readLine().trimmed();
                    if (line.endsWith("| _")) line.chop(3);
                    phonetisations.append(line);
                } while (!stream.atEnd());
                file.close();
            }

            // CHECKS
            bool checks = true;
            bool hasErrors = false;
            if (checks) {
                if (phonetisations.count() != utterances.count()) {
                    ret.append(QString("CountError %1 %2\n").arg(annotationID).arg(speakerID));
                    int i = 0;
                    while (i < phonetisations.count() && i < utterances.count()) {
                        QString utt;
                        foreach (Interval *intv, utterances.at(i)) utt.append(intv->text()).append("|");
                        ret.append(QString("\t%1\t%2\t%3\n").arg(i).arg(utt).arg(phonetisations.at(i)));
                        i++;
                    }
                    continue;
                }

                for (int i = 0; i < utterances.count(); ++i) {
                    utterance = utterances.at(i);
                    QString phonetisation = phonetisations.at(i);
                    // phonetisation.replace("k  | e  | s @  | k", "k e  | s @  | k");
                    QStringList phonetokens = phonetisation.split("|");
                    int j(0);
                    foreach (Interval *intv, utterance) {
                        if (intv->text().endsWith("'") && intv->text() != "jusqu'" && intv->text() != "lorsqu'" && intv->text() != "puisqu'"
                                                       && intv->text() != "parce qu'" && intv->text() != "quelqu'") continue;
                        j++;
                    }
                    if (j != phonetokens.count()) {
                        QString utt;
                        foreach (Interval *intv, utterance) utt.append(intv->text()).append("|");
                        ret.append(QString("%1\t%2\t%3\t%4\t%5\n").arg(annotationID).arg(speakerID).arg(i).arg(utt).arg(phonetisation));
                        hasErrors = true;
                    }
                }
            }

            if (hasErrors) continue;

            // Update phonetisaton from phonetiser output files
            foreach (Interval *intv, tier_tokens->intervals()) {
                intv->setAttribute(d->attributePhonetisationOfTokens, "");
                if (intv->isPauseSilent()) intv->setAttribute(d->attributePhonetisationOfTokens, "_");
            }

            for (int i = 0; i < utterances.count(); ++i) {
                utterance = utterances.at(i);
                QString phonetisation = phonetisations.at(i);
                QStringList phonetokens = phonetisation.split("|");
                int j(0);
                foreach (Interval *intv, utterance) {
                    if (intv->text().endsWith("'") && intv->text() != "jusqu'" && intv->text() != "lorsqu'" && intv->text() != "puisqu'"
                                                   && intv->text() != "parce qu'" && intv->text() != "quelqu'") continue;
                    intv->setAttribute(d->attributePhonetisationOfTokens, QString(phonetokens.at(j)).replace(" ", ""));
                    j++;
                }
            }
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tokens);

            // Apostrophes
            for (int i = 0; i < tier_tokens->count() - 1; ++i) {
                if (!tier_tokens->at(i)->attribute(d->attributePhonetisationOfTokens).toString().isEmpty()) continue;
                QString t = tier_tokens->at(i)->text().toLower();
                QString p = tier_tokens->at(i+1)->attribute(d->attributePhonetisationOfTokens).toString();
                if      (t == "c'" && p.startsWith("s")) {
                    tier_tokens->at(i)->setAttribute(d->attributePhonetisationOfTokens, "s");
                    tier_tokens->at(i+1)->setAttribute(d->attributePhonetisationOfTokens, p.mid(1, -1));
                }
                else if (t == "d'" && p.startsWith("d")) {
                    tier_tokens->at(i)->setAttribute(d->attributePhonetisationOfTokens, "d");
                    tier_tokens->at(i+1)->setAttribute(d->attributePhonetisationOfTokens, p.mid(1, -1));
                }
                else if (t == "j'" && p.startsWith("Z")) {
                    tier_tokens->at(i)->setAttribute(d->attributePhonetisationOfTokens, "Z");
                    tier_tokens->at(i+1)->setAttribute(d->attributePhonetisationOfTokens, p.mid(1, -1));
                }
                else if (t == "l'" && p.startsWith("l")) {
                    tier_tokens->at(i)->setAttribute(d->attributePhonetisationOfTokens, "l");
                    tier_tokens->at(i+1)->setAttribute(d->attributePhonetisationOfTokens, p.mid(1, -1));
                }
                else if (t == "m'" && p.startsWith("m")) {
                    tier_tokens->at(i)->setAttribute(d->attributePhonetisationOfTokens, "m");
                    tier_tokens->at(i+1)->setAttribute(d->attributePhonetisationOfTokens, p.mid(1, -1));
                }
                else if (t == "n'" && p.startsWith("n")) {
                    tier_tokens->at(i)->setAttribute(d->attributePhonetisationOfTokens, "n");
                    tier_tokens->at(i+1)->setAttribute(d->attributePhonetisationOfTokens, p.mid(1, -1));
                }
                else if (t == "qu'" && p.startsWith("k")) {
                    tier_tokens->at(i)->setAttribute(d->attributePhonetisationOfTokens, "k");
                    tier_tokens->at(i+1)->setAttribute(d->attributePhonetisationOfTokens, p.mid(1, -1));
                }
                else if (t == "s'" && p.startsWith("s")) {
                    tier_tokens->at(i)->setAttribute(d->attributePhonetisationOfTokens, "s");
                    tier_tokens->at(i+1)->setAttribute(d->attributePhonetisationOfTokens, p.mid(1, -1));
                }
            }

            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tokens);
            ret.append(QString("Updated phonetisation for %1 %2\n").arg(annotationID).arg(speakerID));

        }
        qDeleteAll(tiersAll);
    }
    return ret;
}

QString PhonetiserExternal::correctPhonemeChains(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: no Communication";
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->levelTokens);
            if (!tier_tokens) continue;
            // exceptionally :
            for (int i = tier_tokens->count() - 2; i >= 1; --i) {
                if (tier_tokens->at(i)->isPauseSilent())
                    tier_tokens->removeInterval(i);
            }
            for (int i = 0; i < tier_tokens->count() - 1; ++i) {
                QString token = tier_tokens->at(i)->text();
                if (!d->citationForms.contains(token)) continue;
                QString citation = d->citationForms.value(token);
                QString phonetisation = tier_tokens->at(i)->attribute(d->attributePhonetisationOfTokens).toString();
                QString phonetisationAfter = tier_tokens->at(i + 1)->attribute(d->attributePhonetisationOfTokens).toString();
                QStringList phones = separatePhones(phonetisation);
                if (phones.isEmpty()) continue;
                QStringList phonesAfter = separatePhones(phonetisationAfter);
                if (phonesAfter.isEmpty()) continue;
                if ((phonetisation + phonesAfter.first() == citation) || (phonesAfter.first().endsWith("*"))) {
                    ret.append(QString("%1\t%2\t").arg(annotationID).arg(speakerID));
                    ret.append(token).append("\t").append(phonetisation).append(" | ").append(phonesAfter.join("")).append("\t-->\t");
                    QString phone = phonesAfter.takeFirst();
                    tier_tokens->at(i)->setAttribute(d->attributePhonetisationOfTokens, phonetisation + phone);
                    tier_tokens->at(i + 1)->setAttribute(d->attributePhonetisationOfTokens, phonesAfter.join(""));
                    ret.append(phonetisation + phone).append(" | ").append(phonesAfter.join("")).append("\n");
                }
            }
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tokens);
        }
        qDeleteAll(tiersAll);
    }
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

QStringList PhonetiserExternal::separatePhones(const QString &phonetisation) {
    // Separate phonemes
    QStringList separated;
    int pos = 0;
    while ((pos = d->regexMatchPhoneme.indexIn(phonetisation, pos)) != -1) {
        separated << d->regexMatchPhoneme.cap(0);
        pos += d->regexMatchPhoneme.matchedLength();
    }
    return separated;
}

bool PhonetiserExternal::readCitationFormDictionary(const QString &filename)
{
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    do {
        QString line = stream.readLine();
        if (line.startsWith("#")) continue;
        QStringList fields = line.split("\t");
        if (fields.count() > 1) d->citationForms.insert(fields.at(0), fields.at(1));
    } while (!stream.atEnd());
    file.close();
    return true;
}
