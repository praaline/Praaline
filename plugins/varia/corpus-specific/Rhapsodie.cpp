#include <QDebug>
#include <QSharedPointer>
#include <QString>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/PointTier.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
#include "pncore/interfaces/praat/PraatPitchFile.h"
using namespace Praaline::Core;

#include "pnlib/asr/phonetiser/DictionaryPhonetiser.h"
#include "pnlib/asr/htk/HTKForcedAligner.h"
#include "pnlib/asr/syllabifier/SyllabifierEasy.h"
using namespace Praaline::ASR;

#include "ProsodicBoundaries.h"
#include "Rhapsodie.h"

struct RhapsodieData {
    DictionaryPhonetiser phonetiser;
};

Rhapsodie::Rhapsodie() : d(new RhapsodieData)
{
    d->phonetiser.readPhoneticDictionary("/mnt/hgfs/DATA/PFCALIGN/phonetisation/fr.dict");
    d->phonetiser.readPhonemeTranslation("/mnt/hgfs/DATA/PFCALIGN/phonetisation/sphinx_to_sampa.json");
    d->phonetiser.readAdditionalPhoneticDictionary(QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_realign/extra_phon.txt");
}

Rhapsodie::~Rhapsodie()
{
    delete d;
}

QString Rhapsodie::prepareMultiSpeakerTextgrids(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    if (!com->ID().contains("Rhap-D")) return ret;
    InterfaceTextFile::setDefaultEncoding("UTF-8");


    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        // Open textgrid
        QSharedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup());
        QString path = QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_files/TextGrids-fev2013/";
        if (!PraatTextGrid::load(path + annotationID + "-Pro.TextGrid" , txg.data())) {
            ret.append("TextGrid not found"); continue;
        }
        // Get Rhapsodie TGs pivot
        IntervalTier *rh_pivot = txg->getIntervalTierByName("pivot");
        if (!rh_pivot) { ret.append(annotationID).append("\tNo tier pivot"); continue; }
        // Get tiers in database
        tiersAll = com->corpus()->repository()->annotations()->getTiersAllSpeakers(annotationID, QStringList()
                                                                                   << "phone" << "syll" << "tok_min");
        // Prepare database tiers by merging pauses
        foreach (QString speakerID, tiersAll.keys()) {
            tiersAll.value(speakerID)->mergeSilentPausesOnAllIntervalTiers("_");
        }
        // Examine all instances of overlap based on pivot
        for (int ipivot = 0; ipivot < rh_pivot->count(); ++ipivot) {
            Interval *pivot = rh_pivot->interval(ipivot);
            if (!pivot->text().contains("$-")) continue;
            QString pivotText = pivot->text().replace("$-", "").replace("$-", "")
                    .replace("  ", " ").replace("  ", " ").replace("  ", " ").replace("  ", " ");
            pivotText = pivotText.replace("'", "' ").replace("~", "/").replace("-", " ");
            // Distribute tokens to speakers
            QMap<QString, QStringList> spkTokensText;
            QString speakerID;
            foreach (QString tokenText, pivotText.split(" ", QString::SkipEmptyParts)) {
                if (tokenText.startsWith("$")) {
                    speakerID = annotationID + "_" + tokenText.trimmed();
                } else {
                    spkTokensText[speakerID].append(tokenText);
                }
            }
            // Sometimes there was no overlap
            if (spkTokensText.keys().count() == 1) continue;
            // User output
            ret.append(annotationID).append("\t").append(QString::number(pivot->tMin().toDouble())).append("\t");
            // Find which speaker's tokens are not there
            foreach (speakerID, spkTokensText.keys()) {
                if (!tiersAll.contains(speakerID)) {
                    ret.append(speakerID).append(" is not there!\n"); continue;
                }
                // Get tiers for this speaker
                IntervalTier *tier_tok_min = tiersAll.value(speakerID)->getIntervalTierByName("tok_min");
                if (!tier_tok_min) { ret.append(speakerID).append(" no tier tok_min\n"); continue; }
                IntervalTier *tier_phone = tiersAll.value(speakerID)->getIntervalTierByName("phone");
                if (!tier_phone) { ret.append(speakerID).append(" no tier phone\n"); continue; }
                IntervalTier *tier_syll = tiersAll.value(speakerID)->getIntervalTierByName("syll");
                if (!tier_syll) { ret.append(speakerID).append(" no tier syll\n"); continue; }
                // Check existing tokens for this speaker
                QString existing = tier_tok_min->getIntervalsTextOverlappingWith(pivot);
                if ((existing != "_") && (!existing.isEmpty())) continue;
                // This speaker's tokens should be patched
                QString utterance;
                foreach (QString tokenText, spkTokensText[speakerID]) utterance.append(tokenText).append(" ");
                utterance = utterance.trimmed();
                // Patch it
                QList<Interval *> tokens = tier_tok_min->patchTextToIntervalsProportional(spkTokensText[speakerID], pivot->tMin(), pivot->tMax());
                // Phonetise tokens
                QString phonetised;
                foreach (Interval *token, tokens) {
                    token->setAttribute("phonetisation", d->phonetiser.phonetiseToken(token->text()));
                    phonetised.append(token->attribute("phonetisation").toString()).append(" ");
                }
                // Align
                HTKForcedAligner aligner;
                QString alignerOutput;
                aligner.alignUtterance(com->recordings().first()->filePath(), rh_pivot, ipivot,
                                       tier_tok_min, tier_phone, alignerOutput);
                // Syllabify
                SyllabifierEasy syllabifier;
                syllabifier.syllabify(tier_phone, tier_syll, pivot->tMin(), pivot->tMax());
                // Fix pauses
                tier_phone->fillEmptyWith("", "_"); tier_phone->mergeIdenticalAnnotations("_");
                tier_syll->fillEmptyWith("", "_");  tier_syll->mergeIdenticalAnnotations("_");
                tier_tok_min->fillEmptyWith("", "_"); tier_tok_min->mergeIdenticalAnnotations("_");
                // User output
                ret.append(speakerID).append("\t").append(utterance).append("\t").append(existing).append("\t")
                        .append(QString::number(tokens.count())).append("\t").append(phonetised).append("\t");
            }
            ret.chop(1);
            ret.append("\n");
        }
        // Output TGs
        foreach (QString speakerID, tiersAll.keys()) {
            IntervalTier *tier_phone = new IntervalTier(tiersAll.value(speakerID)->getIntervalTierByName("phone"));
            IntervalTier *tier_syll = new IntervalTier(tiersAll.value(speakerID)->getIntervalTierByName("syll"));
            IntervalTier *tier_syll_contour = tier_syll->getIntervalTierWithAttributeAsText("rh_contour");
            IntervalTier *tier_syll_hes = tier_syll->getIntervalTierWithAttributeAsText("rh_hes");
            IntervalTier *tier_syll_prom = tier_syll->getIntervalTierWithAttributeAsText("rh_prom");
            IntervalTier *tier_tok_min = new IntervalTier(tiersAll.value(speakerID)->getIntervalTierByName("tok_min"));
            AnnotationTierGroup *txgspk = new AnnotationTierGroup();
            txgspk->addTier(tier_phone);
            txgspk->addTier(tier_syll);
            txgspk->addTier(tier_syll_contour);
            txgspk->addTier(tier_syll_hes);
            txgspk->addTier(tier_syll_prom);
            txgspk->addTier(tier_tok_min);
            txgspk->addTier(new IntervalTier(rh_pivot));
            QString filename = QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_realign/" +
                    QString(speakerID).replace("$", "") + ".TextGrid";
            PraatTextGrid::save(filename, txgspk);
            delete txgspk;
        }
        qDeleteAll(tiersAll);
    }
    d->phonetiser.writeTokensFalseStarts(QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_realign/00falsestarts.txt");
    d->phonetiser.writeTokensOutOfVocabulary(QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_realign/00oov.txt");
    if (!ret.isEmpty()) ret.chop(1);
    return ret;
}



QString Rhapsodie::updateSyllables(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;

    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        // Open textgrid
        QSharedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup());
        QString path = QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_files/TextGrids-fev2013/";
        if (!PraatTextGrid::load(path + annotationID + "-Pro.TextGrid" , txg.data())) {
            ret.append("TextGrid not found"); continue;
        }
        // Get Rhapsodie TGs syllable, prom, hes, contour
        IntervalTier *rh_syll = txg->getIntervalTierByName("syllabe");
        if (!rh_syll) { ret.append(annotationID).append("\tNo tier syll"); continue; }
        IntervalTier *rh_prom = txg->getIntervalTierByName("prom");
        if (!rh_prom) { ret.append(annotationID).append("\tNo tier prom"); continue; }
        IntervalTier *rh_hes = txg->getIntervalTierByName("hes");
        if (!rh_hes) { ret.append(annotationID).append("\tNo tier hes"); continue; }
        IntervalTier *rh_contour = txg->getIntervalTierByName("contour");
        // contour is optional
        if (rh_syll->count() != rh_prom->count()) {
            ret.append(annotationID).append(QString("\tCount mismatch syll %1 - prom %2").arg(rh_syll->count()).arg(rh_prom->count())); continue;
        }
        if (rh_syll->count() != rh_hes->count()) {
            ret.append(annotationID).append(QString("\tCount mismatch syll %1 - hes %2").arg(rh_syll->count()).arg(rh_hes->count())); continue;
        }
        if ((rh_contour) && (rh_syll->count() != rh_contour->count())) {
            ret.append(annotationID).append(QString("\tCount mismatch syll %1 - contour %2").arg(rh_syll->count()).arg(rh_contour->count())); continue;
        }
        // Move all data to rh_syll
        for (int i = 0; i < rh_syll->count(); ++i) {
            rh_syll->at(i)->setAttribute("rh_prom", rh_prom->at(i)->text());
            rh_syll->at(i)->setAttribute("rh_hes", rh_hes->at(i)->text());
            if (rh_contour) rh_syll->at(i)->setAttribute("rh_contour", rh_contour->at(i)->text());
        }
        // Update syllables in tiers, per speaker, note progress
        tiersAll = com->corpus()->repository()->annotations()->getTiersAllSpeakers(annotationID, QStringList() << "syll");
        foreach (QString speakerID, tiersAll.keys()) {
            bool OK = true;
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
            if (!tier_syll) continue;
            // Check syllable correspondance
            foreach (Interval *syll, tier_syll->intervals()) {
                if (syll->isPauseSilent()) continue;
                Interval *rhs = rh_syll->intervalAtTime(syll->tCenter());
                if (rhs->text() != syll->text()) {
                    ret.append(annotationID).append("\t").append(speakerID).append("\t");
                    ret.append(QString::number(syll->tMin().toDouble())).append("\t");
                    ret.append(syll->text()).append("\t").append(rhs->text()).append("\n");
                    OK = false;
                }
            }
            if (OK) {
                // Update syllables
                foreach (Interval *syll, tier_syll->intervals()) {
                    if (syll->isPauseSilent()) {
                        syll->setAttribute("rh_prom", "_");
                        syll->setAttribute("rh_hes", "_");
                        syll->setAttribute("rh_contour", "_");
                        continue;
                    }
                    Interval *rhs = rh_syll->intervalAtTime(syll->tCenter());
                    syll->setAttribute("rh_prom", rhs->attribute("rh_prom"));
                    syll->setAttribute("rh_hes", rhs->attribute("rh_hes"));
                    syll->setAttribute("rh_contour", rhs->attribute("rh_contour"));
                }
                com->corpus()->repository()->annotations()->saveTier(annotationID, speakerID, tier_syll);
                ret.append(annotationID).append("\t").append(speakerID).append(QString("\t%1\tUpdated\n").arg(tier_syll->count()));
            }
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}

QString Rhapsodie::loadPitch(QPointer<CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        // Load pitch file
        QString path = QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_files/Rhap-courbes-liss/";
        PraatPitch pitch;
        if (!PraatPitchFile::load(path + annotationID + "-liss.Pitch", pitch)) {
            ret.append("Pitch not found"); continue;
        }

        PointTier *pitch_tier = new PointTier("pitch_smooth");
        RealTime t = pitch.tstart;
        QList<Point *> points;
        for (int i = 0; i < pitch.frames.count(); ++i) {
            PraatPitchFrame frame = pitch.frames.at(i);
            t = t + pitch.dx;
            Point *p = new Point(t);
            p->setAttribute("frequency", frame.candidates.first().frequency);
            points << p;
        }
        pitch_tier->addPoints(points);
        com->repository()->annotations()->saveTier(annotationID, "pitch", pitch_tier);
        ret.append(annotationID).append(" Pitch imported");
    }
    return ret;
}

