#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include "ImportCorpusItemsWizardFinalPage.h"
#include "ui_ImportCorpusItemsWizardFinalPage.h"

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/FileDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"

#include "pncore/interfaces/praat/PraatTextGrid.h"
#include "pncore/interfaces/transcriber/TranscriberAnnotationGraph.h"
#include "pncore/interfaces/subtitles/SubtitlesFile.h"

struct ImportCorpusItemsWizardFinalPageData {
    ImportCorpusItemsWizardFinalPageData(QPointer<CorpusRepository> repository,
                                         QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &candidateRecordings,
                                         QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &candidateAnnotations,
                                         QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
                                         QSet<QString> &tierNamesCommon) :
        repository(repository), candidateRecordings(candidateRecordings), candidateAnnotations(candidateAnnotations),
        tierCorrespondances(tierCorrespondances), tierNamesCommon(tierNamesCommon)
    {}

    QPointer<CorpusRepository> repository;
    QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &candidateRecordings;
    QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &candidateAnnotations;
    QMultiHash<QString, TierCorrespondance> &tierCorrespondances;
    QSet<QString> &tierNamesCommon;
};

ImportCorpusItemsWizardFinalPage::ImportCorpusItemsWizardFinalPage(
        QPointer<CorpusRepository> repository,
        QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &candidateRecordings,
        QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> > &candidateAnnotations,
        QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
        QSet<QString> &tierNamesCommon,
        QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ImportCorpusItemsWizardFinalPage),
    d(new ImportCorpusItemsWizardFinalPageData (repository, candidateRecordings, candidateAnnotations,
                                                tierCorrespondances, tierNamesCommon))
{
    ui->setupUi(this);
}

ImportCorpusItemsWizardFinalPage::~ImportCorpusItemsWizardFinalPage()
{
    delete ui;
    delete d;
}

void ImportCorpusItemsWizardFinalPage::initializePage()
{

}

bool ImportCorpusItemsWizardFinalPage::validatePage()
{
    QMap<QPair<QString, QString>, QPointer<CorpusRecording> >::iterator i;
    QMap<QPair<QString, QString>, QPointer<CorpusAnnotation> >::iterator j;

//    ui->texteditMessagesFiles->clear();
//    // Corpus media directory to relativise media file paths
//    QDir dirMedia(d->repository->files()->basePath());
//    //

//    // Add recordings
//    int counter = 0;
//    for (i = d->candidateRecordings.begin(); i != d->candidateRecordings.end(); ++i) {
//        QString communicationID = i.key().first;
//        QPointer<CorpusRecording> rec = i.value();
//        if (!rec) continue;
//        QPointer<CorpusCommunication> com = d->corpus->communication(communicationID);
//        if (!com) {
//            com = new CorpusCommunication(communicationID);
//            com->setName(communicationID);
//            d->corpus->addCommunication(com);
//            ui->texteditMessagesFiles->appendPlainText(QString(tr("Communication %1 added.")).arg(com->ID()));
//        }
//        if (com->hasRecording(rec->ID())) {
//            ui->texteditMessagesFiles->appendPlainText(QString(tr("Recording %1 already exists, not added.")).arg(rec->ID()));
//        } else {
//            rec->setFilename(dirMedia.relativeFilePath(rec->filename()));
//            com->addRecording(rec);
//            ui->texteditMessagesFiles->appendPlainText(QString(tr("Recording %1 added.")).arg(rec->ID()));
//        }
//        if ((counter % 100) == 0) QApplication::processEvents();
//        counter++;
//    }
//    ui->texteditMessagesFiles->appendPlainText(QString(tr("Added %1 recording(s).")).arg(d->candidateRecordings.count()));
//    // Add annotations
//    for (j = d->candidateAnnotations.begin(); j != d->candidateAnnotations.end(); ++j) {
//        QString communicationID = j.key().first;
//        QPointer<CorpusAnnotation> annot = j.value();
//        if (!annot) continue;
//        QPointer<CorpusCommunication> com = d->corpus->communication(communicationID);
//        if (!com) {
//            com = new CorpusCommunication(communicationID);
//            com->setName(communicationID);
//            d->corpus->addCommunication(com);
//        }
//        if (com->hasAnnotation(annot->ID())) {
//            ui->texteditMessagesFiles->appendPlainText(QString(tr("Annotation %1 already exists, merging annotations but not metadata.")).arg(annot->ID()));
//        } else {
//            com->addAnnotation(annot);
//        }
//    }
//    ui->texteditMessagesFiles->appendPlainText(QString(tr("Added %1 annotations(s).")).arg(d->candidateAnnotations.count()));
//    // Import annotations
//    ui->progressBarFiles->setValue(0);
//    ui->progressBarFiles->setMaximum(d->candidateAnnotations.count());
//    int count = 0;
//    for (j = d->candidateAnnotations.begin(); j != d->candidateAnnotations.end(); ++j) {
//        QString communicationID = j.key().first;
//        QPointer<CorpusAnnotation> annot = j.value();
//        if (!annot) continue;
//        QPointer<CorpusCommunication> com = d->corpus->communication(communicationID);
//        if (!com) continue;
//        QList<TierCorrespondance> correspondances;
//        foreach (TierCorrespondance c, d->tierCorrespondances.values(annot->filename())) {
//            if (!c.annotationLevelID.isEmpty()) correspondances << c;
//        }
//        if (annot->filename().toLower().endsWith("textgrid")) {
//            importPraat(com, annot, correspondances);
//        }
//        else if (annot->filename().toLower().endsWith("trs")) {
//            importTranscriber(com, annot, correspondances);
//        }
//        else if (annot->filename().toLower().endsWith("srt")) {
//            importSubRipTranscription(com, annot, correspondances);
//        }
//        count++;
//        ui->progressBarFiles->setValue(count);
//        QApplication::processEvents();
//    }
    return true;
}


