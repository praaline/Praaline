/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "VisualiserWindowBase.h"
#include "Document.h"


#include "view/Pane.h"
#include "view/PaneStack.h"
#include "view/ViewManager.h"
#include "data/model/WaveFileModel.h"
#include "data/model/SparseOneDimensionalModel.h"
#include "data/model/NoteModel.h"
#include "data/model/FlexiNoteModel.h"
#include "data/model/Labeller.h"
#include "data/model/TabularModel.h"

#include "layer/WaveformLayer.h"
#include "layer/TimeRulerLayer.h"
#include "layer/TimeInstantLayer.h"
#include "layer/TimeValueLayer.h"
#include "layer/Colour3DPlotLayer.h"
#include "layer/SliceLayer.h"
#include "layer/SliceableLayer.h"
#include "layer/ImageLayer.h"
#include "layer/NoteLayer.h"
#include "layer/FlexiNoteLayer.h"
#include "layer/RegionLayer.h"

#include "widgets/ListInputDialog.h"
#include "widgets/CommandHistory.h"
#include "widgets/ProgressDialog.h"
#include "widgets/MIDIFileImportDialog.h"
#include "widgets/CSVFormatDialog.h"
#include "widgets/ModelDataTableDialog.h"
#include "widgets/InteractiveFileFinder.h"

#include "audioio/AudioCallbackPlaySource.h"
#include "audioio/AudioCallbackPlayTarget.h"
#include "audioio/AudioTargetFactory.h"
#include "audioio/PlaySpeedRangeMapper.h"
#include "data/fileio/DataFileReaderFactory.h"
#include "data/fileio/PlaylistFileReader.h"
#include "data/fileio/WavFileWriter.h"
#include "data/fileio/MIDIFileWriter.h"
#include "data/fileio/BZipFileDevice.h"
#include "data/fileio/FileSource.h"
#include "data/fileio/AudioFileReaderFactory.h"
#include "rdf/RDFImporter.h"

#include "data/fft/FFTDataServer.h"

#include "base/RecentFiles.h"

#include "base/PlayParameterRepository.h"
#include "base/XmlExportable.h"
#include "base/Profiler.h"
#include "base/Preferences.h"
#include "base/TempWriteFile.h"
#include "base/Exceptions.h"
#include "base/ResourceFinder.h"

#include "data/osc/OSCQueue.h"
#include "data/midi/MIDIInput.h"

#include <QApplication>
#include <QMessageBox>
#include <QGridLayout>
#include <QLabel>
#include <QAction>
#include <QToolBar>
#include <QInputDialog>
#include <QStatusBar>
#include <QTreeView>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>
#include <QProcess>
#include <QShortcut>
#include <QSettings>
#include <QDateTime>
#include <QProcess>
#include <QCheckBox>
#include <QRegExp>
#include <QScrollArea>
#include <QDesktopWidget>
#include <QSignalMapper>

#include <iostream>
#include <cstdio>
#include <errno.h>

using std::vector;
using std::map;
using std::set;

#ifdef Q_WS_X11
#define Window X11Window
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>

static int handle_x11_error(Display *dpy, XErrorEvent *err)
{
    char errstr[256];
    XGetErrorText(dpy, err->error_code, errstr, 256);
    if (err->error_code != BadWindow) {
        cerr << "Sonic Visualiser: X Error: "
             << errstr << " " << int(err->error_code)
             << "\nin major opcode:  "
             << int(err->request_code) << endl;
    }
    return 0;
}
#undef Window
#endif

VisualiserWindowBase::VisualiserWindowBase(bool withAudioOutput, bool withMIDIInput, QWidget *parent) :
    QMainWindow(parent),
    m_document(0),
    m_paneStack(0),
    m_viewManager(0),
    m_timeRulerLayer(0),
    m_audioOutput(withAudioOutput),
    m_playSource(0),
    m_playTarget(0),
    m_oscQueue(0),
    m_oscQueueStarter(0),
    m_midiInput(0),
    m_recentFiles("RecentFiles", 20),
    m_recentTransforms("RecentTransforms", 20),
    m_documentModified(false),
    m_openingAudioFile(false),
    m_abandoning(false),
    m_labeller(0),
    m_lastPlayStatusSec(0),
    m_initialDarkBackground(false),
    m_defaultFfwdRwdStep(2, 0),
    m_statusLabel(0)
{
    //Profiler profiler("MainWindowBase::MainWindowBase");

    qRegisterMetaType<sv_frame_t>("sv_frame_t");
    qRegisterMetaType<sv_samplerate_t>("sv_samplerate_t");

#ifdef Q_WS_X11
    XSetErrorHandler(handle_x11_error);
#endif

    connect(CommandHistory::getInstance(), SIGNAL(commandExecuted()),
            this, SLOT(documentModified()));
    connect(CommandHistory::getInstance(), SIGNAL(documentRestored()),
            this, SLOT(documentRestored()));
    
    m_viewManager = new ViewManager();
    connect(m_viewManager, SIGNAL(selectionChanged()),
            this, SLOT(updateMenuStates()));
    connect(m_viewManager, SIGNAL(inProgressSelectionChanged()),
            this, SLOT(inProgressSelectionChanged()));

    // set a sensible default font size for views -- cannot do this
    // in Preferences, which is in base and not supposed to use QtGui
    int viewFontSize = int(QApplication::font().pointSize() * 0.9);
    QSettings settings;
    settings.beginGroup("Preferences");
    viewFontSize = settings.value("view-font-size", viewFontSize).toInt();
    settings.setValue("view-font-size", viewFontSize);
    settings.endGroup();

    Preferences::BackgroundMode mode =
            Preferences::getInstance()->getBackgroundMode();
    m_initialDarkBackground = m_viewManager->getGlobalDarkBackground();
    if (mode != Preferences::BackgroundFromTheme) {
        m_viewManager->setGlobalDarkBackground
                (mode == Preferences::DarkBackground);
    }

    m_paneStack = new PaneStack(0, m_viewManager);
    connect(m_paneStack, SIGNAL(currentPaneChanged(Pane *)),
            this, SLOT(currentPaneChanged(Pane *)));
    connect(m_paneStack, SIGNAL(currentLayerChanged(Pane *, Layer *)),
            this, SLOT(currentLayerChanged(Pane *, Layer *)));
    connect(m_paneStack, SIGNAL(rightButtonMenuRequested(Pane *, QPoint)),
            this, SLOT(rightButtonMenuRequested(Pane *, QPoint)));
    connect(m_paneStack, SIGNAL(contextHelpChanged(const QString &)),
            this, SLOT(contextHelpChanged(const QString &)));
    connect(m_paneStack, SIGNAL(paneAdded(Pane *)),
            this, SLOT(paneAdded(Pane *)));
    connect(m_paneStack, SIGNAL(paneHidden(Pane *)),
            this, SLOT(paneHidden(Pane *)));
    connect(m_paneStack, SIGNAL(paneAboutToBeDeleted(Pane *)),
            this, SLOT(paneAboutToBeDeleted(Pane *)));
    connect(m_paneStack, SIGNAL(dropAccepted(Pane *, QStringList)),
            this, SLOT(paneDropAccepted(Pane *, QStringList)));
    connect(m_paneStack, SIGNAL(dropAccepted(Pane *, QString)),
            this, SLOT(paneDropAccepted(Pane *, QString)));
    connect(m_paneStack, SIGNAL(paneDeleteButtonClicked(Pane *)),
            this, SLOT(paneDeleteButtonClicked(Pane *)));

    m_playSource = new AudioCallbackPlaySource(m_viewManager,
                                               QApplication::applicationName());

    connect(m_playSource, SIGNAL(sampleRateMismatch(sv_samplerate_t, sv_samplerate_t, bool)),
            this,           SLOT(sampleRateMismatch(sv_samplerate_t, sv_samplerate_t, bool)));
    connect(m_playSource, SIGNAL(audioOverloadPluginDisabled()),
            this,           SLOT(audioOverloadPluginDisabled()));
    connect(m_playSource, SIGNAL(audioTimeStretchMultiChannelDisabled()),
            this,           SLOT(audioTimeStretchMultiChannelDisabled()));

    connect(m_viewManager, SIGNAL(outputLevelsChanged(float, float)),
            this, SLOT(outputLevelsChanged(float, float)));

    connect(m_viewManager, SIGNAL(playbackFrameChanged(sv_frame_t)),
            this, SLOT(playbackFrameChanged(sv_frame_t)));

    connect(m_viewManager, SIGNAL(globalCentreFrameChanged(sv_frame_t)),
            this, SLOT(globalCentreFrameChanged(sv_frame_t)));

    connect(m_viewManager, SIGNAL(viewCentreFrameChanged(View *, sv_frame_t)),
            this, SLOT(viewCentreFrameChanged(View *, sv_frame_t)));

    connect(m_viewManager, SIGNAL(viewZoomLevelChanged(View *, int, bool)),
            this, SLOT(viewZoomLevelChanged(View *, int, bool)));

    connect(Preferences::getInstance(),
            SIGNAL(propertyChanged(PropertyContainer::PropertyName)),
            this,
            SLOT(preferenceChanged(PropertyContainer::PropertyName)));

    Labeller::ValueType labellerType = Labeller::ValueFromTwoLevelCounter;
    settings.beginGroup("MainWindow");

    labellerType = (Labeller::ValueType)
            settings.value("labellertype", (int)labellerType).toInt();
    int cycle = settings.value("labellercycle", 4).toInt();

    settings.endGroup();

    m_labeller = new Labeller(labellerType);
    m_labeller->setCounterCycleSize(cycle);

    if (withMIDIInput) {
        m_midiInput = new MIDIInput(QApplication::applicationName(), this);
    }
}

VisualiserWindowBase::~VisualiserWindowBase()
{
    cerr << "MainWindowBase::~MainWindowBase" << endl;
    if (m_playTarget) m_playTarget->shutdown();
    //    delete m_playTarget;
    delete m_playSource;
    delete m_viewManager;
    delete m_oscQueue;
    delete m_oscQueueStarter;
    delete m_midiInput;
    Profiles::getInstance()->dump();
}

void VisualiserWindowBase::menuActionMapperInvoked(QObject *o)
{
    QAction *a = qobject_cast<QAction *>(o);
    if (a && a->isEnabled()) {
        a->trigger();
    }
}

void VisualiserWindowBase::resizeConstrained(QSize size)
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect available = desktop->availableGeometry();
    QSize actual(std::min(size.width(), available.width()),
                 std::min(size.height(), available.height()));
    resize(actual);
}

void VisualiserWindowBase::startOSCQueue()
{
    m_oscQueueStarter = new OSCQueueStarter(this);
    connect(m_oscQueueStarter, SIGNAL(finished()), this, SLOT(oscReady()));
    m_oscQueueStarter->start();
}

void VisualiserWindowBase::oscReady()
{
    if (m_oscQueue && m_oscQueue->isOK()) {
        connect(m_oscQueue, SIGNAL(messagesAvailable()), this, SLOT(pollOSC()));
        QTimer *oscTimer = new QTimer(this);
        connect(oscTimer, SIGNAL(timeout()), this, SLOT(pollOSC()));
        oscTimer->start(1000);
        cerr << "Finished setting up OSC interface" << endl;
    }
}

QString VisualiserWindowBase::getOpenFileName(FileFinder::FileType type)
{
    FileFinder *ff = FileFinder::getInstance();

    if (type == FileFinder::AnyFile) {
        if (getMainModel() != 0 &&
                m_paneStack != 0 &&
                m_paneStack->getCurrentPane() != 0) { // can import a layer
            return ff->getOpenFileName(FileFinder::AnyFile, m_sessionFile);
        } else {
            return ff->getOpenFileName(FileFinder::SessionOrAudioFile,
                                       m_sessionFile);
        }
    }

    QString lastPath = m_sessionFile;

    if (type == FileFinder::AudioFile) {
        lastPath = m_audioFile;
    }

    return ff->getOpenFileName(type, lastPath);
}

QString VisualiserWindowBase::getSaveFileName(FileFinder::FileType type)
{
    QString lastPath = m_sessionFile;

    if (type == FileFinder::AudioFile) {
        lastPath = m_audioFile;
    }

    FileFinder *ff = FileFinder::getInstance();
    return ff->getSaveFileName(type, lastPath);
}

