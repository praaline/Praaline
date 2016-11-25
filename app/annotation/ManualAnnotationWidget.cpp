#include <QPointer>
#include <QString>
#include <QList>
#include <QMessageBox>
#include <QMap>
#include <QDebug>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QtMultimedia>
#include <QSignalBlocker>
#include "ManualAnnotationWidget.h"
#include "ui_ManualAnnotationWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/AnnotationTierGroup.h"

#include "pngui/observers/corpusobserver.h"
#include "pngui/model/checkableproxymodel.h"
#include "pngui/model/corpus/CorpusBookmarkModel.h"
#include "pngui/widgets/annotationtimelineeditor.h"
#include "pngui/widgets/corpusitemselectorwidget.h"
#include "pngui/widgets/timelineeditorconfigwidget.h"
#include "pngui/widgets/waitingspinnerwidget.h"

#include "dis/AnnotationControlsDisfluencies.h"

struct ManualAnnotationWidgetData {
    ManualAnnotationWidgetData() :
        corpusItemSelector(0), currentCorpus(0),
        annotationControls(0), editor(0),
        loopInsideInterval(false), tPauseAt_msec(0),
        autoSave(false)
    { }

    // Corpus
    CorpusItemSelectorWidget *corpusItemSelector;
    QPointer<Corpus> currentCorpus;
    QString currentCommunicationID;
    QString currentRecordingID;
    QString currentAnnotationID;

    // State
    QMap<QString, QPointer<AnnotationTierGroup> > currentTierGroups;

    AnnotationControlsDisfluencies *annotationControls;

    // Main toolbar
    QToolBar *toolbarMain;
    QAction *actionSave;
    QAction *actionPlay;
    QAction *actionPause;
    QAction *actionStop;

    // Editor
    QMainWindow *innerwindowEditor;
    QToolBar *toolbarEditor;
    AnnotationTimelineEditor *editor;
    QAction *actionEditorIntervalSplit;
    QAction *actionEditorIntervalJoin;
    QAction *actionToggleOrientation;
    QAction *actionRemoveSorting;

    // Configuration: levels/attributes + speakers
    TimelineEditorConfigWidget *timelineConfig;

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

    bool autoSave;

    WaitingSpinnerWidget* waitingSpinner;
};


ManualAnnotationWidget::ManualAnnotationWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ManualAnnotationWidget), d(new ManualAnnotationWidgetData)
{
    setParent(parent);
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

    // Toolbars and actions
    d->toolbarMain = new QToolBar(tr("Manual annotation"), this);
    d->toolbarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMain->setIconSize(QSize(24, 24));
    d->toolbarEditor = new QToolBar(tr("Vertical timeline editor"), this);
    d->toolbarEditor->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarEditor->setIconSize(QSize(24, 24));
    this->addToolBar(d->toolbarMain);

    // Editor
    d->innerwindowEditor = new QMainWindow(this);
    d->innerwindowEditor->addToolBar(d->toolbarEditor);
    d->editor = new AnnotationTimelineEditor(this);
    d->innerwindowEditor->setCentralWidget(d->editor);
    ui->gridLayoutEditor->addWidget(d->innerwindowEditor);

    // Annotation controls
    d->annotationControls = new AnnotationControlsDisfluencies(this);
    ui->gridLayoutShortcuts->setMargin(0);
    ui->gridLayoutShortcuts->addWidget(d->annotationControls);
    connect(d->editor, SIGNAL(selectedRowsChanged(QList<int>)),
            this, SLOT(verticalTimelineSelectedRowsChanged(QList<int>)));
    connect(d->editor, SIGNAL(currentIndexChanged(QModelIndex,QModelIndex)),
            this, SLOT(verticalTimelineCurrentIndexChanged(QModelIndex,QModelIndex)));

    d->mediaPlayer = new QMediaPlayer(this);
    d->mediaPlayer->setNotifyInterval(20);
    connect(d->mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(mediaPositionChanged(qint64)));

    setupActions();

    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    d->actionEditorIntervalJoin->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_J));
    d->actionEditorIntervalSplit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));

    ui->splitterLR->setSizes(QList<int>() << 50 << 300);

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
}

ManualAnnotationWidget::~ManualAnnotationWidget()
{
    delete ui;
    qDeleteAll(d->currentTierGroups);
    delete d;
}

