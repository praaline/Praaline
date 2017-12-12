#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>
#include <QHash>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "ProsodicUnits.h"

struct ProsodicUnitsData {
    ProsodicUnitsData() :
        levelTokens("tok_mwu"), attributePOS("pos_mwu"), levelProsodicUnits("prosodic_unit")
    {}

    QString levelTokens;
    QString attributePOS;
    QString levelProsodicUnits;

    QHash<QString, QString> posCategories;
};


ProsodicUnits::ProsodicUnits() :
    d(new ProsodicUnitsData)
{
    QStringList POS_CLI = QStringList( { "PRP:det", "DET:def", "DET:dem", "DET:int", "DET:par", "DET:pos",
                                         "PFX", "PRO:per:objd", "PRO:per:obji", "PRO:per:sjt", "PRO:ref", "PRP" } );
    QStringList POS_INT = QStringList( { "ADV:deg", "ADV:int", "ADV:neg", "CON:coo", "CON:sub", "DET:ind", "ITJ",
                                         "NUM:crd:det", "PRO:dem", "PRO:ind", "PRO:int", "PRO:rel",
                                         "VER:cond:aux", "VER:fut:aux", "VER:impe:aux", "VER:impf:aux", "VER:inf:aux",
                                         "VER:ppas:aux", "VER:ppre:aux", "VER:pres:aux", "VER:pres:entatif",
                                         "VER:pres:pred:aux",  "VER:simp:aux", "VER:subp:aux"} );
    QStringList POS_LEX = QStringList( { "ADJ", "ADV", "ADV:comp", "FRG", "INTROD", "NOM:acr", "NOM:com", "NOM:nom", "NOM:pro",
                                         "NUM:crd", "NUM:crd:nom", "NUM:ord:adj", "PRO:per:ton", "PRO:pos",
                                         "VER:cond", "VER:fut", "VER:impe", "VER:impf", "VER:inf", "VER:ppas",
                                         "VER:ppre", "VER:pres", "VER:simp", "VER:subi", "VER:subp" } );
    foreach (QString pos, POS_CLI) d->posCategories.insert(pos, "CLI");
    foreach (QString pos, POS_INT) d->posCategories.insert(pos, "INT");
    foreach (QString pos, POS_LEX) d->posCategories.insert(pos, "LEX");
}

ProsodicUnits::~ProsodicUnits()
{
    delete d;
}

QString ProsodicUnits::categorise_CLI_INT_LEX(Interval *token)
{
    if (!token) return "0";
    if (token->isPauseSilent()) return "_";
    QString currentPOS = token->attribute(d->attributePOS).toString();
    if (currentPOS == "0") return "FST"; // false starts
    if (d->posCategories.contains(currentPOS))
        return d->posCategories.value(currentPOS);
    return "LEX";
}

QString ProsodicUnits::createProsodicUnits(CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: no communication";
    if (!com->repository()) return "Error: communication not yet saved. Cannot find corpus repository for annotations";
    QMap<QString, QPointer<AnnotationTierGroup> > tiers = com->repository()->annotations()->getTiersAllSpeakers(com->ID());
    foreach (QString speakerID, tiers.keys()) {
        QPointer<AnnotationTierGroup> tiersSpk = tiers.value(speakerID);
        IntervalTier *tier_tok_mwu = tiersSpk->getIntervalTierByName(d->levelTokens);
        if (!tier_tok_mwu) continue;
        bool inside = false;
        RealTime start, end;
        QList<Interval *> units;
        int unitIterator = 1;
        // start at 0
        // 0 --CLI, INT--> 1  0 --pause--> 0  0--LEX-->0 and create unit
        // 1 --LEX-->0 and create unit 1-->
        foreach (Interval *tok_mwu, tier_tok_mwu->intervals()) {
            QString cat = categorise_CLI_INT_LEX(tok_mwu);
            bool groupBreak = (cat == "LEX" || cat == "0");
            if (!inside) {
                if (!tok_mwu->isPauseSilent()) {
                    if (!groupBreak) {
                        inside = true;
                        start = tok_mwu->tMin();
                    } else { // groupBreak==true
                        start = tok_mwu->tMin(); end = tok_mwu->tMax();
                        units << new Interval(start, end, QString::number(unitIterator));
                        start = end; ++unitIterator;
                    }
                } else {
                    start = tok_mwu->tMax();
                }
            } else {
                if (!tok_mwu->isPauseSilent()) {
                    if (groupBreak) {
                        end = tok_mwu->tMax();
                        units << new Interval(start, end, QString::number(unitIterator));
                        start = end; ++unitIterator;
                        inside = false;
                    }
                }
            }
        }
        IntervalTier *tier_units = new IntervalTier(d->levelProsodicUnits, units, RealTime(0, 0), tier_tok_mwu->tMax());
        com->repository()->annotations()->saveTier(com->ID(), speakerID, tier_units);
        ret.append("Created prosodic units: ").append(com->ID()).append("\t").append(speakerID);
    }
    qDeleteAll(tiers);
    return ret;
}

QString ProsodicUnits::transcriptionInProsodicUnits(CorpusCommunication *com)
{
    QString ret;
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiers = com->repository()->annotations()->getTiersAllSpeakers(com->ID());
    foreach (QString speakerID, tiers.keys()) {
        QPointer<AnnotationTierGroup> tiersSpk = tiers.value(speakerID);
        IntervalTier *tier_tok_mwu = tiersSpk->getIntervalTierByName(d->levelTokens);
        if (!tier_tok_mwu) continue;
        IntervalTier *tier_units = tiersSpk->getIntervalTierByName(d->levelProsodicUnits);
        if (!tier_units) continue;
        QString transcript;
        foreach (Interval *unit, tier_units->intervals()) {
            QList<Interval *> tokens = tier_tok_mwu->getIntervalsContainedIn(unit);
            foreach (Interval *token, tokens) {
                transcript.append(token->text()).append(" ");
            }
            transcript.append("| ");
        }
        if (!transcript.isEmpty()) transcript.chop(1);
        ret.append(QString("%1\t%2\t%3\n").arg(com->ID()).arg(speakerID).arg(transcript));
    }
    qDeleteAll(tiers);
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}

