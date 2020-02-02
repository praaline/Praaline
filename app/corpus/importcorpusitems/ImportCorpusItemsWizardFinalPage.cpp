#include <QDebug>
#include <QPointer>
#include <QDir>
#include <QFileInfo>
#include "ImportCorpusItemsWizardFinalPage.h"
#include "ui_ImportCorpusItemsWizardFinalPage.h"

#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/FileDatastore.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"

#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"
#include "PraalineCore/Interfaces/Transcriber/TranscriberAnnotationGraph.h"
#include "PraalineCore/Interfaces/Subtitles/SubtitlesFile.h"

struct ImportCorpusItemsWizardFinalPageData {
    ImportCorpusItemsWizardFinalPageData(QPointer<Corpus> corpus,
                                         QMap<QPair<QString, QString>, CorpusRecording *> &candidateRecordings,
                                         QMap<QPair<QString, QString>, CorpusAnnotation *> &candidateAnnotations,
                                         QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
                                         QSet<QString> &tierNamesCommon) :
        corpus(corpus), candidateRecordings(candidateRecordings), candidateAnnotations(candidateAnnotations),
        tierCorrespondances(tierCorrespondances), tierNamesCommon(tierNamesCommon)
    {}

    QPointer<Corpus> corpus;
    QMap<QPair<QString, QString>, CorpusRecording *> &candidateRecordings;
    QMap<QPair<QString, QString>, CorpusAnnotation *> &candidateAnnotations;
    QMultiHash<QString, TierCorrespondance> &tierCorrespondances;
    QSet<QString> &tierNamesCommon;
};