void VisualiserWindowBase::registerLastOpenedFilePath(FileFinder::FileType type, QString path)
{
    FileFinder *ff = FileFinder::getInstance();
    ff->registerLastOpenedFilePath(type, path);
}

QString VisualiserWindowBase::getDefaultSessionTemplate() const
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    QString templateName = settings.value("sessiontemplate", "").toString();
    if (templateName == "") templateName = "default";
    return templateName;
}

void VisualiserWindowBase::setDefaultSessionTemplate(QString n)
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("sessiontemplate", n);
}    

void VisualiserWindowBase::updateMenuStates()
{
    Pane *currentPane = 0;
    Layer *currentLayer = 0;

    if (m_paneStack) currentPane = m_paneStack->getCurrentPane();
    if (currentPane) currentLayer = currentPane->getSelectedLayer();

    bool havePrevPane = false, haveNextPane = false;
    bool havePrevLayer = false, haveNextLayer = false;

    if (currentPane) {
        for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {
            if (m_paneStack->getPane(i) == currentPane) {
                if (i > 0) havePrevPane = true;
                if (i < m_paneStack->getPaneCount()-1) haveNextPane = true;
                break;
            }
        }
        // the prev/next layer commands actually include the pane
        // itself as one of the selectables -- so we always have a
        // prev and next layer, as long as we have a pane with at
        // least one layer in it
        if (currentPane->getLayerCount() > 0) {
            havePrevLayer = true;
            haveNextLayer = true;
        }
    }

    bool haveCurrentPane = (currentPane != 0);
    bool haveCurrentLayer = (haveCurrentPane && (currentLayer != 0));
    bool haveMainModel = (getMainModel() != 0);
    bool havePlayTarget = (m_playTarget != 0);
    bool haveSelection = (m_viewManager && !m_viewManager->getSelections().empty());
    bool haveCurrentEditableLayer = (haveCurrentLayer && currentLayer->isLayerEditable());
    bool haveCurrentTimeInstantsLayer = (haveCurrentLayer && dynamic_cast<TimeInstantLayer *>(currentLayer));
    bool haveCurrentDurationLayer = (haveCurrentLayer &&
                                     (dynamic_cast<NoteLayer *>(currentLayer) ||
                                      dynamic_cast<FlexiNoteLayer *>(currentLayer) ||
                                      dynamic_cast<RegionLayer *>(currentLayer)));
    bool haveCurrentColour3DPlot = (haveCurrentLayer && dynamic_cast<Colour3DPlotLayer *>(currentLayer));
    bool haveClipboardContents = (m_viewManager && !m_viewManager->getClipboard().empty());
    bool haveTabularLayer = (haveCurrentLayer && dynamic_cast<TabularModel *>(currentLayer->getModel()));

    emit canAddPane(haveMainModel);
    emit canDeleteCurrentPane(haveCurrentPane);
    emit canZoom(haveMainModel && haveCurrentPane);
    emit canScroll(haveMainModel && haveCurrentPane);
    emit canAddLayer(haveMainModel && haveCurrentPane);
    emit canImportMoreAudio(haveMainModel);
    emit canReplaceMainAudio(haveMainModel);
    emit canImportLayer(haveMainModel && haveCurrentPane);
    emit canExportAudio(haveMainModel);
    emit canChangeSessionTemplate(haveMainModel);
    emit canExportLayer(haveMainModel &&
                        (haveCurrentEditableLayer || haveCurrentColour3DPlot));
    emit canExportImage(haveMainModel && haveCurrentPane);
    emit canDeleteCurrentLayer(haveCurrentLayer);
    emit canRenameLayer(haveCurrentLayer);
    emit canEditLayer(haveCurrentEditableLayer);
    emit canEditLayerTabular(haveCurrentEditableLayer || haveTabularLayer);
    emit canMeasureLayer(haveCurrentLayer);
    emit canSelect(haveMainModel && haveCurrentPane);
    emit canPlay(haveMainModel && havePlayTarget);
    emit canFfwd(true);
    emit canRewind(true);
    emit canPaste(haveClipboardContents);
    emit canInsertInstant(haveCurrentPane);
    emit canInsertInstantsAtBoundaries(haveCurrentPane && haveSelection);
    emit canInsertItemAtSelection(haveCurrentPane && haveSelection && haveCurrentDurationLayer);
    emit canRenumberInstants(haveCurrentTimeInstantsLayer && haveSelection);
    emit canPlaySelection(haveMainModel && havePlayTarget && haveSelection);
    emit canClearSelection(haveSelection);
    emit canEditSelection(haveSelection && haveCurrentEditableLayer);
    emit canSave(m_sessionFile != "" && m_documentModified);
    emit canSaveAs(haveMainModel);
    emit canSelectPreviousPane(havePrevPane);
    emit canSelectNextPane(haveNextPane);
    emit canSelectPreviousLayer(havePrevLayer);
    emit canSelectNextLayer(haveNextLayer);
}

void VisualiserWindowBase::documentModified()
{
    //    cerr << "MainWindowBase::documentModified" << endl;

    if (!m_documentModified) {
        //!!! this in subclass implementation?
        setWindowTitle(tr("%1 (modified)").arg(windowTitle()));
    }

    m_documentModified = true;
    updateMenuStates();
}

void VisualiserWindowBase::documentRestored()
{
    //    cerr << "MainWindowBase::documentRestored" << endl;

    if (m_documentModified) {
        //!!! this in subclass implementation?
        QString wt(windowTitle());
        wt.replace(tr(" (modified)"), "");
        setWindowTitle(wt);
    }

    m_documentModified = false;
    updateMenuStates();
}

void VisualiserWindowBase::playLoopToggled()
{
    QAction *action = dynamic_cast<QAction *>(sender());
    
    if (action) {
        m_viewManager->setPlayLoopMode(action->isChecked());
    } else {
        m_viewManager->setPlayLoopMode(!m_viewManager->getPlayLoopMode());
    }
}

void VisualiserWindowBase::playSelectionToggled()
{
    QAction *action = dynamic_cast<QAction *>(sender());
    
    if (action) {
        m_viewManager->setPlaySelectionMode(action->isChecked());
    } else {
        m_viewManager->setPlaySelectionMode(!m_viewManager->getPlaySelectionMode());
    }
}

void VisualiserWindowBase::playSoloToggled()
{
    QAction *action = dynamic_cast<QAction *>(sender());
    
    if (action) {
        m_viewManager->setPlaySoloMode(action->isChecked());
    } else {
        m_viewManager->setPlaySoloMode(!m_viewManager->getPlaySoloMode());
    }

    if (m_viewManager->getPlaySoloMode()) {
        currentPaneChanged(m_paneStack->getCurrentPane());
    } else {
        m_viewManager->setPlaybackModel(0);
        if (m_playSource) {
            m_playSource->clearSoloModelSet();
        }
    }
}

void VisualiserWindowBase::currentPaneChanged(Pane *p)
{
    updateMenuStates();
    updateVisibleRangeDisplay(p);

    if (!p) return;

    if (!(m_viewManager &&
          m_playSource &&
          m_viewManager->getPlaySoloMode())) {
        if (m_viewManager) m_viewManager->setPlaybackModel(0);
        return;
    }

    Model *prevPlaybackModel = m_viewManager->getPlaybackModel();

    // What we want here is not the currently playing frame (unless we
    // are about to clear out the audio playback buffers -- which may
    // or may not be possible, depending on the audio driver).  What
    // we want is the frame that was last committed to the soundcard
    // buffers, as the audio driver will continue playing up to that
    // frame before switching to whichever one we decide we want to
    // switch to, regardless of our efforts.

    sv_frame_t frame = m_playSource->getCurrentBufferedFrame();

    cerr << "currentPaneChanged: current frame (in ref model) = " << frame << endl;

    View::ModelSet soloModels = p->getModels();
    
    View::ModelSet sources;
    for (View::ModelSet::iterator mi = soloModels.begin();
         mi != soloModels.end(); ++mi) {
        // If a model in this pane is derived from something else,
        // then we want to play that model as well -- if the model
        // that's derived from it is not something that is itself
        // individually playable (e.g. a waveform)
        if (*mi &&
                !dynamic_cast<RangeSummarisableTimeValueModel *>(*mi) &&
                (*mi)->getSourceModel()) {
            sources.insert((*mi)->getSourceModel());
        }
    }
    for (View::ModelSet::iterator mi = sources.begin();
         mi != sources.end(); ++mi) {
        soloModels.insert(*mi);
    }

    //!!! Need an "atomic" way of telling the play source that the
    //playback model has changed, and changing it on ViewManager --
    //the play source should be making the setPlaybackModel call to
    //ViewManager

    for (View::ModelSet::iterator mi = soloModels.begin();
         mi != soloModels.end(); ++mi) {
        if (dynamic_cast<RangeSummarisableTimeValueModel *>(*mi)) {
            m_viewManager->setPlaybackModel(*mi);
        }
    }
    
    RangeSummarisableTimeValueModel *a =
            dynamic_cast<RangeSummarisableTimeValueModel *>(prevPlaybackModel);
    RangeSummarisableTimeValueModel *b =
            dynamic_cast<RangeSummarisableTimeValueModel *>(m_viewManager->
                                                            getPlaybackModel());

    m_playSource->setSoloModelSet(soloModels);

    if (a && b && (a != b)) {
        if (m_playSource->isPlaying()) m_playSource->play(frame);
    }
}

void VisualiserWindowBase::currentLayerChanged(Pane *p, Layer *)
{
    updateMenuStates();
    updateVisibleRangeDisplay(p);
}

void VisualiserWindowBase::selectAll()
{
    if (!getMainModel()) return;
    m_viewManager->setSelection(Selection(getMainModel()->getStartFrame(),
                                          getMainModel()->getEndFrame()));
}

void VisualiserWindowBase::selectToStart()
{
    if (!getMainModel()) return;
    m_viewManager->setSelection(Selection(getMainModel()->getStartFrame(),
                                          m_viewManager->getGlobalCentreFrame()));
}

void VisualiserWindowBase::selectToEnd()
{
    if (!getMainModel()) return;
    m_viewManager->setSelection(Selection(m_viewManager->getGlobalCentreFrame(),
                                          getMainModel()->getEndFrame()));
}

void VisualiserWindowBase::selectVisible()
{
    Model *model = getMainModel();
    if (!model) return;

    Pane *currentPane = m_paneStack->getCurrentPane();
    if (!currentPane) return;

    sv_frame_t startFrame, endFrame;

    if (currentPane->getStartFrame() < 0) startFrame = 0;
    else startFrame = currentPane->getStartFrame();

    if (currentPane->getEndFrame() > model->getEndFrame()) endFrame = model->getEndFrame();
    else endFrame = currentPane->getEndFrame();

    m_viewManager->setSelection(Selection(startFrame, endFrame));
}

void VisualiserWindowBase::clearSelection()
{
    m_viewManager->clearSelections();
}

void VisualiserWindowBase::cut()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (!currentPane) return;

    Layer *layer = currentPane->getSelectedLayer();
    if (!layer) return;

    Clipboard &clipboard = m_viewManager->getClipboard();
    clipboard.clear();

    MultiSelection::SelectionList selections = m_viewManager->getSelections();

    CommandHistory::getInstance()->startCompoundOperation(tr("Cut"), true);

    for (MultiSelection::SelectionList::iterator i = selections.begin();
         i != selections.end(); ++i) {
        layer->copy(currentPane, *i, clipboard);
        layer->deleteSelection(*i);
    }

    CommandHistory::getInstance()->endCompoundOperation();
}

void VisualiserWindowBase::copy()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (!currentPane) return;

    Layer *layer = currentPane->getSelectedLayer();
    if (!layer) return;

    Clipboard &clipboard = m_viewManager->getClipboard();
    clipboard.clear();

    MultiSelection::SelectionList selections = m_viewManager->getSelections();

    for (MultiSelection::SelectionList::iterator i = selections.begin();
         i != selections.end(); ++i) {
        layer->copy(currentPane, *i, clipboard);
    }
}

void
VisualiserWindowBase::paste()
{
    pasteRelative(0);
}

