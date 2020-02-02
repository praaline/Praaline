/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "ViewManager.h"
#include "base/AudioPlaySource.h"
#include "PraalineCore/Base/RealTime.h"
#include "data/model/Model.h"
#include "widgets/CommandHistory.h"
#include "View.h"
#include "Overview.h"

#include <QSettings>
#include <QApplication>

#include <iostream>

//#define DEBUG_VIEW_MANAGER 1

ViewManager::ViewManager() :
    m_playSource(0),
    m_globalCentreFrame(0),
    m_globalZoom(1024),
    m_playbackFrame(0),
    m_playbackModel(0),
    m_mainModelSampleRate(0),
    m_lastLeft(0), 
    m_lastRight(0),
    m_inProgressExclusive(true),
    m_toolMode(NavigateMode),
    m_playLoopMode(false),
    m_playSelectionMode(false),
    m_playSoloMode(false),
    m_alignMode(false),
    m_overlayMode(StandardOverlays),
    m_zoomWheelsEnabled(true),
    m_showCentreLine(true),
    m_illuminateLocalFeatures(true),
    m_showWorkTitle(false),
    m_showDuration(true),
    m_lightPalette(QApplication::palette()),
    m_darkPalette(QApplication::palette())
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    m_overlayMode = OverlayMode
        (settings.value("overlay-mode", int(m_overlayMode)).toInt());

    if (m_overlayMode != NoOverlays &&
        m_overlayMode != StandardOverlays &&
        m_overlayMode != AllOverlays) {
        m_overlayMode = StandardOverlays;
    }

    m_zoomWheelsEnabled =
        settings.value("zoom-wheels-enabled", m_zoomWheelsEnabled).toBool();
    m_showCentreLine =
        settings.value("show-centre-line", m_showCentreLine).toBool();
    settings.endGroup();

    if (getGlobalDarkBackground()) {
/*
        cerr << "dark palette:" << endl;
        cerr << "window = " << QApplication::palette().color(QPalette::Window).name() << endl;
        cerr << "windowtext = " << QApplication::palette().color(QPalette::WindowText).name() << endl;
        cerr << "base = " << QApplication::palette().color(QPalette::Base).name() << endl;
        cerr << "alternatebase = " << QApplication::palette().color(QPalette::AlternateBase).name() << endl;
        cerr << "text = " << QApplication::palette().color(QPalette::Text).name() << endl;
        cerr << "button = " << QApplication::palette().color(QPalette::Button).name() << endl;
        cerr << "buttontext = " << QApplication::palette().color(QPalette::ButtonText).name() << endl;
        cerr << "brighttext = " << QApplication::palette().color(QPalette::BrightText).name() << endl;
        cerr << "light = " << QApplication::palette().color(QPalette::Light).name() << endl;
        cerr << "dark = " << QApplication::palette().color(QPalette::Dark).name() << endl;
        cerr << "mid = " << QApplication::palette().color(QPalette::Mid).name() << endl;
*/
        m_lightPalette = QPalette(QColor("#000000"),  // WindowText
                                  QColor("#dddfe4"),  // Button
                                  QColor("#ffffff"),  // Light
                                  QColor("#555555"),  // Dark
                                  QColor("#c7c7c7"),  // Mid
                                  QColor("#000000"),  // Text
                                  QColor("#ffffff"),  // BrightText
                                  QColor("#ffffff"),  // Base
                                  QColor("#efefef")); // Window
                                  

    } else {
/*
        cerr << "light palette:" << endl;
        cerr << "window = " << QApplication::palette().color(QPalette::Window).name() << endl;
        cerr << "windowtext = " << QApplication::palette().color(QPalette::WindowText).name() << endl;
        cerr << "base = " << QApplication::palette().color(QPalette::Base).name() << endl;
        cerr << "alternatebase = " << QApplication::palette().color(QPalette::AlternateBase).name() << endl;
        cerr << "text = " << QApplication::palette().color(QPalette::Text).name() << endl;
        cerr << "button = " << QApplication::palette().color(QPalette::Button).name() << endl;
        cerr << "buttontext = " << QApplication::palette().color(QPalette::ButtonText).name() << endl;
        cerr << "brighttext = " << QApplication::palette().color(QPalette::BrightText).name() << endl;
        cerr << "light = " << QApplication::palette().color(QPalette::Light).name() << endl;
        cerr << "dark = " << QApplication::palette().color(QPalette::Dark).name() << endl;
        cerr << "mid = " << QApplication::palette().color(QPalette::Mid).name() << endl;
*/
        m_darkPalette = QPalette(QColor("#ffffff"),  // WindowText
                                 QColor("#3e3e3e"),  // Button
                                 QColor("#808080"),  // Light
                                 QColor("#1e1e1e"),  // Dark
                                 QColor("#404040"),  // Mid
                                 QColor("#ffffff"),  // Text
                                 QColor("#ffffff"),  // BrightText
                                 QColor("#000000"),  // Base
                                 QColor("#202020")); // Window
    }
}

