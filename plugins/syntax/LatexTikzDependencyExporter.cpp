#include "LatexTikzDependencyExporter.h"

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/RelationTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

struct LatexTikzDependencyExporterData {
    QString tierNameTokens;
    QString attributeNameSentenceID;
    QString attributeNameTokenID;
    QString attributeNamePOS;
    QString attributeNameDepHead;
    QString attributeNameDepRel;
    QString tagDepRelForRoot;
};

LatexTikzDependencyExporter::LatexTikzDependencyExporter(QObject *parent) :
    QObject(parent), d(new LatexTikzDependencyExporterData())
{
    d->tierNameTokens = "tok_ud";
    d->attributeNameSentenceID = "ud_sentid";
    d->attributeNameTokenID = "ud_id";
    d->attributeNamePOS = "ud_upos";
    d->attributeNameDepHead = "ud_head";
    d->attributeNameDepRel = "ud_deprel";
    d->tagDepRelForRoot = "root";
}

LatexTikzDependencyExporter::~LatexTikzDependencyExporter()
{
    delete d;
}

QString LatexTikzDependencyExporter::codeTikzDependencyForSentence(IntervalTier *tier_tokens, int tokenIndexFrom, int tokenIndexTo)
{
    QStringList textTokens;
    QStringList textPOS;
    QStringList dependencies;
    for (int tokenIndex = tokenIndexFrom; tokenIndex <= tokenIndexTo; ++tokenIndex) {
        if ((tokenIndex < 0) || (tokenIndex >= tier_tokens->count())) continue;
        Interval *token = tier_tokens->at(tokenIndex);
        textTokens << token->text();
        textPOS << token->attribute(d->attributeNamePOS).toString();
        if (tier_tokens->at(tokenIndex)->attribute(d->attributeNameDepRel).toString() == d->tagDepRelForRoot) {
            dependencies << QString("\t\\deproot{%1}{%2}\n")
                            .arg(token->attribute(d->attributeNameTokenID).toInt())
                            .arg(token->attribute(d->attributeNameDepRel).toString());
        } else {
            dependencies << QString("\t\\depedge{%1}{%2}{%3}\n")
                            .arg(token->attribute(d->attributeNameTokenID).toInt())
                            .arg(token->attribute(d->attributeNameDepHead).toInt())
                            .arg(token->attribute(d->attributeNameDepRel).toString());
        }
    }
    QString latex;
    latex.append("\\begin{dependency}\n");
    latex.append("\t\\begin{deptext}\n");
    latex.append("\t\t").append(textTokens.join(" \\& ")).append("\n");
    latex.append("\t\t").append(textPOS.join(" \\& ")).append("\n");
    latex.append("\t\\end{deptext}\n");
    latex.append(dependencies.join(""));
    latex.append("\\end{dependency}\n");
    return latex;
}

QString LatexTikzDependencyExporter::process(Praaline::Core::CorpusCommunication *com)
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
            IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->tierNameTokens);
            if (!tier_tokens) {
                ret.append("No tier for tokens ").append(d->tierNameTokens).append(" speakerID ").append(speakerID).append("\n");
                continue;
            }

            int tokenStart(-1), tokenEnd(-1), sentenceID(-1);
            for (int i = 0; i < tier_tokens->count() - 1; ++i) {
                Interval *token = tier_tokens->at(i);
                Interval *next_token = tier_tokens->at(i + 1);
                int sentID = token->attribute(d->attributeNameSentenceID).toInt();
                int nextSentID = next_token->attribute(d->attributeNameSentenceID).toInt();
                if ((sentenceID < 0) && (sentID > 0)) {
                    tokenStart = i;
                    tokenEnd   = i;
                    sentenceID = sentID;
                }
                if (sentID != nextSentID) {
                    tokenEnd = i;
                    sentenceID = -1;
                    ret.append(codeTikzDependencyForSentence(tier_tokens, tokenStart, tokenEnd));
                    tokenStart = -1;
                }
            }
            // ret.append("Processed ").append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}
