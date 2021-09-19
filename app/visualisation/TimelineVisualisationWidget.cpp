#include "TimelineVisualisationWidget.h"
#include "ui_TimelineVisualisationWidget.h"

#include <QPointer>
#include <QString>
#include <QPair>
#include <QModelIndex>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "pngui/widgets/CorpusItemSelectorWidget.h"
#include "pngui/widgets/TimelineEditorConfigWidget.h"
#include "pngui/widgets/AnnotationMultiTierEditorWidget.h"
#include "pngui/widgets/WaitingSpinnerWidget.h"

#include "svgui/layer/Layer.h"

#include "pngui/model/visualiser/MovingAverageModel.h"

#include "VisualiserWidget.h"


struct TimelineVisualisationWidgetData
{
    TimelineVisualisationWidgetData() :
        currentVisualisationIndex(0),
        corpusItemSelector(nullptr), timelineConfig(nullptr), visualiser(nullptr), annotationEditor(nullptr),
        currentCorpus(nullptr), waitingSpinner(nullptr) {}

    // Visualisation selector
    QStringList visualisationNames;
    int currentVisualisationIndex;

    // Visual Elements
    CorpusItemSelectorWidget *corpusItemSelector;   // Corpus items selector
    TimelineEditorConfigWidget *timelineConfig;     // Configuration: levels/attributes + speakers
    VisualiserWidget *visualiser;                   // Visualiser
    AnnotationMultiTierEditorWidget *annotationEditor;     // Annotation timeline editor

    // State / Data
    QPointer<Corpus> currentCorpus;
    QString currentCommunicationID;
    QString currentRecordingID;
    QString currentAnnotationID;
    SpeakerAnnotationTierGroupMap currentTierGroups;

    WaitingSpinnerWidget* waitingSpinner;
};

TimelineVisualisationWidget::TimelineVisualisationWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TimelineVisualisationWidget), d(new TimelineVisualisationWidgetData)
{
    ui->setupUi(this);

    // Visualisation selector
    connect(ui->comboBoxVisualisationSelector, SIGNAL(currentIndexChanged(int)),
            this, SLOT(selectedVisualisationChanged(int)));
    d->visualisationNames << "Default" <<
                             "Annotations and Prosogram" <<
                             "Boundaries" <<
                             "Boundaries vs Pauses" <<
                             "Speech rate" <<
                             "Pitch rate";
    ui->comboBoxVisualisationSelector->addItems(d->visualisationNames);
    readVisualisationIndexFromConfigFile();

    // Corpus item selector
    d->corpusItemSelector = new CorpusItemSelectorWidget(this);
    ui->gridLayoutTopTab->addWidget(d->corpusItemSelector);
    connect(d->corpusItemSelector, &CorpusItemSelectorWidget::selectedCorpusCommunication,
            this, &TimelineVisualisationWidget::selectedCorpusCommunication);
    connect(d->corpusItemSelector, &CorpusItemSelectorWidget::selectedCorpusRecording,
            this, &TimelineVisualisationWidget::selectedCorpusRecording);
    connect(d->corpusItemSelector, &CorpusItemSelectorWidget::selectedCorpusAnnotation,
            this, &TimelineVisualisationWidget::selectedCorpusAnnotation);
    connect(d->corpusItemSelector, &CorpusItemSelectorWidget::moveToAnnotationTime,
            this, &TimelineVisualisationWidget::moveToAnnotationTime);

    // Timeline configuration
    d->timelineConfig = new TimelineEditorConfigWidget(this);
    ui->gridLayoutBottomTab->addWidget(d->timelineConfig);
    connect(d->timelineConfig, &TimelineEditorConfigWidget::selectedLevelsAttributesChanged,
            this, &TimelineVisualisationWidget::selectedLevelsAttributesChanged);
    connect(d->timelineConfig, &TimelineEditorConfigWidget::speakerAdded,
            this, &TimelineVisualisationWidget::speakerAdded);
    connect(d->timelineConfig, &TimelineEditorConfigWidget::speakerRemoved,
            this, &TimelineVisualisationWidget::speakerRemoved);

    // Visualiser
    d->visualiser = new VisualiserWidget("Context.VisualisationMode");
    ui->gridLayoutVisualisationEditor->setMargin(0);
    ui->gridLayoutVisualisationEditor->addWidget(d->visualiser);

    // Annotation Timeline Editor
    d->annotationEditor = new AnnotationMultiTierEditorWidget(this);
    // d->annotationEditor->setOrientation(Qt::Horizontal);
    ui->gridLayoutAnnotationEditor->addWidget(d->annotationEditor);

    // Set splitters
    ui->splitterCanvasTopBottom->setSizes(QList<int>() << 60 << 40);
    ui->splitterLeftRight->setSizes(QList<int>() << 250 << 500);

    // Synchronise visualiser with annotation editor
    connect(d->annotationEditor, &TimelineEditorWidgetBase::currentIndexChanged,
            this, &TimelineVisualisationWidget::annotationEditorCurrentIndexChanged);
    connect(d->visualiser, &SimpleVisualiserWidget::userScrolledToTime,
            this, &TimelineVisualisationWidget::visualiserUserScrolledToTime);
    connect(d->visualiser, &SimpleVisualiserWidget::playbackScrolledToTime,
            this, &TimelineVisualisationWidget::visualiserPlaybackScrolledToTime);

    // Waiting spinner
    d->waitingSpinner = new WaitingSpinnerWidget(this);
    d->waitingSpinner->setRoundness(70.0);
    d->waitingSpinner->setMinimumTrailOpacity(15.0);
    d->waitingSpinner->setTrailFadePercentage(70.0);
    d->waitingSpinner->setNumberOfLines(12);
    d->waitingSpinner->setLineLength(15);
    d->waitingSpinner->setLineWidth(5);
    d->waitingSpinner->setInnerRadius(10);
    d->waitingSpinner->setRevolutionsPerSecond(10);
    d->waitingSpinner->setColor(QColor(81, 4, 71));
}

