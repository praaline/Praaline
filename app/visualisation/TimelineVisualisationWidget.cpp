#include "TimelineVisualisationWidget.h"
#include "ui_TimelineVisualisationWidget.h"

#include <QPointer>
#include <QString>
#include <QPair>
#include <QModelIndex>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "pngui/widgets/CorpusItemSelectorWidget.h"
#include "pngui/widgets/TimelineEditorConfigWidget.h"
#include "pngui/widgets/AnnotationTimelineEditor.h"
#include "pngui/widgets/WaitingSpinnerWidget.h"

#include "svgui/layer/Layer.h"

#include "VisualiserWidget.h"


struct TimelineVisualisationWidgetData
{
    TimelineVisualisationWidgetData() :
        corpusItemSelector(0), visualiser(0), currentCorpus(0) {}

    // Visual Elements
    CorpusItemSelectorWidget *corpusItemSelector;   // Corpus items selector
    TimelineEditorConfigWidget *timelineConfig;     // Configuration: levels/attributes + speakers
    VisualiserWidget *visualiser;                   // Visualiser
    AnnotationTimelineEditor *annotationEditor;     // Annotation timeline editor

    // State / Data
    QPointer<Corpus> currentCorpus;
    QString currentCommunicationID;
    QString currentRecordingID;
    QString currentAnnotationID;
    QMap<QString, QPointer<AnnotationTierGroup> > currentTierGroups;

    WaitingSpinnerWidget* waitingSpinner;
};

TimelineVisualisationWidget::TimelineVisualisationWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TimelineVisualisationWidget), d(new TimelineVisualisationWidgetData)
{
    ui->setupUi(this);

    // Corpus item selector
    d->corpusItemSelector = new CorpusItemSelectorWidget(this);
    ui->gridLayoutTopTab->addWidget(d->corpusItemSelector);
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusCommunication(QPointer<Corpus>,QPointer<CorpusCommunication>)),
            this, SLOT(selectedCorpusCommunication(QPointer<Corpus>,QPointer<CorpusCommunication>)));
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusRecording(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusRecording>)),
            this, SLOT(selectedCorpusRecording(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusRecording>)));
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusAnnotation(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusAnnotation>)),
            this, SLOT(selectedCorpusAnnotation(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusAnnotation>)));
    connect(d->corpusItemSelector, SIGNAL(moveToAnnotationTime(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusAnnotation>,RealTime)),
            this, SLOT(moveToAnnotationTime(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusAnnotation>,RealTime)));

    // Timeline configuration
    d->timelineConfig = new TimelineEditorConfigWidget(this);
    ui->gridLayoutBottomTab->addWidget(d->timelineConfig);
    connect(d->timelineConfig, SIGNAL(selectedLevelsAttributesChanged()),
            this, SLOT(selectedLevelsAttributesChanged()));
    connect(d->timelineConfig, SIGNAL(speakerAdded(QString)),
            this, SLOT(speakerAdded(QString)));
    connect(d->timelineConfig, SIGNAL(speakerRemoved(QString)),
            this, SLOT(speakerRemoved(QString)));

    // Visualiser
    d->visualiser = new VisualiserWidget();
    ui->gridLayoutVisualisationEditor->addWidget(d->visualiser);

    // Annotation Timeline Editor
    d->annotationEditor = new AnnotationTimelineEditor(this);
    // d->annotationEditor->setOrientation(Qt::Horizontal);
    ui->gridLayoutAnnotationEditor->addWidget(d->annotationEditor);

    // Set splitters
    ui->splitterLeftRight->setSizes(QList<int>() << 50 << 300);
    ui->splitterCanvasTopBottom->setSizes(QList<int>() << 60 << 40);

    // Synchronise visualiser with annotation editor
    connect(d->annotationEditor, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)),
            this, SLOT(annotationEditorCurrentIndexChanged(QModelIndex,QModelIndex)));
    connect(d->visualiser, SIGNAL(userScrolledToTime(RealTime)),
            this, SLOT(visualiserUserScrolledToTime(RealTime)));
    connect(d->visualiser, SIGNAL(playbackScrolledToTime(RealTime)),
            this, SLOT(visualiserPlaybackScrolledToTime(RealTime)));

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
    QPointer<CorpusRecording> rec(0);
    QPointer<CorpusAnnotation> annot(0);
    if (!corpus) return;
    if (!com) return;
    if (com->recordingsCount() >= 1) rec = com->recordings().first();
    if (com->annotationsCount() >= 1) annot = com->annotations().first();
    corpusItemSelectionChanged(corpus, com, rec, annot);
}

void TimelineVisualisationWidget::selectedCorpusRecording(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                          QPointer<CorpusRecording> rec)
{
    QPointer<CorpusAnnotation> annot(0);
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
    QPointer<CorpusRecording> rec(0);
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

//    if ((!corpus) || (!com) || (!annot) || (!d->visualiser)) return;
//    IntervalTier *timeline = corpus->datastoreAnnotations()->getSpeakerTimeline(annot->ID(), "tok_min");
//    d->visualiser->addLayerTimeInstantsFromIntevalTier(timeline);
    d->waitingSpinner->stop();
}

void TimelineVisualisationWidget::moveToAnnotationTime(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot, RealTime time)
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

    d->visualiser->addAnnotationPaneToSession(d->currentTierGroups, d->timelineConfig->selectedLevelsAttributes());
    d->visualiser->addProsogramPaneToSession(corpus->communication(d->currentCommunicationID)->recording(d->currentRecordingID));
//    Layer *layer_rate_syll = d->visualiser->addLayerTimeValuesFromAnnotationTier(d->currentTierGroups.first()->tier("speech_rate"),
//                                                        "timeNanoseconds", "rate_syll", "text");
//    if (layer_rate_syll) layer_rate_syll->setDisplayExtents(0.0, 15.0);
//    Layer *layer_rate_phone = d->visualiser->addLayerTimeValuesFromAnnotationTier(d->currentTierGroups.first()->tier("speech_rate"),
//                                                        "timeNanoseconds", "rate_phone", "text");
//    if (layer_rate_phone) layer_rate_phone->setDisplayExtents(0.0, 30.0);
    Layer *layer_joystick_sprate = d->visualiser->addLayerTimeValuesFromAnnotationTier(
                d->currentTierGroups.first()->tier("joystick_speechrate"), "timeNanoseconds", "axis1", "text");
    if (layer_joystick_sprate) layer_joystick_sprate->setDisplayExtents(-32768.0, 32768.0);

    Layer *layer_joystick_pitch = d->visualiser->addLayerTimeValuesFromAnnotationTier(
                d->currentTierGroups.first()->tier("joystick_pitchmovement"), "timeNanoseconds", "axis1", "text");
    if (layer_joystick_pitch) layer_joystick_pitch->setDisplayExtents(-32768.0, 32768.0);


    // d->visualiser->exportPDF(QString("Emilie_%1.pdf").arg(annotationID));
}

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