void processConstNode(const QDomNode& node, QPointer<CorpusCommunication> com)
{
    QDomNode domNode = node.firstChild();
    QDomElement domElement;
    QStringList attributeIDs;
    attributeIDs << "ind" << "num" << "phonetictext" << "shape" << "type" << "localreg" << "name";
    while (!(domNode.isNull())) {
        if(domNode.isElement()) {
            domElement = domNode.toElement();
            if (!(domElement.isNull())) {
                QString ctype = domElement.attribute("ctype");
                if (ctype == "wordsTree") return; // do not process words tree
                if (ctype == "period" || ctype == "package" || ctype == "group" || ctype == "foot") {
                    RealTime tMin = RealTime::fromSeconds(domElement.attribute("start").toDouble());
                    RealTime tMax = RealTime::fromSeconds(domElement.attribute("end").toDouble());
                    QString orthotext = domElement.attribute("orthotext");
                    Interval *intv = new Interval(tMin, tMax, orthotext);
                    intv->setAttribute("ind", domElement.attribute("ind").toInt());
                    intv->setAttribute("num", domElement.attribute("num").toInt());
                    intv->setAttribute("phonetictext", domElement.attribute("phonetictext"));
                    intv->setAttribute("shape", domElement.attribute("shape"));
                    intv->setAttribute("type", domElement.attribute("type"));
                    intv->setAttribute("localreg", domElement.attribute("localreg"));
                    intv->setAttribute("name", domElement.attribute("name"));
                    intv->setAttribute("annotationID", com->ID());
                    intv->setAttribute("speakerID", "prosody");
                    intv->setAttribute("indexNo", domElement.attribute("num").toInt());
                    QList<AnnotationElement *> intervals; intervals << intv;
                    com->repository()->annotations()->saveAnnotationElements(intervals, QString("rh_") + ctype, attributeIDs);
                    qDebug() << domElement.tagName() << domElement.attribute("ctype") << domElement.attribute("num");
                }
            }
        }
        processConstNode(domNode, com);
        domNode = domNode.nextSibling();
    }
}