TimelineVisualisationWidget::~TimelineVisualisationWidget()
{
    delete ui;
    delete d;
}

// ====================================================================================================================
// Responding to corpus item selection changes
// ====================================================================================================================

void TimelineVisualisationWidget::selectedCorpusCommunication(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    QPointer<CorpusRecording> rec(nullptr);
    QPointer<CorpusAnnotation> annot(nullptr);
    if (!corpus) return;
    if (!com) return;
    if (com->recordingsCount() >= 1) rec = com->recordings().first();
    if (com->annotationsCount() >= 1) annot = com->annotations().first();
    corpusItemSelectionChanged(corpus, com, rec, annot);
}

void TimelineVisualisationWidget::selectedCorpusRecording(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                          QPointer<CorpusRecording> rec)
{
    QPointer<CorpusAnnotation> annot(nullptr);
    if (!corpus) return;
    if (!com) return;
    if (!rec) return;
    if (com->annotationsCount() == 1) {
        annot = com->annotations().first();
    } else if (com->annotationsCount() > 0) {
        annot = com->annotation(rec->ID());
    }
    corpusItemSelectionChanged(corpus, com, rec, annot);
}

void TimelineVisualisationWidget::selectedCorpusAnnotation(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                           QPointer<CorpusAnnotation> annot)
{
    QPointer<CorpusRecording> rec(nullptr);
    if (!corpus) return;
    if (!com) return;
    if (!annot) return;
    if (com->recordingsCount() == 1) {
        rec = com->recordings().first();
    } else if (com->annotationsCount() > 0) {
        rec = com->recording(annot->ID());
    }
    corpusItemSelectionChanged(corpus, com, rec, annot);
}

void TimelineVisualisationWidget::corpusItemSelectionChanged(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                             QPointer<CorpusRecording> rec, QPointer<CorpusAnnotation> annot)
{
//    if (d->autoSave) {
//        saveAnnotations();
//    }
    if (!com) {
        d->currentCommunicationID.clear();
        d->currentRecordingID.clear();
        d->currentAnnotationID.clear();
        return;
    }

    d->waitingSpinner->start();
    QApplication::processEvents();

    d->currentCommunicationID = com->ID();
    visualiserNewSession(corpus, com);
    if ((rec) && (d->currentRecordingID != rec->ID())) {
        d->currentRecordingID = rec->ID();
    }
    else {
        d->currentRecordingID.clear();
    }
    if ((annot) && (d->currentAnnotationID != annot->ID())) {
        annotationTimelineEditorOpen(corpus, annot->ID());
        d->currentAnnotationID = annot->ID();
    }
    else {
        d->currentAnnotationID.clear();
    }
    d->waitingSpinner->stop();
}