void importBasic(CorpusRepository *repository, CorpusCommunication *com, CorpusAnnotation *annot, int speakerPolicy, QString speakerID,
                 AnnotationTierGroup *inputTiers, QList<TierCorrespondance> &correspondances)
{
//    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
//    QString spkID;

//    // Step 0: Get tiers for current speaker if they already exist
//    tiersAll = repository->annotations()->getTiersAllSpeakers(annot->ID());
//    // Step 1a: Add a tier for each level and speaker
//    foreach (TierCorrespondance c, correspondances) {
//        if (c.annotationLevelID.isEmpty() || !c.annotationAttributeID.isEmpty()) continue;
//        AnnotationTier *tierL = inputTiers->tier(inputTiers->getTierIndexByName(c.tierName));
//        if (!tierL) continue;
//        tierL->setName(c.annotationLevelID);
//        tierL->setProperty("inputTierName", c.tierName);
//        if (speakerPolicy == SpeakerPolicyTierNames)
//            spkID = c.tierName;
//        else
//            spkID = speakerID;
//        if (!tiersAll.contains(spkID)) tiersAll.insert(spkID, new AnnotationTierGroup());
//        tiersAll[spkID]->addTierReplacing(tierL);
//        if (!corpus->hasSpeaker(spkID))
//            corpus->addSpeaker(new CorpusSpeaker(spkID));
//    }
//    // Step 1b: Add participations of speakers
//    foreach (QString spkID, tiersAll.keys()) {
//        if (!corpus->hasParticipation(com->ID(), spkID)) {
//            corpus->addParticipation(com->ID(), spkID, QObject::tr("Participant"));
//        }
//    }
//    // Step 2: Update attributes for each level and speaker
//    foreach (TierCorrespondance c, correspondances) {
//        if (c.annotationLevelID.isEmpty() || c.annotationAttributeID.isEmpty()) continue;
//        AnnotationTier *tierA = inputTiers->tier(inputTiers->getTierIndexByName(c.tierName));
//        if (!tierA) continue;
//        // Speaker ID as defined by the policy
//        if (speakerPolicy == SpeakerPolicyTierNames)
//            spkID = c.tierName;
//        else
//            spkID = speakerID;
//        if (!tiersAll.contains(spkID)) continue;
//        AnnotationTierGroup *tiers = tiersAll.value(spkID);
//        // corresponding level
//        AnnotationTier *tierL = tiers->tier(tiers->getTierIndexByName(c.annotationLevelID));
//        if (!tierL) continue;
//        // apply correspondance
//        if (tierL->tierType() == AnnotationTier::TierType_Intervals && tierA->tierType() == AnnotationTier::TierType_Intervals) {
//            IntervalTier *tierLI = qobject_cast<IntervalTier *>(tierL);
//            IntervalTier *tierAI = qobject_cast<IntervalTier *>(tierA);
//            for (int i = 0; i < tierLI->count(); ++i) {
//                Interval *intv = tierAI->intervalAtTime(tierLI->interval(i)->tCenter());
//                if (intv) tierLI->interval(i)->setAttribute(c.annotationAttributeID, intv->text());
//            }
//        }
//        else if (tierL->tierType() == AnnotationTier::TierType_Intervals && tierA->tierType() == AnnotationTier::TierType_Points) {
//            IntervalTier *tierLI = qobject_cast<IntervalTier *>(tierL);
//            PointTier *tierAP = qobject_cast<PointTier *>(tierA);
//            for (int i = 0; i < tierLI->count(); ++i) {
//                Point *pnt = tierAP->pointAtTime(tierLI->interval(i)->tCenter());
//                if (pnt) tierLI->interval(i)->setAttribute(c.annotationAttributeID, pnt->text());
//            }
//        }
//        else if (tierL->tierType() == AnnotationTier::TierType_Points && tierA->tierType() == AnnotationTier::TierType_Intervals) {
//            PointTier *tierLP = qobject_cast<PointTier *>(tierL);
//            IntervalTier *tierAI = qobject_cast<IntervalTier *>(tierA);
//            for (int i = 0; i < tierLP->count(); ++i) {
//                Interval *intv = tierAI->intervalAtTime(tierLP->point(i)->time());
//                if (intv) tierLP->point(i)->setAttribute(c.annotationAttributeID, intv->text());
//            }
//        }
//        else if (tierL->tierType() == AnnotationTier::TierType_Points && tierA->tierType() == AnnotationTier::TierType_Points) {
//            PointTier *tierLP = qobject_cast<PointTier *>(tierL);
//            PointTier *tierAP = qobject_cast<PointTier *>(tierA);
//            for (int i = 0; i < tierLP->count(); ++i) {
//                Point *pnt = tierAP->pointAtTime(tierLP->point(i)->time());
//                if (pnt) tierLP->point(i)->setAttribute(c.annotationAttributeID, pnt->text());
//            }
//        }
//    }
//    repository->annotations()->saveTiersAllSpeakers(annot->ID(), tiersAll);
//    qDeleteAll(tiersAll);
}