void
VisualiserWindowBase::pasteAtPlaybackPosition()
{
    sv_frame_t pos = getFrame();
    Clipboard &clipboard = m_viewManager->getClipboard();
    if (!clipboard.empty()) {
        sv_frame_t firstEventFrame = clipboard.getPoints()[0].getFrame();
        sv_frame_t offset = 0;
        if (firstEventFrame < 0) {
            offset = pos - firstEventFrame;
        } else if (firstEventFrame < pos) {
            offset = pos - firstEventFrame;
        } else {
            offset = -(firstEventFrame - pos);
        }
        pasteRelative(offset);
    }
}

void
VisualiserWindowBase::pasteRelative(sv_frame_t offset)
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (!currentPane) return;

    Layer *layer = currentPane->getSelectedLayer();

    Clipboard &clipboard = m_viewManager->getClipboard();

    bool inCompound = false;

    if (!layer || !layer->isLayerEditable()) {
        
        CommandHistory::getInstance()->startCompoundOperation
                (tr("Paste"), true);

        // no suitable current layer: create one of the most
        // appropriate sort
        LayerFactory::LayerType type =
                LayerFactory::getInstance().getLayerTypeForClipboardContents(clipboard);
        layer = m_document->createEmptyLayer(type);

        if (!layer) {
            CommandHistory::getInstance()->endCompoundOperation();
            return;
        }

        m_document->addLayerToView(currentPane, layer);
        m_paneStack->setCurrentLayer(currentPane, layer);

        inCompound = true;
    }

    layer->paste(currentPane, clipboard, offset, true);

    if (inCompound) CommandHistory::getInstance()->endCompoundOperation();
}

void
VisualiserWindowBase::deleteSelected()
{
    if (m_paneStack->getCurrentPane() &&
            m_paneStack->getCurrentPane()->getSelectedLayer()) {
        
        Layer *layer = m_paneStack->getCurrentPane()->getSelectedLayer();

        if (m_viewManager) {

            if (m_viewManager->getToolMode() == ViewManager::MeasureMode) {

                layer->deleteCurrentMeasureRect();

            } else {

                MultiSelection::SelectionList selections =
                        m_viewManager->getSelections();

                for (MultiSelection::SelectionList::iterator i = selections.begin();
                     i != selections.end(); ++i) {
                    layer->deleteSelection(*i);
                }
            }
        }
    }
}

// FrameTimer method

sv_frame_t
VisualiserWindowBase::getFrame() const
{
    if (m_playSource && m_playSource->isPlaying()) {
        return m_playSource->getCurrentPlayingFrame();
    } else {
        return m_viewManager->getPlaybackFrame();
    }
}    

void
VisualiserWindowBase::insertInstant()
{
    insertInstantAt(getFrame());
}

void
VisualiserWindowBase::insertInstantsAtBoundaries()
{
    MultiSelection::SelectionList selections = m_viewManager->getSelections();
    for (MultiSelection::SelectionList::iterator i = selections.begin();
         i != selections.end(); ++i) {
        sv_frame_t start = i->getStartFrame();
        sv_frame_t end = i->getEndFrame();
        if (start != end) {
            insertInstantAt(start);
            insertInstantAt(end);
        }
    }
}

void
VisualiserWindowBase::insertInstantAt(sv_frame_t frame)
{
    Pane *pane = m_paneStack->getCurrentPane();
    if (!pane) {
        return;
    }

    frame = pane->alignFromReference(frame);

    Layer *layer = dynamic_cast<TimeInstantLayer *>
            (pane->getSelectedLayer());

    if (!layer) {
        for (int i = pane->getLayerCount(); i > 0; --i) {
            layer = dynamic_cast<TimeInstantLayer *>(pane->getLayer(i - 1));
            if (layer) break;
        }

        if (!layer) {
            CommandHistory::getInstance()->startCompoundOperation
                    (tr("Add Point"), true);
            layer = m_document->createEmptyLayer(LayerFactory::Type("TimeInstants"));
            if (layer) {
                m_document->addLayerToView(pane, layer);
                m_paneStack->setCurrentLayer(pane, layer);
            }
            CommandHistory::getInstance()->endCompoundOperation();
        }
    }

    if (layer) {

        Model *model = layer->getModel();
        SparseOneDimensionalModel *sodm = dynamic_cast<SparseOneDimensionalModel *>
                (model);

        if (sodm) {
            SparseOneDimensionalModel::Point point(frame, "");

            SparseOneDimensionalModel::Point prevPoint(0);
            bool havePrevPoint = false;

            SparseOneDimensionalModel::EditCommand *command =
                    new SparseOneDimensionalModel::EditCommand(sodm, tr("Add Point"));

            if (m_labeller) {

                if (m_labeller->requiresPrevPoint()) {

                    SparseOneDimensionalModel::PointList prevPoints =
                            sodm->getPreviousPoints(frame);

                    if (!prevPoints.empty()) {
                        prevPoint = *prevPoints.begin();
                        havePrevPoint = true;
                    }
                }

                m_labeller->setSampleRate(sodm->getSampleRate());

                if (m_labeller->actingOnPrevPoint() && havePrevPoint) {
                    command->deletePoint(prevPoint);
                }

                m_labeller->label<SparseOneDimensionalModel::Point>
                        (point, havePrevPoint ? &prevPoint : 0);

                if (m_labeller->actingOnPrevPoint() && havePrevPoint) {
                    command->addPoint(prevPoint);
                }
            }
            
            command->addPoint(point);

            command->setName(tr("Add Point at %1 s")
                             .arg(RealTime::frame2RealTime
                                  (frame,
                                   sodm->getSampleRate())
                                  .toText(false).c_str()));

            SVCommand *c = command->finish();
            if (c) CommandHistory::getInstance()->addCommand(c, false);
        }
    }
}

void
VisualiserWindowBase::insertItemAtSelection()
{
    MultiSelection::SelectionList selections = m_viewManager->getSelections();
    for (MultiSelection::SelectionList::iterator i = selections.begin();
         i != selections.end(); ++i) {
        sv_frame_t start = i->getStartFrame();
        sv_frame_t end = i->getEndFrame();
        if (start < end) {
            insertItemAt(start, end - start);
        }
    }
}

void
VisualiserWindowBase::insertItemAt(sv_frame_t frame, sv_frame_t duration)
{
    Pane *pane = m_paneStack->getCurrentPane();
    if (!pane) {
        return;
    }

    // ugh!

    sv_frame_t alignedStart = pane->alignFromReference(frame);
    sv_frame_t alignedEnd = pane->alignFromReference(frame + duration);
    if (alignedStart >= alignedEnd) return;
    sv_frame_t alignedDuration = alignedEnd - alignedStart;

    SVCommand *c = 0;

    QString name = tr("Add Item at %1 s")
            .arg(RealTime::frame2RealTime
                 (alignedStart,
                  getMainModel()->getSampleRate())
                 .toText(false).c_str());

    Layer *layer = pane->getSelectedLayer();
    if (!layer) return;

    RegionModel *rm = dynamic_cast<RegionModel *>(layer->getModel());
    if (rm) {
        RegionModel::Point point(alignedStart,
                                 rm->getValueMaximum() + 1,
                                 alignedDuration,
                                 "");
        RegionModel::EditCommand *command =
                new RegionModel::EditCommand(rm, tr("Add Point"));
        command->addPoint(point);
        command->setName(name);
        c = command->finish();
    }

    if (c) {
        CommandHistory::getInstance()->addCommand(c, false);
        return;
    }

    NoteModel *nm = dynamic_cast<NoteModel *>(layer->getModel());
    if (nm) {
        NoteModel::Point point(alignedStart,
                               nm->getValueMinimum(),
                               alignedDuration,
                               1.f,
                               "");
        NoteModel::EditCommand *command =
                new NoteModel::EditCommand(nm, tr("Add Point"));
        command->addPoint(point);
        command->setName(name);
        c = command->finish();
    }

    if (c) {
        CommandHistory::getInstance()->addCommand(c, false);
        return;
    }

    FlexiNoteModel *fnm = dynamic_cast<FlexiNoteModel *>(layer->getModel());
    if (fnm) {
        FlexiNoteModel::Point point(alignedStart,
                                    fnm->getValueMinimum(),
                                    alignedDuration,
                                    1.f,
                                    "");
        FlexiNoteModel::EditCommand *command =
                new FlexiNoteModel::EditCommand(fnm, tr("Add Point"));
        command->addPoint(point);
        command->setName(name);
        c = command->finish();
    }
    
    if (c) {
        CommandHistory::getInstance()->addCommand(c, false);
        return;
    }
}