QString Rhapsodie::readProsodicConstituencyTree(QPointer<CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;

    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        // Open XML file
        QString path = QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_files/Rhap-proso-all-xml/";
        QFile file(path + annotationID + "-Pro.xml");
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            ret.append(annotationID).append("\tCannot open XML file :").append(file.errorString());
            continue;
        }
        QDomDocument domDocument;
        QString errorStr;
        int errorLine, errorColumn;
        if (!domDocument.setContent(&file, true, &errorStr, &errorLine, &errorColumn)) {
            ret.append(annotationID).append(QString("\tError parsing XML at %1, %2 : %3").arg(errorLine).arg(errorColumn).arg(errorStr));
            continue;
        }
        // Move data from XML to annotation tiers
        QDomElement element = domDocument.documentElement();
        QDomNode n = element.firstChild();
        processConstNode(n, com);

        ret = ret.append(annotationID).append("\tOK");
        qDeleteAll(tiersAll);
    }
    return ret;
}

QString Rhapsodie::noteProsodicBoundaryOnSyll(QPointer<CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;

    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->corpus()->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
            if (!tier_syll) continue;
            QList<QPair<QString, QString> > groupingTierNames;
            groupingTierNames << QPair<QString, QString>("rh_foot", "FT");
            groupingTierNames << QPair<QString, QString>("rh_group", "GRP");
            groupingTierNames << QPair<QString, QString>("rh_package", "PCK");
            groupingTierNames << QPair<QString, QString>("rh_period", "PER");
            QPair<QString, QString> groupingTierName;
            foreach (groupingTierName, groupingTierNames) {
                IntervalTier *tier_group = tiersAll.value("prosody")->getIntervalTierByName(groupingTierName.first);
                if (!tier_group) { ret.append("Tier ").append(groupingTierName.first).append(" not found\n"); continue; }
                foreach (Interval *group, tier_group->intervals()) {
                    if (group->attribute("phonetictext").toString().isEmpty()) continue;
                    if (group->attribute("phonetictext").toString().trimmed() == "_") continue;
                    Interval *syll = tier_syll->intervalAtTime(group->tMax(), true);
                    if (syll && !syll->isPauseSilent()) {
                        syll->setAttribute("boundary", groupingTierName.second);
                        ret.append(annotationID).append("\t").append(speakerID).append("\t").append(syll->text()).append("\t");
                        ret.append(groupingTierName.second).append("\t").append(group->attribute("num").toString()).append("\t");
                        ret.append(group->attribute("phonetictext").toString()).append("\t").append(group->text()).append("\n");
                    }
                }
            }
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_syll);
        }
        qDeleteAll(tiersAll);
        // ret = ret.append(annotationID).append("\tOK");
    }
    return ret;
}