void ImportCorpusItemsWizardFinalPage::importTranscriber(QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot,
                                                         QList<TierCorrespondance> &correspondances)
{
//    Q_UNUSED(correspondances)
//    ui->texteditMessagesFiles->appendPlainText(QString(tr("Importing %1/%2...")).arg(com->ID()).arg(annot->ID()));

//    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
//    QList<QPointer<CorpusSpeaker> > speakers;
//    bool result = TranscriberAnnotationGraph::load(annot->filename(), speakers, tiersAll);
//    if (!result) {
//        return;
//    }
//    // Add speakers if necessary
//    foreach (QPointer<CorpusSpeaker> spk, speakers) {
//        if (!d->corpus->hasSpeaker(spk->ID())) {
//            d->corpus->addSpeaker(spk);
//        }
//    }
//    // Add participations of speakers
//    foreach (QString spkID, tiersAll.keys()) {
//        if (!d->corpus->hasParticipation(com->ID(), spkID)) {
//            d->corpus->addParticipation(com->ID(), spkID, tr("Participant"));
//        }
//    }
//    d->corpus->datastoreAnnotations()->saveTiersAllSpeakers(annot->ID(), tiersAll);
//    qDeleteAll(tiersAll);
}

void ImportCorpusItemsWizardFinalPage::importSubRipTranscription(QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot,
                                                                 QList<TierCorrespondance> &correspondances)
{
    Q_UNUSED(correspondances)
    ui->texteditMessagesFiles->appendPlainText(QString(tr("Importing %1/%2...")).arg(com->ID()).arg(annot->ID()));

    QPointer<IntervalTier> tierTranscription = new IntervalTier();
    bool result = SubtitlesFile::loadSRT(annot->filename(), tierTranscription);
    tierTranscription->setName("transcription");
    if (!result) {
        return;
    }
//    // Add speakers if necessary
//    QString speakerID = annot->ID();
//    if (!d->corpus->hasSpeaker(speakerID))
//        d->corpus->addSpeaker(new CorpusSpeaker(speakerID));
//    if (!d->corpus->hasParticipation(com->ID(), speakerID))
//        d->corpus->addParticipation(com->ID(), speakerID, tr("Participant"));
//    d->corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tierTranscription);
}