void
VisualiserWindowBase::renumberInstants()
{
    Pane *pane = m_paneStack->getCurrentPane();
    if (!pane) return;

    Layer *layer = dynamic_cast<TimeInstantLayer *>(pane->getSelectedLayer());
    if (!layer) return;

    MultiSelection ms(m_viewManager->getSelection());
    
    Model *model = layer->getModel();
    SparseOneDimensionalModel *sodm = dynamic_cast<SparseOneDimensionalModel *>
            (model);
    if (!sodm) return;

    if (!m_labeller) return;

    Labeller labeller(*m_labeller);
    labeller.setSampleRate(sodm->getSampleRate());

    // This uses a command

    labeller.labelAll<SparseOneDimensionalModel::Point>(*sodm, &ms);
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openPath(QString fileOrUrl, AudioFileOpenMode mode)
{
    ProgressDialog dialog(tr("Opening file or URL..."), true, 2000, this);
    connect(&dialog, SIGNAL(showing()), this, SIGNAL(hideSplash()));
    return open(FileSource(fileOrUrl, &dialog), mode);
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::open(FileSource source, AudioFileOpenMode mode)
{
    FileOpenStatus status;

    if (!source.isAvailable()) return FileOpenFailed;
    source.waitForData();

    bool canImportLayer = (getMainModel() != 0 &&
            m_paneStack != 0 &&
            m_paneStack->getCurrentPane() != 0);

    bool rdf = (source.getExtension().toLower() == "rdf" ||
                source.getExtension().toLower() == "n3" ||
                source.getExtension().toLower() == "ttl");

    bool audio = AudioFileReaderFactory::getKnownExtensions().contains
            (source.getExtension().toLower());

    bool rdfSession = false;
    if (rdf) {
        RDFImporter::RDFDocumentType rdfType =
                RDFImporter::identifyDocumentType
                (QUrl::fromLocalFile(source.getLocalFilename()).toString());
        if (rdfType == RDFImporter::AudioRefAndAnnotations ||
                rdfType == RDFImporter::AudioRef) {
            rdfSession = true;
        } else if (rdfType == RDFImporter::NotRDF) {
            rdf = false;
        }
    }

    if (rdf) {
        if (rdfSession) {
            bool cancel = false;
            if (!canImportLayer || shouldCreateNewSessionForRDFAudio(&cancel)) {
                return openSession(source);
            } else if (cancel) {
                return FileOpenCancelled;
            } else {
                return openLayer(source);
            }
        } else {
            if ((status = openSession(source)) != FileOpenFailed) {
                return status;
            } else if (!canImportLayer) {
                return FileOpenWrongMode;
            } else if ((status = openLayer(source)) != FileOpenFailed) {
                return status;
            } else {
                return FileOpenFailed;
            }
        }
    }

    if (audio && (status = openAudio(source, mode)) != FileOpenFailed) {
        return status;
    } else if ((status = openSession(source)) != FileOpenFailed) {
        return status;
    } else if ((status = openPlaylist(source, mode)) != FileOpenFailed) {
        return status;
    } else if (!canImportLayer) {
        return FileOpenWrongMode;
    } else if ((status = openImage(source)) != FileOpenFailed) {
        return status;
    } else if ((status = openLayer(source)) != FileOpenFailed) {
        return status;
    } else {
        return FileOpenFailed;
    }
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openAudio(FileSource source, AudioFileOpenMode mode,
                                QString templateName)
{
    cerr << "MainWindowBase::openAudio(" << source.getLocation() << ") with mode " << mode << " and template " << templateName << endl;

    if (templateName == "") {
        templateName = getDefaultSessionTemplate();
    }

    //    cerr << "template is: \"" << templateName << "\"" << endl;

    if (!source.isAvailable()) {
        if (source.wasCancelled()) {
            return FileOpenCancelled;
        } else {
            return FileOpenFailed;
        }
    }

    source.waitForData();

    m_openingAudioFile = true;

    sv_samplerate_t rate = 0;

    if (Preferences::getInstance()->getFixedSampleRate() != 0) {
        rate = Preferences::getInstance()->getFixedSampleRate();
    } else if (Preferences::getInstance()->getResampleOnLoad()) {
        rate = m_playSource->getSourceSampleRate();
    }

    WaveFileModel *newModel = new WaveFileModel(source, rate);

    if (!newModel->isOK()) {
        delete newModel;
        m_openingAudioFile = false;
        if (source.wasCancelled()) {
            return FileOpenCancelled;
        } else {
            return FileOpenFailed;
        }
    }

    //    cerr << "mode = " << mode << endl;

    if (mode == AskUser) {
        if (getMainModel()) {

            QSettings settings;
            settings.beginGroup("MainWindow");
            int lastMode = settings.value("lastaudioopenmode", 0).toBool();
            settings.endGroup();
            int imode = 0;
            
            QStringList items;
            items << tr("Close the current session and start a new one")
                  << tr("Replace the main audio file in this session")
                  << tr("Add the audio file to this session");

            bool ok = false;
            QString item = ListInputDialog::getItem
                    (this, tr("Select target for import"),
                     tr("<b>Select a target for import</b><p>You already have an audio file loaded.<br>What would you like to do with the new audio file?"),
                     items, lastMode, &ok);
            
            if (!ok || item.isEmpty()) {
                delete newModel;
                m_openingAudioFile = false;
                return FileOpenCancelled;
            }
            
            for (int i = 0; i < items.size(); ++i) {
                if (item == items[i]) imode = i;
            }

            settings.beginGroup("MainWindow");
            settings.setValue("lastaudioopenmode", imode);
            settings.endGroup();

            mode = (AudioFileOpenMode)imode;

        } else {
            // no main model: make a new session
            mode = ReplaceSession;
        }
    }

    if (mode == ReplaceCurrentPane) {

        Pane *pane = m_paneStack->getCurrentPane();
        if (pane) {
            if (getMainModel()) {
                View::ModelSet models(pane->getModels());
                if (models.find(getMainModel()) != models.end()) {
                    // Current pane contains main model: replace that
                    mode = ReplaceMainModel;
                }
                // Otherwise the current pane has a non-default model,
                // which we will deal with later
            } else {
                // We have no main model, so start a new session with
                // optional template
                mode = ReplaceSession;
            }
        } else {
            // We seem to have no current pane!  Oh well
            mode = CreateAdditionalModel;
        }
    }

    if (mode == CreateAdditionalModel && !getMainModel()) {
        cerr << "Mode is CreateAdditionalModel but we have no main model, switching to ReplaceSession mode" << endl;
        mode = ReplaceSession;
    }

    bool loadedTemplate = false;

    if (mode == ReplaceSession) {

        if (!checkSaveModified()) return FileOpenCancelled;

        cerr << "SV looking for template " << templateName << endl;
        if (templateName != "") {
            FileOpenStatus tplStatus = openSessionTemplate(templateName);
            if (tplStatus == FileOpenCancelled) {
                cerr << "Template load cancelled" << endl;
                return FileOpenCancelled;
            }
            if (tplStatus != FileOpenFailed) {
                cerr << "Template load succeeded" << endl;
                loadedTemplate = true;
            }
        }

        if (!loadedTemplate) {
            cerr << "No template found: closing session, creating new empty document" << endl;
            closeSession();
            createDocument();
        }

        cerr << "Now switching to ReplaceMainModel mode" << endl;
        mode = ReplaceMainModel;
    }

    emit activity(tr("Import audio file \"%1\"").arg(source.getLocation()));

    if (mode == ReplaceMainModel) {

        Model *prevMain = getMainModel();
        if (prevMain) {
            m_playSource->removeModel(prevMain);
            PlayParameterRepository::getInstance()->removePlayable(prevMain);
        }
        PlayParameterRepository::getInstance()->addPlayable(newModel);

        cerr << "SV about to call setMainModel(" << newModel << "): prevMain is " << prevMain << endl;

        m_document->setMainModel(newModel);

        setupMenus();

        if (loadedTemplate || (m_sessionFile == "")) {
            //!!! shouldn't be dealing directly with title from here -- call a method
            setWindowTitle(tr("%1: %2")
                           .arg(QApplication::applicationName())
                           .arg(source.getLocation()));
            CommandHistory::getInstance()->clear();
            CommandHistory::getInstance()->documentSaved();
            m_documentModified = false;
        } else {
            setWindowTitle(tr("%1: %2 [%3]")
                           .arg(QApplication::applicationName())
                           .arg(QFileInfo(m_sessionFile).fileName())
                           .arg(source.getLocation()));
            if (m_documentModified) {
                m_documentModified = false;
                documentModified(); // so as to restore "(modified)" window title
            }
        }

        if (!source.isRemote()) m_audioFile = source.getLocalFilename();

    } else if (mode == CreateAdditionalModel) {

        CommandHistory::getInstance()->startCompoundOperation
                (tr("Import \"%1\"").arg(source.getBasename()), true);

        m_document->addImportedModel(newModel);

        AddPaneCommand *command = new AddPaneCommand(this);
        CommandHistory::getInstance()->addCommand(command);

        Pane *pane = command->getPane();

        if (m_timeRulerLayer) {
            m_document->addLayerToView(pane, m_timeRulerLayer);
        }

        Layer *newLayer = m_document->createImportedLayer(newModel);

        if (newLayer) {
            m_document->addLayerToView(pane, newLayer);
        }

        CommandHistory::getInstance()->endCompoundOperation();

    } else if (mode == ReplaceCurrentPane) {

        // We know there is a current pane, otherwise we would have
        // reset the mode to CreateAdditionalModel above; and we know
        // the current pane does not contain the main model, otherwise
        // we would have reset it to ReplaceMainModel.  But we don't
        // know whether the pane contains a waveform model at all.
        
        Pane *pane = m_paneStack->getCurrentPane();
        Layer *replace = 0;

        for (int i = 0; i < pane->getLayerCount(); ++i) {
            Layer *layer = pane->getLayer(i);
            if (dynamic_cast<WaveformLayer *>(layer)) {
                replace = layer;
                break;
            }
        }

        CommandHistory::getInstance()->startCompoundOperation
                (tr("Import \"%1\"").arg(source.getBasename()), true);

        m_document->addImportedModel(newModel);

        if (replace) {
            m_document->removeLayerFromView(pane, replace);
        }

        Layer *newLayer = m_document->createImportedLayer(newModel);

        if (newLayer) {
            m_document->addLayerToView(pane, newLayer);
        }

        CommandHistory::getInstance()->endCompoundOperation();
    }

    updateMenuStates();
    m_recentFiles.addFile(source.getLocation());
    if (!source.isRemote()) {
        // for file dialog
        registerLastOpenedFilePath(FileFinder::AudioFile,
                                   source.getLocalFilename());
    }
    m_openingAudioFile = false;

    currentPaneChanged(m_paneStack->getCurrentPane());

    emit audioFileLoaded();

    return FileOpenSucceeded;
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openPlaylist(FileSource source, AudioFileOpenMode mode)
{
    cerr << "MainWindowBase::openPlaylist(" << source.getLocation() << ")" << endl;

    std::set<QString> extensions;
    PlaylistFileReader::getSupportedExtensions(extensions);
    QString extension = source.getExtension().toLower();
    if (extensions.find(extension) == extensions.end()) return FileOpenFailed;

    if (!source.isAvailable()) return FileOpenFailed;
    source.waitForData();

    PlaylistFileReader reader(source.getLocalFilename());
    if (!reader.isOK()) return FileOpenFailed;

    PlaylistFileReader::Playlist playlist = reader.load();

    bool someSuccess = false;

    for (PlaylistFileReader::Playlist::const_iterator i = playlist.begin();
         i != playlist.end(); ++i) {

        ProgressDialog dialog(tr("Opening playlist..."), true, 2000, this);
        connect(&dialog, SIGNAL(showing()), this, SIGNAL(hideSplash()));
        FileOpenStatus status = openAudio(FileSource(*i, &dialog), mode);

        if (status == FileOpenCancelled) {
            return FileOpenCancelled;
        }

        if (status == FileOpenSucceeded) {
            someSuccess = true;
            mode = CreateAdditionalModel;
        }
    }

    if (someSuccess) return FileOpenSucceeded;
    else return FileOpenFailed;
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openLayer(FileSource source)
{
    cerr << "MainWindowBase::openLayer(" << source.getLocation() << ")" << endl;

    Pane *pane = m_paneStack->getCurrentPane();
    
    if (!pane) {
        // shouldn't happen, as the menu action should have been disabled
        cerr << "WARNING: MainWindowBase::openLayer: no current pane" << endl;
        return FileOpenWrongMode;
    }

    if (!getMainModel()) {
        // shouldn't happen, as the menu action should have been disabled
        cerr << "WARNING: MainWindowBase::openLayer: No main model -- hence no default sample rate available" << endl;
        return FileOpenWrongMode;
    }

    if (!source.isAvailable()) return FileOpenFailed;
    source.waitForData();

    QString path = source.getLocalFilename();

    RDFImporter::RDFDocumentType rdfType =
            RDFImporter::identifyDocumentType(QUrl::fromLocalFile(path).toString());

    //    cerr << "RDF type:  (in layer) " << (int) rdfType << endl;

    if (rdfType != RDFImporter::NotRDF) {

        return openLayersFromRDF(source);

    } else if (source.getExtension().toLower() == "svl" ||
               (source.getExtension().toLower() == "xml" &&
                (SVFileReader::identifyXmlFile(source.getLocalFilename())
                 == SVFileReader::SVLayerFile))) {

        PaneCallback callback(this);
        QFile file(path);
        
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            cerr << "ERROR: MainWindowBase::openLayer("
                 << source.getLocation()
                 << "): Failed to open file for reading" << endl;
            return FileOpenFailed;
        }
        
        SVFileReader reader(m_document, callback, source.getLocation());
        connect
                (&reader, SIGNAL(modelRegenerationFailed(QString, QString, QString)),
                 this, SLOT(modelRegenerationFailed(QString, QString, QString)));
        connect
                (&reader, SIGNAL(modelRegenerationWarning(QString, QString, QString)),
                 this, SLOT(modelRegenerationWarning(QString, QString, QString)));
        reader.setCurrentPane(pane);
        
        QXmlInputSource inputSource(&file);
        reader.parse(inputSource);
        
        if (!reader.isOK()) {
            cerr << "ERROR: MainWindowBase::openLayer("
                 << source.getLocation()
                 << "): Failed to read XML file: "
                 << reader.getErrorString() << endl;
            return FileOpenFailed;
        }

        emit activity(tr("Import layer XML file \"%1\"").arg(source.getLocation()));

        m_recentFiles.addFile(source.getLocation());

        if (!source.isRemote()) {
            registerLastOpenedFilePath(FileFinder::LayerFile, path); // for file dialog
        }

        return FileOpenSucceeded;

    } else {
        
        try {

            MIDIFileImportDialog midiDlg(this);

            Model *model = DataFileReaderFactory::loadNonCSV
                    (path, &midiDlg, getMainModel()->getSampleRate());

            if (!model) {
                CSVFormat format(path);
                format.setSampleRate(getMainModel()->getSampleRate());
                CSVFormatDialog *dialog = new CSVFormatDialog(this, format);
                if (dialog->exec() == QDialog::Accepted) {
                    model = DataFileReaderFactory::loadCSV
                            (path, dialog->getFormat(),
                             getMainModel()->getSampleRate());
                }
            }

            if (model) {

                cerr << "MainWindowBase::openLayer: Have model" << endl;

                emit activity(tr("Import MIDI file \"%1\"").arg(source.getLocation()));

                Layer *newLayer = m_document->createImportedLayer(model);

                if (newLayer) {

                    m_document->addLayerToView(pane, newLayer);
                    m_paneStack->setCurrentLayer(pane, newLayer);

                    m_recentFiles.addFile(source.getLocation());
                    
                    if (!source.isRemote()) {
                        registerLastOpenedFilePath
                                (FileFinder::LayerFile,
                                 path); // for file dialog
                    }

                    return FileOpenSucceeded;
                }
            }
        } catch (DataFileReaderFactory::Exception e) {
            if (e == DataFileReaderFactory::ImportCancelled) {
                return FileOpenCancelled;
            }
        }
    }
    
    return FileOpenFailed;
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openImage(FileSource source)
{
    cerr << "MainWindowBase::openImage(" << source.getLocation() << ")" << endl;

    Pane *pane = m_paneStack->getCurrentPane();
    
    if (!pane) {
        // shouldn't happen, as the menu action should have been disabled
        cerr << "WARNING: MainWindowBase::openImage: no current pane" << endl;
        return FileOpenWrongMode;
    }

    if (!m_document->getMainModel()) {
        return FileOpenWrongMode;
    }

    bool newLayer = false;

    ImageLayer *il = dynamic_cast<ImageLayer *>(pane->getSelectedLayer());
    if (!il) {
        for (int i = pane->getLayerCount()-1; i >= 0; --i) {
            il = dynamic_cast<ImageLayer *>(pane->getLayer(i));
            if (il) break;
        }
    }
    if (!il) {
        il = dynamic_cast<ImageLayer *>
                (m_document->createEmptyLayer(LayerFactory::Type("Image")));
        if (!il) return FileOpenFailed;
        newLayer = true;
    }

    // We don't put the image file in Recent Files

    cerr << "openImage: trying location \"" << source.getLocation() << "\" in image layer" << endl;

    if (!il->addImage(m_viewManager->getGlobalCentreFrame(), source.getLocation())) {
        if (newLayer) {
            m_document->deleteLayer(il); // also releases its model
        }
        return FileOpenFailed;
    } else {
        if (newLayer) {
            m_document->addLayerToView(pane, il);
        }
        m_paneStack->setCurrentLayer(pane, il);
    }

    return FileOpenSucceeded;
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openSessionPath(QString fileOrUrl)
{
    ProgressDialog dialog(tr("Opening session..."), true, 2000, this);
    connect(&dialog, SIGNAL(showing()), this, SIGNAL(hideSplash()));
    return openSession(FileSource(fileOrUrl, &dialog));
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openSession(FileSource source)
{
    cerr << "MainWindowBase::openSession(" << source.getLocation() << ")" << endl;

    if (!source.isAvailable()) return FileOpenFailed;
    source.waitForData();

    QString sessionExt =
            InteractiveFileFinder::getInstance()->getApplicationSessionExtension();

    if (source.getExtension().toLower() != sessionExt) {

        RDFImporter::RDFDocumentType rdfType =
                RDFImporter::identifyDocumentType
                (QUrl::fromLocalFile(source.getLocalFilename()).toString());

        //        cerr << "RDF type: " << (int)rdfType << endl;

        if (rdfType == RDFImporter::AudioRefAndAnnotations ||
                rdfType == RDFImporter::AudioRef) {
            return openSessionFromRDF(source);
        } else if (rdfType != RDFImporter::NotRDF) {
            return FileOpenFailed;
        }

        if (source.getExtension().toLower() == "xml") {
            if (SVFileReader::identifyXmlFile(source.getLocalFilename()) ==
                    SVFileReader::SVSessionFile) {
                cerr << "This XML file looks like a session file, attempting to open it as a session" << endl;
            } else {
                return FileOpenFailed;
            }
        } else {
            return FileOpenFailed;
        }
    }

    QXmlInputSource *inputSource = 0;
    BZipFileDevice *bzFile = 0;
    QFile *rawFile = 0;

    if (source.getExtension().toLower() == sessionExt) {
        bzFile = new BZipFileDevice(source.getLocalFilename());
        if (!bzFile->open(QIODevice::ReadOnly)) {
            delete bzFile;
            return FileOpenFailed;
        }
        inputSource = new QXmlInputSource(bzFile);
    } else {
        rawFile = new QFile(source.getLocalFilename());
        inputSource = new QXmlInputSource(rawFile);
    }

    if (!checkSaveModified()) {
        if (bzFile) bzFile->close();
        delete inputSource;
        delete bzFile;
        delete rawFile;
        return FileOpenCancelled;
    }

    QString error;
    closeSession();
    createDocument();

    PaneCallback callback(this);
    m_viewManager->clearSelections();

    SVFileReader reader(m_document, callback, source.getLocation());
    connect
            (&reader, SIGNAL(modelRegenerationFailed(QString, QString, QString)),
             this, SLOT(modelRegenerationFailed(QString, QString, QString)));
    connect
            (&reader, SIGNAL(modelRegenerationWarning(QString, QString, QString)),
             this, SLOT(modelRegenerationWarning(QString, QString, QString)));

    reader.parse(*inputSource);
    
    if (!reader.isOK()) {
        error = tr("SV XML file read error:\n%1").arg(reader.getErrorString());
    }
    
    if (bzFile) bzFile->close();

    delete inputSource;
    delete bzFile;
    delete rawFile;

    bool ok = (error == "");

    if (ok) {

        emit activity(tr("Import session file \"%1\"").arg(source.getLocation()));

        setWindowTitle(tr("%1: %2")
                       .arg(QApplication::applicationName())
                       .arg(source.getLocation()));

        if (!source.isRemote()) m_sessionFile = source.getLocalFilename();

        setupMenus();

        CommandHistory::getInstance()->clear();
        CommandHistory::getInstance()->documentSaved();
        m_documentModified = false;
        updateMenuStates();

        m_recentFiles.addFile(source.getLocation());

        if (!source.isRemote()) {
            // for file dialog
            registerLastOpenedFilePath(FileFinder::SessionFile,
                                       source.getLocalFilename());
        }

        emit sessionLoaded();

    } else {
        setWindowTitle(QApplication::applicationName());
    }

    return ok ? FileOpenSucceeded : FileOpenFailed;
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openSessionTemplate(QString templateName)
{
    // Template in the user's template directory takes
    // priority over a bundled one; we don't unbundle, but
    // open directly from the bundled file (where applicable)
    ResourceFinder rf;
    QString tfile = rf.getResourcePath("templates", templateName + ".svt");
    if (tfile != "") {
        cerr << "SV loading template file " << tfile << endl;
        return openSessionTemplate(FileSource("file:" + tfile));
    } else {
        return FileOpenFailed;
    }
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openSessionTemplate(FileSource source)
{
    cerr << "MainWindowBase::openSessionTemplate(" << source.getLocation() << ")" << endl;

    if (!source.isAvailable()) return FileOpenFailed;
    source.waitForData();

    QXmlInputSource *inputSource = 0;
    QFile *file = 0;

    file = new QFile(source.getLocalFilename());
    inputSource = new QXmlInputSource(file);

    if (!checkSaveModified()) {
        delete inputSource;
        delete file;
        return FileOpenCancelled;
    }

    QString error;
    closeSession();
    createDocument();

    PaneCallback callback(this);
    m_viewManager->clearSelections();

    SVFileReader reader(m_document, callback, source.getLocation());
    connect
            (&reader, SIGNAL(modelRegenerationFailed(QString, QString, QString)),
             this, SLOT(modelRegenerationFailed(QString, QString, QString)));
    connect
            (&reader, SIGNAL(modelRegenerationWarning(QString, QString, QString)),
             this, SLOT(modelRegenerationWarning(QString, QString, QString)));

    reader.parse(*inputSource);
    
    if (!reader.isOK()) {
        error = tr("SV XML file read error:\n%1").arg(reader.getErrorString());
    }
    
    delete inputSource;
    delete file;

    bool ok = (error == "");

    setWindowTitle(QApplication::applicationName());

    if (ok) {

        emit activity(tr("Open session template \"%1\"").arg(source.getLocation()));

        setupMenus();

        CommandHistory::getInstance()->clear();
        CommandHistory::getInstance()->documentSaved();
        m_documentModified = false;
        updateMenuStates();

        emit sessionLoaded();
    }

    return ok ? FileOpenSucceeded : FileOpenFailed;
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openSessionFromRDF(FileSource source)
{
    cerr << "MainWindowBase::openSessionFromRDF(" << source.getLocation() << ")" << endl;

    if (!source.isAvailable()) return FileOpenFailed;
    source.waitForData();

    if (!checkSaveModified()) {
        return FileOpenCancelled;
    }
    
    closeSession();
    createDocument();

    FileOpenStatus status = openLayersFromRDF(source);

    setupMenus();
    
    setWindowTitle(tr("%1: %2")
                   .arg(QApplication::applicationName())
                   .arg(source.getLocation()));
    CommandHistory::getInstance()->clear();
    CommandHistory::getInstance()->documentSaved();
    m_documentModified = false;

    emit sessionLoaded();

    return status;
}

VisualiserWindowBase::FileOpenStatus
VisualiserWindowBase::openLayersFromRDF(FileSource source)
{
    sv_samplerate_t rate = 0;

    cerr << "MainWindowBase::openLayersFromRDF" << endl;

    ProgressDialog dialog(tr("Importing from RDF..."), true, 2000, this);
    connect(&dialog, SIGNAL(showing()), this, SIGNAL(hideSplash()));

    if (getMainModel()) {
        rate = getMainModel()->getSampleRate();
    } else if (Preferences::getInstance()->getResampleOnLoad()) {
        rate = m_playSource->getSourceSampleRate();
    }

    RDFImporter importer
            (QUrl::fromLocalFile(source.getLocalFilename()).toString(), rate);

    if (!importer.isOK()) {
        if (importer.getErrorString() != "") {
            QMessageBox::critical
                    (this, tr("Failed to import RDF"),
                     tr("<b>Failed to import RDF</b><p>Importing data from RDF document at \"%1\" failed: %2</p>")
                     .arg(source.getLocation()).arg(importer.getErrorString()));
        }
        return FileOpenFailed;
    }

    std::vector<Model *> models = importer.getDataModels(&dialog);

    dialog.setMessage(tr("Importing from RDF..."));

    if (models.empty()) {
        QMessageBox::critical
                (this, tr("Failed to import RDF"),
                 tr("<b>Failed to import RDF</b><p>No suitable data models found for import from RDF document at \"%1\"</p>").arg(source.getLocation()));
        return FileOpenFailed;
    }

    emit activity(tr("Import RDF document \"%1\"").arg(source.getLocation()));

    std::set<Model *> added;

    for (int i = 0; i < (int)models.size(); ++i) {

        Model *m = models[i];
        WaveFileModel *w = dynamic_cast<WaveFileModel *>(m);

        if (w) {

            Pane *pane = addPaneToStack();
            Layer *layer = 0;

            if (m_timeRulerLayer) {
                m_document->addLayerToView(pane, m_timeRulerLayer);
            }

            if (!getMainModel()) {
                m_document->setMainModel(w);
                layer = m_document->createMainModelLayer(LayerFactory::Type("Waveform"));
            } else {
                layer = m_document->createImportedLayer(w);
            }

            m_document->addLayerToView(pane, layer);

            added.insert(w);
            
            for (int j = 0; j < (int)models.size(); ++j) {

                Model *dm = models[j];

                if (dm == m) continue;
                if (dm->getSourceModel() != m) continue;

                layer = m_document->createImportedLayer(dm);

                if (layer->isLayerOpaque() ||
                        dynamic_cast<Colour3DPlotLayer *>(layer)) {

                    // these always go in a new pane, with nothing
                    // else going in the same pane

                    Pane *singleLayerPane = addPaneToStack();
                    if (m_timeRulerLayer) {
                        m_document->addLayerToView(singleLayerPane, m_timeRulerLayer);
                    }
                    m_document->addLayerToView(singleLayerPane, layer);

                } else if (layer->getLayerColourSignificance() ==
                           Layer::ColourHasMeaningfulValue) {

                    // these can go in a pane with something else, but
                    // only if none of the something elses also have
                    // this quality

                    bool needNewPane = false;
                    for (int i = 0; i < pane->getLayerCount(); ++i) {
                        Layer *otherLayer = pane->getLayer(i);
                        if (otherLayer &&
                                (otherLayer->getLayerColourSignificance() ==
                                 Layer::ColourHasMeaningfulValue)) {
                            needNewPane = true;
                            break;
                        }
                    }
                    if (needNewPane) {
                        pane = addPaneToStack();
                    }

                    m_document->addLayerToView(pane, layer);

                } else {

                    if (pane->getLayerCount() > 4) {
                        pane = addPaneToStack();
                    }

                    m_document->addLayerToView(pane, layer);
                }

                added.insert(dm);
            }
        }
    }

    for (int i = 0; i < (int)models.size(); ++i) {

        Model *m = models[i];

        if (added.find(m) == added.end()) {
            
            Layer *layer = m_document->createImportedLayer(m);
            if (!layer) return FileOpenFailed;

            Pane *singleLayerPane = addPaneToStack();
            if (m_timeRulerLayer) {
                m_document->addLayerToView(singleLayerPane, m_timeRulerLayer);
            }
            m_document->addLayerToView(singleLayerPane, layer);
        }
    }

    m_recentFiles.addFile(source.getLocation());
    return FileOpenSucceeded;
}

void
VisualiserWindowBase::createPlayTarget()
{
    if (m_playTarget) return;

    QSettings settings;
    settings.beginGroup("Preferences");
    QString targetName = settings.value("audio-target", "").toString();
    settings.endGroup();

    AudioTargetFactory *factory = AudioTargetFactory::getInstance();

    factory->setDefaultCallbackTarget(targetName);
    m_playTarget = factory->createCallbackTarget(m_playSource);

    if (!m_playTarget) {
        emit hideSplash();

        if (factory->isAutoCallbackTarget(targetName)) {
            QMessageBox::warning
                    (this, tr("Couldn't open audio device"),
                     tr("<b>No audio available</b><p>Could not open an audio device for playback.<p>Automatic audio device detection failed. Audio playback will not be available during this session.</p>"),
                     QMessageBox::Ok);
        } else {
            QMessageBox::warning
                    (this, tr("Couldn't open audio device"),
                     tr("<b>No audio available</b><p>Failed to open your preferred audio device (\"%1\").<p>Audio playback will not be available during this session.</p>")
                     .arg(factory->getCallbackTargetDescription(targetName)),
                     QMessageBox::Ok);
        }
    }
}

WaveFileModel *
VisualiserWindowBase::getMainModel()
{
    if (!m_document) return 0;
    return m_document->getMainModel();
}

const WaveFileModel *
VisualiserWindowBase::getMainModel() const
{
    if (!m_document) return 0;
    return m_document->getMainModel();
}

void
VisualiserWindowBase::createDocument()
{
    m_document = new Document;

    connect(m_document, SIGNAL(layerAdded(Layer *)),
            this, SLOT(layerAdded(Layer *)));
    connect(m_document, SIGNAL(layerRemoved(Layer *)),
            this, SLOT(layerRemoved(Layer *)));
    connect(m_document, SIGNAL(layerAboutToBeDeleted(Layer *)),
            this, SLOT(layerAboutToBeDeleted(Layer *)));
    connect(m_document, SIGNAL(layerInAView(Layer *, bool)),
            this, SLOT(layerInAView(Layer *, bool)));

    connect(m_document, SIGNAL(modelAdded(Model *)),
            this, SLOT(modelAdded(Model *)));
    connect(m_document, SIGNAL(mainModelChanged(WaveFileModel *)),
            this, SLOT(mainModelChanged(WaveFileModel *)));
    connect(m_document, SIGNAL(modelAboutToBeDeleted(Model *)),
            this, SLOT(modelAboutToBeDeleted(Model *)));

    connect(m_document, SIGNAL(modelGenerationFailed(QString, QString)),
            this, SLOT(modelGenerationFailed(QString, QString)));
    connect(m_document, SIGNAL(modelRegenerationWarning(QString, QString, QString)),
            this, SLOT(modelRegenerationWarning(QString, QString, QString)));
    connect(m_document, SIGNAL(modelGenerationFailed(QString, QString)),
            this, SLOT(modelGenerationFailed(QString, QString)));
    connect(m_document, SIGNAL(modelRegenerationWarning(QString, QString, QString)),
            this, SLOT(modelRegenerationWarning(QString, QString, QString)));
    connect(m_document, SIGNAL(alignmentFailed(QString, QString)),
            this, SLOT(alignmentFailed(QString, QString)));

    emit replacedDocument();
}

bool
VisualiserWindowBase::saveSessionFile(QString path)
{
    try {

        TempWriteFile temp(path);

        BZipFileDevice bzFile(temp.getTemporaryFilename());
        if (!bzFile.open(QIODevice::WriteOnly)) {
            cerr << "Failed to open session file \""
                 << temp.getTemporaryFilename()
                 << "\" for writing: "
                 << bzFile.errorString() << endl;
            return false;
        }

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        QTextStream out(&bzFile);
        out.setCodec(QTextCodec::codecForName("UTF-8"));
        toXml(out, false);
        out.flush();

        QApplication::restoreOverrideCursor();

        if (!bzFile.isOK()) {
            QMessageBox::critical(this, tr("Failed to write file"),
                                  tr("<b>Save failed</b><p>Failed to write to file \"%1\": %2")
                                  .arg(path).arg(bzFile.errorString()));
            bzFile.close();
            return false;
        }

        bzFile.close();
        temp.moveToTarget();
        return true;

    } catch (FileOperationFailed &f) {
        
        QMessageBox::critical(this, tr("Failed to write file"),
                              tr("<b>Save failed</b><p>Failed to write to file \"%1\": %2")
                              .arg(path).arg(f.what()));
        return false;
    }
}

bool
VisualiserWindowBase::saveSessionTemplate(QString path)
{
    try {

        TempWriteFile temp(path);

        QFile file(temp.getTemporaryFilename());
        if (!file.open(QIODevice::WriteOnly)) {
            cerr << "Failed to open session template file \""
                 << temp.getTemporaryFilename()
                 << "\" for writing: "
                 << file.errorString() << endl;
            return false;
        }
        
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        QTextStream out(&file);
        out.setCodec(QTextCodec::codecForName("UTF-8"));
        toXml(out, true);
        out.flush();

        QApplication::restoreOverrideCursor();

        file.close();
        temp.moveToTarget();
        return true;

    } catch (FileOperationFailed &f) {
        
        QMessageBox::critical(this, tr("Failed to write file"),
                              tr("<b>Save failed</b><p>Failed to write to file \"%1\": %2")
                              .arg(path).arg(f.what()));
        return false;
    }
}

void
VisualiserWindowBase::toXml(QTextStream &out, bool asTemplate)
{
    QString indent("  ");

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<!DOCTYPE sonic-visualiser>\n";
    out << "<sv>\n";

    if (asTemplate) {
        m_document->toXmlAsTemplate(out, "", "");
    } else {
        m_document->toXml(out, "", "");
    }

    out << "<display>\n";

    out << QString("  <window width=\"%1\" height=\"%2\"/>\n")
           .arg(width()).arg(height());

    for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {

        Pane *pane = m_paneStack->getPane(i);

        if (pane) {
            pane->toXml(out, indent);
        }
    }

    out << "</display>\n";

    m_viewManager->getSelection().toXml(out);

    out << "</sv>\n";
}

Pane *
VisualiserWindowBase::addPaneToStack()
{
    cerr << "MainWindowBase::addPaneToStack()" << endl;
    AddPaneCommand *command = new AddPaneCommand(this);
    CommandHistory::getInstance()->addCommand(command);
    Pane *pane = command->getPane();
    return pane;
}

void
VisualiserWindowBase::zoomIn()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (currentPane) currentPane->zoom(true);
}

void
VisualiserWindowBase::zoomOut()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (currentPane) currentPane->zoom(false);
}

void
VisualiserWindowBase::zoomToFit()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (!currentPane) return;

    Model *model = getMainModel();
    if (!model) return;
    
    sv_frame_t start = model->getStartFrame();
    sv_frame_t end = model->getEndFrame();
    if (m_playSource) end = std::max(end, m_playSource->getPlayEndFrame());
    int pixels = currentPane->width();

    int sw = currentPane->getVerticalScaleWidth();
    if (pixels > sw * 2) pixels -= sw * 2;
    else pixels = 1;
    if (pixels > 4) pixels -= 4;

    int zoomLevel = int((end - start) / pixels);
    if (zoomLevel < 1) zoomLevel = 1;

    currentPane->setZoomLevel(zoomLevel);
    currentPane->setCentreFrame((start + end) / 2);
}

void
VisualiserWindowBase::zoomDefault()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    QSettings settings;
    settings.beginGroup("MainWindow");
    int zoom = settings.value("zoom-default", 1024).toInt();
    settings.endGroup();
    if (currentPane) currentPane->setZoomLevel(zoom);
}

void
VisualiserWindowBase::scrollLeft()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (currentPane) currentPane->scroll(false, false);
}

void
VisualiserWindowBase::jumpLeft()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (currentPane) currentPane->scroll(false, true);
}

void
VisualiserWindowBase::peekLeft()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (currentPane) currentPane->scroll(false, false, false);
}

void
VisualiserWindowBase::scrollRight()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (currentPane) currentPane->scroll(true, false);
}

void
VisualiserWindowBase::jumpRight()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (currentPane) currentPane->scroll(true, true);
}

void
VisualiserWindowBase::peekRight()
{
    Pane *currentPane = m_paneStack->getCurrentPane();
    if (currentPane) currentPane->scroll(true, false, false);
}

void
VisualiserWindowBase::showNoOverlays()
{
    m_viewManager->setOverlayMode(ViewManager::NoOverlays);
}

void
VisualiserWindowBase::showMinimalOverlays()
{
    m_viewManager->setOverlayMode(ViewManager::StandardOverlays);
}

void
VisualiserWindowBase::showAllOverlays()
{
    m_viewManager->setOverlayMode(ViewManager::AllOverlays);
}

void
VisualiserWindowBase::toggleTimeRulers()
{
    bool haveRulers = false;
    bool someHidden = false;

    for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {

        Pane *pane = m_paneStack->getPane(i);
        if (!pane) continue;

        for (int j = 0; j < pane->getLayerCount(); ++j) {

            Layer *layer = pane->getLayer(j);
            if (!dynamic_cast<TimeRulerLayer *>(layer)) continue;

            haveRulers = true;
            if (layer->isLayerDormant(pane)) someHidden = true;
        }
    }

    if (haveRulers) {

        bool show = someHidden;

        for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {

            Pane *pane = m_paneStack->getPane(i);
            if (!pane) continue;

            for (int j = 0; j < pane->getLayerCount(); ++j) {

                Layer *layer = pane->getLayer(j);
                if (!dynamic_cast<TimeRulerLayer *>(layer)) continue;

                layer->showLayer(pane, show);
            }
        }
    }
}

void
VisualiserWindowBase::toggleZoomWheels()
{
    if (m_viewManager->getZoomWheelsEnabled()) {
        m_viewManager->setZoomWheelsEnabled(false);
    } else {
        m_viewManager->setZoomWheelsEnabled(true);
    }
}

void
VisualiserWindowBase::togglePropertyBoxes()
{
    if (m_paneStack->getLayoutStyle() == PaneStack::NoPropertyStacks) {
        if (Preferences::getInstance()->getPropertyBoxLayout() ==
                Preferences::VerticallyStacked) {
            m_paneStack->setLayoutStyle(PaneStack::PropertyStackPerPaneLayout);
        } else {
            m_paneStack->setLayoutStyle(PaneStack::SinglePropertyStackLayout);
        }
    } else {
        m_paneStack->setLayoutStyle(PaneStack::NoPropertyStacks);
    }
}

QLabel *
VisualiserWindowBase::getStatusLabel() const
{
    if (!m_statusLabel) {
        m_statusLabel = new QLabel();
        statusBar()->addWidget(m_statusLabel, 1);
    }
    QList<QFrame *> frames = statusBar()->findChildren<QFrame *>();
    foreach (QFrame *f, frames) {
        f->setFrameStyle(QFrame::NoFrame);
    }
    return m_statusLabel;
}

void VisualiserWindowBase::toggleStatusBar()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    bool sb = settings.value("showstatusbar", true).toBool();
    if (sb) {
        statusBar()->hide();
    } else {
        statusBar()->show();
    }
    settings.setValue("showstatusbar", !sb);
    settings.endGroup();
}

