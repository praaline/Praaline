#include <QObject>
#include <QString>
#include <QLabel>
#include <QStatusBar>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QMenu>

#include "svcore/data/model/WaveFileModel.h"
#include "svgui/view/Pane.h"
#include "svgui/view/PaneStack.h"
#include "svgui/view/Overview.h"
#include "svgui/view/PaneStack.h"
#include "svgui/layer/WaveformLayer.h"
#include "svgui/layer/TimeRulerLayer.h"
#include "svgui/widgets/AudioDial.h"
#include "svgui/widgets/Fader.h"
#include "svgui/widgets/CommandHistory.h"
#include "svapp/framework/Document.h"

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "pngui/widgets/corpusitemselectorwidget.h"
#include "pngui/widgets/annotationtimelineeditor.h"

#include "../visualisation/SimpleVisualiserWidget.h"

#include "TranscriberWidget.h"
#include "ui_TranscriberWidget.h"

struct TranscriberWidgetData {
    TranscriberWidgetData() :
        corpusItemSelector(0), annotationEditor(0), currentCorpus(0) {}

    CorpusItemSelectorWidget *corpusItemSelector;   // Corpus items selector
    AnnotationTimelineEditor *annotationEditor;     // Annotation timeline editor

    // State (currently open corpus, communication, recording and annotation)
    QPointer<Corpus> currentCorpus;
    QString currentCommunicationID;
    QString currentRecordingID;
    QString currentAnnotationID;
};

TranscriberWidget::TranscriberWidget(QWidget *parent) :
    SimpleVisualiserWidget("Transcriber", true, true),
    ui(new Ui::TranscriberWidget), d(new TranscriberWidgetData)
{
    Q_UNUSED(parent)
    ui->setupUi(this);

    // Create corpus items selector
    d->corpusItemSelector = new CorpusItemSelectorWidget(this);
    ui->gridLayoutTopTab->addWidget(d->corpusItemSelector);
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusCommunication(QPointer<Corpus>,QPointer<CorpusCommunication>)),
            this, SLOT(selectedCorpusCommunication(QPointer<Corpus>,QPointer<CorpusCommunication>)));
    connect(d->corpusItemSelector, SIGNAL(selectedCorpusRecording(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusRecording>)),
            this, SLOT(selectedCorpusRecording(QPointer<Corpus>,QPointer<CorpusCommunication>,QPointer<CorpusRecording>)));

    // Prepare simple visualiser
    QGridLayout *layout = new QGridLayout;
    QFrame *playControlsSpacer = new QFrame;
    layout->setSpacing(4);
    layout->addWidget(m_mainScroll, 0, 0, 1, 5);
    layout->addWidget(m_overview, 1, 0, 1, 2);
    layout->addWidget(playControlsSpacer, 1, 2);
    layout->addWidget(m_playSpeed, 1, 3);
    layout->addWidget(m_fader, 1, 4);
    int playControlsWidth = m_fader->width() + m_playSpeed->width() + layout->spacing() * 2;
    layout->setColumnMinimumWidth(0, 14);
    layout->setColumnStretch(0, 0);
    m_paneStack->setPropertyStackMinWidth(playControlsWidth + 2 + layout->spacing());
    playControlsSpacer->setFixedSize(QSize(2, 2));
    layout->setColumnStretch(1, 10);
    //connect(m_paneStack, SIGNAL(propertyStacksResized(int)), this, SLOT(propertyStacksResized(int)));
    m_visualiserFrame->setLayout(layout);
    ui->gridLayoutVisualiser->addWidget(m_visualiserFrame);
    // Menus and toolbars
    setupMenus();
    m_rightButtonMenu = new QMenu();
    setupPlaybackMenusAndToolbar();
    //setupToolbars();
    statusBar();
    m_currentLabel = new QLabel;
    statusBar()->addPermanentWidget(m_currentLabel);
    finaliseMenus();
    newSession();

    // Create transcription editor
    d->annotationEditor = new AnnotationTimelineEditor(this);
    // d->annotationEditor->setOrientation(Qt::Horizontal);
    ui->gridLayoutEditor->addWidget(d->annotationEditor);

    m_viewManager->setToolMode(ViewManager::SelectMode);
}