void TimelineVisualisationWidget::moveToAnnotationTime(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                       QPointer<CorpusAnnotation> annot, RealTime time)
{
    if (!corpus) return;
    if (!com) return;
    if (!annot) return;

    if (d->currentCorpus == corpus && d->currentCommunicationID == com->ID() && d->currentAnnotationID == annot->ID()) {
        d->visualiser->jumpToTime(time);
        d->annotationEditor->moveToTime(time);
    }
    else {
        visualiserNewSession(corpus, com);
        annotationTimelineEditorOpen(corpus, annot->ID());
        d->visualiser->jumpToTime(time);
        d->annotationEditor->moveToTime(time);
    }
}

void TimelineVisualisationWidget::visualiserNewSession(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    if ((!corpus) || (!com) || (!d->visualiser)) return;
    d->visualiser->newSessionWithCommunication(com);
    d->currentCommunicationID = com->ID();
}

void TimelineVisualisationWidget::annotationTimelineEditorOpen(QPointer<Corpus> corpus, const QString &annotationID)
{
    if (!corpus) return;
    if (!corpus->repository()) return;
    d->currentCorpus = corpus;
    d->currentAnnotationID = annotationID;
    qDeleteAll(d->currentTierGroups);
    d->currentTierGroups.clear();
    d->currentTierGroups = corpus->repository()->annotations()->getTiersAllSpeakers(d->currentAnnotationID);

    d->annotationEditor->setData(d->currentTierGroups, d->timelineConfig->selectedLevelsAttributes());
    d->timelineConfig->updateSpeakerList(d->currentTierGroups.keys());

    loadVisualisation();

    // d->visualiser->exportPDF(QString("Emilie_%1.pdf").arg(annotationID));
}

// ====================================================================================================================
// Visualisation selector
// ====================================================================================================================

void TimelineVisualisationWidget::readVisualisationIndexFromConfigFile()
{
    QString configFilename = QDir::homePath() + "/Praaline/visualiserConfig.txt";
    if (!QFile::exists(configFilename))
        configFilename = QApplication::applicationDirPath() + "/visualiserConfig.txt";
    QFile fileConfig(configFilename);
    if (fileConfig.open( QIODevice::ReadOnly | QIODevice::Text )) {
        QTextStream stream(&fileConfig);
        do {
            QString line = stream.readLine().trimmed();
            if (line.startsWith("#")) continue;
            if (line.startsWith("loadVisualisation")) {
                if (line.split(" ").count() > 1) {
                    ui->comboBoxVisualisationSelector->setCurrentIndex(line.section(" ", 1, 1).toInt());
                }
            }
        } while (!stream.atEnd());
        fileConfig.close();
    }
}

void TimelineVisualisationWidget::selectedVisualisationChanged(int selectorIndex)
{
    if (selectorIndex != d->currentVisualisationIndex) {
        d->currentVisualisationIndex = selectorIndex;
        loadVisualisation();
    }
}

void TimelineVisualisationWidget::loadVisualisation()
{
    d->visualiser->clearVisualisationPanes();
    if      (d->currentVisualisationIndex == 1)
        loadVisualisationAnnotationsProsogram();
    else if (d->currentVisualisationIndex == 2)
        loadVisualisationBoundariesLOCAS();
    else if (d->currentVisualisationIndex == 3)
        loadVisualisationBoundariesTITEUF();
    else if (d->currentVisualisationIndex == 4)
        loadVisualisationMacroprosodyTITEUF("speechrate");
    else if (d->currentVisualisationIndex == 5)
        loadVisualisationMacroprosodyTITEUF("pitchmovement");
    else
        loadVisualisationDefault();
}