ViewManager::~ViewManager()
{
}

sv_frame_t
ViewManager::getGlobalCentreFrame() const
{
#ifdef DEBUG_VIEW_MANAGER
    cerr << "ViewManager::getGlobalCentreFrame: returning " << m_globalCentreFrame << endl;
#endif
    return m_globalCentreFrame;
}

void
ViewManager::setGlobalCentreFrame(sv_frame_t f)
{
#ifdef DEBUG_VIEW_MANAGER
    cerr << "ViewManager::setGlobalCentreFrame to " << f << endl;
#endif
    m_globalCentreFrame = f;
    emit globalCentreFrameChanged(f);
}

int
ViewManager::getGlobalZoom() const
{
#ifdef DEBUG_VIEW_MANAGER
    cerr << "ViewManager::getGlobalZoom: returning " << m_globalZoom << endl;
#endif
    return m_globalZoom;
}

sv_frame_t
ViewManager::getPlaybackFrame() const
{
    if (m_playSource && m_playSource->isPlaying()) {
	m_playbackFrame = m_playSource->getCurrentPlayingFrame();
    }
    return m_playbackFrame;
}

void
ViewManager::setPlaybackFrame(sv_frame_t f)
{
    if (m_playbackFrame != f) {
	m_playbackFrame = f;
	emit playbackFrameChanged(f);
	if (m_playSource && m_playSource->isPlaying()) {
	    m_playSource->play(f);
	}
    }
}

Model *
ViewManager::getPlaybackModel() const
{
    return m_playbackModel;
}

void
ViewManager::setPlaybackModel(Model *model)
{
    m_playbackModel = model;
}

sv_frame_t
ViewManager::alignPlaybackFrameToReference(sv_frame_t frame) const
{
#ifdef DEBUG_VIEW_MANAGER
    cerr << "ViewManager::alignPlaybackFrameToReference(" << frame << "): playback model is " << m_playbackModel << endl;
#endif
    if (!m_playbackModel) {
        return frame;
    } else {
        sv_frame_t f = m_playbackModel->alignToReference(frame);
#ifdef DEBUG_VIEW_MANAGER
        cerr << "aligned frame = " << f << endl;
#endif
        return f;
    }
}

sv_frame_t
ViewManager::alignReferenceToPlaybackFrame(sv_frame_t frame) const
{
#ifdef DEBUG_VIEW_MANAGER
    cerr << "ViewManager::alignReferenceToPlaybackFrame(" << frame << "): playback model is " << m_playbackModel << endl;
#endif
    if (!m_playbackModel) {
        return frame;
    } else {
        sv_frame_t f = m_playbackModel->alignFromReference(frame);
#ifdef DEBUG_VIEW_MANAGER
        cerr << "aligned frame = " << f << endl;
#endif
        return f;
    }
}

bool
ViewManager::haveInProgressSelection() const
{
    return !m_inProgressSelection.isEmpty();
}

const Selection &
ViewManager::getInProgressSelection(bool &exclusive) const
{
    exclusive = m_inProgressExclusive;
    return m_inProgressSelection;
}

void
ViewManager::setInProgressSelection(const Selection &selection, bool exclusive)
{
    m_inProgressExclusive = exclusive;
    m_inProgressSelection = selection;
    if (exclusive) clearSelections();
    emit inProgressSelectionChanged();
}

void
ViewManager::clearInProgressSelection()
{
    m_inProgressSelection = Selection();
    emit inProgressSelectionChanged();
}

const MultiSelection &
ViewManager::getSelection() const
{
    return m_selections;
}

const MultiSelection::SelectionList &
ViewManager::getSelections() const
{
    return m_selections.getSelections();
}