void VisualiserWindowBase::toggleCentreLine()
{
    if (m_viewManager->shouldShowCentreLine()) {
        m_viewManager->setShowCentreLine(false);
    } else {
        m_viewManager->setShowCentreLine(true);
    }
}

void VisualiserWindowBase::preferenceChanged(PropertyContainer::PropertyName name)
{
    if (name == "Property Box Layout") {
        if (m_paneStack->getLayoutStyle() != PaneStack::NoPropertyStacks) {
            if (Preferences::getInstance()->getPropertyBoxLayout() ==
                    Preferences::VerticallyStacked) {
                m_paneStack->setLayoutStyle(PaneStack::PropertyStackPerPaneLayout);
            } else {
                m_paneStack->setLayoutStyle(PaneStack::SinglePropertyStackLayout);
            }
        }
    } else if (name == "Background Mode" && m_viewManager) {
        Preferences::BackgroundMode mode =
                Preferences::getInstance()->getBackgroundMode();
        if (mode == Preferences::BackgroundFromTheme) {
            m_viewManager->setGlobalDarkBackground(m_initialDarkBackground);
        } else if (mode == Preferences::DarkBackground) {
            m_viewManager->setGlobalDarkBackground(true);
        } else {
            m_viewManager->setGlobalDarkBackground(false);
        }
    }
}

