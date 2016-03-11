#include <QMessageBox>

#include "svcore/data/model/WaveFileModel.h"
#include "svgui/view/Pane.h"
#include "svgui/view/PaneStack.h"
#include "svgui/view/Overview.h"
#include "svgui/layer/ColourDatabase.h"
#include "svgui/layer/WaveformLayer.h"
#include "svgui/layer/TimeRulerLayer.h"
#include "svgui/widgets/AudioDial.h"
#include "svgui/widgets/IconLoader.h"
#include "svgui/widgets/KeyReference.h"
#include "svgui/widgets/ActivityLog.h"
#include "svgui/widgets/Fader.h"
#include "svapp/audioio/AudioCallbackPlaySource.h"
#include "svapp/audioio/AudioCallbackPlayTarget.h"
#include "svapp/audioio/PlaySpeedRangeMapper.h"
#include "svapp/framework/Document.h"

#include "SimpleVisualiserWidget.h"

#include "../external/qtilities/include/QtilitiesCore/QtilitiesCore"
#include "../external/qtilities/include/QtilitiesCoreGui/QtilitiesCoreGui"
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;


SimpleVisualiserWidget::SimpleVisualiserWidget(const QString &contextStringID, bool withAudioOutput, bool withOSCSupport) :
    VisualiserWindowBase(withAudioOutput, withOSCSupport), m_contextStringID(contextStringID),
    m_visualiserFrame(0),
    m_overview(0),
    m_rightButtonMenu(0),
    m_rightButtonPlaybackMenu(0),
    m_soloAction(0),
    m_rwdStartAction(0),
    m_rwdSimilarAction(0),
    m_rwdAction(0),
    m_ffwdAction(0),
    m_ffwdSimilarAction(0),
    m_ffwdEndAction(0),
    m_playAction(0),
    m_playSelectionAction(0),
    m_playLoopAction(0),
    m_soloModified(false),
    m_prevSolo(false)
{
    // Get objects from global object pool
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("KeyReference");
    foreach (QObject* obj, list) {
        KeyReference *keyReference = qobject_cast<KeyReference *>(obj);
        if (keyReference) m_keyReference = keyReference;
    }
    list = OBJECT_MANAGER->registeredInterfaces("ActivityLog");
    foreach (QObject* obj, list) {
        ActivityLog *activityLog = qobject_cast<ActivityLog *>(obj);
        if (activityLog) m_activityLog = activityLog;
    }

    m_visualiserFrame = new QFrame;

    m_descriptionLabel = new QLabel;

    m_mainScroll = new QScrollArea(m_visualiserFrame);
    m_mainScroll->setWidgetResizable(true);
    m_mainScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_mainScroll->setFrameShape(QFrame::NoFrame);

    m_mainScroll->setWidget(m_paneStack);

    m_overview = new Overview(m_visualiserFrame);
    m_overview->setViewManager(m_viewManager);
    m_overview->setFixedHeight(40);
#ifndef _WIN32
    // For some reason, the contents of the overview never appear if we
    // make this setting on Windows.  I have no inclination at the moment
    // to track down the reason why.
    m_overview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
#endif
    connect(m_overview, SIGNAL(contextHelpChanged(const QString &)),
            this, SLOT(contextHelpChanged(const QString &)));

    m_panLayer = new WaveformLayer;
    m_panLayer->setChannelMode(WaveformLayer::MergeChannels);
    m_panLayer->setAggressiveCacheing(true);
    m_overview->addLayer(m_panLayer);

    if (m_viewManager->getGlobalDarkBackground()) {
        m_panLayer->setBaseColour(ColourDatabase::getInstance()->getColourIndex(tr("Bright Green")));
    } else {
        m_panLayer->setBaseColour(ColourDatabase::getInstance()->getColourIndex(tr("Green")));
    }

    m_fader = new Fader(m_visualiserFrame, false);
    connect(m_fader, SIGNAL(mouseEntered()), this, SLOT(mouseEnteredWidget()));
    connect(m_fader, SIGNAL(mouseLeft()), this, SLOT(mouseLeftWidget()));

    m_playSpeed = new AudioDial(m_visualiserFrame);
    m_playSpeed->setMinimum(60);
    m_playSpeed->setMaximum(120);
    m_playSpeed->setValue(100);
    m_playSpeed->setFixedWidth(32);
    m_playSpeed->setFixedHeight(32);
    m_playSpeed->setNotchesVisible(true);
    m_playSpeed->setPageStep(10);
    m_playSpeed->setObjectName(tr("Playback Speed"));
    m_playSpeed->setRangeMapper(new PlaySpeedRangeMapper(60, 120));
    m_playSpeed->setDefaultValue(100);
    m_playSpeed->setShowToolTip(true);
    connect(m_playSpeed, SIGNAL(valueChanged(int)), this, SLOT(playSpeedChanged(int)));
    connect(m_playSpeed, SIGNAL(mouseEntered()), this, SLOT(mouseEnteredWidget()));
    connect(m_playSpeed, SIGNAL(mouseLeft()), this, SLOT(mouseLeftWidget()));

    connect(m_viewManager, SIGNAL(activity(QString)),
            m_activityLog, SLOT(activityHappened(QString)));
    connect(m_playSource, SIGNAL(activity(QString)),
            m_activityLog, SLOT(activityHappened(QString)));
    connect(CommandHistory::getInstance(), SIGNAL(activity(QString)),
            m_activityLog, SLOT(activityHappened(QString)));
    connect(this, SIGNAL(activity(QString)),
            m_activityLog, SLOT(activityHappened(QString)));
    connect(this, SIGNAL(replacedDocument()), this, SLOT(documentReplaced()));
    m_activityLog->hide();

    // Signals from base connect to slots in the visualiser widget, which emits signals
    // in order to be able to synchronise itselft with other widgets (e.g. the annotations editor)
    connect(m_viewManager, SIGNAL(globalCentreFrameChanged(sv_frame_t)),
            this, SLOT(baseGlobalCentreFrameChanged(sv_frame_t)));
    connect(m_viewManager, SIGNAL(playbackFrameChanged(sv_frame_t)),
            this, SLOT(basePlaybackFrameChanged(sv_frame_t)));
}