void
ViewManager::setSelection(const Selection &selection)
{
    MultiSelection ms(m_selections);
    ms.setSelection(selection);
    setSelections(ms);
}

void
ViewManager::addSelection(const Selection &selection)
{
    MultiSelection ms(m_selections);
    ms.addSelection(selection);
    setSelections(ms);
}

void
ViewManager::addSelectionQuietly(const Selection &selection)
{
    MultiSelection ms(m_selections);
    ms.addSelection(selection);
    setSelections(ms, true);
}

void
ViewManager::removeSelection(const Selection &selection)
{
    MultiSelection ms(m_selections);
    ms.removeSelection(selection);
    setSelections(ms);
}

void
ViewManager::clearSelections()
{
    MultiSelection ms(m_selections);
    ms.clearSelections();
    setSelections(ms);
}

void
ViewManager::setSelections(const MultiSelection &ms, bool quietly)
{
    if (m_selections.getSelections() == ms.getSelections()) return;
    SetSelectionCommand *command = new SetSelectionCommand(this, ms);
    CommandHistory::getInstance()->addCommand(command);
    if (!quietly) {
        emit selectionChangedByUser();
    }
}

sv_frame_t
ViewManager::constrainFrameToSelection(sv_frame_t frame) const
{
    MultiSelection::SelectionList sl = getSelections();
    if (sl.empty()) return frame;

    for (MultiSelection::SelectionList::const_iterator i = sl.begin();
         i != sl.end(); ++i) {

        if (frame < i->getEndFrame()) {
            if (frame < i->getStartFrame()) {
                return i->getStartFrame();
            } else {
                return frame;
            }
        }
    }

    return sl.begin()->getStartFrame();
}

void
ViewManager::signalSelectionChange()
{
    emit selectionChanged();
}

ViewManager::SetSelectionCommand::SetSelectionCommand(ViewManager *vm,
						      const MultiSelection &ms) :
    m_vm(vm),
    m_oldSelection(vm->m_selections),
    m_newSelection(ms)
{
}

ViewManager::SetSelectionCommand::~SetSelectionCommand() { }

void
ViewManager::SetSelectionCommand::execute()
{
    m_vm->m_selections = m_newSelection;
    m_vm->signalSelectionChange();
}

void
ViewManager::SetSelectionCommand::unexecute()
{
    m_vm->m_selections = m_oldSelection;
    m_vm->signalSelectionChange();
}

QString
ViewManager::SetSelectionCommand::getName() const
{
    if (m_newSelection.getSelections().empty()) return tr("Clear Selection");
    if (m_newSelection.getSelections().size() > 1) return tr("Select Multiple Regions");
    else return tr("Select Region");
}

Selection
ViewManager::getContainingSelection(sv_frame_t frame, bool defaultToFollowing) const
{
    return m_selections.getContainingSelection(frame, defaultToFollowing);
}

void
ViewManager::setToolMode(ToolMode mode)
{
    m_toolMode = mode;

    emit toolModeChanged();

    switch (mode) {
    case NavigateMode: emit activity(tr("Enter Navigate mode")); break;
    case SelectMode: emit activity(tr("Enter Select mode")); break;
    case EditMode: emit activity(tr("Enter Edit mode")); break;
    case DrawMode: emit activity(tr("Enter Draw mode")); break;
    case EraseMode: emit activity(tr("Enter Erase mode")); break;
    case MeasureMode: emit activity(tr("Enter Measure mode")); break;
    case NoteEditMode: emit activity(tr("Enter NoteEdit mode")); break; // GF: NoteEditMode activity (I'm not yet certain why we need to emit this.)
    };
}

ViewManager::ToolMode
ViewManager::getToolModeFor(const View *v) const
{
    if (m_toolModeOverrides.find(v) == m_toolModeOverrides.end()) {
        return getToolMode();
    } else {
        return m_toolModeOverrides.find(v)->second;
    }
}

void
ViewManager::setToolModeFor(const View *v, ToolMode mode)
{
    m_toolModeOverrides[v] = mode;
}

void
ViewManager::clearToolModeOverrides()
{
    m_toolModeOverrides.clear();
}

