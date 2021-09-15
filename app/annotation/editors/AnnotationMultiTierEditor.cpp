#include <QString>
#include <QList>
#include <QPointer>
#include <QMap>
#include <QMainWindow>
#include <QToolBar>
#include <QtMultimedia>
#include <QSignalBlocker>

#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"
using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusAnnotation.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "pngui/PraalineUserInterfaceOptions.h"

#include "pngui/widgets/AnnotationMultiTierEditorWidget.h"
#include "pngui/widgets/WaitingSpinnerWidget.h"
#include "pngui/widgets/TimelineEditorConfigWidget.h"

// Annotation widgets: they add functionality to the editor and are activated depending on the currently edited level/attribute
#include "../widgets/AnnotationWidgetDisfluencies.h"
#include "../widgets/AnnotationWidgetSequences.h"

#include "AnnotationMultiTierEditor.h"


struct AnnotationMultiTierEditorData {
    AnnotationMultiTierEditorData() :
        annotationWidgetForDisfluencies(nullptr), annotationWidgetForSequences(nullptr), editor(nullptr),
        loopInsideInterval(false), tPauseAt_msec(0),
        autoSave(false)
    {}

    // Additional annotation widgets
    AnnotationWidgetDisfluencies *annotationWidgetForDisfluencies;
    AnnotationWidgetSequences *annotationWidgetForSequences;
    // Main toolbar
    QToolBar *toolbarMain;
    QAction *actionSave;
    QAction *actionPlay;
    QAction *actionPause;
    QAction *actionStop;
    // Editor
    AnnotationMultiTierEditorWidget *editor;
    QToolBar *toolbarEditor;
    QAction *actionEditorIntervalSplit;
    QAction *actionEditorIntervalJoin;
    QAction *actionToggleOrientation;
    QAction *actionRemoveSorting;
    QAction *actionToggleTimelineConfig;
    QAction *actionToggleSecondaryEditor;
    // Configuration: levels/attributes + speakers
    QDockWidget *dockTimelineConfig;
    TimelineEditorConfigWidget *widgetTimelineConfig;
    // Secondary editors
    QTabWidget *tabSecondaryEditors;
    // Waiting spinner
    WaitingSpinnerWidget* waitingSpinner;
    // Media player
    QMediaPlayer *mediaPlayer;
    qint64 tMin_msec_selected;
    qint64 tMax_msec_selected;
    bool loopInsideInterval;
    // Sync
    int currentRow;
    QAction *actionMoveMinBoundaryLeft;
    QAction *actionMoveMaxBoundaryRight;
    qint64 tPauseAt_msec;
    // Options
    bool autoSave;
    // State
    SpeakerAnnotationTierGroupMap currentTierGroups;
    QPointer<Corpus> currentCorpus;
    QString currentCommunicationID;
    QString currentRecordingID;
    QString currentAnnotationID;
};