TranscriberWidget::~TranscriberWidget()
{
    delete ui;
    delete d;
}

void TranscriberWidget::newSession()
{
    if (!checkSaveModified()) return;
    closeSession();
    createDocument();
    Pane *pane = m_paneStack->addPane();
    connect(pane, SIGNAL(contextHelpChanged(const QString &)),
            this, SLOT(contextHelpChanged(const QString &)));
    if (!m_timeRulerLayer) {
        m_timeRulerLayer = m_document->createMainModelLayer(LayerFactory::Type("TimeRuler"));
    }
    m_document->addLayerToView(pane, m_timeRulerLayer);
    Layer *waveform = m_document->createMainModelLayer(LayerFactory::Type("Waveform"));
    m_document->addLayerToView(pane, waveform);
    m_overview->registerView(pane);
    CommandHistory::getInstance()->clear();
    CommandHistory::getInstance()->documentSaved();
    documentRestored();
    updateMenuStates();
}

void TranscriberWidget::selectedCorpusCommunication(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    QPointer<CorpusRecording> rec(0);
    QPointer<CorpusAnnotation> annot(0);
    if (!corpus) return;
    if (!com) return;
    if (com->recordingsCount() >= 1) rec = com->recordings().first();
    if (com->annotationsCount() >= 1) annot = com->annotations().first();
    selectionChanged(corpus, com, rec, annot);
}

void TranscriberWidget::selectedCorpusRecording(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
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
    selectionChanged(corpus, com, rec, annot);
}

void TranscriberWidget::selectedCorpusAnnotation(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
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
    selectionChanged(corpus, com, rec, annot);
}

void TranscriberWidget::selectionChanged(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
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
    d->currentCommunicationID = com->ID();
    closeSession();
    newSessionWithCommunication(com);
    if ((rec) && (d->currentRecordingID != rec->ID())) {
        d->currentRecordingID = rec->ID();
    }
    else {
        closeSession();
        d->currentRecordingID.clear();
    }
    if ((annot) && (d->currentAnnotationID != annot->ID())) {
        // openForEditing(corpus, annot->ID());
        d->currentAnnotationID = annot->ID();
    }
    else {
        d->currentAnnotationID.clear();
    }
}

void TranscriberWidget::moveToAnnotationTime(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot, RealTime time)
{
//    if (!corpus) return;
//    if (!com) return;
//    if (!annot) return;

//    if (d->currentCorpus == corpus && d->currentAnnotationID == annot->ID()) {
//        d->editor->moveToTime(time);
//        d->editor->setFocus();
//    }
//    else {
//        if (com->recordingsCount() == 1) {
//            d->mediaPlayer->setMedia(QUrl::fromLocalFile(corpus->baseMediaPath() + "/" + com->recordings().first()->filename()));
//        }
//        openForEditing(corpus, annot->ID());
//        d->editor->moveToTime(time);
//        d->editor->setFocus();
//    }
}

//void TranscriberWidget::openForEditing(Corpus *corpus, const QString &annotationID)
//{
//    if (!corpus) return;
//    if (!corpus->datastoreAnnotations()) return;
//    d->currentCorpus = corpus;
//    d->currentAnnotationID = annotationID;
//    qDeleteAll(d->currentTierGroups);
//    d->currentTierGroups.clear();

//    d->waitingSpinner->start();
//    QApplication::processEvents();

//    d->currentTierGroups = corpus->datastoreAnnotations()->getTiersAllSpeakers(d->currentAnnotationID);
//    d->editor->setData(d->currentTierGroups, d->timelineConfig->selectedLevelsAttributes());
//    d->timelineConfig->updateSpeakerList(d->currentTierGroups.keys());
//    updateAnnotationControls();

//    d->waitingSpinner->stop();
//}