void ManualAnnotationWidget::setupActions()
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

    d->actionToggleOrientation = new QAction(QIcon(":/icons/actions/change_orientation.png"), tr("Change orientation"), this);
    connect(d->actionToggleOrientation, SIGNAL(triggered()), this, SLOT(toggleOrientation()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.ToggleOrientation", d->actionToggleOrientation, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarEditor->addAction(d->actionToggleOrientation);

    d->actionRemoveSorting = new QAction(QIcon(":/icons/actions/sort_remove.png"), tr("Remove Sort"), this);
    connect(d->actionRemoveSorting, SIGNAL(triggered()), d->editor, SLOT(removeSorting()));
    command = ACTION_MANAGER->registerAction("Annotation.TimelineEditor.RemoveSorting", d->actionRemoveSorting, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarEditor->addAction(d->actionRemoveSorting);

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

// ====================================================================================================================
// Corpus item selection (selector -> editor)
// ====================================================================================================================

void ManualAnnotationWidget::selectedCorpusCommunication(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    QPointer<CorpusRecording> rec(0);
    QPointer<CorpusAnnotation> annot(0);
    if (!corpus) return;
    if (!com) return;
    if (com->recordingsCount() >= 1) rec = com->recordings().first();
    if (com->annotationsCount() >= 1) annot = com->annotations().first();
    selectionChanged(corpus, com, rec, annot);
}

void ManualAnnotationWidget::selectedCorpusRecording(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
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

void ManualAnnotationWidget::selectedCorpusAnnotation(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
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

void ManualAnnotationWidget::selectionChanged(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                              QPointer<CorpusRecording> rec, QPointer<CorpusAnnotation> annot)
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
        d->mediaPlayer->setMedia(QUrl::fromLocalFile(corpus->baseMediaPath() + "/" + rec->filename()));
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

void ManualAnnotationWidget::moveToAnnotationTime(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com, QPointer<CorpusAnnotation> annot, RealTime time)
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
            d->mediaPlayer->setMedia(QUrl::fromLocalFile(corpus->baseMediaPath() + "/" + com->recordings().first()->filename()));
        }
        openForEditing(corpus, annot->ID());
        d->editor->moveToTime(time);
        d->editor->setFocus();
    }
}

void ManualAnnotationWidget::openForEditing(Corpus *corpus, const QString &annotationID)
{
    if (!corpus) return;
    if (!corpus->datastoreAnnotations()) return;
    d->currentCorpus = corpus;
    d->currentAnnotationID = annotationID;
    qDeleteAll(d->currentTierGroups);
    d->currentTierGroups.clear();

    d->waitingSpinner->start();
    QApplication::processEvents();

    d->currentTierGroups = corpus->datastoreAnnotations()->getTiersAllSpeakers(d->currentAnnotationID);
    d->editor->setData(d->currentTierGroups, d->timelineConfig->selectedLevelsAttributes());
    d->timelineConfig->updateSpeakerList(d->currentTierGroups.keys());
    updateAnnotationControls();

    d->waitingSpinner->stop();
}

void ManualAnnotationWidget::saveAnnotations()
{
    foreach (QString speakerID, d->currentTierGroups.keys())
        d->currentCorpus->datastoreAnnotations()->saveTiers(d->currentAnnotationID, speakerID, d->currentTierGroups.value(speakerID));
}

void ManualAnnotationWidget::selectedLevelsAttributesChanged()
{
    QList<QPair<QString, QString> > columns = d->timelineConfig->selectedLevelsAttributes();
    d->editor->setData(d->currentTierGroups, columns);
    updateAnnotationControls();
}

void ManualAnnotationWidget::speakerAdded(const QString &speakerID)
{
    if (!d->currentTierGroups.contains(speakerID)) return;
    d->editor->addTierGroup(speakerID, d->currentTierGroups.value(speakerID));
    updateAnnotationControls();
}

void ManualAnnotationWidget::speakerRemoved(const QString &speakerID)
{
    d->editor->removeTierGroup(speakerID);
    updateAnnotationControls();
}

void ManualAnnotationWidget::updateAnnotationControls()
{
    // Annotation controls
    if (d->editor->model()) {
        d->annotationControls->setModel(d->editor->model(), 3, 5, 8);
    }
}

void ManualAnnotationWidget::toggleOrientation()
{
    d->editor->toggleOrientation();
}

void ManualAnnotationWidget::verticalTimelineSelectedRowsChanged(QList<int> rows)
{
    if (d->annotationControls)
        d->annotationControls->setSelection(rows);
    if (d->editor->model() && !rows.isEmpty()) {
        d->currentRow = rows.first();
        d->tMin_msec_selected = d->editor->model()->data(d->editor->model()->index(rows.first(), 1), Qt::DisplayRole).toDouble() * 1000;
        d->tMax_msec_selected = d->editor->model()->data(d->editor->model()->index(rows.first(), 2), Qt::DisplayRole).toDouble() * 1000;
        if (d->mediaPlayer->state() != QMediaPlayer::PlayingState) {
            const QSignalBlocker blocker(d->mediaPlayer);
            // no signals here
            d->mediaPlayer->setPosition(d->tMin_msec_selected);
            // qDebug() << "verticalTimelineSelectedRowsChanged : mediaplayer->set position" << d->tMin_msec_selected;
        }
        d->tPauseAt_msec = 0;
    } else {
        d->currentRow = 0;
        d->tMin_msec_selected = 0;
        d->tMax_msec_selected = 0;
    }
}

void ManualAnnotationWidget::verticalTimelineCurrentIndexChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (d->editor->model()) {
        d->currentRow = current.row();
        d->tMin_msec_selected = d->editor->model()->data(d->editor->model()->index(d->currentRow, 1), Qt::DisplayRole).toDouble() * 1000;
        d->tMax_msec_selected = d->editor->model()->data(d->editor->model()->index(d->currentRow, 2), Qt::DisplayRole).toDouble() * 1000;
        if (d->mediaPlayer->state() != QMediaPlayer::PlayingState) {
            const QSignalBlocker blocker(d->mediaPlayer);
            // no signals here
            d->mediaPlayer->setPosition(d->tMin_msec_selected);
            // qDebug() << "verticalTimelineCurrentIndexChanged : mediaplayer->set position" << d->tMin_msec_selected;
        }
        d->tPauseAt_msec = 0;
    } else {
        d->currentRow = 0;
        d->tMin_msec_selected = 0;
        d->tMax_msec_selected = 0;
    }
}

void ManualAnnotationWidget::mediaPositionChanged(qint64 position)
{
    if (!d->loopInsideInterval) {
        if (!(d->editor->model())) return;
        RealTime t = RealTime::fromMilliseconds(position);
        d->editor->moveToTime(t);
        // qDebug() << "mediaPositionChanged, move to time" << t.toDouble();
    } else {
        if (position > d->tMax_msec_selected) {
            mediaPause();
            if (d->tMin_msec_selected > 0) d->mediaPlayer->setPosition(d->tMin_msec_selected);
        }
        else if (d->tPauseAt_msec > 0 && position > d->tPauseAt_msec) {
            mediaPause();
            if (d->tMin_msec_selected > 0) d->mediaPlayer->setPosition(d->tMin_msec_selected);
            d->tPauseAt_msec = 0;
        }
    }
}

void ManualAnnotationWidget::mediaPlay()
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

void ManualAnnotationWidget::mediaPause()
{
    d->mediaPlayer->pause();
    d->tPauseAt_msec = 0;
    d->actionPause->setShortcut(QKeySequence());
    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
}

void ManualAnnotationWidget::mediaStop()
{
    d->mediaPlayer->stop();
    d->tPauseAt_msec = 0;
    d->actionPause->setShortcut(QKeySequence());
    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
}

void ManualAnnotationWidget::intervalJoin()
{
    d->editor->annotationsMerge();
    d->tMin_msec_selected = d->editor->model()->data(d->editor->model()->index(d->currentRow, 1), Qt::DisplayRole).toDouble() * 1000;
    d->tMax_msec_selected = d->editor->model()->data(d->editor->model()->index(d->currentRow, 2), Qt::DisplayRole).toDouble() * 1000;
    d->mediaPlayer->setPosition(d->tMin_msec_selected);
    d->tPauseAt_msec = 0;
}

void ManualAnnotationWidget::intervalSplit()
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

void ManualAnnotationWidget::moveMinBoundaryLeft()
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

void ManualAnnotationWidget::moveMaxBoundaryRight()
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