AnnotationMultiTierEditor::AnnotationMultiTierEditor(QWidget *parent) :
    QMainWindow(parent), d(new AnnotationMultiTierEditorData())
{
    // Toolbars and actions
    d->toolbarMain = new QToolBar(tr("Manual annotation"), this);
    d->toolbarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMain->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    d->toolbarEditor = new QToolBar(tr("Vertical timeline editor"), this);
    d->toolbarEditor->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarEditor->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    // Editor grid
    d->editor = new AnnotationMultiTierEditorWidget(this);
    d->editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // Timeline configuration
    d->widgetTimelineConfig = new TimelineEditorConfigWidget(this);
    connect(d->widgetTimelineConfig, SIGNAL(selectedLevelsAttributesChanged()),
            this, SLOT(selectedLevelsAttributesChanged()));
    connect(d->widgetTimelineConfig, SIGNAL(speakerAdded(QString)),
            this, SLOT(speakerAdded(QString)));
    connect(d->widgetTimelineConfig, SIGNAL(speakerRemoved(QString)),
            this, SLOT(speakerRemoved(QString)));
    d->dockTimelineConfig = new QDockWidget(tr("Timeline Configuration"), this);
    d->dockTimelineConfig->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    d->dockTimelineConfig->setWidget(d->widgetTimelineConfig);
    addDockWidget(Qt::RightDockWidgetArea, d->dockTimelineConfig);
    // Annotation widgets
    d->annotationWidgetForDisfluencies = new AnnotationWidgetDisfluencies(this);
    d->annotationWidgetForSequences = new AnnotationWidgetSequences(this);
    // Annotation widgets need to be informed about changes to the selected row/column
    connect(d->editor, SIGNAL(selectedRowsChanged(QList<int>)),
            this, SLOT(timelineSelectedRowsChanged(QList<int>)));
    connect(d->editor, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)),
            this, SLOT(timelineCurrentIndexChanged(QModelIndex,QModelIndex)));
    // Media player
    d->mediaPlayer = new QMediaPlayer(this);
    d->mediaPlayer->setNotifyInterval(20);
    connect(d->mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(mediaPositionChanged(qint64)));
    // Waiting spinner while load corpus data
    d->waitingSpinner = new WaitingSpinnerWidget(this);
    d->waitingSpinner->setRoundness(70.0);
    d->waitingSpinner->setMinimumTrailOpacity(15.0);
    d->waitingSpinner->setTrailFadePercentage(70.0);
    d->waitingSpinner->setNumberOfLines(12);
    d->waitingSpinner->setLineLength(15);
    d->waitingSpinner->setLineWidth(5);
    d->waitingSpinner->setInnerRadius(10);
    d->waitingSpinner->setRevolutionsPerSecond(1);
    d->waitingSpinner->setColor(QColor(81, 4, 71));
    // Layout: toolbar at the top, followed by a tab widget containing the annotation widgets
    // and editor grid as the main control.
    // ...toolbars
    this->addToolBar(d->toolbarMain);
    this->addToolBar(d->toolbarEditor);
    // ...annotation widgets in tabs
    d->tabSecondaryEditors = new QTabWidget(this);
    d->tabSecondaryEditors->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    d->tabSecondaryEditors->addTab(d->annotationWidgetForSequences, "Sequences");
    d->tabSecondaryEditors->addTab(d->annotationWidgetForDisfluencies, "Disfluencies");
    // ...main layout
    QSplitter *mainContents = new QSplitter(this);
    mainContents->setOrientation(Qt::Vertical);
    QGridLayout *layout = new QGridLayout(mainContents);
    layout->setMargin(0);
    layout->addWidget(d->editor);
    layout->addWidget(d->tabSecondaryEditors);
    mainContents->setLayout(layout);
    this->setCentralWidget(mainContents);
    // Actions
    setupActions();
    // Shortcuts
    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    d->actionEditorIntervalJoin->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_J));
    d->actionEditorIntervalSplit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
}

AnnotationMultiTierEditor::~AnnotationMultiTierEditor()
{
    qDeleteAll(d->currentTierGroups);
    delete d;
}