void VisualiserWindowBase::play()
{
    if (m_playSource->isPlaying()) {
        stop();
    } else {
        playbackFrameChanged(m_viewManager->getPlaybackFrame());
        m_playSource->play(m_viewManager->getPlaybackFrame());
    }
}

void VisualiserWindowBase::ffwd()
{
    if (!getMainModel()) return;
    sv_frame_t frame = m_viewManager->getPlaybackFrame();
    ++frame;
    Pane *pane = m_paneStack->getCurrentPane();
    Layer *layer = getSnapLayer();
    sv_samplerate_t sr = getMainModel()->getSampleRate();
    if (!layer) {
        frame = RealTime::realTime2Frame
                (RealTime::frame2RealTime(frame, sr) + m_defaultFfwdRwdStep, sr);
        if (frame > getMainModel()->getEndFrame()) {
            frame = getMainModel()->getEndFrame();
        }
    } else {
        int resolution = 0;
        if (pane) frame = pane->alignFromReference(frame);
        if (layer->snapToFeatureFrame(m_paneStack->getCurrentPane(),
                                      frame, resolution, Layer::SnapRight)) {
            if (pane) frame = pane->alignToReference(frame);
        } else {
            frame = getMainModel()->getEndFrame();
        }
    }
    if (frame < 0) frame = 0;
    if (m_viewManager->getPlaySelectionMode()) {
        frame = m_viewManager->constrainFrameToSelection(frame);
    }
    m_viewManager->setPlaybackFrame(frame);
    if (frame == getMainModel()->getEndFrame() &&
            m_playSource &&
            m_playSource->isPlaying() &&
            !m_viewManager->getPlayLoopMode()) {
        stop();
    }
}