void TimelineVisualisationWidget::loadVisualisationDefault()
{
    loadVisualisationAnnotationsProsogram();
}

void TimelineVisualisationWidget::loadVisualisationAnnotationsProsogram()
{
    // Create annotation pane
    d->visualiser->setAnnotationTiers(d->currentTierGroups);
    d->visualiser->setAnnotationLevelAttributeSelection(d->timelineConfig->selectedLevelsAttributes());
    QVariantHash annotationPaneParameters;
    QStringList excludedSpeakers;
    for (int i = 1; i <=300; ++i) excludedSpeakers << QString("P%1").arg(i);
    annotationPaneParameters.insert("excludedSpeakers", excludedSpeakers);
    d->visualiser->addAnnotationPane(annotationPaneParameters);

    // Create a prosogram pane for each recording
    if ((d->currentCorpus) && (d->currentCorpus->communication(d->currentCommunicationID))) {
        foreach (CorpusRecording *rec, d->currentCorpus->communication(d->currentCommunicationID)->recordings()) {
            if (!rec) continue;
            foreach (CorpusAnnotation *annot, d->currentCorpus->communication(d->currentCommunicationID)->annotations()) {
                if (!annot) continue;
                // TODO : ANNOTATION CORRESPONDANCE !!!
                //if (annot->ID() != rec->ID()) continue;
                d->visualiser->addProsogramPaneToSession(rec, annot);
            }
        }
    }
}

void TimelineVisualisationWidget::loadVisualisationBoundariesLOCAS()
{
    loadVisualisationAnnotationsProsogram();

    // addTappingDataPane(m_tiers); // for Emilie
    d->visualiser->addTappingDataPane("tappingAdj_naive", "smooth_naive", "boundaryNaive");
    d->visualiser->addTappingDataPane("tappingAdj_expert", "smooth_expert", "boundaryExpert");
}

void TimelineVisualisationWidget::loadVisualisationBoundariesTITEUF()
{
    loadVisualisationAnnotationsProsogram();

    d->visualiser->addTappingDataPane("tapping_boundariesAdj", "tapping_boundaries_smooth", "boundary");
    d->visualiser->addTappingDataPane("tapping_pausesAdj", "tapping_pauses_smooth", "pause");
}