void AnnotationMultiTierEditor::setupActions()
{
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    Command* command;

    // MAIN TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionSave = new QAction(QIcon(":/icons/actions/action_save.png"), tr("Save Annotations"), this);
    connect(d->actionSave, SIGNAL(triggered()), SLOT(saveAnnotations()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.Save", d->actionSave, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMain->addAction(d->actionSave);
    d->actionSave->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));

    d->actionPlay = new QAction(QIcon(":/icons/media/media_play.png"), tr("Play"), this);
    connect(d->actionPlay, SIGNAL(triggered()), SLOT(mediaPlay()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.Play", d->actionPlay, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMain->addAction(d->actionPlay);

    d->actionPause = new QAction(QIcon(":/icons/media/media_pause.png"), tr("Pause"), this);
    connect(d->actionPause, SIGNAL(triggered()), SLOT(mediaPause()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.Pause", d->actionPause, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMain->addAction(d->actionPause);

    d->actionStop = new QAction(QIcon(":/icons/media/media_stop.png"), tr("Stop"), this);
    connect(d->actionStop, SIGNAL(triggered()), SLOT(mediaStop()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.Stop", d->actionStop, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMain->addAction(d->actionStop);

    // EDITOR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionEditorIntervalJoin = new QAction(QIcon(":/icons/actions/interval_join.png"), tr("Join Intervals"), this);
    connect(d->actionEditorIntervalJoin, SIGNAL(triggered()), this, SLOT(intervalJoin()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.IntervalJoin", d->actionEditorIntervalJoin, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarEditor->addAction(d->actionEditorIntervalJoin);

    d->actionEditorIntervalSplit = new QAction(QIcon(":/icons/actions/interval_split.png"), tr("Split Intervals"), this);
    connect(d->actionEditorIntervalSplit, SIGNAL(triggered()), this, SLOT(intervalSplit()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.IntervalSplit", d->actionEditorIntervalSplit, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarEditor->addAction(d->actionEditorIntervalSplit);

    d->actionToggleOrientation = new QAction(QIcon(":/icons/actions/change_orientation.png"), tr("Change Orientation"), this);
    connect(d->actionToggleOrientation, SIGNAL(triggered()), this, SLOT(toggleOrientation()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.ToggleOrientation", d->actionToggleOrientation, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarEditor->addAction(d->actionToggleOrientation);

    d->actionRemoveSorting = new QAction(QIcon(":/icons/actions/sort_remove.png"), tr("Remove Sort"), this);
    connect(d->actionRemoveSorting, SIGNAL(triggered()), d->editor, SLOT(removeSorting()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.RemoveSorting", d->actionRemoveSorting, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarEditor->addAction(d->actionRemoveSorting);

    d->actionToggleSecondaryEditor = new QAction(QIcon(":/icons/actions/toggle_secondary_editor.png"), tr("Secondary Editor"), this);
    connect(d->actionToggleSecondaryEditor, SIGNAL(triggered()), this, SLOT(toggleSecondaryEditor()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.ToggleSecondaryEditor", d->actionToggleSecondaryEditor, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarEditor->addAction(d->actionToggleSecondaryEditor);

    d->actionToggleTimelineConfig = new QAction(QIcon(":/icons/actions/toggle_config.png"), tr("Options"), this);
    connect(d->actionToggleTimelineConfig, SIGNAL(triggered()), this, SLOT(toggleTimelineConfig()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.ToggleTimelineConfig", d->actionToggleTimelineConfig, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarEditor->addAction(d->actionToggleTimelineConfig);

    // SYNC ACTIONS
    // ----------------------------------------------------------------------------------------------------------------
    d->actionMoveMinBoundaryLeft = new QAction(tr("Move min boundary left"), this);
    connect(d->actionMoveMinBoundaryLeft, SIGNAL(triggered()), this, SLOT(moveMinBoundaryLeft()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.MoveMinBoundaryLeft", d->actionMoveMinBoundaryLeft, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->editor->addAction(d->actionMoveMinBoundaryLeft);
    d->actionMoveMinBoundaryLeft->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus));

    d->actionMoveMaxBoundaryRight = new QAction(tr("Move max boundary right"), this);
    connect(d->actionMoveMaxBoundaryRight, SIGNAL(triggered()), this, SLOT(moveMaxBoundaryRight()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.MoveMaxBoundaryRight", d->actionMoveMaxBoundaryRight, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->editor->addAction(d->actionMoveMaxBoundaryRight);
    d->actionMoveMaxBoundaryRight->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus));

}

// ============================================================================================================================================================
// Implementation of AnnotationEditorBase
// ============================================================================================================================================================

void AnnotationMultiTierEditor::open(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot)
{
    if (d->autoSave) {
        saveAnnotations();
    }
    if (!com) {
        d->currentCommunicationID.clear();
        d->currentRecordingID.clear();
        d->currentAnnotationID.clear();
        return;
    }
    d->currentCommunicationID = com->ID();
    if ((rec) && (d->currentRecordingID != rec->ID())) {
        d->mediaPlayer->setMedia(QUrl::fromLocalFile(rec->filePath()));
        d->currentRecordingID = rec->ID();
    }
    else {
        d->mediaPlayer->setMedia(QMediaContent());
        d->currentRecordingID.clear();
    }
    if ((annot) && (d->currentAnnotationID != annot->ID())) {
        openForEditing(corpus, annot->ID());
        d->currentAnnotationID = annot->ID();
    }
    else {
        d->currentAnnotationID.clear();
    }
}

void AnnotationMultiTierEditor::jumpToTime(Corpus *corpus, CorpusCommunication *com, CorpusAnnotation *annot, const RealTime &time)
{
    if (!corpus) return;
    if (!com) return;
    if (!annot) return;

    if (d->currentCorpus == corpus && d->currentAnnotationID == annot->ID()) {
        d->editor->moveToTime(time);
        d->editor->setFocus();
    }
    else {
        if (com->recordingsCount() == 1) {
            d->mediaPlayer->setMedia(QUrl::fromLocalFile(com->recordings().first()->filePath()));
        }
        openForEditing(corpus, annot->ID());
        d->editor->moveToTime(time);
        d->editor->setFocus();
    }
}

// ============================================================================================================================================================

void AnnotationMultiTierEditor::speakerAdded(const QString &speakerID)
{
    if (!d->currentTierGroups.contains(speakerID)) return;
    d->editor->addTierGroup(speakerID, d->currentTierGroups.value(speakerID));
    updateAnnotationControls();
}

void AnnotationMultiTierEditor::speakerRemoved(const QString &speakerID)
{
    d->editor->removeTierGroup(speakerID);
    updateAnnotationControls();
}

void AnnotationMultiTierEditor::selectedLevelsAttributesChanged()
{
    d->editor->setData(d->currentTierGroups, d->widgetTimelineConfig->selectedLevelsAttributes());
    updateAnnotationControls();
}

void AnnotationMultiTierEditor::saveAnnotations()
{
    foreach (QString speakerID, d->currentTierGroups.keys())
        d->currentCorpus->repository()->annotations()->saveTiers(d->currentAnnotationID, speakerID, d->currentTierGroups.value(speakerID));
}

void AnnotationMultiTierEditor::openForEditing(Corpus *corpus, const QString &annotationID)
{
    if (!corpus) return;
    if (!corpus->repository()) return;
    d->currentCorpus = corpus;
    d->currentAnnotationID = annotationID;
    qDeleteAll(d->currentTierGroups);
    d->currentTierGroups.clear();

    d->waitingSpinner->start();
    QApplication::processEvents();

    d->currentTierGroups = corpus->repository()->annotations()->getTiersAllSpeakers(d->currentAnnotationID);
    d->editor->setData(d->currentTierGroups, d->widgetTimelineConfig->selectedLevelsAttributes());
    d->editor->resizeColumnsToContents();
    d->widgetTimelineConfig->updateSpeakerList(d->currentTierGroups.keys());
    updateAnnotationControls();

    d->waitingSpinner->stop();
}

void AnnotationMultiTierEditor::updateAnnotationControls()
{
    // Annotation controls
    if (d->editor->model()) {
        d->annotationWidgetForDisfluencies->setModel(d->editor->model());
        if (d->currentCorpus)
            d->annotationWidgetForSequences->setCorpusRepositoryAndModel(d->currentCorpus->repository(), d->editor->model());
    }
}

void AnnotationMultiTierEditor::toggleOrientation()
{
    d->editor->toggleOrientation();
}

void AnnotationMultiTierEditor::toggleSecondaryEditor()
{
    d->tabSecondaryEditors->setVisible(!d->tabSecondaryEditors->isVisible());
}

void AnnotationMultiTierEditor::toggleTimelineConfig()
{
    d->dockTimelineConfig->toggleViewAction()->activate(QAction::Trigger);
}

void AnnotationMultiTierEditor::timelineSelectedRowsChanged(QList<int> rows)
{
    // Inform the annotation controls about the change in selection
    if (d->annotationWidgetForDisfluencies) {
        d->annotationWidgetForDisfluencies->setSelection(rows);
    }
    if (d->annotationWidgetForSequences) {
        d->annotationWidgetForSequences->setSelection(rows);
    }
    // Media player
    if (d->editor->model() && !rows.isEmpty()) {
        d->currentRow = rows.first();
        d->tMin_msec_selected = d->editor->model()->data(d->editor->model()->index(rows.first(), 1), Qt::DisplayRole).toDouble() * 1000;
        d->tMax_msec_selected = d->editor->model()->data(d->editor->model()->index(rows.first(), 2), Qt::DisplayRole).toDouble() * 1000;
        if (d->mediaPlayer->state() != QMediaPlayer::PlayingState) {
            const QSignalBlocker blocker(d->mediaPlayer);
            // no signals here
            d->mediaPlayer->setPosition(d->tMin_msec_selected);
            qDebug() << "verticalTimelineSelectedRowsChanged : mediaplayer->set position" << d->tMin_msec_selected;
        }
        d->tPauseAt_msec = 0;
    } else {
        d->currentRow = 0;
        d->tMin_msec_selected = 0;
        d->tMax_msec_selected = 0;
    }
}

void AnnotationMultiTierEditor::timelineCurrentIndexChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (d->editor->model()) {
        d->currentRow = current.row();
        d->tMin_msec_selected = d->editor->model()->data(d->editor->model()->index(d->currentRow, 1), Qt::DisplayRole).toDouble() * 1000;
        d->tMax_msec_selected = d->editor->model()->data(d->editor->model()->index(d->currentRow, 2), Qt::DisplayRole).toDouble() * 1000;
        if (d->mediaPlayer->state() != QMediaPlayer::PlayingState) {
            const QSignalBlocker blocker(d->mediaPlayer);
            // no signals here
            d->mediaPlayer->setPosition(d->tMin_msec_selected);
            qDebug() << "verticalTimelineCurrentIndexChanged : mediaplayer->set position" << d->tMin_msec_selected;
        }
        d->tPauseAt_msec = 0;
    } else {
        d->currentRow = 0;
        d->tMin_msec_selected = 0;
        d->tMax_msec_selected = 0;
    }
}

void AnnotationMultiTierEditor::mediaPositionChanged(qint64 position)
{
    if (!d->loopInsideInterval) {
        if (!(d->editor->model())) return;
        if (d->mediaPlayer->state() == QMediaPlayer::PlayingState) {
            d->editor->moveToTime(RealTime::fromMilliseconds(position));
            // qDebug() << "mediaPositionChanged, move to time" << RealTime::fromMilliseconds(position).toDouble();
        }
    } else {
        if (position > d->tMax_msec_selected) {
            mediaPause();
            if (d->tMin_msec_selected > 0) {
                const QSignalBlocker blocker(d->mediaPlayer);
                // no signals here
                d->mediaPlayer->setPosition(d->tMin_msec_selected);
            }
        }
        else if (d->tPauseAt_msec > 0 && position > d->tPauseAt_msec) {
            mediaPause();
            if (d->tMin_msec_selected > 0) {
                const QSignalBlocker blocker(d->mediaPlayer);
                // no signals here
                d->mediaPlayer->setPosition(d->tMin_msec_selected);
            }
            d->tPauseAt_msec = 0;
        }
    }
}

// ********************************************************************************************************************
// Media Player controls
// ********************************************************************************************************************

void AnnotationMultiTierEditor::mediaPlay()
{
    if ((d->tMin_msec_selected > 0) && (d->tPauseAt_msec == 0)) {
        {
            const QSignalBlocker blocker(d->mediaPlayer);
            // no signals here
            d->mediaPlayer->setPosition(d->tMin_msec_selected);
            // qDebug() << "current index changed, mediaplayer->set position" << d->tMin_msec_selected;
        }
    }
    d->actionPlay->setShortcut(QKeySequence());
    d->actionPause->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    d->mediaPlayer->play();
}

void AnnotationMultiTierEditor::mediaPause()
{
    d->mediaPlayer->pause();
    d->tPauseAt_msec = 0;
    d->actionPause->setShortcut(QKeySequence());
    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
}

void AnnotationMultiTierEditor::mediaStop()
{
    d->mediaPlayer->stop();
    d->tPauseAt_msec = 0;
    d->actionPause->setShortcut(QKeySequence());
    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
}

// ********************************************************************************************************************
// Interval Editing: join, split, moving boundaries
// ********************************************************************************************************************

void AnnotationMultiTierEditor::intervalJoin()
{
    d->editor->annotationsMerge();
    d->tMin_msec_selected = d->editor->model()->data(d->editor->model()->index(d->currentRow, 1), Qt::DisplayRole).toDouble() * 1000;
    d->tMax_msec_selected = d->editor->model()->data(d->editor->model()->index(d->currentRow, 2), Qt::DisplayRole).toDouble() * 1000;
    d->mediaPlayer->setPosition(d->tMin_msec_selected);
    d->tPauseAt_msec = 0;
}

void AnnotationMultiTierEditor::intervalSplit()
{
    if ((d->mediaPlayer->state() == QMediaPlayer::PlayingState) ||
            (d->mediaPlayer->state() == QMediaPlayer::PausedState)) {
        qint64 position = d->mediaPlayer->position();
        d->editor->annotationsSplit(RealTime::fromMilliseconds(position));
    } else {
        d->editor->annotationsSplit();
    }
    d->tMin_msec_selected = d->editor->model()->data(d->editor->model()->index(d->currentRow, 1), Qt::DisplayRole).toDouble() * 1000;
    d->tMax_msec_selected = d->editor->model()->data(d->editor->model()->index(d->currentRow, 2), Qt::DisplayRole).toDouble() * 1000;
    d->mediaPlayer->setPosition(d->tMin_msec_selected);
    d->tPauseAt_msec = 0;
}

void AnnotationMultiTierEditor::moveMinBoundaryLeft()
{
    if (d->editor->model() && (d->currentRow != 0)) {
        double tMin = d->editor->model()->data(d->editor->model()->index(d->currentRow, 1), Qt::DisplayRole).toDouble();
        tMin = tMin - 0.050;
        d->editor->model()->setData(d->editor->model()->index(d->currentRow, 1), tMin);
        d->tMin_msec_selected = tMin * 1000;
        d->mediaPlayer->setPosition(tMin * 1000);
        d->tPauseAt_msec = tMin * 1000 + 500;
        mediaPlay();
    }
}

void AnnotationMultiTierEditor::moveMaxBoundaryRight()
{
    if (d->editor->model() && (d->currentRow != 0)) {
        double tMax = d->editor->model()->data(d->editor->model()->index(d->currentRow, 2), Qt::DisplayRole).toDouble();
        tMax = tMax + 0.050;
        d->editor->model()->setData(d->editor->model()->index(d->currentRow, 2), tMax);
        d->tMax_msec_selected = tMax * 1000;
        qint64 startFrom = tMax * 1000 - 500;
        if (startFrom < 0) startFrom = 0;
        if (startFrom < d->tMin_msec_selected) startFrom = d->tMin_msec_selected;
        d->tPauseAt_msec = tMax * 1000;
        d->mediaPlayer->setPosition(startFrom);
        mediaPlay();
    }
}