SimpleVisualiserWidget::~SimpleVisualiserWidget()
{

}

void SimpleVisualiserWidget::setupMenus()
{

}

void SimpleVisualiserWidget::setupEditMenu()
{

}

void SimpleVisualiserWidget::setupViewMenu()
{

}

void SimpleVisualiserWidget::setupPlaybackMenusAndToolbar()
{
    m_keyReference->setCategory(tr("Playback and Transport Controls"));

    IconLoader il;
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID("Context.VisualisationMode"));

    m_rightButtonMenu->addSeparator();
    m_rightButtonPlaybackMenu = m_rightButtonMenu->addMenu(tr("Playback"));

    QToolBar *toolbar = addToolBar(tr("Playback Toolbar"));

    m_rwdStartAction = toolbar->addAction(il.load("rewind-start"), tr("Rewind to Start"));
    m_rwdStartAction->setShortcut(tr("Home"));
    m_rwdStartAction->setStatusTip(tr("Rewind to the start"));
    connect(m_rwdStartAction, SIGNAL(triggered()), this, SLOT(rewindStart()));
    connect(this, SIGNAL(canPlay(bool)), m_rwdStartAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.RewindStart", m_rwdStartAction, context);

    m_rwdAction = toolbar->addAction(il.load("rewind"), tr("Rewind"));
    m_rwdAction->setShortcut(tr("PgUp"));
    m_rwdAction->setStatusTip(tr("Rewind to the previous time instant or time ruler notch"));
    connect(m_rwdAction, SIGNAL(triggered()), this, SLOT(rewind()));
    connect(this, SIGNAL(canRewind(bool)), m_rwdAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.Rewind", m_rwdAction, context);

    m_rwdSimilarAction = new QAction(tr("Rewind to Similar Point"), this);
    m_rwdSimilarAction->setShortcut(tr("Shift+PgUp"));
    m_rwdSimilarAction->setStatusTip(tr("Rewind to the previous similarly valued time instant"));
    connect(m_rwdSimilarAction, SIGNAL(triggered()), this, SLOT(rewindSimilar()));
    connect(this, SIGNAL(canRewind(bool)), m_rwdSimilarAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.RewindSimilar", m_rwdSimilarAction, context);

    m_playAction = toolbar->addAction(il.load("playpause"), tr("Play / Pause"));
    m_playAction->setCheckable(true);
    m_playAction->setShortcut(tr("Space"));
    m_playAction->setStatusTip(tr("Start or stop playback from the current position"));
    connect(m_playAction, SIGNAL(triggered()), this, SLOT(play()));
    connect(m_playSource, SIGNAL(playStatusChanged(bool)),
            m_playAction, SLOT(setChecked(bool)));
    connect(m_playSource, SIGNAL(playStatusChanged(bool)),
            this, SLOT(playStatusChanged(bool)));
    connect(this, SIGNAL(canPlay(bool)), m_playAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.PlayPause", m_playAction, context);

    m_ffwdAction = toolbar->addAction(il.load("ffwd"), tr("Fast Forward"));
    m_ffwdAction->setShortcut(tr("PgDown"));
    m_ffwdAction->setStatusTip(tr("Fast-forward to the next time instant or time ruler notch"));
    connect(m_ffwdAction, SIGNAL(triggered()), this, SLOT(ffwd()));
    connect(this, SIGNAL(canFfwd(bool)), m_ffwdAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.FastForward", m_ffwdAction, context);

    m_ffwdSimilarAction = new QAction(tr("Fast Forward to Similar Point"), this);
    m_ffwdSimilarAction->setShortcut(tr("Shift+PgDown"));
    m_ffwdSimilarAction->setStatusTip(tr("Fast-forward to the next similarly valued time instant"));
    connect(m_ffwdSimilarAction, SIGNAL(triggered()), this, SLOT(ffwdSimilar()));
    connect(this, SIGNAL(canFfwd(bool)), m_ffwdSimilarAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.FastForwardSimilar", m_ffwdSimilarAction, context);

    m_ffwdEndAction = toolbar->addAction(il.load("ffwd-end"), tr("Fast Forward to End"));
    m_ffwdEndAction->setShortcut(tr("End"));
    m_ffwdEndAction->setStatusTip(tr("Fast-forward to the end"));
    connect(m_ffwdEndAction, SIGNAL(triggered()), this, SLOT(ffwdEnd()));
    connect(this, SIGNAL(canPlay(bool)), m_ffwdEndAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.FastForwardEnd", m_ffwdEndAction, context);

    toolbar = addToolBar(tr("Play Mode Toolbar"));

    m_playSelectionAction = toolbar->addAction(il.load("playselection"), tr("Constrain Playback to Selection"));
    m_playSelectionAction->setCheckable(true);
    m_playSelectionAction->setChecked(m_viewManager->getPlaySelectionMode());
    m_playSelectionAction->setShortcut(tr("Ctrl+Alt+S"));
    m_playSelectionAction->setStatusTip(tr("Constrain playback to the selected regions"));
    connect(m_viewManager, SIGNAL(playSelectionModeChanged(bool)),
            m_playSelectionAction, SLOT(setChecked(bool)));
    connect(m_playSelectionAction, SIGNAL(triggered()), this, SLOT(playSelectionToggled()));
    connect(this, SIGNAL(canPlaySelection(bool)), m_playSelectionAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.PlaySelection", m_playSelectionAction, context);

    m_playLoopAction = toolbar->addAction(il.load("playloop"), tr("Loop Playback"));
    m_playLoopAction->setCheckable(true);
    m_playLoopAction->setChecked(m_viewManager->getPlayLoopMode());
    m_playLoopAction->setShortcut(tr("Ctrl+Alt+L"));
    m_playLoopAction->setStatusTip(tr("Loop playback"));
    connect(m_viewManager, SIGNAL(playLoopModeChanged(bool)),
            m_playLoopAction, SLOT(setChecked(bool)));
    connect(m_playLoopAction, SIGNAL(triggered()), this, SLOT(playLoopToggled()));
    connect(this, SIGNAL(canPlay(bool)), m_playLoopAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.PlayLoop", m_playLoopAction, context);

    m_soloAction = toolbar->addAction(il.load("solo"), tr("Solo Current Pane"));
    m_soloAction->setCheckable(true);
    m_soloAction->setChecked(m_viewManager->getPlaySoloMode());
    m_prevSolo = m_viewManager->getPlaySoloMode();
    m_soloAction->setShortcut(tr("Ctrl+Alt+O"));
    m_soloAction->setStatusTip(tr("Solo the current pane during playback"));
    connect(m_viewManager, SIGNAL(playSoloModeChanged(bool)),
            m_soloAction, SLOT(setChecked(bool)));
    connect(m_soloAction, SIGNAL(triggered()), this, SLOT(playSoloToggled()));
    connect(this, SIGNAL(canChangeSolo(bool)), m_soloAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.PlaySolo", m_soloAction, context);

    QAction *alAction = 0;
    if (Document::canAlign()) {
        alAction = toolbar->addAction(il.load("align"), tr("Align File Timelines"));
        alAction->setCheckable(true);
        alAction->setChecked(m_viewManager->getAlignMode());
        alAction->setStatusTip(tr("Treat multiple audio files as versions of the same recording, and align their timelines"));
        connect(m_viewManager, SIGNAL(alignModeChanged(bool)),
                alAction, SLOT(setChecked(bool)));
        connect(alAction, SIGNAL(triggered()), this, SLOT(alignToggled()));
        connect(this, SIGNAL(canAlign(bool)), alAction, SLOT(setEnabled(bool)));
        ACTION_MANAGER->registerAction("Playback.Align", alAction, context);
    }

    m_keyReference->registerShortcut(m_playAction);
    m_keyReference->registerShortcut(m_playSelectionAction);
    m_keyReference->registerShortcut(m_playLoopAction);
    m_keyReference->registerShortcut(m_soloAction);
    if (alAction) m_keyReference->registerShortcut(alAction);
    m_keyReference->registerShortcut(m_rwdAction);
    m_keyReference->registerShortcut(m_ffwdAction);
    m_keyReference->registerShortcut(m_rwdSimilarAction);
    m_keyReference->registerShortcut(m_ffwdSimilarAction);
    m_keyReference->registerShortcut(m_rwdStartAction);
    m_keyReference->registerShortcut(m_ffwdEndAction);

    m_rightButtonPlaybackMenu->addAction(m_playAction);
    m_rightButtonPlaybackMenu->addAction(m_playSelectionAction);
    m_rightButtonPlaybackMenu->addAction(m_playLoopAction);
    m_rightButtonPlaybackMenu->addAction(m_soloAction);
    if (alAction) m_rightButtonPlaybackMenu->addAction(alAction);
    m_rightButtonPlaybackMenu->addSeparator();
    m_rightButtonPlaybackMenu->addAction(m_rwdAction);
    m_rightButtonPlaybackMenu->addAction(m_ffwdAction);
    m_rightButtonPlaybackMenu->addSeparator();
    m_rightButtonPlaybackMenu->addAction(m_rwdStartAction);
    m_rightButtonPlaybackMenu->addAction(m_ffwdEndAction);
    m_rightButtonPlaybackMenu->addSeparator();

    QAction *fastAction = new QAction(tr("Speed Up"), this);
    fastAction->setShortcut(tr("Ctrl+Alt+PgUp"));
    fastAction->setStatusTip(tr("Time-stretch playback to speed it up without changing pitch"));
    connect(fastAction, SIGNAL(triggered()), this, SLOT(speedUpPlayback()));
    connect(this, SIGNAL(canSpeedUpPlayback(bool)), fastAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.SpeedUp", fastAction, context);

    QAction *slowAction = new QAction(tr("Slow Down"), this);
    slowAction->setShortcut(tr("Ctrl+Alt+PgDown"));
    slowAction->setStatusTip(tr("Time-stretch playback to slow it down without changing pitch"));
    connect(slowAction, SIGNAL(triggered()), this, SLOT(slowDownPlayback()));
    connect(this, SIGNAL(canSlowDownPlayback(bool)), slowAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.SlowDown", slowAction, context);

    QAction *normalAction = new QAction(tr("Restore Normal Speed"), this);
    normalAction->setShortcut(tr("Ctrl+Alt+Home"));
    normalAction->setStatusTip(tr("Restore non-time-stretched playback"));
    connect(normalAction, SIGNAL(triggered()), this, SLOT(restoreNormalPlayback()));
    connect(this, SIGNAL(canChangePlaybackSpeed(bool)), normalAction, SLOT(setEnabled(bool)));
    ACTION_MANAGER->registerAction("Playback.RestoreNormalSpeed", normalAction, context);

    m_keyReference->registerShortcut(fastAction);
    m_keyReference->registerShortcut(slowAction);
    m_keyReference->registerShortcut(normalAction);

    m_rightButtonPlaybackMenu->addAction(fastAction);
    m_rightButtonPlaybackMenu->addAction(slowAction);
    m_rightButtonPlaybackMenu->addAction(normalAction);
}


// public slot
void SimpleVisualiserWidget::preferenceChanged(PropertyContainer::PropertyName name)
{
    VisualiserWindowBase::preferenceChanged(name);
    if (name == "Background Mode" && m_viewManager) {
        if (m_viewManager->getGlobalDarkBackground()) {
            m_panLayer->setBaseColour(ColourDatabase::getInstance()->getColourIndex(tr("Bright Green")));
        } else {
            m_panLayer->setBaseColour(ColourDatabase::getInstance()->getColourIndex(tr("Green")));
        }
    }
}

// public slot
bool SimpleVisualiserWidget::commitData(bool mayAskUser)
{
    return true;
}

// public slot
void SimpleVisualiserWidget::jumpToTime(const RealTime &time)
{
    if (!getMainModel()) return;
    sv_frame_t frame = RealTime::realTime2Frame(time, getMainModel()->getSampleRate());
    m_viewManager->setGlobalCentreFrame(frame);
    m_viewManager->setPlaybackFrame(frame);
}

// public slot
void SimpleVisualiserWidget::baseGlobalCentreFrameChanged(sv_frame_t frame)
{
    if (!getMainModel()) return;
    RealTime time = RealTime::frame2RealTime(frame, getMainModel()->getSampleRate());
    emit userScrolledToTime(time);
}

// public slot
void SimpleVisualiserWidget::basePlaybackFrameChanged(sv_frame_t frame)
{
    if (!getMainModel()) return;
    RealTime time = RealTime::frame2RealTime(frame, getMainModel()->getSampleRate());
    emit playbackScrolledToTime(time);
}

void SimpleVisualiserWidget::paneDropAccepted(Pane *, QStringList)
{
}

void SimpleVisualiserWidget::paneDropAccepted(Pane *, QString)
{
}

void SimpleVisualiserWidget::importAudio()
{

}

void SimpleVisualiserWidget::importMoreAudio()
{

}

void SimpleVisualiserWidget::replaceMainAudio()
{

}

void SimpleVisualiserWidget::openSomething()
{

}

void SimpleVisualiserWidget::openLocation()
{

}

void SimpleVisualiserWidget::exportAudio()
{

}

void SimpleVisualiserWidget::exportAudioData()
{

}

void SimpleVisualiserWidget::importLayer()
{

}

void SimpleVisualiserWidget::exportLayer()
{

}

void SimpleVisualiserWidget::exportImage()
{

}

void SimpleVisualiserWidget::saveSession()
{

}

void SimpleVisualiserWidget::saveSessionAs()
{

}

void SimpleVisualiserWidget::newSession()
{

}

void SimpleVisualiserWidget::closeSession()
{

}

void SimpleVisualiserWidget::sampleRateMismatch(sv_samplerate_t requested, sv_samplerate_t actual, bool willResample)
{
    if (!willResample) {
        QMessageBox::information
                (this, tr("Sample rate mismatch"),
                 tr("<b>Wrong sample rate</b><p>The sample rate of this audio file (%1 Hz) does not match\nthe current playback rate (%2 Hz).<p>The file will play at the wrong speed and pitch.<p>Change the <i>Resample mismatching files on import</i> option under <i>File</i> -> <i>Preferences</i> if you want to alter this behaviour.")
                 .arg(requested).arg(actual));
    }
    updateDescriptionLabel();
}

void SimpleVisualiserWidget::audioOverloadPluginDisabled()
{
    QMessageBox::information
            (this, tr("Audio processing overload"),
             tr("<b>Overloaded</b><p>Audio effects plugin auditioning has been disabled due to a processing overload."));
}

void SimpleVisualiserWidget::audioTimeStretchMultiChannelDisabled()
{
    static bool shownOnce = false;
    if (shownOnce) return;
    QMessageBox::information
            (this, tr("Audio processing overload"),
             tr("<b>Overloaded</b><p>Audio playback speed processing has been reduced to a single channel, due to a processing overload."));
    shownOnce = true;
}

void SimpleVisualiserWidget::toolNavigateSelected()
{

}

void SimpleVisualiserWidget::toolSelectSelected()
{

}

void SimpleVisualiserWidget::toolEditSelected()
{

}

void SimpleVisualiserWidget::toolDrawSelected()
{

}

void SimpleVisualiserWidget::toolEraseSelected()
{

}

void SimpleVisualiserWidget::toolMeasureSelected()
{

}

void SimpleVisualiserWidget::documentModified()
{
    //!!!
    VisualiserWindowBase::documentModified();
}

void SimpleVisualiserWidget::documentRestored()
{
    //!!!
    VisualiserWindowBase::documentRestored();
}

void SimpleVisualiserWidget::documentReplaced()
{
    if (m_document) {
        connect(m_document, SIGNAL(activity(QString)),
                m_activityLog, SLOT(activityHappened(QString)));
    }
}

void SimpleVisualiserWidget::updateMenuStates()
{

}

void SimpleVisualiserWidget::updateDescriptionLabel()
{
    if (!getMainModel()) {
        m_descriptionLabel->setText(tr("No audio file loaded."));
        return;
    }
    QString description;
    sv_samplerate_t ssr = getMainModel()->getSampleRate();
    sv_samplerate_t tsr = ssr;
    if (m_playSource) tsr = m_playSource->getTargetSampleRate();
    if (ssr != tsr) {
        description = tr("%1Hz (resampling to %2Hz)").arg(ssr).arg(tsr);
    } else {
        description = QString("%1Hz").arg(ssr);
    }
    description = QString("%1 - %2")
            .arg(RealTime::frame2RealTime(getMainModel()->getEndFrame(), ssr).toText(false).c_str())
            .arg(description);
    m_descriptionLabel->setText(description);
}

void SimpleVisualiserWidget::updateVisibleRangeDisplay(Pane *p) const
{
    if (!getMainModel() || !p) {
        return;
    }
    bool haveSelection = false;
    sv_frame_t startFrame = 0, endFrame = 0;
    if (m_viewManager && m_viewManager->haveInProgressSelection()) {
        bool exclusive = false;
        Selection s = m_viewManager->getInProgressSelection(exclusive);
        if (!s.isEmpty()) {
            haveSelection = true;
            startFrame = s.getStartFrame();
            endFrame = s.getEndFrame();
        }
    }
    if (!haveSelection) {
        startFrame = p->getFirstVisibleFrame();
        endFrame = p->getLastVisibleFrame();
    }
    RealTime start = RealTime::frame2RealTime(startFrame, getMainModel()->getSampleRate());
    RealTime end = RealTime::frame2RealTime(endFrame, getMainModel()->getSampleRate());
    RealTime duration = end - start;
    QString startStr, endStr, durationStr;
    startStr = start.toText(true).c_str();
    endStr = end.toText(true).c_str();
    durationStr = duration.toText(true).c_str();
    if (haveSelection) {
        m_myStatusMessage = tr("Selection: %1 to %2 (duration %3)")
                .arg(startStr).arg(endStr).arg(durationStr);
    } else {
        m_myStatusMessage = tr("Visible: %1 to %2 (duration %3)")
                .arg(startStr).arg(endStr).arg(durationStr);
    }
    if (getStatusLabel()->text() != m_myStatusMessage) {
        getStatusLabel()->setText(m_myStatusMessage);
    }
    updatePositionStatusDisplays();
}

void SimpleVisualiserWidget::updatePositionStatusDisplays() const
{
    if (!statusBar()->isVisible()) return;
    Pane *pane = 0;
    sv_frame_t frame = m_viewManager->getPlaybackFrame();
    if (m_paneStack) pane = m_paneStack->getCurrentPane();
    if (!pane) return;
    int layers = pane->getLayerCount();
    if (layers == 0) m_currentLabel->setText("");
    for (int i = layers-1; i >= 0; --i) {
        Layer *layer = pane->getLayer(i);
        if (!layer) continue;
        if (!layer->isLayerEditable()) continue;
        QString label = layer->getLabelPreceding
                (pane->alignFromReference(frame));
        m_currentLabel->setText(label);
        break;
    }
}

void SimpleVisualiserWidget::modelGenerationFailed(QString transformName, QString message)
{
    QString quoted;
    if (transformName != "") {
        quoted = QString("\"%1\" ").arg(transformName);
    }
    if (message != "") {
        QMessageBox::warning
                (this, tr("Failed to generate layer"),
                 tr("<b>Layer generation failed</b><p>Failed to generate derived layer.<p>The layer transform %1failed:<p>%2")
                 .arg(quoted).arg(message), QMessageBox::Ok);
    } else {
        QMessageBox::warning
                (this, tr("Failed to generate layer"),
                 tr("<b>Layer generation failed</b><p>Failed to generate a derived layer.<p>The layer transform %1failed.<p>No error information is available.")
                 .arg(quoted), QMessageBox::Ok);
    }
}

void SimpleVisualiserWidget::modelGenerationWarning(QString /* transformName */, QString message)
{
    QMessageBox::warning(this, tr("Warning"), message, QMessageBox::Ok);
}

void SimpleVisualiserWidget::modelRegenerationFailed(QString layerName, QString transformName, QString message)
{
    if (message != "") {
        QMessageBox::warning
                (this, tr("Failed to regenerate layer"),
                 tr("<b>Layer generation failed</b><p>Failed to regenerate derived layer \"%1\" using new data model as input.<p>The layer transform \"%2\" failed:<p>%3")
                 .arg(layerName).arg(transformName).arg(message), QMessageBox::Ok);
    } else {
        QMessageBox::warning
                (this, tr("Failed to regenerate layer"),
                 tr("<b>Layer generation failed</b><p>Failed to regenerate derived layer \"%1\" using new data model as input.<p>The layer transform \"%2\" failed.<p>No error information is available.")
                 .arg(layerName).arg(transformName), QMessageBox::Ok);
    }
}

void SimpleVisualiserWidget::modelRegenerationWarning(QString layerName, QString /* transformName */, QString message)
{
    QMessageBox::warning(
                this, tr("Warning"),
                tr("<b>Warning when regenerating layer</b><p>When regenerating the derived layer \"%1\" using new data model as input:<p>%2")
                .arg(layerName).arg(message), QMessageBox::Ok);
}

void SimpleVisualiserWidget::alignmentFailed(QString transformName, QString message)
{
    QMessageBox::warning(
                this, tr("Failed to calculate alignment"),
                tr("<b>Alignment calculation failed</b><p>Failed to calculate an audio alignment using transform \"%1\":<p>%2")
                .arg(transformName).arg(message), QMessageBox::Ok);
}

void SimpleVisualiserWidget::rightButtonMenuRequested(Pane *pane, QPoint position)
{
    // cerr << "VisualiserWidget::rightButtonMenuRequested(" << pane << ", " << position.x() << ", " << position.y() << ")" << endl;
    m_paneStack->setCurrentPane(pane);
    m_rightButtonMenu->popup(position);
}

void SimpleVisualiserWidget::propertyStacksResized(int)
{

}

void SimpleVisualiserWidget::paneAdded(Pane *pane)
{
    if (m_overview) m_overview->registerView(pane);
}

void SimpleVisualiserWidget::paneHidden(Pane *pane)
{
    if (m_overview) m_overview->unregisterView(pane);
}

void SimpleVisualiserWidget::paneAboutToBeDeleted(Pane *pane)
{
    if (m_overview) m_overview->unregisterView(pane);
}

void SimpleVisualiserWidget::playSoloToggled()
{
    VisualiserWindowBase::playSoloToggled();
    m_soloModified = true;
}

void SimpleVisualiserWidget::playSpeedChanged(int position)
{
    PlaySpeedRangeMapper mapper(60, 120);
    double percent = m_playSpeed->mappedValue();
    double factor = mapper.getFactorForValue(percent);
    //    cerr << "play speed position = " << position << " (range 0-120) percent = " << percent << " factor = " << factor << endl;
    int centre = m_playSpeed->defaultValue();
    // Percentage is shown to 0dp if >100, to 1dp if <100; factor is
    // shown to 3sf
    char pcbuf[30];
    char facbuf[30];
    if (position == centre) {
        contextHelpChanged(tr("Playback speed: Normal"));
    } else if (position < centre) {
        sprintf(pcbuf, "%.1f", percent);
        sprintf(facbuf, "%.3g", 1.0 / factor);
        contextHelpChanged(tr("Playback speed: %1% (%2x slower)")
                           .arg(pcbuf)
                           .arg(facbuf));
    } else {
        sprintf(pcbuf, "%.0f", percent);
        sprintf(facbuf, "%.3g", factor);
        contextHelpChanged(tr("Playback speed: %1% (%2x faster)")
                           .arg(pcbuf)
                           .arg(facbuf));
    }
    m_playSource->setTimeStretch(1.0 / factor); // factor is a speedup
    updateMenuStates();
}

void SimpleVisualiserWidget::speedUpPlayback()
{
    int value = m_playSpeed->value();
    value = value + m_playSpeed->pageStep();
    if (value > m_playSpeed->maximum()) value = m_playSpeed->maximum();
    m_playSpeed->setValue(value);
}

void SimpleVisualiserWidget::slowDownPlayback()
{
    int value = m_playSpeed->value();
    value = value - m_playSpeed->pageStep();
    if (value < m_playSpeed->minimum()) value = m_playSpeed->minimum();
    m_playSpeed->setValue(value);
}

void SimpleVisualiserWidget::restoreNormalPlayback()
{
    m_playSpeed->setValue(m_playSpeed->defaultValue());
}

void SimpleVisualiserWidget::currentPaneChanged(Pane *pane)
{
    VisualiserWindowBase::currentPaneChanged(pane);
    if (!pane || !m_panLayer) return;
    // If this pane contains the main model, it usually makes sense to show the main model in the
    // pan layer even if it isn't the top layer in the pane (e.g. if the top layer is one derived from
    // the main model).
    bool containsMainModel = false;
    for (int i = pane->getLayerCount(); i > 0; ) {
        --i;
        Layer *layer = pane->getLayer(i);
        if (layer &&
                LayerFactory::getInstance().getLayerType(layer) ==
                LayerFactory::Type("Waveform") &&
                layer->getModel() == getMainModel()) {
            containsMainModel = true;
            break;
        }
    }
    if (containsMainModel) {
        m_panLayer->setModel(getMainModel());
        return;
    }
    for (int i = pane->getLayerCount(); i > 0; ) {
        --i;
        Layer *layer = pane->getLayer(i);
        if (LayerFactory::getInstance().getLayerType(layer) == LayerFactory::Type("Waveform")) {
            RangeSummarisableTimeValueModel *tvm = dynamic_cast<RangeSummarisableTimeValueModel *>(layer->getModel());
            if (tvm) {
                m_panLayer->setModel(tvm);
                return;
            }
        }
    }
}

void SimpleVisualiserWidget::outputLevelsChanged(float left, float right)
{
    m_fader->setPeakLeft(left);
    m_fader->setPeakRight(right);
}

void SimpleVisualiserWidget::mainModelChanged(WaveFileModel *model)
{
    m_panLayer->setModel(model);
    VisualiserWindowBase::mainModelChanged(model);
    if (m_playTarget) {
        connect(m_fader, SIGNAL(valueChanged(float)),
                this, SLOT(mainModelGainChanged(float)));
    }
}

void SimpleVisualiserWidget::mainModelGainChanged(float gain)
{
    if (m_playTarget) {
        m_playTarget->setOutputGain(gain);
    }
}

void SimpleVisualiserWidget::modelAboutToBeDeleted(Model *model)
{
    if (model == m_panLayer->getModel()) {
        if (model == getMainModel()) {
            m_panLayer->setModel(0);
        } else {
            m_panLayer->setModel(getMainModel());
        }
    }
    VisualiserWindowBase::modelAboutToBeDeleted(model);
}

void SimpleVisualiserWidget::mouseEnteredWidget()
{
    QWidget *w = dynamic_cast<QWidget *>(sender());
    if (!w) return;
    if (w == m_fader) {
        contextHelpChanged(tr("Adjust the master playback level"));
    } else if (w == m_playSpeed) {
        contextHelpChanged(tr("Adjust the master playback speed"));
    }
}

void SimpleVisualiserWidget::mouseLeftWidget()
{
    contextHelpChanged("");
}

void SimpleVisualiserWidget::handleOSCMessage(const OSCMessage &)
{

}

void SimpleVisualiserWidget::midiEventsAvailable()
{

}

//void SimpleVisualiserWidget::addPane(const LayerConfiguration &configuration, QString text)
//{

//}

bool SimpleVisualiserWidget::checkSaveModified()
{
    return true;
}

void SimpleVisualiserWidget::exportAudio(bool asData)
{

}