void TimelineVisualisationWidget::loadVisualisationMacroprosodyTITEUF(const QString &measure)
{
    // Create annotation pane
    d->visualiser->setAnnotationTiers(d->currentTierGroups);
    d->visualiser->setAnnotationLevelAttributeSelection(d->timelineConfig->selectedLevelsAttributes());
    QVariantHash annotationPaneParameters;
    QStringList excludedSpeakers;
    for (int i = 1; i <=300; ++i) excludedSpeakers << QString("P%1").arg(i);
    annotationPaneParameters.insert("excludedSpeakers", excludedSpeakers);
    d->visualiser->addAnnotationPane(annotationPaneParameters);

    // Create a prosogram pane for each recording
    if ((d->currentCorpus) && (d->currentCorpus->communication(d->currentCommunicationID))) {
        foreach (CorpusRecording *rec, d->currentCorpus->communication(d->currentCommunicationID)->recordings()) {
            if (!rec) continue;
            foreach (CorpusAnnotation *annot, d->currentCorpus->communication(d->currentCommunicationID)->annotations()) {
                if (!annot) continue;
                // TODO : ANNOTATION CORRESPONDANCE !!!
                //if (annot->ID() != rec->ID()) continue;
                d->visualiser->addProsogramPaneToSession(rec, annot);
            }
        }
    }

    // Tapping data pane
    d->visualiser->addTappingDataPane("tapping_boundariesAdj", "tapping_boundaries_smooth", "boundary");
    d->visualiser->addTappingDataPane("tapping_pausesAdj", "tapping_pauses_smooth", "pause");

    // Continuous measures
    QString tierName = "joystick_" + measure;

    // All participants, combined with CI
    AnnotationTierGroup *tiersCombined = d->currentTierGroups.value("combined");
    if (tiersCombined && tiersCombined->hasTiers()) {
        Layer *layer_combined = d->visualiser->addLayerTimeValuesFromAnnotationTier(
                tiersCombined->tier(tierName + "_combined"), "timeNanoseconds", "value_mean", "", true, "", "value_stdev");
        if (layer_combined) {
            layer_combined->setDisplayExtents(-4.0, 4.0);
            layer_combined->setProperty("Plot Type", 2); // 4 = Curve 0 = Points, 2 = Connected points
            layer_combined->setProperty("Show Confidence Interval", 1);
        }
    }

    // Objective measure
    d->visualiser->addMovingAveragePane("syll", "");

//    AnnotationTierGroup *tiersSpk = d->currentTierGroups.value(annotationID);
//    if (tierName.endsWith("speechrate") && tiersSpk) {
//        Layer *layer_regions_speechrate = d->visualiser->addLayerTimeValuesFromAnnotationTier(
//                    tiersSpk->tier("speech_rate"), "timeNanoseconds", "rate_syll", "", true);
//        if (layer_regions_speechrate) {
//            layer_regions_speechrate->setDisplayExtents(0.0, 10.0);
//            layer_regions_speechrate->setProperty("Plot Type", 4); // 4 = Curve 0 = Points
//            layer_regions_speechrate->setPresentationName("Speech rate syll/sec");
//        }
//    }
//    else if (tierName.endsWith("pitchmovement") && tiersSpk) {
//        Layer *layer_regions_pitchmovement = d->visualiser->addLayerTimeValuesFromAnnotationTier(
//                    tiersSpk->tier("pitch_movement"), "timeNanoseconds", "rate_pitch", "", true);
//        if (layer_regions_pitchmovement) {
//            layer_regions_pitchmovement->setDisplayExtents(0.0, 10.0);
//            layer_regions_pitchmovement->setProperty("Plot Type", 4); // 4 = Curve 0 = Points
//            layer_regions_pitchmovement->setPresentationName("Pitch movement ST/sec");
//        }
//    }
}

//void TimelineVisualisationWidget::loadVisualisationNassima2(QPointer<Corpus> corpus, const QString &annotationID)
//{
//    QString measure = "speechrate";

//    // Create annotation pane
//    d->visualiser->setAnnotationTiers(d->currentTierGroups);
//    d->visualiser->setAnnotationLevelAttributeSelection(d->timelineConfig->selectedLevelsAttributes());
//    QVariantHash annotationPaneParameters;
//    QStringList excludedSpeakers;
//    for (int i = 1; i <=300; ++i) excludedSpeakers << QString("P%1").arg(i);
//    annotationPaneParameters.insert("excludedSpeakers", excludedSpeakers);
//    d->visualiser->addAnnotationPane(annotationPaneParameters);

//    // Create a prosogram pane for each recording
//    if (corpus->communication(d->currentCommunicationID)) {
//        foreach (CorpusRecording *rec, corpus->communication(d->currentCommunicationID)->recordings()) {
//            if (!rec) continue;
//            d->visualiser->addProsogramPaneToSession(rec);
//        }
//    }

//    //    Layer *layer_rate_phone = d->visualiser->addLayerTimeValuesFromAnnotationTier(d->currentTierGroups.first()->tier("speech_rate"),
//    //                                                        "timeNanoseconds", "rate_phone", "text");
//    //    if (layer_rate_phone) layer_rate_phone->setDisplayExtents(0.0, 30.0);


//    bool first(true);
//    QString tierName = "joystick_" + measure;
//    foreach (QString participantID, d->currentTierGroups.keys()) {
//        if (!participantID.startsWith("P")) continue;
//        AnnotationTierGroup *tiers = d->currentTierGroups.value(participantID);
//        Layer *layer_joystick = d->visualiser->addLayerTimeValuesFromAnnotationTier(
//                    tiers->tier(tierName), "timeNanoseconds", "value_norm", "", first, participantID);
//        if (layer_joystick) {
//            layer_joystick->setDisplayExtents(-4.0, 4.0); // (-32768.0, 32768.0);
//            layer_joystick->setProperty("Plot Type", 4); // 4 = Curve 0 = Points
//            if (first) first = false;
//        }
//    }

