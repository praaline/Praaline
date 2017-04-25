#include <QMainWindow>
#include <QToolBar>
#include <QGridLayout>
#include <QtMultimedia>
#include <QPlainTextEdit>

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
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "PraalineUserInterfaceOptions.h"

#include "TranscriptAnnotationEditor.h"

struct TranscriptAnnotationEditorData {
    TranscriptAnnotationEditorData() :
        autoSave(false), levelTokens("tok_min")
    {}
    // Main toolbar
    QToolBar *toolbarMain;
    QAction *actionSave;
    QAction *actionPlay;
    QAction *actionPause;
    QAction *actionStop;
    // Media player
    QMediaPlayer *mediaPlayer;
    // Transcription editor
    QPlainTextEdit *editor;
    // Options
    bool autoSave;
    QString levelTokens;
    // State
    QMap<QString, QPointer<AnnotationTierGroup> > currentTierGroups;
    QPointer<Corpus> currentCorpus;
    QString currentCommunicationID;
    QString currentRecordingID;
    QString currentAnnotationID;
};

TranscriptAnnotationEditor::TranscriptAnnotationEditor(QWidget *parent) :
    AnnotationEditorBase(parent), d(new TranscriptAnnotationEditorData)
{
    // Toolbars and actions
    d->toolbarMain = new QToolBar(tr("Manual annotation"), this);
    d->toolbarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMain->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    // Editor
    d->editor = new QPlainTextEdit(this);
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    d->editor->setFont(fixedFont);
    // Media player
    d->mediaPlayer = new QMediaPlayer(this);
    d->mediaPlayer->setNotifyInterval(20);
    connect(d->mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(mediaPositionChanged(qint64)));
    // Layout
    this->addToolBar(d->toolbarMain);
    QWidget *contents = new QWidget(this);
    QGridLayout *layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->addWidget(d->editor);
    contents->setLayout(layout);
    this->setCentralWidget(contents);
    // Actions
    setupActions();
}

TranscriptAnnotationEditor::~TranscriptAnnotationEditor()
{
    qDeleteAll(d->currentTierGroups);
    delete d;
}

void TranscriptAnnotationEditor::setupActions()
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

}

void TranscriptAnnotationEditor::open(Corpus *corpus, CorpusCommunication *com, CorpusRecording *rec, CorpusAnnotation *annot)
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

void TranscriptAnnotationEditor::jumpToTime(Corpus *corpus, CorpusCommunication *com, CorpusAnnotation *annot, const RealTime &time)
{
    if (!corpus) return;
    if (!com) return;
    if (!annot) return;

    if (d->currentCorpus == corpus && d->currentAnnotationID == annot->ID()) {
        // moveToTime(time);
        d->editor->setFocus();
    }
    else {
        if (com->recordingsCount() == 1) {
            d->mediaPlayer->setMedia(QUrl::fromLocalFile(com->recordings().first()->filePath()));
        }
        openForEditing(corpus, annot->ID());
        // moveToTime(time);
        d->editor->setFocus();
    }
}

void TranscriptAnnotationEditor::mediaPositionChanged(qint64 position)
{

}

void TranscriptAnnotationEditor::mediaPlay()
{
    d->actionPlay->setShortcut(QKeySequence());
    d->actionPause->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    d->mediaPlayer->play();
}

void TranscriptAnnotationEditor::mediaPause()
{
    d->mediaPlayer->pause();
    // d->tPauseAt_msec = 0;
    d->actionPause->setShortcut(QKeySequence());
    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
}

void TranscriptAnnotationEditor::mediaStop()
{
    d->mediaPlayer->stop();
    // d->tPauseAt_msec = 0;
    d->actionPause->setShortcut(QKeySequence());
    d->actionPlay->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
}

void TranscriptAnnotationEditor::openForEditing(Corpus *corpus, const QString &annotationID)
{
    if (!corpus) return;
    if (!corpus->repository()) return;
    d->currentCorpus = corpus;
    d->currentAnnotationID = annotationID;
    qDeleteAll(d->currentTierGroups);
    d->currentTierGroups.clear();
    // Load tiers
    d->currentTierGroups = corpus->repository()->annotations()->getTiersAllSpeakers(d->currentAnnotationID);
    // Create transcript
    // createTranscriptionText();
    foreach (QString speakerID, d->currentTierGroups.keys()) {
        IntervalTier *tierTokens = d->currentTierGroups.value(speakerID)->getIntervalTierByName(d->levelTokens);
        if (!tierTokens) continue;


    }
}

void TranscriptAnnotationEditor::saveAnnotations()
{
    foreach (QString speakerID, d->currentTierGroups.keys())
        d->currentCorpus->repository()->annotations()->saveTiers(d->currentAnnotationID, speakerID, d->currentTierGroups.value(speakerID));
}

void TranscriptAnnotationEditor::createTranscriptionText()
{
    // Create transcription
    QString transcription;
    foreach (QString speakerID, d->currentTierGroups.keys()) {
        IntervalTier *tierTokens = d->currentTierGroups.value(speakerID)->getIntervalTierByName(d->levelTokens);
        if (!tierTokens) continue;
        foreach (Interval *token, tierTokens->intervals()) {
            transcription.append(token->text()).append(" ");
        }
    }
    d->editor->setPlainText(transcription);
    // Sequence highlight
    QBrush backBrush(Qt::yellow);
    QBrush textBrush(Qt::black );
    QPen outlinePen(Qt::gray, 1);
    QList<QTextEdit::ExtraSelection> extraSelections;
    for(int i = 0; i < d->editor->document()->blockCount(); i++) {
        QTextBlock block = d->editor->document()->findBlockByNumber(i);
        if(block.isValid()) {
            QString text = block.text();
            int p;
            if ((p=text.indexOf("euh")) != -1) {
                int pos = block.position() + p;
                QTextEdit::ExtraSelection selection;
                selection.cursor = QTextCursor(d->editor->document());
                selection.cursor.setPosition(pos);
                selection.cursor.setPosition(pos + QString("euh").length(), QTextCursor::KeepAnchor);
                selection.format.setBackground(backBrush);
                selection.format.setForeground(textBrush);
                selection.format.setProperty(QTextFormat::OutlinePen, outlinePen);
                extraSelections.append(selection);
            }
        }
    }
    d->editor->setExtraSelections(extraSelections);
}