void
ViewManager::setPlayLoopMode(bool mode)
{
    if (m_playLoopMode != mode) {

        m_playLoopMode = mode;

        emit playLoopModeChanged();
        emit playLoopModeChanged(mode);

        if (mode) emit activity(tr("Switch on Loop mode"));
        else emit activity(tr("Switch off Loop mode"));
    }
}

void
ViewManager::setPlaySelectionMode(bool mode)
{
    if (m_playSelectionMode != mode) {

        m_playSelectionMode = mode;

        emit playSelectionModeChanged();
        emit playSelectionModeChanged(mode);

        if (mode) emit activity(tr("Switch on Play Selection mode"));
        else emit activity(tr("Switch off Play Selection mode"));
    }
}

void
ViewManager::setPlaySoloMode(bool mode)
{
    if (m_playSoloMode != mode) {

        m_playSoloMode = mode;

        emit playSoloModeChanged();
        emit playSoloModeChanged(mode);

        if (mode) emit activity(tr("Switch on Play Solo mode"));
        else emit activity(tr("Switch off Play Solo mode"));
    }
}

void
ViewManager::setAlignMode(bool mode)
{
    if (m_alignMode != mode) {

        m_alignMode = mode;

        emit alignModeChanged();
        emit alignModeChanged(mode);

        if (mode) emit activity(tr("Switch on Alignment mode"));
        else emit activity(tr("Switch off Alignment mode"));
    }
}

sv_samplerate_t 
ViewManager::getPlaybackSampleRate() const
{
    if (m_playSource) {
        return m_playSource->getSourceSampleRate();
    }
    return 0;
}

sv_samplerate_t
ViewManager::getOutputSampleRate() const
{
    if (m_playSource) {
	return m_playSource->getTargetSampleRate();
    }
    return 0;
}

void
ViewManager::setAudioPlaySource(AudioPlaySource *source)
{
    if (!m_playSource) {
	QTimer::singleShot(100, this, SLOT(checkPlayStatus()));
    }
    m_playSource = source;
}

void
ViewManager::playStatusChanged(bool /* playing */)
{
#ifdef DEBUG_VIEW_MANAGER
    cerr << "ViewManager::playStatusChanged" << endl;
#endif
    checkPlayStatus();
}

void
ViewManager::checkPlayStatus()
{
    if (m_playSource && m_playSource->isPlaying()) {

	float left = 0, right = 0;
	if (m_playSource->getOutputLevels(left, right)) {
	    if (left != m_lastLeft || right != m_lastRight) {
		emit outputLevelsChanged(left, right);
		m_lastLeft = left;
		m_lastRight = right;
	    }
	}

	m_playbackFrame = m_playSource->getCurrentPlayingFrame();

#ifdef DEBUG_VIEW_MANAGER
	cerr << "ViewManager::checkPlayStatus: Playing, frame " << m_playbackFrame << ", levels " << m_lastLeft << "," << m_lastRight << endl;
#endif

	emit playbackFrameChanged(m_playbackFrame);

	QTimer::singleShot(20, this, SLOT(checkPlayStatus()));

    } else {

	if (m_lastLeft != 0.0 || m_lastRight != 0.0) {
	    emit outputLevelsChanged(0.0, 0.0);
	    m_lastLeft = 0.0;
	    m_lastRight = 0.0;
	}

#ifdef DEBUG_VIEW_MANAGER
	cerr << "ViewManager::checkPlayStatus: Not playing" << endl;
#endif
    }
}

bool
ViewManager::isPlaying() const
{
    return m_playSource && m_playSource->isPlaying();
}

void
ViewManager::viewCentreFrameChanged(sv_frame_t f, bool locked,
                                    PlaybackFollowMode mode)
{
    View *v = dynamic_cast<View *>(sender());

#ifdef DEBUG_VIEW_MANAGER
    cerr << "ViewManager::viewCentreFrameChanged(" << f << ", " << locked << ", " << mode << "), view is " << v << endl;
#endif

    if (locked) {
        m_globalCentreFrame = f;
        emit globalCentreFrameChanged(f);
    } else {
        if (v) emit viewCentreFrameChanged(v, f);
    }

    if (!dynamic_cast<Overview *>(v) || (mode != PlaybackIgnore)) {
        if (m_mainModelSampleRate != 0) {
            emit activity(tr("Scroll to %1")
                          .arg(RealTime::frame2RealTime
                               (f, m_mainModelSampleRate).toText().c_str()));
        }
    }

    if (mode == PlaybackScrollPageWithCentre ||
        mode == PlaybackScrollContinuous) {
        seek(f);
    }
}