void VisualiserWindowBase::ffwdEnd()
{
    if (!getMainModel()) return;
    if (m_playSource && m_playSource->isPlaying() && !m_viewManager->getPlayLoopMode()) {
        stop();
    }
    sv_frame_t frame = getMainModel()->getEndFrame();
    if (m_viewManager->getPlaySelectionMode()) {
        frame = m_viewManager->constrainFrameToSelection(frame);
    }
    m_viewManager->setPlaybackFrame(frame);
}

void
VisualiserWindowBase::ffwdSimilar()
{
    if (!getMainModel()) return;

    Layer *layer = getSnapLayer();
    if (!layer) { ffwd(); return; }

    Pane *pane = m_paneStack->getCurrentPane();
    sv_frame_t frame = m_viewManager->getPlaybackFrame();

    int resolution = 0;
    if (pane) frame = pane->alignFromReference(frame);
    if (layer->snapToSimilarFeature(m_paneStack->getCurrentPane(),
                                    frame, resolution, Layer::SnapRight)) {
        if (pane) frame = pane->alignToReference(frame);
    } else {
        frame = getMainModel()->getEndFrame();
    }

    if (frame < 0) frame = 0;

    if (m_viewManager->getPlaySelectionMode()) {
        frame = m_viewManager->constrainFrameToSelection(frame);
    }
    
    m_viewManager->setPlaybackFrame(frame);

    if (frame == getMainModel()->getEndFrame() &&
            m_playSource &&
            m_playSource->isPlaying() &&
            !m_viewManager->getPlayLoopMode()) {
        stop();
    }
}

void
VisualiserWindowBase::rewind()
{
    if (!getMainModel()) return;

    sv_frame_t frame = m_viewManager->getPlaybackFrame();
    if (frame > 0) --frame;

    Pane *pane = m_paneStack->getCurrentPane();
    Layer *layer = getSnapLayer();
    sv_samplerate_t sr = getMainModel()->getSampleRate();
    
    // when rewinding during playback, we want to allow a period
    // following a rewind target point at which the rewind will go to
    // the prior point instead of the immediately neighbouring one
    if (m_playSource && m_playSource->isPlaying()) {
        RealTime ct = RealTime::frame2RealTime(frame, sr);
        ct = ct - RealTime::fromSeconds(0.15);
        if (ct < RealTime::zeroTime) ct = RealTime::zeroTime;
        frame = RealTime::realTime2Frame(ct, sr);
    }

    if (!layer) {
        
        frame = RealTime::realTime2Frame
                (RealTime::frame2RealTime(frame, sr) - m_defaultFfwdRwdStep, sr);
        if (frame < getMainModel()->getStartFrame()) {
            frame = getMainModel()->getStartFrame();
        }

    } else {

        int resolution = 0;
        if (pane) frame = pane->alignFromReference(frame);
        if (layer->snapToFeatureFrame(m_paneStack->getCurrentPane(),
                                      frame, resolution, Layer::SnapLeft)) {
            if (pane) frame = pane->alignToReference(frame);
        } else {
            frame = getMainModel()->getStartFrame();
        }
    }

    if (frame < 0) frame = 0;

    if (m_viewManager->getPlaySelectionMode()) {
        frame = m_viewManager->constrainFrameToSelection(frame);
    }

    m_viewManager->setPlaybackFrame(frame);
}

void
VisualiserWindowBase::rewindStart()
{
    if (!getMainModel()) return;

    sv_frame_t frame = getMainModel()->getStartFrame();

    if (m_viewManager->getPlaySelectionMode()) {
        frame = m_viewManager->constrainFrameToSelection(frame);
    }

    m_viewManager->setPlaybackFrame(frame);
}

void
VisualiserWindowBase::rewindSimilar()
{
    if (!getMainModel()) return;

    Layer *layer = getSnapLayer();
    if (!layer) { rewind(); return; }

    Pane *pane = m_paneStack->getCurrentPane();
    sv_frame_t frame = m_viewManager->getPlaybackFrame();

    int resolution = 0;
    if (pane) frame = pane->alignFromReference(frame);
    if (layer->snapToSimilarFeature(m_paneStack->getCurrentPane(),
                                    frame, resolution, Layer::SnapLeft)) {
        if (pane) frame = pane->alignToReference(frame);
    } else {
        frame = getMainModel()->getStartFrame();
    }

    if (frame < 0) frame = 0;

    if (m_viewManager->getPlaySelectionMode()) {
        frame = m_viewManager->constrainFrameToSelection(frame);
    }
    
    m_viewManager->setPlaybackFrame(frame);
}

Layer *
VisualiserWindowBase::getSnapLayer() const
{
    Pane *pane = m_paneStack->getCurrentPane();
    if (!pane) return 0;

    Layer *layer = pane->getSelectedLayer();

    if (!dynamic_cast<TimeInstantLayer *>(layer) &&
            !dynamic_cast<TimeValueLayer *>(layer) &&
            !dynamic_cast<RegionLayer *>(layer) &&
            !dynamic_cast<TimeRulerLayer *>(layer)) {

        layer = 0;

        for (int i = pane->getLayerCount(); i > 0; --i) {
            Layer *l = pane->getLayer(i-1);
            if (dynamic_cast<TimeRulerLayer *>(l)) {
                layer = l;
                break;
            }
        }
    }

    return layer;
}

void
VisualiserWindowBase::stop()
{
    m_playSource->stop();

    if (m_paneStack && m_paneStack->getCurrentPane()) {
        updateVisibleRangeDisplay(m_paneStack->getCurrentPane());
    } else {
        m_myStatusMessage = "";
        getStatusLabel()->setText("");
    }
}

VisualiserWindowBase::AddPaneCommand::AddPaneCommand(VisualiserWindowBase *mw) :
    m_mw(mw),
    m_pane(0),
    m_prevCurrentPane(0),
    m_added(false)
{
}

VisualiserWindowBase::AddPaneCommand::~AddPaneCommand()
{
    if (m_pane && !m_added) {
        m_mw->m_paneStack->deletePane(m_pane);
    }
}

QString
VisualiserWindowBase::AddPaneCommand::getName() const
{
    return tr("Add Pane");
}

void
VisualiserWindowBase::AddPaneCommand::execute()
{
    if (!m_pane) {
        m_prevCurrentPane = m_mw->m_paneStack->getCurrentPane();
        m_pane = m_mw->m_paneStack->addPane();

        connect(m_pane, SIGNAL(contextHelpChanged(const QString &)),
                m_mw, SLOT(contextHelpChanged(const QString &)));
    } else {
        m_mw->m_paneStack->showPane(m_pane);
    }

    m_mw->m_paneStack->setCurrentPane(m_pane);
    m_added = true;
}

void
VisualiserWindowBase::AddPaneCommand::unexecute()
{
    m_mw->m_paneStack->hidePane(m_pane);
    m_mw->m_paneStack->setCurrentPane(m_prevCurrentPane);
    m_added = false;
}

VisualiserWindowBase::RemovePaneCommand::RemovePaneCommand(VisualiserWindowBase *mw, Pane *pane) :
    m_mw(mw),
    m_pane(pane),
    m_prevCurrentPane(0),
    m_added(true)
{
}

VisualiserWindowBase::RemovePaneCommand::~RemovePaneCommand()
{
    if (m_pane && !m_added) {
        m_mw->m_paneStack->deletePane(m_pane);
    }
}

QString
VisualiserWindowBase::RemovePaneCommand::getName() const
{
    return tr("Remove Pane");
}

void
VisualiserWindowBase::RemovePaneCommand::execute()
{
    m_prevCurrentPane = m_mw->m_paneStack->getCurrentPane();
    m_mw->m_paneStack->hidePane(m_pane);
    m_added = false;
}

void
VisualiserWindowBase::RemovePaneCommand::unexecute()
{
    m_mw->m_paneStack->showPane(m_pane);
    m_mw->m_paneStack->setCurrentPane(m_prevCurrentPane);
    m_added = true;
}

void
VisualiserWindowBase::deleteCurrentPane()
{
    CommandHistory::getInstance()->startCompoundOperation
            (tr("Delete Pane"), true);

    Pane *pane = m_paneStack->getCurrentPane();
    if (pane) {
        while (pane->getLayerCount() > 0) {
            Layer *layer = pane->getLayer(0);
            if (layer) {
                m_document->removeLayerFromView(pane, layer);
            } else {
                break;
            }
        }

        RemovePaneCommand *command = new RemovePaneCommand(this, pane);
        CommandHistory::getInstance()->addCommand(command);
    }

    CommandHistory::getInstance()->endCompoundOperation();

    updateMenuStates();
}

void
VisualiserWindowBase::deleteCurrentLayer()
{
    Pane *pane = m_paneStack->getCurrentPane();
    if (pane) {
        Layer *layer = pane->getSelectedLayer();
        if (layer) {
            m_document->removeLayerFromView(pane, layer);
        }
    }
    updateMenuStates();
}

