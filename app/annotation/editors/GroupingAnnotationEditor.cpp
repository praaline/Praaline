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

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "PraalineUserInterfaceOptions.h"

#include "pngui/widgets/GroupingAnnotationEditorWidget.h"
#include "pngui/widgets/WaitingSpinnerWidget.h"
#include "pngui/widgets/TimelineEditorConfigWidget.h"

#include "GroupingAnnotationEditor.h"


struct GroupingAnnotationEditorData {
    GroupingAnnotationEditorData() :
        editor(0),
        loopInsideInterval(false), tPauseAt_msec(0),
        autoSave(false)
    {}

    // Main toolbar
    QToolBar *toolbarMain;
    QAction *actionSave;
    QAction *actionPlay;
    QAction *actionPause;
    QAction *actionStop;
    // Editor
    GroupingAnnotationEditorWidget *editor;
    QToolBar *toolbarEditor;
    QAction *actionRemoveSorting;
    QAction *actionToggleTimelineConfig;
    // Configuration: levels/attributes + speakers
    QDockWidget *dockTimelineConfig;
    TimelineEditorConfigWidget *widgetTimelineConfig;
    // Waiting spinner
    WaitingSpinnerWidget* waitingSpinner;
    // Media player
    QMediaPlayer *mediaPlayer;
    qint64 tMin_msec_selected;
    qint64 tMax_msec_selected;
    bool loopInsideInterval;
    // Sync
    int currentRow;
    qint64 tPauseAt_msec;
    // Options
    bool autoSave;
    // State
    QMap<QString, QPointer<AnnotationTierGroup> > currentTierGroups;
    QPointer<Corpus> currentCorpus;
    QString currentCommunicationID;
    QString currentRecordingID;
    QString currentAnnotationID;
};

GroupingAnnotationEditor::GroupingAnnotationEditor(QWidget *parent) :
    AnnotationEditorBase(parent), d(new GroupingAnnotationEditorData())
{
    // Toolbars and actions
    d->toolbarMain = new QToolBar(tr("Manual annotation"), this);
    d->toolbarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMain->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    d->toolbarEditor = new QToolBar(tr("Vertical timeline editor"), this);
    d->toolbarEditor->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarEditor->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    // Editor grid
    d->editor = new GroupingAnnotationEditorWidget(this);
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
    // Changes to editor state
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
    // Layout
    this->addToolBar(d->toolbarMain);
    this->addToolBar(d->toolbarEditor);
    QWidget *contents = new QWidget(this);
    QGridLayout *layout = new QGridLayout(contents);
    layout->setMargin(0);
    layout->addWidget(d->editor);
    contents->setLayout(layout);
    this->setCentralWidget(contents);
    // Actions
    setupActions();
    // Shortcuts
    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
}

GroupingAnnotationEditor::~GroupingAnnotationEditor()
{
    qDeleteAll(d->currentTierGroups);
    delete d;
}

void GroupingAnnotationEditor::setupActions()
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

    d->actionRemoveSorting = new QAction(QIcon(":/icons/actions/sort_remove.png"), tr("Remove Sort"), this);
    connect(d->actionRemoveSorting, SIGNAL(triggered()), d->editor, SLOT(removeSorting()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.RemoveSorting", d->actionRemoveSorting, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarEditor->addAction(d->actionRemoveSorting);

    d->actionToggleTimelineConfig = new QAction(QIcon(":/icons/actions/toggle_config.png"), tr("Editor Options"), this);
    connect(d->actionToggleTimelineConfig, SIGNAL(triggered()), this, SLOT(toggleTimelineConfig()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.ToggleTimelineConfig", d->actionToggleTimelineConfig, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarEditor->addAction(d->actionToggleTimelineConfig);

}

// ============================================================================================================================================================
// Implementation of AnnotationEditorBase
// ============================================================================================================================================================

void GroupingAnnotationEditor::open(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot)
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

void GroupingAnnotationEditor::jumpToTime(Corpus *corpus, CorpusCommunication *com, CorpusAnnotation *annot, const RealTime &time)
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

void GroupingAnnotationEditor::speakerAdded(const QString &speakerID)
{
    if (!d->currentTierGroups.contains(speakerID)) return;
    d->editor->addTierGroup(speakerID, d->currentTierGroups.value(speakerID));
}

void GroupingAnnotationEditor::speakerRemoved(const QString &speakerID)
{
    d->editor->removeTierGroup(speakerID);
}

void GroupingAnnotationEditor::selectedLevelsAttributesChanged()
{
    d->editor->setData(d->currentTierGroups, "transcription", "tok_min");
}

void GroupingAnnotationEditor::saveAnnotations()
{
    foreach (QString speakerID, d->currentTierGroups.keys())
        d->currentCorpus->repository()->annotations()->saveTiers(d->currentAnnotationID, speakerID, d->currentTierGroups.value(speakerID));
}

void GroupingAnnotationEditor::openForEditing(Corpus *corpus, const QString &annotationID)
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
    d->editor->setData(d->currentTierGroups, "", "");
    d->editor->resizeColumnsToContents();
    d->widgetTimelineConfig->updateSpeakerList(d->currentTierGroups.keys());

    d->waitingSpinner->stop();
}

void GroupingAnnotationEditor::toggleTimelineConfig()
{
    d->dockTimelineConfig->toggleViewAction()->activate(QAction::Trigger);
}

void GroupingAnnotationEditor::timelineSelectedRowsChanged(QList<int> rows)
{
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

void GroupingAnnotationEditor::timelineCurrentIndexChanged(const QModelIndex &current, const QModelIndex &previous)
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

void GroupingAnnotationEditor::mediaPositionChanged(qint64 position)
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

void GroupingAnnotationEditor::mediaPlay()
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

void GroupingAnnotationEditor::mediaPause()
{
    d->mediaPlayer->pause();
    d->tPauseAt_msec = 0;
    d->actionPause->setShortcut(QKeySequence());
    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
}

void GroupingAnnotationEditor::mediaStop()
{
    d->mediaPlayer->stop();
    d->tPauseAt_msec = 0;
    d->actionPause->setShortcut(QKeySequence());
    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
}
