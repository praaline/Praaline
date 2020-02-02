#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "PraalineASR/Sphinx/SphinxLanguageModelBuilder.h"

namespace Praaline {
namespace ASR {

struct SphinxLanguageModelBuilderData {
    SphinxLanguageModelBuilderData() :
        minimumNumberOfTokensInUtteranceFilter(0)
    {}

    QString levelUtterances, attributeUtterances;
    QString levelTokens, attributeTokens;
    QStringList speakersInclude;
    QStringList speakersExclude;
    int minimumNumberOfTokensInUtteranceFilter;
};

SphinxLanguageModelBuilder::SphinxLanguageModelBuilder(QObject *parent) :
    QObject(parent), d(new SphinxLanguageModelBuilderData)
{
    // Default values for utterance and token tiers
    d->levelTokens = "segment";
    d->levelTokens = "tok_min";
}

SphinxLanguageModelBuilder::~SphinxLanguageModelBuilder()
{
    delete d;
}

QString SphinxLanguageModelBuilder::utterancesTierLevelID() const
{
    return d->levelUtterances;
}

QString SphinxLanguageModelBuilder::utterancesTierAttributeID() const
{
    return d->attributeUtterances;
}

void SphinxLanguageModelBuilder::setUtterancesTier(const QString &levelID, const QString &attributeID)
{
    d->levelUtterances = levelID;
    d->attributeUtterances = attributeID;
}

QString SphinxLanguageModelBuilder::tokensTierLevelID() const
{
    return d->levelTokens;
}

QString SphinxLanguageModelBuilder::tokensTierAnnotationID() const
{
    return d->attributeTokens;
}

void SphinxLanguageModelBuilder::setTokensTier(const QString &levelID, const QString &attributeID)
{
    d->levelTokens = levelID;
    d->attributeTokens = attributeID;
}

QStringList SphinxLanguageModelBuilder::speakersIncludeFilter() const
{
    return d->speakersInclude;
}

void SphinxLanguageModelBuilder::setSpeakersIncludeFilter(const QStringList &speakerIDs)
{
    d->speakersInclude = speakerIDs;
}

QStringList SphinxLanguageModelBuilder::speakersExcludeFilter() const
{
    return d->speakersExclude;
}

void SphinxLanguageModelBuilder::setSpeakersExcludeFilter(const QStringList &speakerIDs)
{
    d->speakersExclude = speakerIDs;
}

int SphinxLanguageModelBuilder::minimumNumberOfTokensInUtteranceFilter() const
{
    return d->minimumNumberOfTokensInUtteranceFilter;
}

void SphinxLanguageModelBuilder::setMinimumNumberOfTokensInUtteranceFilter(int min)
{
    d->minimumNumberOfTokensInUtteranceFilter = min;
}

QStringList SphinxLanguageModelBuilder::getNormalisedUtterances(CorpusAnnotation *annotation)
{
    QStringList normalisedUtterances;
    // Sanity checks
    if (!annotation) return normalisedUtterances;
    if (!annotation->repository()) return normalisedUtterances;
    if (!annotation->repository()->annotations()) return normalisedUtterances;
    // Process annotation
    QString annotationID = annotation->ID();
    SpeakerAnnotationTierGroupMap tiersAll = annotation->repository()->annotations()->getTiersAllSpeakers(annotationID);
    foreach (QString speakerID, tiersAll.keys()) {
        // Process all speakers based on the inclusion/exclusion filters
        if ((!d->speakersInclude.isEmpty()) && (!d->speakersInclude.contains(speakerID))) continue;
        if ((!d->speakersExclude.isEmpty()) && (d->speakersExclude.contains(speakerID))) continue;
        AnnotationTierGroup *tiers = tiersAll.value(speakerID);
        if (!tiers) continue;
        // Get tiers: an utterance tier is necessary, a token tier is optional
        IntervalTier *tier_utterance = tiers->getIntervalTierByName(d->levelUtterances);
        if (!tier_utterance) continue;
        IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->levelTokens);
        // Find transcribed utterances
        QList<Interval *> list_utterances;
        int i = 0;
        foreach (Interval *intv, tier_utterance->intervals()) {
            // Check if utterance is empty, or if has less tokens than the minimum number
            if (intv->isPauseSilent()) { i++; continue; }
            QList<Interval *> tokens;
            if (tier_tokens) {
                tokens = tier_tokens->getIntervalsContainedIn(intv);
                if (tokens.count() < d->minimumNumberOfTokensInUtteranceFilter) { i++; continue; }
            }
            QString utteranceID = QString("%1_%2_%3").arg(annotationID).arg(speakerID).arg(i);
            // OK to proceed: add utterance to the list
            Interval *utt = intv->clone();
            utt->setAttribute("utteranceID", utteranceID);
            list_utterances << utt;
            QString transcription;
            if (tier_tokens) {
                transcription.append("<s> ");
                foreach (Interval *token, tokens) {
                    if (token->isPauseSilent()) continue;
                    QString t = (d->attributeTokens.isEmpty()) ? token->text() : token->attribute(d->attributeTokens).toString();
                    transcription.append(t).append(" ");
                    // if ((d->dictionary) && (!d->dictionary->contains(t))) outUnknownWordsList << t;
                }
                transcription.append("</s>");
            } else {
                QString transcript = (d->attributeUtterances.isEmpty()) ? utt->text() : utt->attribute(d->attributeUtterances).toString();
                transcript = transcript.replace("'", "' ");
                foreach (QString word, transcript.split(" ")) {
                    // if (!d->dictionary->contains(word)) outUnknownWordsList << word;
                }
                transcription.append("<s> ").append(transcript).append(" </s>");
            }
            i++; // proceed to next utterance
            normalisedUtterances << transcription;
        }
        qDeleteAll(list_utterances);
    }
    qDeleteAll(tiersAll);
    // emit printMessage(QString("Created Sphinx transcription files for %1/%2").arg(com->ID()).arg(rec->ID()));
    return normalisedUtterances;
}

} // namespace ASR
} // namespace Praaline