ImportCorpusItemsWizardFinalPage::ImportCorpusItemsWizardFinalPage(
        QPointer<Corpus> corpus,
        QMap<QPair<QString, QString>, CorpusRecording *> &candidateRecordings,
        QMap<QPair<QString, QString>, CorpusAnnotation *> &candidateAnnotations,
        QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
        QSet<QString> &tierNamesCommon,
        QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ImportCorpusItemsWizardFinalPage),
    d(new ImportCorpusItemsWizardFinalPageData (corpus, candidateRecordings, candidateAnnotations,
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
    QMap<QPair<QString, QString>, CorpusRecording *>::iterator i;
    QMap<QPair<QString, QString>, CorpusAnnotation *>::iterator j;

    if (!d->corpus) return false;
    if (!d->corpus->repository()) return false;
    if (!d->corpus->repository()->files()) return false;

    ui->texteditMessagesFiles->clear();
    // Corpus media directory to relativise media file paths
    QDir dirMedia(d->corpus->repository()->files()->basePath());
    // Add recordings
    int counter = 0;
    for (i = d->candidateRecordings.begin(); i != d->candidateRecordings.end(); ++i) {
        QString communicationID = i.key().first;
        CorpusRecording *rec = i.value();
        if (!rec) continue;
        CorpusCommunication *com = d->corpus->communication(communicationID);
        if (!com) {
            com = new CorpusCommunication(communicationID);
            com->setName(communicationID);
            d->corpus->addCommunication(com);
            ui->texteditMessagesFiles->appendPlainText(QString(tr("Communication %1 added.")).arg(com->ID()));
        }
        if (com->hasRecording(rec->ID())) {
            ui->texteditMessagesFiles->appendPlainText(QString(tr("Recording %1 already exists, not added.")).arg(rec->ID()));
        } else {
            rec->setFilename(dirMedia.relativeFilePath(rec->filename()));
            com->addRecording(rec);
            ui->texteditMessagesFiles->appendPlainText(QString(tr("Recording %1 added.")).arg(rec->ID()));
        }
        if ((counter % 100) == 0) QApplication::processEvents();
        counter++;
    }
    ui->texteditMessagesFiles->appendPlainText(QString(tr("Added %1 recording(s).")).arg(d->candidateRecordings.count()));
    // Add annotations
    for (j = d->candidateAnnotations.begin(); j != d->candidateAnnotations.end(); ++j) {
        QString communicationID = j.key().first;
        CorpusAnnotation *annot = j.value();
        if (!annot) continue;
        CorpusCommunication *com = d->corpus->communication(communicationID);
        if (!com) {
            com = new CorpusCommunication(communicationID);
            com->setName(communicationID);
            d->corpus->addCommunication(com);
        }
        if (com->hasAnnotation(annot->ID())) {
            ui->texteditMessagesFiles->appendPlainText(QString(tr("Annotation %1 already exists, merging annotations but not metadata.")).arg(annot->ID()));
        } else {
            com->addAnnotation(annot);
        }
    }
    ui->texteditMessagesFiles->appendPlainText(QString(tr("Added %1 annotations(s).")).arg(d->candidateAnnotations.count()));
    // Import annotations
    ui->progressBarFiles->setValue(0);
    ui->progressBarFiles->setMaximum(d->candidateAnnotations.count());
    int count = 0;
    for (j = d->candidateAnnotations.begin(); j != d->candidateAnnotations.end(); ++j) {
        QString communicationID = j.key().first;
        CorpusAnnotation *annot = j.value();
        if (!annot) continue;
        CorpusCommunication *com = d->corpus->communication(communicationID);
        if (!com) continue;
        QList<TierCorrespondance> correspondances;
        // First: level correspondances then attribute correspondances
        foreach (TierCorrespondance c, d->tierCorrespondances.values(annot->filename())) {
            if (!c.annotationLevelID.isEmpty() && c.annotationAttributeID.isEmpty()) correspondances << c;
        }
        foreach (TierCorrespondance c, d->tierCorrespondances.values(annot->filename())) {
            if (!c.annotationLevelID.isEmpty() && !c.annotationAttributeID.isEmpty()) correspondances << c;
        }
        if (annot->filename().toLower().endsWith("textgrid")) {
            importPraat(com, annot, correspondances);
        }
        else if (annot->filename().toLower().endsWith("trs")) {
            importTranscriber(com, annot, correspondances);
        }
        else if (annot->filename().toLower().endsWith("srt")) {
            importSubRipTranscription(com, annot, correspondances);
        }
        count++;
        ui->progressBarFiles->setValue(count);
        QApplication::processEvents();
    }
    // Save the corpus metadata, as they have been modified (this step is essential to keep coherence with the annotations).
    return d->corpus->save();
}

// ==============================================================================================================================
// Import modules
// ==============================================================================================================================

void ImportCorpusItemsWizardFinalPage::importTranscriber(CorpusCommunication *com, CorpusAnnotation *annot,
                                                         QList<TierCorrespondance> &correspondances)
{
    Q_UNUSED(correspondances)
    if (!d->corpus) return;
    if (!d->corpus->repository()) return;
    if (!d->corpus->repository()->annotations()) return;

    ui->texteditMessagesFiles->appendPlainText(QString(tr("Importing %1/%2...")).arg(com->ID()).arg(annot->ID()));

    SpeakerAnnotationTierGroupMap tiersAll;
    QList<CorpusSpeaker *> speakers;
    bool result = TranscriberAnnotationGraph::load(annot->filename(), speakers, tiersAll);
    if (!result) {
        return;
    }
    // Add speakers if necessary
    foreach (QPointer<CorpusSpeaker> spk, speakers) {
        if (!d->corpus->hasSpeaker(spk->ID())) {
            d->corpus->addSpeaker(spk);
        }
    }
    // Add participations of speakers
    foreach (QString spkID, tiersAll.keys()) {
        if (!d->corpus->hasParticipation(com->ID(), spkID)) {
            d->corpus->addParticipation(com->ID(), spkID, tr("Participant"));
        }
    }
    d->corpus->repository()->annotations()->saveTiersAllSpeakers(annot->ID(), tiersAll);
    qDeleteAll(tiersAll);
}

void ImportCorpusItemsWizardFinalPage::importSubRipTranscription(CorpusCommunication *com, CorpusAnnotation *annot,
                                                                 QList<TierCorrespondance> &correspondances)
{
    Q_UNUSED(correspondances)
    if (!d->corpus) return;
    if (!d->corpus->repository()) return;
    if (!d->corpus->repository()->annotations()) return;

    ui->texteditMessagesFiles->appendPlainText(QString(tr("Importing %1/%2...")).arg(com->ID()).arg(annot->ID()));

    QPointer<IntervalTier> tierTranscription = new IntervalTier();
    bool result = SubtitlesFile::loadSRT(annot->filename(), tierTranscription);
    tierTranscription->setName("transcription");
    if (!result) {
        return;
    }
    // Add speakers if necessary
    QString speakerID = annot->ID();
    if (!d->corpus->hasSpeaker(speakerID))
        d->corpus->addSpeaker(new CorpusSpeaker(speakerID));
    if (!d->corpus->hasParticipation(com->ID(), speakerID))
        d->corpus->addParticipation(com->ID(), speakerID, tr("Participant"));
    d->corpus->repository()->annotations()->saveTier(annot->ID(), speakerID, tierTranscription);
}


void ImportCorpusItemsWizardFinalPage::importPraat(CorpusCommunication *com, CorpusAnnotation *annot,
                                                   QList<TierCorrespondance> &correspondances)
{
    ui->texteditMessagesFiles->appendPlainText(QString(tr("Importing %1/%2...")).arg(com->ID()).arg(annot->ID()));
    AnnotationTierGroup *inputTiers = new AnnotationTierGroup();

    PraatTextGrid::load(annot->filename(), inputTiers);

    ImportAnnotations::SpeakerPolicy policy = ImportAnnotations::speakerPolicyFromInt(annot->property("speakerPolicy").toInt());
    QString annotSpeakerID = annot->property("speakerPolicyData").toString();
    if (policy == ImportAnnotations::SpeakerPolicySingle) {
        if (annotSpeakerID.isEmpty()) {
            // no speaker ID given, use the filename
            ImportAnnotations::importBasic(d->corpus, com, annot, policy, QFile(annot->filename()).fileName().remove(".textgrid"),
                                           inputTiers, correspondances);
        }
        else {
            ImportAnnotations::importBasic(d->corpus, com, annot, policy, annotSpeakerID, inputTiers, correspondances);
        }
    }
    else if (policy == ImportAnnotations::SpeakerPolicyTierNames) {
        ImportAnnotations::importBasic(d->corpus, com, annot, policy, "", inputTiers, correspondances);
    }
    else if ((policy == ImportAnnotations::SpeakerPolicyPrimaryAndSecondary) || (policy == ImportAnnotations::SpeakerPolicyIntervals)) {
        IntervalTier *tierSpeaker = nullptr;
        bool shouldDeleteSpeakerTier = false;
        if (policy == ImportAnnotations::SpeakerPolicyPrimaryAndSecondary) {
            QString tiernameSpeaker1 = annot->property("speakerPolicyData").toString().section(";", 0, 0);
            QString tiernameSpeaker2 = annot->property("speakerPolicyData").toString().section(";", 1, 1);
            IntervalTier *tierSpeaker1 = inputTiers->getIntervalTierByName(tiernameSpeaker1);
            IntervalTier *tierSpeaker2 = inputTiers->getIntervalTierByName(tiernameSpeaker2);
            if (tierSpeaker1 && tierSpeaker2) {
                tierSpeaker = IntervalTier::multiplex("speaker", tierSpeaker1, tierSpeaker2, "S1", "S2", "S1");
                shouldDeleteSpeakerTier = true;
            }
        }
        else if (policy == ImportAnnotations::SpeakerPolicyIntervals) {
            // There is a speaker ID tier
            QString speakerTierName = annot->property("speakerPolicyData").toString();
            tierSpeaker = inputTiers->getIntervalTierByName(speakerTierName);
        }
        if (!tierSpeaker) {
            ImportAnnotations::importBasic(d->corpus, com, annot, ImportAnnotations::SpeakerPolicySingle, annot->ID(), inputTiers, correspondances);
            return;
        }
//      // Rhapsodie hack
//        foreach (Interval *intv, tierSpeaker->intervals()) {
//            QString s = intv->text().section("-$", 0, 0);
//            intv->setText(s);
//        }
//        tierSpeaker->mergeIdenticalAnnotations();
        QStringList speakerNames;
        QStringList speakerLabels = tierSpeaker->getDistinctLabels();
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
                            intervalsTierSpk << intv->clone();
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
                            pointsTierSpk << point->clone();
                    }
                    PointTier *tierSpk = new PointTier(tier->name(), pointsTierSpk, tier->tMin(), tier->tMax(), tiersSpk);
                    tiersSpk->addTier(tierSpk);
                }
            }
            ImportAnnotations::importBasic(d->corpus, com, annot, ImportAnnotations::SpeakerPolicySingle,
                                           QString("%1_%2").arg(annot->ID()).arg(speakerID), tiersSpk, correspondances);
            delete tiersSpk;
        }
        if (shouldDeleteSpeakerTier && tierSpeaker) {
            delete tierSpeaker;
        }
    }
    delete inputTiers;
}