void
VisualiserWindowBase::editCurrentLayer()
{
    Layer *layer = 0;
    Pane *pane = m_paneStack->getCurrentPane();
    if (pane) layer = pane->getSelectedLayer();
    if (!layer) return;

    Model *model = layer->getModel();
    if (!model) return;

    TabularModel *tabular = dynamic_cast<TabularModel *>(model);
    if (!tabular) {
        //!!! how to prevent this function from being active if not
        //appropriate model type?  or will we ultimately support
        //tabular display for all editable models?
        cerr << "NOTE: Not a tabular model" << endl;
        return;
    }

    if (m_layerDataDialogMap.find(layer) != m_layerDataDialogMap.end()) {
        if (!m_layerDataDialogMap[layer].isNull()) {
            m_layerDataDialogMap[layer]->show();
            m_layerDataDialogMap[layer]->raise();
            return;
        }
    }

    QString title = layer->getLayerPresentationName();

    ModelDataTableDialog *dialog = new ModelDataTableDialog(tabular, title, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    
    connectLayerEditDialog(dialog);

    m_layerDataDialogMap[layer] = dialog;
    m_viewDataDialogMap[pane].insert(dialog);

    dialog->show();
}

void
VisualiserWindowBase::connectLayerEditDialog(ModelDataTableDialog *dialog)
{
    connect(m_viewManager,
            SIGNAL(globalCentreFrameChanged(sv_frame_t)),
            dialog,
            SLOT(userScrolledToFrame(sv_frame_t)));

    connect(m_viewManager,
            SIGNAL(playbackFrameChanged(sv_frame_t)),
            dialog,
            SLOT(playbackScrolledToFrame(sv_frame_t)));

    connect(dialog,
            SIGNAL(scrollToFrame(sv_frame_t)),
            m_viewManager,
            SLOT(setGlobalCentreFrame(sv_frame_t)));

    connect(dialog,
            SIGNAL(scrollToFrame(sv_frame_t)),
            m_viewManager,
            SLOT(setPlaybackFrame(sv_frame_t)));
}    

void
VisualiserWindowBase::previousPane()
{
    if (!m_paneStack) return;

    Pane *currentPane = m_paneStack->getCurrentPane();
    if (!currentPane) return;

    for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {
        if (m_paneStack->getPane(i) == currentPane) {
            if (i == 0) return;
            m_paneStack->setCurrentPane(m_paneStack->getPane(i-1));
            updateMenuStates();
            return;
        }
    }
}

void
VisualiserWindowBase::nextPane()
{
    if (!m_paneStack) return;

    Pane *currentPane = m_paneStack->getCurrentPane();
    if (!currentPane) return;

    for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {
        if (m_paneStack->getPane(i) == currentPane) {
            if (i == m_paneStack->getPaneCount()-1) return;
            m_paneStack->setCurrentPane(m_paneStack->getPane(i+1));
            updateMenuStates();
            return;
        }
    }
}

void
VisualiserWindowBase::previousLayer()
{
    if (!m_paneStack) return;

    Pane *currentPane = m_paneStack->getCurrentPane();
    if (!currentPane) return;

    int count = currentPane->getLayerCount();
    if (count == 0) return;

    Layer *currentLayer = currentPane->getSelectedLayer();

    if (!currentLayer) {
        // The pane itself is current
        m_paneStack->setCurrentLayer
                (currentPane, currentPane->getFixedOrderLayer(count-1));
    } else {
        for (int i = 0; i < count; ++i) {
            if (currentPane->getFixedOrderLayer(i) == currentLayer) {
                if (i == 0) {
                    m_paneStack->setCurrentLayer
                            (currentPane, 0); // pane
                } else {
                    m_paneStack->setCurrentLayer
                            (currentPane, currentPane->getFixedOrderLayer(i-1));
                }
                break;
            }
        }
    }

    updateMenuStates();
}

void
VisualiserWindowBase::nextLayer()
{
    if (!m_paneStack) return;

    Pane *currentPane = m_paneStack->getCurrentPane();
    if (!currentPane) return;

    int count = currentPane->getLayerCount();
    if (count == 0) return;

    Layer *currentLayer = currentPane->getSelectedLayer();

    if (!currentLayer) {
        // The pane itself is current
        m_paneStack->setCurrentLayer
                (currentPane, currentPane->getFixedOrderLayer(0));
    } else {
        for (int i = 0; i < count; ++i) {
            if (currentPane->getFixedOrderLayer(i) == currentLayer) {
                if (i == currentPane->getLayerCount()-1) {
                    m_paneStack->setCurrentLayer
                            (currentPane, 0); // pane
                } else {
                    m_paneStack->setCurrentLayer
                            (currentPane, currentPane->getFixedOrderLayer(i+1));
                }
                break;
            }
        }
    }

    updateMenuStates();
}

void
VisualiserWindowBase::playbackFrameChanged(sv_frame_t frame)
{
    if (!(m_playSource && m_playSource->isPlaying()) || !getMainModel()) return;

    updatePositionStatusDisplays();

    RealTime now = RealTime::frame2RealTime
            (frame, getMainModel()->getSampleRate());

    if (now.sec == m_lastPlayStatusSec) return;

    RealTime then = RealTime::frame2RealTime
            (m_playSource->getPlayEndFrame(), getMainModel()->getSampleRate());

    QString nowStr;
    QString thenStr;
    QString remainingStr;

    if (then.sec > 10) {
        nowStr = now.toSecText().c_str();
        thenStr = then.toSecText().c_str();
        remainingStr = (then - now).toSecText().c_str();
        m_lastPlayStatusSec = now.sec;
    } else {
        nowStr = now.toText(true).c_str();
        thenStr = then.toText(true).c_str();
        remainingStr = (then - now).toText(true).c_str();
    }

    m_myStatusMessage = tr("Playing: %1 of %2 (%3 remaining)")
            .arg(nowStr).arg(thenStr).arg(remainingStr);

    getStatusLabel()->setText(m_myStatusMessage);
}

void
VisualiserWindowBase::globalCentreFrameChanged(sv_frame_t )
{
    if ((m_playSource && m_playSource->isPlaying()) || !getMainModel()) return;
    Pane *p = 0;
    if (!m_paneStack || !(p = m_paneStack->getCurrentPane())) return;
    if (!p->getFollowGlobalPan()) return;
    updateVisibleRangeDisplay(p);
}

void
VisualiserWindowBase::viewCentreFrameChanged(View *v, sv_frame_t frame)
{
    //    cerr << "MainWindowBase::viewCentreFrameChanged(" << v << "," << frame << ")" << endl;

    if (m_viewDataDialogMap.find(v) != m_viewDataDialogMap.end()) {
        for (DataDialogSet::iterator i = m_viewDataDialogMap[v].begin();
             i != m_viewDataDialogMap[v].end(); ++i) {
            (*i)->userScrolledToFrame(frame);
        }
    }
    if ((m_playSource && m_playSource->isPlaying()) || !getMainModel()) return;
    Pane *p = 0;
    if (!m_paneStack || !(p = m_paneStack->getCurrentPane())) return;
    if (v == p) updateVisibleRangeDisplay(p);
}

void
VisualiserWindowBase::viewZoomLevelChanged(View *v, int , bool )
{
    if ((m_playSource && m_playSource->isPlaying()) || !getMainModel()) return;
    Pane *p = 0;
    if (!m_paneStack || !(p = m_paneStack->getCurrentPane())) return;
    if (v == p) updateVisibleRangeDisplay(p);
}

void
VisualiserWindowBase::layerAdded(Layer *)
{
    //    cerr << "MainWindowBase::layerAdded(" << layer << ")" << endl;
    updateMenuStates();
}

void
VisualiserWindowBase::layerRemoved(Layer *)
{
    //    cerr << "MainWindowBase::layerRemoved(" << layer << ")" << endl;
    updateMenuStates();
}

void
VisualiserWindowBase::layerAboutToBeDeleted(Layer *layer)
{
    //    cerr << "MainWindowBase::layerAboutToBeDeleted(" << layer << ")" << endl;

    removeLayerEditDialog(layer);

    if (m_timeRulerLayer && (layer == m_timeRulerLayer)) {
        //	cerr << "(this is the time ruler layer)" << endl;
        m_timeRulerLayer = 0;
    }
}

void
VisualiserWindowBase::layerInAView(Layer *layer, bool inAView)
{
    //    cerr << "MainWindowBase::layerInAView(" << layer << "," << inAView << ")" << endl;

    if (!inAView) removeLayerEditDialog(layer);

    // Check whether we need to add or remove model from play source
    Model *model = layer->getModel();
    if (model) {
        if (inAView) {
            m_playSource->addModel(model);
        } else {
            bool found = false;
            for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {
                Pane *pane = m_paneStack->getPane(i);
                if (!pane) continue;
                for (int j = 0; j < pane->getLayerCount(); ++j) {
                    Layer *pl = pane->getLayer(j);
                    if (pl &&
                            !dynamic_cast<TimeRulerLayer *>(pl) &&
                            (pl->getModel() == model)) {
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            if (!found) {
                m_playSource->removeModel(model);
            }
        }
    }

    updateMenuStates();
}

void
VisualiserWindowBase::removeLayerEditDialog(Layer *layer)
{
    if (m_layerDataDialogMap.find(layer) != m_layerDataDialogMap.end()) {

        ModelDataTableDialog *dialog = m_layerDataDialogMap[layer];

        for (ViewDataDialogMap::iterator vi = m_viewDataDialogMap.begin();
             vi != m_viewDataDialogMap.end(); ++vi) {
            vi->second.erase(dialog);
        }

        m_layerDataDialogMap.erase(layer);
        delete dialog;
    }
}

void
VisualiserWindowBase::modelAdded(Model *model)
{
    //    cerr << "MainWindowBase::modelAdded(" << model << ")" << endl;
    std::cerr << "\nAdding model " << model->getTypeName() << " to playsource " << std::endl;
    m_playSource->addModel(model);
}

void
VisualiserWindowBase::mainModelChanged(WaveFileModel *model)
{
    //    cerr << "MainWindowBase::mainModelChanged(" << model << ")" << endl;
    updateDescriptionLabel();
    if (model) m_viewManager->setMainModelSampleRate(model->getSampleRate());
    if (model && !m_playTarget && m_audioOutput) {
        createPlayTarget();
    }
}

void
VisualiserWindowBase::modelAboutToBeDeleted(Model *model)
{
    //    cerr << "MainWindowBase::modelAboutToBeDeleted(" << model << ")" << endl;
    if (model == m_viewManager->getPlaybackModel()) {
        m_viewManager->setPlaybackModel(0);
    }
    m_playSource->removeModel(model);
    FFTDataServer::modelAboutToBeDeleted(model);
}

void
VisualiserWindowBase::paneDeleteButtonClicked(Pane *pane)
{
    bool found = false;
    for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {
        if (m_paneStack->getPane(i) == pane) {
            found = true;
            break;
        }
    }
    if (!found) {
        cerr << "MainWindowBase::paneDeleteButtonClicked: Unknown pane "
                << pane << endl;
        return;
    }

    CommandHistory::getInstance()->startCompoundOperation
            (tr("Delete Pane"), true);

    while (pane->getLayerCount() > 0) {
        Layer *layer = pane->getLayer(0);
        if (layer) {
            m_document->removeLayerFromView(pane, layer);
        } else {
            break;
        }
    }

    RemovePaneCommand *command = new RemovePaneCommand(this, pane);
    CommandHistory::getInstance()->addCommand(command);

    CommandHistory::getInstance()->endCompoundOperation();

    updateMenuStates();
}

void
VisualiserWindowBase::pollOSC()
{
    if (!m_oscQueue || m_oscQueue->isEmpty()) return;
    cerr << "MainWindowBase::pollOSC: have " << m_oscQueue->getMessagesAvailable() << " messages" << endl;

    if (m_openingAudioFile) return;

    OSCMessage message = m_oscQueue->readMessage();

    if (message.getTarget() != 0) {
        return; //!!! for now -- this class is target 0, others not handled yet
    }

    handleOSCMessage(message);
}

void
VisualiserWindowBase::inProgressSelectionChanged()
{
    Pane *currentPane = 0;
    if (m_paneStack) currentPane = m_paneStack->getCurrentPane();
    if (currentPane) {
        //cerr << "JTEST: mouse event on selection pane" << endl;
        updateVisibleRangeDisplay(currentPane);
    }
}

void
VisualiserWindowBase::contextHelpChanged(const QString &s)
{
    QLabel *lab = getStatusLabel();

    if (s == "" && m_myStatusMessage != "") {
        if (lab->text() != m_myStatusMessage) {
            lab->setText(m_myStatusMessage);
        }
        return;
    }

    lab->setText(s);
}

void
VisualiserWindowBase::openHelpUrl(QString url)
{
    // This method mostly lifted from Qt Assistant source code

    QProcess *process = new QProcess(this);
    connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));

    QStringList args;

#ifdef Q_OS_MAC
    args.append(url);
    process->start("open", args);
#else
#ifdef Q_OS_WIN32
    QString pf(getenv("ProgramFiles"));
    QString command = pf + QString("\\Internet Explorer\\IEXPLORE.EXE");

    args.append(url);
    process->start(command, args);
#else
    if (!qgetenv("KDE_FULL_SESSION").isEmpty()) {
        args.append("exec");
        args.append(url);
        process->start("kfmclient", args);
    } else if (!qgetenv("BROWSER").isEmpty()) {
        args.append(url);
        process->start(qgetenv("BROWSER"), args);
    } else {
        args.append(url);
        process->start("firefox", args);
    }
#endif
#endif
}