QString Rhapsodie::importMicrosyntaxCONLL(QPointer<CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        // Load pitch file
        QString path = QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_files/Microsyntaxe_conll/";
        QFile file(path + QString(annotationID).replace("-", ".") + ".micro.conll");



        // com->repository()->annotations()->saveTier(annotationID, "pitch", pitch_tier);
        // ret.append(annotationID).append(" Pitch imported");
    }
    return ret;
}

QString Rhapsodie::importMicrosyntaxTabular(QPointer<CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        // Load pitch file
        QString path = QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_files/Microsyntaxe_conll/";
        QFile file(path + QString(annotationID).replace("-", ".") + ".micro.conll");



        // com->repository()->annotations()->saveTier(annotationID, "pitch", pitch_tier);
        // ret.append(annotationID).append(" Pitch imported");
    }
    return ret;
}

QString Rhapsodie::exportProsodicBoundariesAnalysisTable(QPointer<Praaline::Core::Corpus> corpus)
{
    QString filename = QDir::homePath() + "/Dropbox/CORPORA/Boundaries_Rhapsodie.txt";
    QFile file(filename);
    if ( !file.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) ) return "Error writing output file";
    QTextStream out(&file);
    out.setCodec("UTF-8");
    ProsodicBoundaries PBAnalyser;
    PBAnalyser.setAdditionalAttributeIDs(QStringList() << "boundarySyntactic" << "tok_mwu_text"
                                         << "sequence_text" << "rection_text"
                                         << "promise_pos" << "promise_boundary" << "promise_contour"
                                         << "rh_prom" << "rh_hes" << "rh_contour");
    out << PBAnalyser.headerLineForTables() << "\n";
    foreach (QString annotationID, corpus->annotationIDs()) {
        PBAnalyser.analyseAnnotationToStream(out, corpus, annotationID);
    }
    file.close();
    return "OK";
}
