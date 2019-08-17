#include <QString>
#include <QMap>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"

using namespace Praaline::Core;

#include "HTKAcousticModelTrainer.h"

namespace Praaline {
namespace ASR {

struct HTKAcousticModelTrainerData {
    QHash<QString, QString> phonemeTranslations;
};


HTKAcousticModelTrainer::HTKAcousticModelTrainer(QObject *parent) :
    QObject(parent), d(new HTKAcousticModelTrainerData)
{
    d->phonemeTranslations.insert("9~", "oe~");
    d->phonemeTranslations.insert("2", "eu");
    d->phonemeTranslations.insert("9", "oe");
    d->phonemeTranslations.insert("A", "a");
    d->phonemeTranslations.insert("*", "");
}

HTKAcousticModelTrainer::~HTKAcousticModelTrainer()
{
    delete d;
}

bool HTKAcousticModelTrainer::createMLF(
        const QString &filename, QList<CorpusCommunication *> trainingCommunications,
        const QString &levelSegment, const QString &levelToken, const QString &levelPhone, const QString &attributePhonetisation)
{
    // Create master label file
    QFile fileMLF(filename);
    if ( !fileMLF.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream mlf(&fileMLF);
    mlf.setCodec("ISO 8859-1");
    mlf << "#!MLF!#\n";
    // Process training corpus
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusCommunication *com, trainingCommunications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        if (!com->repository()) continue;
        if (!com->repository()->annotations()) continue;
        foreach (CorpusAnnotation *annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_segment = tiers->getIntervalTierByName(levelSegment);
                if (!tier_segment) continue;
                IntervalTier *tier_token = tiers->getIntervalTierByName(levelToken);
                if (!tier_token) continue;
                IntervalTier *tier_phone = 0;
                if (!levelPhone.isEmpty()) {
                    tier_phone = tiers->getIntervalTierByName(levelPhone);
                    if (!tier_phone) continue;
                }
                // Create list of utterances to process
                QList<Interval *> list_utterances;
                int i = 0;
                foreach (Interval *intv, tier_segment->intervals()) {
                    if (intv->isPauseSilent()) { i++; continue; }
                    Interval *utt = intv->clone();
                    QString spk = speakerID.replace(annotationID, "");
                    if (spk.startsWith("_")) spk = spk.remove(0, 1);
                    QString utteranceID = QString("%1_%2_%3").arg(annotationID).arg(spk).arg(i);
                    utt->setAttribute("utteranceID", utteranceID);
                    list_utterances << utt;
                    i++;
                }
                // Add utterances to MLF file
                foreach (Interval *utterance, list_utterances) {
                    QString utteranceID = utterance->attribute("utteranceID").toString();
                    QList<Interval *> tokens = tier_token->getIntervalsContainedIn(utterance);
                    if (tokens.isEmpty()) continue;
                    // Create phoneme string
                    QStringList phonemeString;
                    phonemeString << "sil";
                    foreach (Interval *token, tokens) {
                        // skip tokens starting with ( = paraverbal annotations
                        if (token->text().startsWith("(")) continue;
                        // phonemes for this token
                        if (!levelPhone.isEmpty()) {
                            // from annotation level
                            QList<Interval *> phones = tier_phone->getIntervalsContainedIn(token);
                            if (phones.isEmpty()) continue;
                            foreach (Interval *phone, phones) {
                                QString p = phone->text().trimmed();
                                if (d->phonemeTranslations.contains(p)) p = d->phonemeTranslations.value(p);
                                if (p == "_") p = "sp";
                                phonemeString << p;
                            }
                        } else {
                            // from phonetiser
                            QString phonetisation = token->attribute(attributePhonetisation).toString().trimmed();
                            foreach (QString p, phonetisation.split(" ", QString::SkipEmptyParts)) {
                                if (d->phonemeTranslations.contains(p)) p = d->phonemeTranslations.value(p);
                                if (p == "_") p = "sp";
                                phonemeString << p;
                            }
                        }
                        if (!token->text().endsWith("'"))
                            phonemeString << "sp";
                    }
                    phonemeString << "sil";
                    // Output phoneme string
                    if (phonemeString.count() == 2) continue; // only sil sil
                    mlf << "\"*/" << utteranceID << ".lab\"\n";
                    bool seenSilence = false;
                    for (int i = 0; i < phonemeString.count(); ++i) {
                        QString p = phonemeString.at(i);
                        if (seenSilence && ((p == "sil") || (p == "sp")) && i < phonemeString.count() - 1) continue;
                        if (p.startsWith("^")) continue;
                        p = p.remove("*");
                        if (p.isEmpty()) continue;
                        mlf << p << "\n";
                        if ((p == "sil") || (p == "sp"))
                            seenSilence = true;
                        else
                            seenSilence = false;
                    }
                    mlf << ".\n";
                }
            }
            qDeleteAll(tiersAll);
        }
    }
    fileMLF.close();
    return true;
}

bool HTKAcousticModelTrainer::createMasterLabelFileFromTokens(
        const QString &filename, QList<CorpusCommunication *> trainingCommunications,
        const QString &levelSegment, const QString &levelToken, const QString &attributePhonetisation)
{
    return createMLF(filename, trainingCommunications, levelSegment, levelToken, "", attributePhonetisation);
}

bool HTKAcousticModelTrainer::createMasterLabelFileFromTokensAndPhones(
        const QString &filename, QList<CorpusCommunication *> trainingCommunications,
        const QString &levelSegment, const QString &levelToken, const QString &levelPhone)
{
    return createMLF(filename, trainingCommunications, levelSegment, levelToken, levelPhone, "");
}

} // namespace ASR
} // namespace Praaline