void ImportCorpusItemsWizardFinalPage::importPraat(QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot,
                                                   QList<TierCorrespondance> &correspondances)
{
    ui->texteditMessagesFiles->appendPlainText(QString(tr("Importing %1/%2...")).arg(com->ID()).arg(annot->ID()));
    AnnotationTierGroup *inputTiers = new AnnotationTierGroup();

    PraatTextGrid::load(annot->filename(), inputTiers);

    int policy = annot->property("speakerPolicy").toInt();
    QString annotSpeakerID = annot->property("speakerPolicyData").toString();
    if (policy == SpeakerPolicySingle) {
        if (annotSpeakerID.isEmpty()) {
            // no speaker ID given, use the filename
            importBasic(d->repository, com, annot, policy, QFile(annot->filename()).fileName().remove(".textgrid"),
                        inputTiers, correspondances);
        }
        else {
            importBasic(d->repository, com, annot, policy, annotSpeakerID, inputTiers, correspondances);
        }
    }
    else if (policy == SpeakerPolicyTierNames) {
        importBasic(d->repository, com, annot, policy, "", inputTiers, correspondances);
    }
    else if ((policy == SpeakerPolicyPrimaryAndSecondary) || (policy == SpeakerPolicyIntervals)) {
        IntervalTier *tierSpeaker = 0;
        bool shouldDeleteSpeakerTier = false;
        if (policy == SpeakerPolicyPrimaryAndSecondary) {
            QString tiernameSpeaker1 = annot->property("speakerPolicyData").toString().section(";", 0, 0);
            QString tiernameSpeaker2 = annot->property("speakerPolicyData").toString().section(";", 1, 1);
            IntervalTier *tierSpeaker1 = inputTiers->getIntervalTierByName(tiernameSpeaker1);
            IntervalTier *tierSpeaker2 = inputTiers->getIntervalTierByName(tiernameSpeaker2);
            if (tierSpeaker1 && tierSpeaker2) {
                tierSpeaker = new IntervalTier("speaker", tierSpeaker1, tierSpeaker2, "S1", "S2", "S1");
                shouldDeleteSpeakerTier = true;
            }
        }
        else if (policy == SpeakerPolicyIntervals) {
            // There is a speaker ID tier
            QString speakerTierName = annot->property("speakerPolicyData").toString();
            tierSpeaker = inputTiers->getIntervalTierByName(speakerTierName);
            // Hack for CHumour
            // tierSpeaker->replaceText("gap", "");
        }
        if (!tierSpeaker) {
            importBasic(d->repository, com, annot, SpeakerPolicySingle, annot->ID(), inputTiers, correspondances);
            return;
        }
//      // Rhapsodie hack
//        foreach (Interval *intv, tierSpeaker->intervals()) {
//            QString s = intv->text().section("-$", 0, 0);
//            intv->setText(s);
//        }
//        tierSpeaker->mergeIdenticalAnnotations();
        QStringList speakerNames;
        QStringList speakerLabels = tierSpeaker->getDistinctTextLabels();
        speakerLabels.removeOne(""); speakerLabels.removeOne("_");
        foreach (QString speakerLabel, speakerLabels) {
            speakerLabel = speakerLabel.replace(" ", "");
            if (speakerLabel.contains("+"))
                speakerNames << speakerLabel.split("+");
            else
                speakerNames << speakerLabel;
        }

        foreach (QString speakerID, speakerNames) {
            AnnotationTierGroup *tiersSpk = new AnnotationTierGroup();
            foreach (AnnotationTier *tier, inputTiers->tiers()) {
                if (tier->tierType() == AnnotationTier::TierType_Intervals) {
                    QList<Interval *> intervalsTierSpk;
                    foreach (Interval *ispk, tierSpeaker->intervals()) {
                        QString spkLabel = ispk->text();
                        if (!spkLabel.split("+").contains(speakerID)) continue;
                        // if (spkLabel != speakerID) continue;
                        foreach (Interval *intv, qobject_cast<IntervalTier *>(tier)->getIntervalsContainedIn(ispk))
                            intervalsTierSpk << new Interval(intv);
                    }
                    IntervalTier *tierSpk = new IntervalTier(tier->name(), intervalsTierSpk, tier->tMin(), tier->tMax(), tiersSpk);
                    tiersSpk->addTier(tierSpk);
                }
                else if (tier->tierType() == AnnotationTier::TierType_Points) {
                    QList<Point *> pointsTierSpk;
                    foreach  (Interval *ispk, tierSpeaker->intervals()) {
                        QString spkLabel = ispk->text();
                        if (!spkLabel.split("+").contains(speakerID)) continue;
                        // if (ispk->text() != speakerID) continue;
                        foreach (Point *point, qobject_cast<PointTier *>(tier)->getPointsContainedIn(ispk))
                            pointsTierSpk << new Point(point);
                    }
                    PointTier *tierSpk = new PointTier(tier->name(), pointsTierSpk, tier->tMin(), tier->tMax(), tiersSpk);
                    tiersSpk->addTier(tierSpk);
                }
            }
            importBasic(d->repository, com, annot, SpeakerPolicySingle, QString("%1_%2").arg(annot->ID()).arg(speakerID), tiersSpk, correspondances);
            delete tiersSpk;
        }
        if (shouldDeleteSpeakerTier && tierSpeaker) {
            delete tierSpeaker;
        }
    }
    delete inputTiers;
}