//    // Combined
//    AnnotationTierGroup *tiersCombined = d->currentTierGroups.value("combined");
//    if (tiersCombined && tiersCombined->hasTiers()) {
//        Layer *layer_combined = d->visualiser->addLayerTimeValuesFromAnnotationTier(
//                tiersCombined->tier(tierName + "_combined"), "timeNanoseconds", "value_mean", "", true);
//        if (layer_combined) {
//            layer_combined->setDisplayExtents(-4.0, 4.0);
//            layer_combined->setProperty("Plot Type", 0); // 4 = Curve 0 = Points
//        }
//    }

//    // Objective measure
//    d->visualiser->addMovingAveragePane("syll", "");

//    AnnotationTierGroup *tiersSpk = d->currentTierGroups.value(annotationID);
//    if (tierName.endsWith("speechrate") && tiersSpk) {
//        Layer *layer_regions_speechrate = d->visualiser->addLayerTimeValuesFromAnnotationTier(
//                    tiersSpk->tier("speech_rate"), "timeNanoseconds", "rate_syll", "", true);
//        if (layer_regions_speechrate) {
//            layer_regions_speechrate->setDisplayExtents(0.0, 10.0);
//            layer_regions_speechrate->setProperty("Plot Type", 0); // 4 = Curve 0 = Points
//            layer_regions_speechrate->setPresentationName("Speech rate syll/sec");
//        }
//    }
//    else if (tierName.endsWith("pitchmovement") && tiersSpk) {
//        Layer *layer_regions_pitchmovement = d->visualiser->addLayerTimeValuesFromAnnotationTier(
//                    tiersSpk->tier("pitch_movement"), "timeNanoseconds", "rate_pitch", "", true);
//        if (layer_regions_pitchmovement) {
//            layer_regions_pitchmovement->setDisplayExtents(0.0, 10.0);
//            layer_regions_pitchmovement->setProperty("Plot Type", 4); // 4 = Curve 0 = Points
//            layer_regions_pitchmovement->setPresentationName("Pitch movement ST/sec");
//        }
//    }
// }

// ====================================================================================================================
// Responding to changes in the timeline configuration >> change the annotation editor view
// ====================================================================================================================

void TimelineVisualisationWidget::selectedLevelsAttributesChanged()
{
    QList<QPair<QString, QString> > columns = d->timelineConfig->selectedLevelsAttributes();
    d->annotationEditor->setData(d->currentTierGroups, columns);
}

void TimelineVisualisationWidget::speakerAdded(const QString &speakerID)
{
    if (!d->currentTierGroups.contains(speakerID)) return;
    d->annotationEditor->addTierGroup(speakerID, d->currentTierGroups.value(speakerID));
}

void TimelineVisualisationWidget::speakerRemoved(const QString &speakerID)
{
    d->annotationEditor->removeTierGroup(speakerID);
}

// ====================================================================================================================
// Synchronise visualiser and annotation editor
// ====================================================================================================================

void TimelineVisualisationWidget::annotationEditorCurrentIndexChanged(QModelIndex previous, QModelIndex current)
{
    Q_UNUSED(previous)
    Q_UNUSED(current)
    if (!d->annotationEditor->model()) return;
    RealTime time = d->annotationEditor->currentTime();
    if (time > RealTime())
        d->visualiser->jumpToTime(time);
}

void TimelineVisualisationWidget::visualiserUserScrolledToTime(RealTime time)
{
    if (!d->annotationEditor->model()) return;
    const QSignalBlocker blocker(d->annotationEditor);
    // no signals here
    d->annotationEditor->moveToTime(time);
}

void TimelineVisualisationWidget::visualiserPlaybackScrolledToTime(RealTime time)
{
    if (!d->annotationEditor->model()) return;
    const QSignalBlocker blocker(d->annotationEditor);
    // no signals here
    d->annotationEditor->moveToTime(time);
}

void TimelineVisualisationWidget::setAnnotationEditorOrientation(const Qt::Orientation orientation)
{
    if (!d->annotationEditor->model()) return;
    d->annotationEditor->setOrientation(orientation);
}
