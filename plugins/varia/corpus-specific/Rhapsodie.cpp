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

#include "ProsodicBoundaries.h"
#include "Rhapsodie.h"

Rhapsodie::Rhapsodie()
{

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