void
ViewManager::seek(sv_frame_t f)
{
#ifdef DEBUG_VIEW_MANAGER 
    cerr << "ViewManager::seek(" << f << ")" << endl;
#endif

    if (m_playSource && m_playSource->isPlaying()) {
	sv_frame_t playFrame = m_playSource->getCurrentPlayingFrame();
	sv_frame_t diff = std::max(f, playFrame) - std::min(f, playFrame);
	if (diff > 20000) {
	    m_playbackFrame = f;
	    m_playSource->play(f);
#ifdef DEBUG_VIEW_MANAGER 
	    cerr << "ViewManager::considerSeek: reseeking from " << playFrame << " to " << f << endl;
#endif
            emit playbackFrameChanged(f);
	}
    } else {
        if (m_playbackFrame != f) {
            m_playbackFrame = f;
            emit playbackFrameChanged(f);
        }
    }
}

void
ViewManager::viewZoomLevelChanged(int z, bool locked)
{
    View *v = dynamic_cast<View *>(sender());

    if (!v) {
        cerr << "ViewManager::viewZoomLevelChanged: WARNING: sender is not a view" << endl;
        return;
    }

//!!!    emit zoomLevelChanged();
    
    if (locked) {
	m_globalZoom = z;
    }

#ifdef DEBUG_VIEW_MANAGER 
    cerr << "ViewManager::viewZoomLevelChanged(" << v << ", " << z << ", " << locked << ")" << endl;
#endif

    emit viewZoomLevelChanged(v, z, locked);

    if (!dynamic_cast<Overview *>(v)) {
        emit activity(tr("Zoom to %n sample(s) per pixel", "", z));
    }
}

void
ViewManager::setOverlayMode(OverlayMode mode)
{
    if (m_overlayMode != mode) {
        m_overlayMode = mode;
        emit overlayModeChanged();
        emit activity(tr("Change overlay level"));
    }

    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("overlay-mode", int(m_overlayMode));
    settings.endGroup();
}

void
ViewManager::setZoomWheelsEnabled(bool enabled)
{
    if (m_zoomWheelsEnabled != enabled) {
        m_zoomWheelsEnabled = enabled;
        emit zoomWheelsEnabledChanged();
        if (enabled) emit activity("Show zoom wheels");
        else emit activity("Hide zoom wheels");
    }

    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("zoom-wheels-enabled", m_zoomWheelsEnabled);
    settings.endGroup();
}

void
ViewManager::setShowCentreLine(bool show)
{
    if (m_showCentreLine != show) {
        m_showCentreLine = show;
        emit showCentreLineChanged();
        if (show) emit activity("Show centre line");
        else emit activity("Hide centre line");
    }

    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("show-centre-line", int(m_showCentreLine));
    settings.endGroup();
}

void
ViewManager::setGlobalDarkBackground(bool dark)
{
    // also save the current palette, in case the user has changed it
    // since construction
    if (getGlobalDarkBackground()) {
        m_darkPalette = QApplication::palette();
    } else {
        m_lightPalette = QApplication::palette();
    }

#ifndef Q_OS_MAC
    if (dark) {
        QApplication::setPalette(m_darkPalette);
    } else {
        QApplication::setPalette(m_lightPalette);
    }
#endif
}

bool
ViewManager::getGlobalDarkBackground() const
{
    bool dark = false;
    QColor windowBg = QApplication::palette().color(QPalette::Window);
    if (windowBg.red() + windowBg.green() + windowBg.blue() < 384) {
        dark = true;
    }
    return dark;
}

int
ViewManager::scalePixelSize(int pixels)
{
    static double ratio = 0.0;
    if (ratio == 0.0) {
        double baseEm;
#ifdef Q_OS_MAC
        baseEm = 17.0;
#else
        baseEm = 15.0;
#endif
        double em = QFontMetrics(QFont()).height();
        ratio = em / baseEm;
    }

    int scaled = int(pixels * ratio + 0.5);
//    cerr << "scaledSize: " << pixels << " -> " << scaled << " at ratio " << ratio << endl;
    if (pixels != 0 && scaled == 0) scaled = 1;
    return scaled;
}
