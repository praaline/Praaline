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

#include "../Version.h"

#include "VisualiserWidget.h"

#include "svcore/base/Preferences.h"
#include "svcore/base/ResourceFinder.h"
#include "svcore/base/PlayParameterRepository.h"
#include "svcore/base/XmlExportable.h"
#include "svcore/base/Clipboard.h"
#include "svcore/base/UnitDatabase.h"

#include "svcore/data/fileio/DataFileReaderFactory.h"
#include "svcore/data/fileio/PlaylistFileReader.h"
#include "svcore/data/fileio/WavFileWriter.h"
#include "svcore/data/fileio/CSVFileWriter.h"
#include "svcore/data/fileio/MIDIFileWriter.h"
#include "svcore/data/fileio/BZipFileDevice.h"
#include "svcore/data/fileio/FileSource.h"
#include "svcore/data/midi/MIDIInput.h"

#include "svcore/data/model/WaveFileModel.h"
#include "svcore/data/model/SparseOneDimensionalModel.h"
#include "svcore/data/model/RangeSummarisableTimeValueModel.h"
#include "svcore/data/model/NoteModel.h"
#include "svcore/data/model/AggregateWaveModel.h"
#include "svcore/data/model/Labeller.h"
#include "svcore/data/osc/OSCQueue.h"

#include "svcore/transform/TransformFactory.h"
#include "svcore/transform/ModelTransformerFactory.h"
#include "svcore/rdf/PluginRDFIndexer.h"
#include "svcore/rdf/RDFExporter.h"

#include "svgui/view/Pane.h"
#include "svgui/view/PaneStack.h"
#include "svgui/view/ViewManager.h"
#include "svgui/view/Overview.h"

#include "svgui/layer/WaveformLayer.h"
#include "svgui/layer/TimeRulerLayer.h"
#include "svgui/layer/TimeInstantLayer.h"
#include "svgui/layer/TimeValueLayer.h"
#include "svgui/layer/NoteLayer.h"
#include "svgui/layer/Colour3DPlotLayer.h"
#include "svgui/layer/SliceLayer.h"
#include "svgui/layer/SliceableLayer.h"
#include "svgui/layer/ImageLayer.h"
#include "svgui/layer/RegionLayer.h"
#include "svgui/layer/ColourDatabase.h"

#include "svgui/widgets/Fader.h"
#include "svgui/widgets/PropertyBox.h"
#include "svgui/widgets/PropertyStack.h"
#include "svgui/widgets/AudioDial.h"
#include "svgui/widgets/IconLoader.h"
#include "svgui/widgets/LayerTreeDialog.h"
#include "svgui/widgets/ListInputDialog.h"
#include "svgui/widgets/SubdividingMenu.h"
#include "svgui/widgets/NotifyingPushButton.h"
#include "svgui/widgets/KeyReference.h"
#include "svgui/widgets/TransformFinder.h"
#include "svgui/widgets/LabelCounterInputDialog.h"
#include "svgui/widgets/KeyReference.h"
#include "svgui/widgets/ActivityLog.h"
#include "svgui/widgets/CommandHistory.h"
#include "svgui/widgets/ModelDataTableDialog.h"

#include "svapp/framework/Document.h"
#include "svapp/framework/TransformUserConfigurator.h"

#include "svapp/audioio/AudioCallbackPlaySource.h"
#include "svapp/audioio/AudioCallbackPlayTarget.h"
#include "svapp/audioio/PlaySpeedRangeMapper.h"

#include "ExportVisualisationDialog.h"
#include "NetworkPermissionTester.h"
#include "SimpleVisualiserWidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QGridLayout>
#include <QLabel>
#include <QAction>
#include <QMenuBar>
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
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileSystemWatcher>

#include <iostream>
#include <cstdio>
#include <errno.h>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusRecording.h"
#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/PointTier.h"
using namespace Praaline::Core;

#include "pngui/model/annotation/AnnotationGridModel.h"
#include "pngui/model/annotation/ProsogramModel.h"
#include "pngui/layer/AnnotationGridLayer.h"
#include "pngui/layer/ProsogramLayer.h"

#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

using std::vector;
using std::map;
using std::set;


VisualiserWidget::VisualiserWidget(const QString &contextStringID, bool withAudioOutput, bool withOSCSupport) :
    SimpleVisualiserWidget(contextStringID, withAudioOutput, withOSCSupport),
    m_mainMenusCreated(false),
    m_paneMenu(0),
    m_layerMenu(0),
    m_transformsMenu(0),
    m_existingLayersMenu(0),
    m_sliceMenu(0),
    m_recentTransformsMenu(0),
    m_templatesMenu(0),
    m_rightButtonLayerMenu(0),
    m_rightButtonTransformsMenu(0),
    m_layerTreeDialog(0),
    m_templateWatcher(0),
    m_playControlsSpacer(0),
    m_playControlsWidth(0),
    m_deleteSelectedAction(0)
{
    setCentralWidget(m_visualiserFrame);

    QGridLayout *layout = new QGridLayout;

    m_playControlsSpacer = new QFrame;

    layout->setSpacing(4);
    layout->addWidget(m_mainScroll, 0, 0, 1, 5);
    layout->addWidget(m_overview, 1, 0, 1, 2);
    layout->addWidget(m_playControlsSpacer, 1, 2);
    layout->addWidget(m_playSpeed, 1, 3);
    layout->addWidget(m_fader, 1, 4);

    m_playControlsWidth = m_fader->width() + m_playSpeed->width() + layout->spacing() * 2;

    layout->setColumnMinimumWidth(0, 14);
    layout->setColumnStretch(0, 0);

    m_paneStack->setPropertyStackMinWidth(m_playControlsWidth + 2 + layout->spacing());
    m_playControlsSpacer->setFixedSize(QSize(2, 2));

    layout->setColumnStretch(1, 10);

    connect(m_paneStack, SIGNAL(propertyStacksResized(int)), this, SLOT(propertyStacksResized(int)));

    m_visualiserFrame->setLayout(layout);

    setupMenus();
    setupToolbars();

    statusBar();
    m_currentLabel = new QLabel;
    statusBar()->addPermanentWidget(m_currentLabel);

    newSession();

    connect(m_midiInput, SIGNAL(eventsAvailable()), this, SLOT(midiEventsAvailable()));

    NetworkPermissionTester tester;
    bool networkPermission = tester.havePermission();
    if (networkPermission) {
        if (withOSCSupport) {
            startOSCQueue();
        }
        TransformFactory::getInstance()->startPopulationThread();
    }
}

VisualiserWidget::~VisualiserWidget()
{
    //    cerr << "VisualiserWidget::~VisualiserWidget" << endl;
    delete m_layerTreeDialog;
    Profiles::getInstance()->dump();
    //    cerr << "VisualiserWidget::~VisualiserWidget finishing" << endl;
}

void
VisualiserWidget::setupMenus()
{
    if (!m_mainMenusCreated) {

#ifdef Q_OS_LINUX
        // In Ubuntu 14.04 the window's menu bar goes missing entirely
        // if the user is running any desktop environment other than Unity
        // (in which the faux single-menubar appears). The user has a
        // workaround, to remove the appmenu-qt5 package, but that is
        // awkward and the problem is so severe that it merits disabling
        // the system menubar integration altogether. Like this:
        menuBar()->setNativeMenuBar(false);  // fix #1039
#endif

        m_rightButtonMenu = new QMenu();

        // No -- we don't want tear-off enabled on the right-button
        // menu.  If it is enabled, then simply right-clicking and
        // releasing will pop up the menu, activate the tear-off, and
        // leave the torn-off menu window in front of the main window.
        // That isn't desirable.  I'm not sure it ever would be, in a
        // context menu -- perhaps technically a Qt bug?
        //        m_rightButtonMenu->setTearOffEnabled(true);
    }

    if (m_rightButtonTransformsMenu) {
        m_rightButtonTransformsMenu->clear();
    } else {
        m_rightButtonTransformsMenu = m_rightButtonMenu->addMenu(tr("&Transform"));
        m_rightButtonTransformsMenu->setTearOffEnabled(true);
        m_rightButtonMenu->addSeparator();
    }

    // This will be created (if not found) or cleared (if found) in
    // setupPaneAndLayerMenus, but we want to ensure it's created
    // sooner so it can go nearer the top of the right button menu
    if (m_rightButtonLayerMenu) {
        m_rightButtonLayerMenu->clear();
    } else {
        m_rightButtonLayerMenu = m_rightButtonMenu->addMenu(tr("&Layer"));
        m_rightButtonLayerMenu->setTearOffEnabled(true);
        m_rightButtonMenu->addSeparator();
    }

    if (!m_mainMenusCreated) {
        CommandHistory::getInstance()->registerMenu(m_rightButtonMenu);
        m_rightButtonMenu->addSeparator();
    }

    setupFileMenu();
    setupEditMenu();
    setupAnnotationMenu();
    setupViewMenu();
    setupPaneAndLayerMenus();
    setupTransformsMenu();

    m_mainMenusCreated = true;
}

void VisualiserWidget::goFullScreen()
{
    if (m_viewManager->getZoomWheelsEnabled()) {
        // The wheels seem to end up in the wrong place in full-screen mode
        toggleZoomWheels();
    }
    QWidget *ps = m_mainScroll->takeWidget();
    ps->setParent(0);
    QShortcut *sc;
    sc = new QShortcut(QKeySequence("Esc"), ps);
    connect(sc, SIGNAL(activated()), this, SLOT(endFullScreen()));
    sc = new QShortcut(QKeySequence("F11"), ps);
    connect(sc, SIGNAL(activated()), this, SLOT(endFullScreen()));
    QAction *acts[] = {
        m_playAction, m_zoomInAction, m_zoomOutAction, m_zoomFitAction,
        m_scrollLeftAction, m_scrollRightAction, m_showPropertyBoxesAction
    };
    for (int i = 0; i < int(sizeof(acts)/sizeof(acts[0])); ++i) {
        sc = new QShortcut(acts[i]->shortcut(), ps);
        connect(sc, SIGNAL(activated()), acts[i], SLOT(trigger()));
    }
    ps->showFullScreen();
}

void VisualiserWidget::endFullScreen()
{
    // these were only created in goFullScreen:
    QObjectList cl = m_paneStack->children();
    foreach (QObject *o, cl) {
        QShortcut *sc = qobject_cast<QShortcut *>(o);
        if (sc) delete sc;
    }
    m_paneStack->showNormal();
    m_mainScroll->setWidget(m_paneStack);
}

void VisualiserWidget::setupFileMenu()
{
    if (m_mainMenusCreated) return;

    Command *command;
    IconLoader il;
    bool existed;
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(m_contextStringID));
    ActionContainer* menubar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer* menu_visualisation = ACTION_MANAGER->createMenu(tr("V&isualisation"), existed);
    if (!existed) menubar->addMenu(menu_visualisation, tr("&Playback"));

    QToolBar *toolbar = addToolBar(tr("Visualisation Toolbar"));

    m_keyReference->setCategory(tr("File and Session Management"));

    QIcon icon = il.load("filenew");
    QAction *action = new QAction(icon, tr("&New Session"), this);
    action->setStatusTip(tr("Abandon the current visualiser session and start a new one"));
    connect(action, SIGNAL(triggered()), this, SLOT(newSession()));
    m_keyReference->registerShortcut(action);
    toolbar->addAction(action);
    command = ACTION_MANAGER->registerAction("Visualisation.NewSession", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    icon = il.load("fileopen");
    action = new QAction(icon, tr("&Open Session..."), this);
    action->setStatusTip(tr("Open a session file, audio file, or layer"));
    connect(action, SIGNAL(triggered()), this, SLOT(openSomething()));
    m_keyReference->registerShortcut(action);
    toolbar->addAction(action);
    command = ACTION_MANAGER->registerAction("Visualisation.OpenSession", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    icon = il.load("fileopen");
    action = new QAction(icon, tr("&Export..."), this);
    action->setStatusTip(tr("Export Visualisation"));
    connect(action, SIGNAL(triggered()), this, SLOT(exportVisualisation()));
    m_keyReference->registerShortcut(action);
    toolbar->addAction(action);


    // We want this one to go on the toolbar now, if we add it at all, but on the menu later
    QAction *iaction = new QAction(tr("&Import More Audio..."), this);
    iaction->setShortcut(tr("Ctrl+I"));
    iaction->setStatusTip(tr("Import an extra audio file into a new pane"));
    connect(iaction, SIGNAL(triggered()), this, SLOT(importMoreAudio()));
    connect(this, SIGNAL(canImportMoreAudio(bool)), iaction, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(iaction);

    // We want this one to go on the toolbar now, if we add it at all, but on the menu later
    QAction *raction = new QAction(tr("Replace &Main Audio..."), this);
    raction->setStatusTip(tr("Replace the main audio file of the session with a different file"));
    connect(raction, SIGNAL(triggered()), this, SLOT(replaceMainAudio()));
    connect(this, SIGNAL(canReplaceMainAudio(bool)), raction, SLOT(setEnabled(bool)));

    action = new QAction(tr("Open Lo&cation..."), this);
    action->setShortcut(tr("Ctrl+Shift+O"));
    action->setStatusTip(tr("Open or import a file from a remote URL"));
    connect(action, SIGNAL(triggered()), this, SLOT(openLocation()));
    m_keyReference->registerShortcut(action);
    command = ACTION_MANAGER->registerAction("Visualisation.OpenLocation", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    menu_visualisation->addSeparator();

    icon = il.load("filesave");
    action = new QAction(icon, tr("&Save Session"), this);
    action->setShortcut(tr("Ctrl+S"));
    action->setStatusTip(tr("Save the current session into a %1 session file").arg(QApplication::applicationName()));
    connect(action, SIGNAL(triggered()), this, SLOT(saveSession()));
    connect(this, SIGNAL(canSave(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    toolbar->addAction(action);
    command = ACTION_MANAGER->registerAction("Visualisation.SaveSession", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    icon = il.load("filesaveas");
    action = new QAction(icon, tr("Save Session &As..."), this);
    action->setShortcut(tr("Ctrl+Shift+S"));
    action->setStatusTip(tr("Save the current session into a new %1 session file").arg(QApplication::applicationName()));
    connect(action, SIGNAL(triggered()), this, SLOT(saveSessionAs()));
    toolbar->addAction(action);
    command = ACTION_MANAGER->registerAction("Visualisation.SaveSessionAs", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    menu_visualisation->addSeparator();

    /*
    icon = il.load("fileopenaudio");
    action = new QAction(icon, tr("&Import Audio File..."), this);
    action->setShortcut(tr("Ctrl+I"));
    action->setStatusTip(tr("Import an existing audio file"));
    connect(action, SIGNAL(triggered()), this, SLOT(importAudio()));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);
*/

    // the Replace action we made earlier
    command = ACTION_MANAGER->registerAction("Visualisation.ReplaceAudio", raction, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    // the Import action we made earlier
    command = ACTION_MANAGER->registerAction("Visualisation.ImportMoreAudio", iaction, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    action = new QAction(tr("&Export Audio File..."), this);
    action->setStatusTip(tr("Export selection as an audio file"));
    connect(action, SIGNAL(triggered()), this, SLOT(exportAudio()));
    connect(this, SIGNAL(canExportAudio(bool)), action, SLOT(setEnabled(bool)));
    command = ACTION_MANAGER->registerAction("Visualisation.ExportAudio", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    action = new QAction(tr("Export Audio Data..."), this);
    action->setStatusTip(tr("Export audio from selection into a data file"));
    connect(action, SIGNAL(triggered()), this, SLOT(exportAudioData()));
    connect(this, SIGNAL(canExportAudio(bool)), action, SLOT(setEnabled(bool)));
    command = ACTION_MANAGER->registerAction("Visualisation.ExportAudioData", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    menu_visualisation->addSeparator();

    action = new QAction(tr("Import Annotation &Layer..."), this);
    action->setShortcut(tr("Ctrl+L"));
    action->setStatusTip(tr("Import layer data from an existing file"));
    connect(action, SIGNAL(triggered()), this, SLOT(importLayer()));
    connect(this, SIGNAL(canImportLayer(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    command = ACTION_MANAGER->registerAction("Visualisation.ImportAnnotationLayer", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    action = new QAction(tr("Export Annotation La&yer..."), this);
    action->setShortcut(tr("Ctrl+Y"));
    action->setStatusTip(tr("Export layer data to a file"));
    connect(action, SIGNAL(triggered()), this, SLOT(exportLayer()));
    connect(this, SIGNAL(canExportLayer(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    command = ACTION_MANAGER->registerAction("Visualisation.ExportAnnotationLayer", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    menu_visualisation->addSeparator();

    action = new QAction(tr("Export Image File..."), this);
    action->setStatusTip(tr("Export a single pane to an image file"));
    connect(action, SIGNAL(triggered()), this, SLOT(exportVisualisation()));
    connect(this, SIGNAL(canExportImage(bool)), action, SLOT(setEnabled(bool)));
    command = ACTION_MANAGER->registerAction("Visualisation.ExportImage", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    menu_visualisation->addSeparator();

    QString templatesMenuLabel = tr("Apply Session Template");
    m_templatesMenu = menu_visualisation->menu()->addMenu(templatesMenuLabel);
    m_templatesMenu->setTearOffEnabled(true);
    // We need to have a main model for this option to be useful:
    // canExportAudio captures that
    connect(this, SIGNAL(canExportAudio(bool)), m_templatesMenu, SLOT(setEnabled(bool)));

    // Set up the menu in a moment, after m_manageTemplatesAction constructed

    action = new QAction(tr("Export Session as Template..."), this);
    connect(action, SIGNAL(triggered()), this, SLOT(saveSessionAsTemplate()));
    // We need to have something in the session for this to be useful:
    // canDeleteCurrentLayer captures that
    connect(this, SIGNAL(canExportAudio(bool)), action, SLOT(setEnabled(bool)));
    command = ACTION_MANAGER->registerAction("Visualisation.SaveTemplate", action, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    m_manageTemplatesAction = new QAction(tr("Manage Exported Templates"), this);
    connect(m_manageTemplatesAction, SIGNAL(triggered()), this, SLOT(manageSavedTemplates()));
    command = ACTION_MANAGER->registerAction("Visualisation.ManageTemplates", m_manageTemplatesAction, context);
    command->setCategory(QtilitiesCategory("Visualisation"));
    menu_visualisation->addAction(command);

    setupTemplatesMenu();

    menu_visualisation->addSeparator();
}

void VisualiserWidget::setupEditMenu()
{
    if (m_mainMenusCreated) return;

    QAction *action;
    IconLoader il;
    bool existed;
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(m_contextStringID));
    ActionContainer* menubar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer* menu_visualisation = ACTION_MANAGER->createMenu(tr("V&isualisation"), existed);
    if (!existed) menubar->addMenu(menu_visualisation, tr("&Playback"));

    m_keyReference->setCategory(tr("Editing"));

    action = new QAction(il.load("editcut"), tr("Cu&t"), this);
    action->setShortcut(tr("Ctrl+X"));
    action->setStatusTip(tr("Cut the selection from the current layer to the clipboard"));
    connect(action, SIGNAL(triggered()), this, SLOT(cut()));
    connect(this, SIGNAL(canEditSelection(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    m_rightButtonMenu->addAction(action);
    ACTION_MANAGER->registerAction("Edit.Cut", action, context);

    action = new QAction(il.load("editcopy"), tr("&Copy"), this);
    action->setShortcut(tr("Ctrl+C"));
    action->setStatusTip(tr("Copy the selection from the current layer to the clipboard"));
    connect(action, SIGNAL(triggered()), this, SLOT(copy()));
    connect(this, SIGNAL(canEditSelection(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    m_rightButtonMenu->addAction(action);
    ACTION_MANAGER->registerAction("Edit.Copy", action, context);

    action = new QAction(il.load("editpaste"), tr("&Paste"), this);
    action->setShortcut(tr("Ctrl+V"));
    action->setStatusTip(tr("Paste from the clipboard to the current layer"));
    connect(action, SIGNAL(triggered()), this, SLOT(paste()));
    connect(this, SIGNAL(canPaste(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    m_rightButtonMenu->addAction(action);
    ACTION_MANAGER->registerAction("Edit.Paste", action, context);

    action = new QAction(tr("Paste at Playback Position"), this);
    action->setShortcut(tr("Ctrl+Shift+V"));
    action->setStatusTip(tr("Paste from the clipboard to the current layer, placing the first item at the playback position"));
    connect(action, SIGNAL(triggered()), this, SLOT(pasteAtPlaybackPosition()));
    connect(this, SIGNAL(canPaste(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    //menu->addAction(action);
    m_rightButtonMenu->addAction(action);

    m_deleteSelectedAction = new QAction(tr("&Delete Selected Items"), this);
    m_deleteSelectedAction->setShortcut(tr("Del"));
    m_deleteSelectedAction->setStatusTip(tr("Delete items in current selection from the current layer"));
    connect(m_deleteSelectedAction, SIGNAL(triggered()), this, SLOT(deleteSelected()));
    connect(this, SIGNAL(canDeleteSelection(bool)), m_deleteSelectedAction, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(m_deleteSelectedAction);
    //menu->addAction(m_deleteSelectedAction);
    m_rightButtonMenu->addAction(m_deleteSelectedAction);

    //menu->addSeparator();
    m_rightButtonMenu->addSeparator();

    m_keyReference->setCategory(tr("Selection"));

    action = new QAction(tr("Select &All"), this);
    action->setShortcut(tr("Ctrl+A"));
    action->setStatusTip(tr("Select the whole duration of the current session"));
    connect(action, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(this, SIGNAL(canSelect(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    //menu->addAction(action);
    m_rightButtonMenu->addAction(action);

    action = new QAction(tr("Select &Visible Range"), this);
    action->setShortcut(tr("Ctrl+Shift+A"));
    action->setStatusTip(tr("Select the time range corresponding to the current window width"));
    connect(action, SIGNAL(triggered()), this, SLOT(selectVisible()));
    connect(this, SIGNAL(canSelect(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    //menu->addAction(action);

    action = new QAction(tr("Select to &Start"), this);
    action->setShortcut(tr("Shift+Left"));
    action->setStatusTip(tr("Select from the start of the session to the current playback position"));
    connect(action, SIGNAL(triggered()), this, SLOT(selectToStart()));
    connect(this, SIGNAL(canSelect(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    //menu->addAction(action);

    action = new QAction(tr("Select to &End"), this);
    action->setShortcut(tr("Shift+Right"));
    action->setStatusTip(tr("Select from the current playback position to the end of the session"));
    connect(action, SIGNAL(triggered()), this, SLOT(selectToEnd()));
    connect(this, SIGNAL(canSelect(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    //menu->addAction(action);

    action = new QAction(tr("C&lear Selection"), this);
    action->setShortcut(tr("Esc"));
    action->setStatusTip(tr("Clear the selection"));
    connect(action, SIGNAL(triggered()), this, SLOT(clearSelection()));
    connect(this, SIGNAL(canClearSelection(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    //menu->addAction(action);
    m_rightButtonMenu->addAction(action);

    //menu->addSeparator();
}

void VisualiserWidget::setupAnnotationMenu()
{
//    // ANNOTATION
//    m_keyReference->setCategory(tr("Tapping Time Instants"));

//    action = new QAction(tr("&Insert Instant at Playback Position"), this);
//    action->setShortcut(tr("Enter"));
//    action->setStatusTip(tr("Insert a new time instant at the current playback position, in a new layer if necessary"));
//    connect(action, SIGNAL(triggered()), this, SLOT(insertInstant()));
//    connect(this, SIGNAL(canInsertInstant(bool)), action, SLOT(setEnabled(bool)));
//    m_keyReference->registerShortcut(action);
//    menu->addAction(action);

//    // Laptop shortcut (no keypad Enter key)
//    QString shortcut(tr(";"));
//    connect(new QShortcut(shortcut, this), SIGNAL(activated()),
//            this, SLOT(insertInstant()));
//    m_keyReference->registerAlternativeShortcut(action, shortcut);

//    action = new QAction(tr("Insert Instants at Selection &Boundaries"), this);
//    action->setShortcut(tr("Shift+Enter"));
//    action->setStatusTip(tr("Insert new time instants at the start and end of the current selected regions, in a new layer if necessary"));
//    connect(action, SIGNAL(triggered()), this, SLOT(insertInstantsAtBoundaries()));
//    connect(this, SIGNAL(canInsertInstantsAtBoundaries(bool)), action, SLOT(setEnabled(bool)));
//    m_keyReference->registerShortcut(action);
//    menu->addAction(action);

//    action = new QAction(tr("Insert Item at Selection"), this);
//    action->setShortcut(tr("Ctrl+Shift+Return"));
//    action->setStatusTip(tr("Insert a new note or region item corresponding to the current selection"));
//    connect(action, SIGNAL(triggered()), this, SLOT(insertItemAtSelection()));
//    connect(this, SIGNAL(canInsertItemAtSelection(bool)), action, SLOT(setEnabled(bool)));
//    m_keyReference->registerShortcut(action);
//    menu->addAction(action);

//    menu->addSeparator();

//    QMenu *numberingMenu = menu->addMenu(tr("Number New Instants with"));
//    numberingMenu->setTearOffEnabled(true);
//    QActionGroup *numberingGroup = new QActionGroup(this);

//    Labeller::TypeNameMap types = m_labeller->getTypeNames();
//    for (Labeller::TypeNameMap::iterator i = types.begin(); i != types.end(); ++i) {

//        if (i->first == Labeller::ValueFromLabel ||
//                i->first == Labeller::ValueFromExistingNeighbour) continue;

//        action = new QAction(i->second, this);
//        connect(action, SIGNAL(triggered()), this, SLOT(setInstantsNumbering()));
//        action->setCheckable(true);
//        action->setChecked(m_labeller->getType() == i->first);
//        numberingGroup->addAction(action);
//        numberingMenu->addAction(action);
//        m_numberingActions[action] = (int)i->first;

//        if (i->first == Labeller::ValueFromTwoLevelCounter) {

//            QMenu *cycleMenu = numberingMenu->addMenu(tr("Cycle size"));
//            QActionGroup *cycleGroup = new QActionGroup(this);

//            int cycles[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 16 };
//            for (int i = 0; i < int(sizeof(cycles)/sizeof(cycles[0])); ++i) {
//                action = new QAction(QString("%1").arg(cycles[i]), this);
//                connect(action, SIGNAL(triggered()), this, SLOT(setInstantsCounterCycle()));
//                action->setCheckable(true);
//                action->setChecked(cycles[i] == m_labeller->getCounterCycleSize());
//                cycleGroup->addAction(action);
//                cycleMenu->addAction(action);
//            }
//        }

//        if (i->first == Labeller::ValueNone ||
//                i->first == Labeller::ValueFromTwoLevelCounter ||
//                i->first == Labeller::ValueFromRealTime) {
//            numberingMenu->addSeparator();
//        }
//    }

//    action = new QAction(tr("Reset Numbering Counters"), this);
//    action->setStatusTip(tr("Reset to 1 all the counters used for counter-based labelling"));
//    connect(action, SIGNAL(triggered()), this, SLOT(resetInstantsCounters()));
//    connect(this, SIGNAL(replacedDocument()), action, SLOT(trigger()));
//    menu->addAction(action);

//    action = new QAction(tr("Set Numbering Counters..."), this);
//    action->setStatusTip(tr("Set the counters used for counter-based labelling"));
//    connect(action, SIGNAL(triggered()), this, SLOT(setInstantsCounters()));
//    menu->addAction(action);

//    action = new QAction(tr("Renumber Selected Instants"), this);
//    action->setStatusTip(tr("Renumber the selected instants using the current labelling scheme"));
//    connect(action, SIGNAL(triggered()), this, SLOT(renumberInstants()));
//    connect(this, SIGNAL(canRenumberInstants(bool)), action, SLOT(setEnabled(bool)));
//    //    m_keyReference->registerShortcut(action);
//    menu->addAction(action);
}


void VisualiserWidget::setupViewMenu()
{
    if (m_mainMenusCreated) return;

    IconLoader il;
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID("Context.VisualisationMode"));

    QAction *action = 0;

    m_keyReference->setCategory(tr("Panning and Navigation"));

    bool existed;
    ActionContainer* view_menu = ACTION_MANAGER->createMenu(qti_action_VIEW, existed);
    QMenu *menu = view_menu->menu();

    menu->setTearOffEnabled(true);
    m_scrollLeftAction = new QAction(tr("Scroll &Left"), this);
    m_scrollLeftAction->setShortcut(tr("Left"));
    m_scrollLeftAction->setStatusTip(tr("Scroll the current pane to the left"));
    connect(m_scrollLeftAction, SIGNAL(triggered()), this, SLOT(scrollLeft()));
    connect(this, SIGNAL(canScroll(bool)), m_scrollLeftAction, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(m_scrollLeftAction);
    menu->addAction(m_scrollLeftAction);

    m_scrollRightAction = new QAction(tr("Scroll &Right"), this);
    m_scrollRightAction->setShortcut(tr("Right"));
    m_scrollRightAction->setStatusTip(tr("Scroll the current pane to the right"));
    connect(m_scrollRightAction, SIGNAL(triggered()), this, SLOT(scrollRight()));
    connect(this, SIGNAL(canScroll(bool)), m_scrollRightAction, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(m_scrollRightAction);
    menu->addAction(m_scrollRightAction);

    action = new QAction(tr("&Jump Left"), this);
    action->setShortcut(tr("Ctrl+Left"));
    action->setStatusTip(tr("Scroll the current pane a big step to the left"));
    connect(action, SIGNAL(triggered()), this, SLOT(jumpLeft()));
    connect(this, SIGNAL(canScroll(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    action = new QAction(tr("J&ump Right"), this);
    action->setShortcut(tr("Ctrl+Right"));
    action->setStatusTip(tr("Scroll the current pane a big step to the right"));
    connect(action, SIGNAL(triggered()), this, SLOT(jumpRight()));
    connect(this, SIGNAL(canScroll(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    action = new QAction(tr("Peek Left"), this);
    action->setShortcut(tr("Alt+Left"));
    action->setStatusTip(tr("Scroll the current pane to the left without moving the playback cursor or other panes"));
    connect(action, SIGNAL(triggered()), this, SLOT(peekLeft()));
    connect(this, SIGNAL(canScroll(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    action = new QAction(tr("Peek Right"), this);
    action->setShortcut(tr("Alt+Right"));
    action->setStatusTip(tr("Scroll the current pane to the right without moving the playback cursor or other panes"));
    connect(action, SIGNAL(triggered()), this, SLOT(peekRight()));
    connect(this, SIGNAL(canScroll(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    menu->addSeparator();

    m_keyReference->setCategory(tr("Zoom"));

    m_zoomInAction = new QAction(il.load("zoom-in"), tr("Zoom &In"), this);
    m_zoomInAction->setShortcut(tr("Up"));
    m_zoomInAction->setStatusTip(tr("Increase the zoom level"));
    connect(m_zoomInAction, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(this, SIGNAL(canZoom(bool)), m_zoomInAction, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(m_zoomInAction);
    menu->addAction(m_zoomInAction);

    m_zoomOutAction = new QAction(il.load("zoom-out"), tr("Zoom &Out"), this);
    m_zoomOutAction->setShortcut(tr("Down"));
    m_zoomOutAction->setStatusTip(tr("Decrease the zoom level"));
    connect(m_zoomOutAction, SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(this, SIGNAL(canZoom(bool)), m_zoomOutAction, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(m_zoomOutAction);
    menu->addAction(m_zoomOutAction);

    action = new QAction(tr("Restore &Default Zoom"), this);
    action->setStatusTip(tr("Restore the zoom level to the default"));
    connect(action, SIGNAL(triggered()), this, SLOT(zoomDefault()));
    connect(this, SIGNAL(canZoom(bool)), action, SLOT(setEnabled(bool)));
    menu->addAction(action);

    m_zoomFitAction = new QAction(il.load("zoom-fit"), tr("Zoom to &Fit"), this);
    m_zoomFitAction->setShortcut(tr("F"));
    m_zoomFitAction->setStatusTip(tr("Zoom to show the whole file"));
    connect(m_zoomFitAction, SIGNAL(triggered()), this, SLOT(zoomToFit()));
    connect(this, SIGNAL(canZoom(bool)), m_zoomFitAction, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(m_zoomFitAction);
    menu->addAction(m_zoomFitAction);

    menu->addSeparator();

    m_keyReference->setCategory(tr("Display Features"));

    action = new QAction(tr("Show &Centre Line"), this);
    action->setShortcut(tr("'"));
    action->setStatusTip(tr("Show or hide the centre line"));
    connect(action, SIGNAL(triggered()), this, SLOT(toggleCentreLine()));
    action->setCheckable(true);
    action->setChecked(m_viewManager->shouldShowCentreLine());
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    action = new QAction(tr("Toggle All Time Rulers"), this);
    action->setShortcut(tr("#"));
    action->setStatusTip(tr("Show or hide all time rulers"));
    connect(action, SIGNAL(triggered()), this, SLOT(toggleTimeRulers()));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    menu->addSeparator();

    QActionGroup *overlayGroup = new QActionGroup(this);

    ViewManager::OverlayMode mode = m_viewManager->getOverlayMode();

    action = new QAction(tr("Show &No Overlays"), this);
    action->setShortcut(tr("0"));
    action->setStatusTip(tr("Hide times, layer names, and scale"));
    connect(action, SIGNAL(triggered()), this, SLOT(showNoOverlays()));
    action->setCheckable(true);
    action->setChecked(mode == ViewManager::NoOverlays);
    overlayGroup->addAction(action);
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    action = new QAction(tr("Show &Minimal Overlays"), this);
    action->setShortcut(tr("9"));
    action->setStatusTip(tr("Show times and basic scale"));
    connect(action, SIGNAL(triggered()), this, SLOT(showMinimalOverlays()));
    action->setCheckable(true);
    action->setChecked(mode == ViewManager::StandardOverlays);
    overlayGroup->addAction(action);
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    action = new QAction(tr("Show &All Overlays"), this);
    action->setShortcut(tr("8"));
    action->setStatusTip(tr("Show times, layer names, and scale"));
    connect(action, SIGNAL(triggered()), this, SLOT(showAllOverlays()));
    action->setCheckable(true);
    action->setChecked(mode == ViewManager::AllOverlays);
    overlayGroup->addAction(action);
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    menu->addSeparator();

    action = new QAction(tr("Show &Zoom Wheels"), this);
    action->setShortcut(tr("Z"));
    action->setStatusTip(tr("Show thumbwheels for zooming horizontally and vertically"));
    connect(action, SIGNAL(triggered()), this, SLOT(toggleZoomWheels()));
    action->setCheckable(true);
    action->setChecked(m_viewManager->getZoomWheelsEnabled());
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    m_showPropertyBoxesAction = new QAction(tr("Show Property Bo&xes"), this);
    m_showPropertyBoxesAction->setShortcut(tr("X"));
    m_showPropertyBoxesAction->setStatusTip(tr("Show the layer property boxes at the side of the main window"));
    connect(m_showPropertyBoxesAction, SIGNAL(triggered()), this, SLOT(togglePropertyBoxes()));
    m_showPropertyBoxesAction->setCheckable(true);
    m_showPropertyBoxesAction->setChecked(true);
    m_keyReference->registerShortcut(m_showPropertyBoxesAction);
    menu->addAction(m_showPropertyBoxesAction);

    action = new QAction(tr("Show Status &Bar"), this);
    action->setStatusTip(tr("Show context help information in the status bar at the bottom of the window"));
    connect(action, SIGNAL(triggered()), this, SLOT(toggleStatusBar()));
    action->setCheckable(true);
    action->setChecked(true);
    menu->addAction(action);

    QSettings settings;
    settings.beginGroup("VisualiserWidget");
    bool sb = settings.value("showstatusbar", true).toBool();
    if (!sb) {
        action->setChecked(false);
        statusBar()->hide();
    }
    settings.endGroup();

    menu->addSeparator();

    action = new QAction(tr("Show La&yer Summary"), this);
    action->setShortcut(tr("Y"));
    action->setStatusTip(tr("Open a window displaying the hierarchy of panes and layers in this session"));
    connect(action, SIGNAL(triggered()), this, SLOT(showLayerTree()));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    menu->addSeparator();

    action = new QAction(tr("Go Full-Screen"), this);
    action->setShortcut(tr("F11"));
    action->setStatusTip(tr("Expand the pane area to the whole screen"));
    connect(action, SIGNAL(triggered()), this, SLOT(goFullScreen()));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);
}

void VisualiserWidget::setupPaneAndLayerMenus()
{
    if (m_paneMenu) {
        m_paneActions.clear();
        m_paneMenu->clear();
    } else {
        m_paneMenu = new QMenu(tr("&Pane"), this);
        m_paneMenu->setTearOffEnabled(true);
    }

    if (m_layerMenu) {
        m_layerActions.clear();
        m_layerMenu->clear();
    } else {
        m_layerMenu = new QMenu(tr("&Layer"), this);
        m_layerMenu->setTearOffEnabled(true);
    }

    if (m_rightButtonLayerMenu) {
        m_rightButtonLayerMenu->clear();
    } else {
        m_rightButtonLayerMenu = m_rightButtonMenu->addMenu(tr("&Layer"));
        m_rightButtonLayerMenu->setTearOffEnabled(true);
        m_rightButtonMenu->addSeparator();
    }

    QMenu *menu = m_paneMenu;

    IconLoader il;

    m_keyReference->setCategory(tr("Managing Panes and Layers"));

    QAction *action = new QAction(il.load("pane"), tr("Add &New Pane"), this);
    action->setShortcut(tr("N"));
    action->setStatusTip(tr("Add a new pane containing only a time ruler"));
    connect(action, SIGNAL(triggered()), this, SLOT(addPane()));
    connect(this, SIGNAL(canAddPane(bool)), action, SLOT(setEnabled(bool)));
    m_paneActions[action] = LayerConfiguration(LayerFactory::Type("TimeRuler"));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    menu->addSeparator();

    menu = m_layerMenu;

    //    menu->addSeparator();

    LayerFactory::LayerTypeSet emptyLayerTypes =
            LayerFactory::getInstance().getValidEmptyLayerTypes();

    for (LayerFactory::LayerTypeSet::iterator i = emptyLayerTypes.begin();
         i != emptyLayerTypes.end(); ++i) {

        QIcon icon;
        QString mainText, tipText, channelText;
        LayerFactory::LayerType type = *i;
        QString name = LayerFactory::getInstance().getLayerPresentationName(type);

        icon = il.load(LayerFactory::getInstance().getLayerIconName(type));

        mainText = tr("Add New %1 Layer").arg(name);
        tipText = tr("Add a new empty layer of type %1").arg(name);

        action = new QAction(icon, mainText, this);
        action->setStatusTip(tipText);

        if (type == LayerFactory::Type("Text")) {
            action->setShortcut(tr("T"));
            m_keyReference->registerShortcut(action);
        }

        connect(action, SIGNAL(triggered()), this, SLOT(addLayer()));
        connect(this, SIGNAL(canAddLayer(bool)), action, SLOT(setEnabled(bool)));
        m_layerActions[action] = LayerConfiguration(type);
        menu->addAction(action);
        m_rightButtonLayerMenu->addAction(action);
    }

    m_rightButtonLayerMenu->addSeparator();
    menu->addSeparator();

    LayerFactory::LayerType backgroundTypes[] = {
        LayerFactory::Type("Waveform"),
        LayerFactory::Type("Spectrogram"),
        LayerFactory::Type("PraatSpectrogram"),
        LayerFactory::Type("MelodicRangeSpectrogram"),
        LayerFactory::Type("PeakFrequencySpectrogram"),
        LayerFactory::Type("Spectrum")
    };

    std::vector<Model *> models;
    if (m_document) models = m_document->getTransformInputModels();
    bool plural = (models.size() > 1);
    if (models.empty()) {
        models.push_back(getMainModel()); // probably 0
    }

    for (unsigned int i = 0;
         i < sizeof(backgroundTypes)/sizeof(backgroundTypes[0]); ++i) {

        const int paneMenuType = 0, layerMenuType = 1;

        for (int menuType = paneMenuType; menuType <= layerMenuType; ++menuType) {

            if (menuType == paneMenuType) menu = m_paneMenu;
            else menu = m_layerMenu;

            QMenu *submenu = 0;

            QIcon icon;
            QString mainText, shortcutText, tipText, channelText;
            LayerFactory::LayerType type = backgroundTypes[i];
            bool mono = true;

            // Avoid warnings/errors with -Wextra because we aren't explicitly
            // handling all layer types (-Wall is OK with this because of the
            // default but the stricter level insists)
#pragma GCC diagnostic ignored "-Wswitch-enum"

            if (type == LayerFactory::Type("Waveform")) {
                icon = il.load("waveform");
                mainText = tr("Add &Waveform");
                if (menuType == paneMenuType) {
                    shortcutText = tr("W");
                    tipText = tr("Add a new pane showing a waveform view");
                } else {
                    shortcutText = tr("Shift+W");
                    tipText = tr("Add a new layer showing a waveform view");
                }
                mono = false;
            }
            else if (type == LayerFactory::Type("Spectrogram")) {
                icon = il.load("spectrogram");
                mainText = tr("Add Spectro&gram");
                if (menuType == paneMenuType) {
                    shortcutText = tr("G");
                    tipText = tr("Add a new pane showing a spectrogram");
                } else {
                    shortcutText = tr("Shift+G");
                    tipText = tr("Add a new layer showing a spectrogram");
                }
            }
            else if (type == LayerFactory::Type("PraatSpectrogram")) {
                icon = il.load("spectrogram");
                mainText = tr("Add &Phonetics Spectrogram");
                if (menuType == paneMenuType) {
                    shortcutText = tr("P");
                    tipText = tr("Add a new pane showing a spectrogram set up for phonetic analysis (Praat settings)");
                } else {
                    shortcutText = tr("Shift+P");
                    tipText = tr("Add a new layer showing a spectrogram set up for phonetic analysis (Praat settings)");
                }
            }
            else if (type == LayerFactory::Type("MelodicRangeSpectrogram")) {
                icon = il.load("spectrogram");
                mainText = tr("Add &Melodic Range Spectrogram");
                if (menuType == paneMenuType) {
                    shortcutText = tr("M");
                    tipText = tr("Add a new pane showing a spectrogram set up for an overview of note pitches");
                } else {
                    shortcutText = tr("Shift+M");
                    tipText = tr("Add a new layer showing a spectrogram set up for an overview of note pitches");
                }
            }
            else if (type == LayerFactory::Type("PeakFrequencySpectrogram")) {
                icon = il.load("spectrogram");
                mainText = tr("Add Pea&k Frequency Spectrogram");
                if (menuType == paneMenuType) {
                    shortcutText = tr("K");
                    tipText = tr("Add a new pane showing a spectrogram set up for tracking frequencies");
                } else {
                    shortcutText = tr("Shift+K");
                    tipText = tr("Add a new layer showing a spectrogram set up for tracking frequencies");
                }
            }
            else if (type == LayerFactory::Type("Spectrum")) {
                icon = il.load("spectrum");
                mainText = tr("Add Spectr&um");
                if (menuType == paneMenuType) {
                    shortcutText = tr("U");
                    tipText = tr("Add a new pane showing a frequency spectrum");
                } else {
                    shortcutText = tr("Shift+U");
                    tipText = tr("Add a new layer showing a frequency spectrum");
                }
            }

            std::vector<Model *> candidateModels = models;

            for (std::vector<Model *>::iterator mi =
                 candidateModels.begin();
                 mi != candidateModels.end(); ++mi) {

                Model *model = *mi;

                int channels = 0;
                if (model) {
                    DenseTimeValueModel *dtvm =
                            dynamic_cast<DenseTimeValueModel *>(model);
                    if (dtvm) channels = dtvm->getChannelCount();
                }
                if (channels < 1 && getMainModel()) {
                    channels = getMainModel()->getChannelCount();
                }
                if (channels < 1) channels = 1;

                for (int c = 0; c <= channels; ++c) {

                    if (c == 1 && channels == 1) continue;
                    bool isDefault = (c == 0);
                    bool isOnly = (isDefault && (channels == 1));

                    if (isOnly && !plural) {

                        action = new QAction(icon, mainText, this);

                        action->setShortcut(shortcutText);
                        action->setStatusTip(tipText);
                        if (menuType == paneMenuType) {
                            connect(action, SIGNAL(triggered()),
                                    this, SLOT(addPane()));
                            connect(this, SIGNAL(canAddPane(bool)),
                                    action, SLOT(setEnabled(bool)));
                            m_paneActions[action] =
                                    LayerConfiguration(type, model);
                        } else {
                            connect(action, SIGNAL(triggered()),
                                    this, SLOT(addLayer()));
                            connect(this, SIGNAL(canAddLayer(bool)),
                                    action, SLOT(setEnabled(bool)));
                            m_layerActions[action] =
                                    LayerConfiguration(type, model);
                        }
                        if (shortcutText != "") {
                            m_keyReference->registerShortcut(action);
                        }
                        menu->addAction(action);

                    } else {

                        if (!submenu) {
                            submenu = menu->addMenu(mainText);
                            submenu->setTearOffEnabled(true);
                        } else if (isDefault) {
                            submenu->addSeparator();
                        }

                        QString actionText;
                        if (c == 0) {
                            if (mono) {
                                actionText = tr("&All Channels Mixed");
                            } else {
                                actionText = tr("&All Channels");
                            }
                        } else {
                            actionText = tr("Channel &%1").arg(c);
                        }

                        if (model) {
                            actionText = tr("%1: %2")
                                    .arg(model->objectName())
                                    .arg(actionText);
                        }

                        if (isDefault) {
                            action = new QAction(icon, actionText, this);
                            if (!model || model == getMainModel()) {
                                action->setShortcut(shortcutText);
                            }
                        } else {
                            action = new QAction(actionText, this);
                        }

                        action->setStatusTip(tipText);

                        if (menuType == paneMenuType) {
                            connect(action, SIGNAL(triggered()),
                                    this, SLOT(addPane()));
                            connect(this, SIGNAL(canAddPane(bool)),
                                    action, SLOT(setEnabled(bool)));
                            m_paneActions[action] =
                                    LayerConfiguration(type, model, c - 1);
                        } else {
                            connect(action, SIGNAL(triggered()),
                                    this, SLOT(addLayer()));
                            connect(this, SIGNAL(canAddLayer(bool)),
                                    action, SLOT(setEnabled(bool)));
                            m_layerActions[action] =
                                    LayerConfiguration(type, model, c - 1);
                        }

                        submenu->addAction(action);
                    }

                    if (isDefault && menuType == layerMenuType &&
                            mi == candidateModels.begin()) {
                        // only add for one model, one channel, one menu on
                        // right button -- the action itself will discover
                        // which model is the correct one (based on pane)
                        action = new QAction(icon, mainText, this);
                        action->setStatusTip(tipText);
                        connect(action, SIGNAL(triggered()),
                                this, SLOT(addLayer()));
                        connect(this, SIGNAL(canAddLayer(bool)),
                                action, SLOT(setEnabled(bool)));
                        m_layerActions[action] = LayerConfiguration(type, 0, 0);
                        m_rightButtonLayerMenu->addAction(action);
                    }
                }
            }
        }
    }

    m_rightButtonLayerMenu->addSeparator();

    menu = m_paneMenu;
    menu->addSeparator();

    action = new QAction(tr("Switch to Previous Pane"), this);
    action->setShortcut(tr("["));
    action->setStatusTip(tr("Make the next pane up in the pane stack current"));
    connect(action, SIGNAL(triggered()), this, SLOT(previousPane()));
    connect(this, SIGNAL(canSelectPreviousPane(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    action = new QAction(tr("Switch to Next Pane"), this);
    action->setShortcut(tr("]"));
    action->setStatusTip(tr("Make the next pane down in the pane stack current"));
    connect(action, SIGNAL(triggered()), this, SLOT(nextPane()));
    connect(this, SIGNAL(canSelectNextPane(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    menu->addSeparator();

    action = new QAction(il.load("editdelete"), tr("&Delete Pane"), this);
    action->setShortcut(tr("Ctrl+Shift+D"));
    action->setStatusTip(tr("Delete the currently active pane"));
    connect(action, SIGNAL(triggered()), this, SLOT(deleteCurrentPane()));
    connect(this, SIGNAL(canDeleteCurrentPane(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    menu = m_layerMenu;

    action = new QAction(il.load("timeruler"), tr("Add &Time Ruler"), this);
    action->setStatusTip(tr("Add a new layer showing a time ruler"));
    connect(action, SIGNAL(triggered()), this, SLOT(addLayer()));
    connect(this, SIGNAL(canAddLayer(bool)), action, SLOT(setEnabled(bool)));
    m_layerActions[action] = LayerConfiguration(LayerFactory::Type("TimeRuler"));
    menu->addAction(action);

    menu->addSeparator();

    m_existingLayersMenu = menu->addMenu(tr("Add &Existing Layer"));
    m_existingLayersMenu->setTearOffEnabled(true);
    m_rightButtonLayerMenu->addMenu(m_existingLayersMenu);

    m_sliceMenu = menu->addMenu(tr("Add S&lice of Layer"));
    m_sliceMenu->setTearOffEnabled(true);
    m_rightButtonLayerMenu->addMenu(m_sliceMenu);

    setupExistingLayersMenus();

    menu->addSeparator();

    action = new QAction(tr("Switch to Previous Layer"), this);
    action->setShortcut(tr("{"));
    action->setStatusTip(tr("Make the previous layer in the pane current"));
    connect(action, SIGNAL(triggered()), this, SLOT(previousLayer()));
    connect(this, SIGNAL(canSelectPreviousLayer(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    action = new QAction(tr("Switch to Next Layer"), this);
    action->setShortcut(tr("}"));
    action->setStatusTip(tr("Make the next layer in the pane current"));
    connect(action, SIGNAL(triggered()), this, SLOT(nextLayer()));
    connect(this, SIGNAL(canSelectNextLayer(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);

    m_rightButtonLayerMenu->addSeparator();
    menu->addSeparator();

    QAction *raction = new QAction(tr("&Rename Layer..."), this);
    raction->setShortcut(tr("R"));
    raction->setStatusTip(tr("Rename the currently active layer"));
    connect(raction, SIGNAL(triggered()), this, SLOT(renameCurrentLayer()));
    connect(this, SIGNAL(canRenameLayer(bool)), raction, SLOT(setEnabled(bool)));
    menu->addAction(raction);
    m_rightButtonLayerMenu->addAction(raction);

    QAction *eaction = new QAction(tr("Edit Layer Data"), this);
    eaction->setShortcut(tr("E"));
    eaction->setStatusTip(tr("Edit the currently active layer as a data grid"));
    connect(eaction, SIGNAL(triggered()), this, SLOT(editCurrentLayer()));
    connect(this, SIGNAL(canEditLayerTabular(bool)), eaction, SLOT(setEnabled(bool)));
    menu->addAction(eaction);
    m_rightButtonLayerMenu->addAction(eaction);

    action = new QAction(il.load("editdelete"), tr("&Delete Layer"), this);
    action->setShortcut(tr("Ctrl+D"));
    action->setStatusTip(tr("Delete the currently active layer"));
    connect(action, SIGNAL(triggered()), this, SLOT(deleteCurrentLayer()));
    connect(this, SIGNAL(canDeleteCurrentLayer(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    menu->addAction(action);
    m_rightButtonLayerMenu->addAction(action);

    m_keyReference->registerShortcut(raction); // rename after delete, so delete layer goes next to delete pane
    m_keyReference->registerShortcut(eaction); // edit also after delete

    // finaliseMenus();
}

void VisualiserWidget::setupTransformsMenu()
{
    if (m_transformsMenu) {
        m_transformActions.clear();
        m_transformActionsReverse.clear();
        m_transformsMenu->clear();
    } else {
        m_transformsMenu = new QMenu(tr("&Transform"));
        m_transformsMenu->setTearOffEnabled(true);
        m_transformsMenu->setSeparatorsCollapsible(true);
    }

    TransformFactory *factory = TransformFactory::getInstance();

    TransformList transforms = factory->getAllTransformDescriptions();
    vector<TransformDescription::Type> types = factory->getAllTransformTypes();

    map<TransformDescription::Type, map<QString, SubdividingMenu *> > categoryMenus;
    map<TransformDescription::Type, map<QString, SubdividingMenu *> > makerMenus;

    map<TransformDescription::Type, SubdividingMenu *> byPluginNameMenus;
    map<TransformDescription::Type, map<QString, QMenu *> > pluginNameMenus;

    set<SubdividingMenu *> pendingMenus;

    m_recentTransformsMenu = m_transformsMenu->addMenu(tr("&Recent Transforms"));
    m_recentTransformsMenu->setTearOffEnabled(true);
    m_rightButtonTransformsMenu->addMenu(m_recentTransformsMenu);
    connect(&m_recentTransforms, SIGNAL(recentChanged()),
            this, SLOT(setupRecentTransformsMenu()));

    m_transformsMenu->addSeparator();
    m_rightButtonTransformsMenu->addSeparator();

    for (vector<TransformDescription::Type>::iterator i = types.begin();
         i != types.end(); ++i) {

        if (i != types.begin()) {
            m_transformsMenu->addSeparator();
            m_rightButtonTransformsMenu->addSeparator();
        }

        QString byCategoryLabel = tr("%1 by Category")
                .arg(factory->getTransformTypeName(*i));
        SubdividingMenu *byCategoryMenu = new SubdividingMenu(byCategoryLabel,
                                                              20, 40);
        byCategoryMenu->setTearOffEnabled(true);
        m_transformsMenu->addMenu(byCategoryMenu);
        m_rightButtonTransformsMenu->addMenu(byCategoryMenu);
        pendingMenus.insert(byCategoryMenu);

        vector<QString> categories = factory->getTransformCategories(*i);

        for (vector<QString>::iterator j = categories.begin();
             j != categories.end(); ++j) {

            QString category = *j;
            if (category == "") category = tr("Unclassified");

            if (categories.size() < 2) {
                categoryMenus[*i][category] = byCategoryMenu;
                continue;
            }

            QStringList components = category.split(" > ");
            QString key;

            for (QStringList::iterator k = components.begin();
                 k != components.end(); ++k) {

                QString parentKey = key;
                if (key != "") key += " > ";
                key += *k;

                if (categoryMenus[*i].find(key) == categoryMenus[*i].end()) {
                    SubdividingMenu *m = new SubdividingMenu(*k, 20, 40);
                    m->setTearOffEnabled(true);
                    pendingMenus.insert(m);
                    categoryMenus[*i][key] = m;
                    if (parentKey == "") {
                        byCategoryMenu->addMenu(m);
                    } else {
                        categoryMenus[*i][parentKey]->addMenu(m);
                    }
                }
            }
        }

        QString byPluginNameLabel = tr("%1 by Plugin Name")
                .arg(factory->getTransformTypeName(*i));
        byPluginNameMenus[*i] = new SubdividingMenu(byPluginNameLabel);
        byPluginNameMenus[*i]->setTearOffEnabled(true);
        m_transformsMenu->addMenu(byPluginNameMenus[*i]);
        m_rightButtonTransformsMenu->addMenu(byPluginNameMenus[*i]);
        pendingMenus.insert(byPluginNameMenus[*i]);

        QString byMakerLabel = tr("%1 by Maker")
                .arg(factory->getTransformTypeName(*i));
        SubdividingMenu *byMakerMenu = new SubdividingMenu(byMakerLabel, 20, 40);
        byMakerMenu->setTearOffEnabled(true);
        m_transformsMenu->addMenu(byMakerMenu);
        m_rightButtonTransformsMenu->addMenu(byMakerMenu);
        pendingMenus.insert(byMakerMenu);

        vector<QString> makers = factory->getTransformMakers(*i);

        for (vector<QString>::iterator j = makers.begin();
             j != makers.end(); ++j) {

            QString maker = *j;
            if (maker == "") maker = tr("Unknown");
            maker.replace(QRegExp(tr(" [\\(<].*$")), "");

            makerMenus[*i][maker] = new SubdividingMenu(maker, 30, 40);
            makerMenus[*i][maker]->setTearOffEnabled(true);
            byMakerMenu->addMenu(makerMenus[*i][maker]);
            pendingMenus.insert(makerMenus[*i][maker]);
        }
    }

    // Names should only be duplicated here if they have the same
    // plugin name, output name and maker but are in different library
    // .so names -- that won't happen often I hope
    std::map<QString, QString> idNameSonameMap;
    std::set<QString> seenNames, duplicateNames;
    for (unsigned int i = 0; i < transforms.size(); ++i) {
        QString name = transforms[i].name;
        if (seenNames.find(name) != seenNames.end()) {
            duplicateNames.insert(name);
        } else {
            seenNames.insert(name);
        }
    }

    for (unsigned int i = 0; i < transforms.size(); ++i) {

        QString name = transforms[i].name;
        if (name == "") name = transforms[i].identifier;

        //        cerr << "Plugin Name: " << name << endl;

        TransformDescription::Type type = transforms[i].type;
        QString typeStr = factory->getTransformTypeName(type);

        QString category = transforms[i].category;
        if (category == "") category = tr("Unclassified");

        QString maker = transforms[i].maker;
        if (maker == "") maker = tr("Unknown");
        maker.replace(QRegExp(tr(" [\\(<].*$")), "");

        QString pluginName = name.section(": ", 0, 0);
        QString output = name.section(": ", 1);

        if (duplicateNames.find(pluginName) != duplicateNames.end()) {
            pluginName = QString("%1 <%2>")
                    .arg(pluginName)
                    .arg(transforms[i].identifier.section(':', 1, 1));
            if (output == "") name = pluginName;
            else name = QString("%1: %2")
                    .arg(pluginName)
                    .arg(output);
        }

        QAction *action = new QAction(tr("%1...").arg(name), this);
        connect(action, SIGNAL(triggered()), this, SLOT(addLayer()));
        m_transformActions[action] = transforms[i].identifier;
        m_transformActionsReverse[transforms[i].identifier] = action;
        connect(this, SIGNAL(canAddLayer(bool)), action, SLOT(setEnabled(bool)));

        action->setStatusTip(transforms[i].longDescription);

        if (categoryMenus[type].find(category) == categoryMenus[type].end()) {
            cerr << "WARNING: VisualiserWidget::setupMenus: Internal error: "
                 << "No category menu for transform \""
                 << name << "\" (category = \""
                 << category << "\")" << endl;
        } else {
            categoryMenus[type][category]->addAction(action);
        }

        if (makerMenus[type].find(maker) == makerMenus[type].end()) {
            cerr << "WARNING: VisualiserWidget::setupMenus: Internal error: "
                 << "No maker menu for transform \""
                 << name << "\" (maker = \""
                 << maker << "\")" << endl;
        } else {
            makerMenus[type][maker]->addAction(action);
        }

        action = new QAction(tr("%1...").arg(output == "" ? pluginName : output), this);
        connect(action, SIGNAL(triggered()), this, SLOT(addLayer()));
        m_transformActions[action] = transforms[i].identifier;
        connect(this, SIGNAL(canAddLayer(bool)), action, SLOT(setEnabled(bool)));
        action->setStatusTip(transforms[i].longDescription);

        //        cerr << "Transform: \"" << name << "\": plugin name \"" << pluginName << "\"" << endl;

        if (pluginNameMenus[type].find(pluginName) ==
                pluginNameMenus[type].end()) {

            SubdividingMenu *parentMenu = byPluginNameMenus[type];
            parentMenu->setTearOffEnabled(true);

            if (output == "") {
                parentMenu->addAction(pluginName, action);
            } else {
                pluginNameMenus[type][pluginName] =
                        parentMenu->addMenu(pluginName);
                connect(this, SIGNAL(canAddLayer(bool)),
                        pluginNameMenus[type][pluginName],
                        SLOT(setEnabled(bool)));
            }
        }

        if (pluginNameMenus[type].find(pluginName) !=
                pluginNameMenus[type].end()) {
            pluginNameMenus[type][pluginName]->addAction(action);
        }
    }

    for (set<SubdividingMenu *>::iterator i = pendingMenus.begin();
         i != pendingMenus.end(); ++i) {
        (*i)->entriesAdded();
    }

    m_transformsMenu->addSeparator();
    m_rightButtonTransformsMenu->addSeparator();

    QAction *action = new QAction(tr("Find a Transform..."), this);
    action->setStatusTip(tr("Search for a transform from the installed plugins, by name or description"));
    action->setShortcut(tr("Ctrl+M"));
    connect(action, SIGNAL(triggered()), this, SLOT(findTransform()));
    //    connect(this, SIGNAL(canAddLayer(bool)), action, SLOT(setEnabled(bool)));
    m_keyReference->registerShortcut(action);
    m_transformsMenu->addAction(action);
    m_rightButtonTransformsMenu->addAction(action);

    setupRecentTransformsMenu();
}

void VisualiserWidget::setupTemplatesMenu()
{
    m_templatesMenu->clear();

    QAction *defaultAction = new QAction(tr("Standard Waveform"), this);
    defaultAction->setObjectName("default");
    connect(defaultAction, SIGNAL(triggered()), this, SLOT(applyTemplate()));
    m_templatesMenu->addAction(defaultAction);

    m_templatesMenu->addSeparator();

    QAction *action = 0;

    QStringList templates = ResourceFinder().getResourceFiles("templates", "svt");

    bool havePersonal = false;

    // (ordered by name)
    std::set<QString> byName;
    foreach (QString t, templates) {
        if (!t.startsWith(":")) havePersonal = true;
        byName.insert(QFileInfo(t).baseName());
    }

    foreach (QString t, byName) {
        if (t.toLower() == "default") continue;
        action = new QAction(t, this);
        connect(action, SIGNAL(triggered()), this, SLOT(applyTemplate()));
        m_templatesMenu->addAction(action);
    }

    if (!templates.empty()) m_templatesMenu->addSeparator();

    if (!m_templateWatcher) {
        m_templateWatcher = new QFileSystemWatcher(this);
        m_templateWatcher->addPath(ResourceFinder().getResourceSaveDir("templates"));
        connect(m_templateWatcher, SIGNAL(directoryChanged(const QString &)),
                this, SLOT(setupTemplatesMenu()));
    }

    QAction *setDefaultAction = new QAction(tr("Choose Default Template..."), this);
    setDefaultAction->setObjectName("set_default_template");
    connect(setDefaultAction, SIGNAL(triggered()), this, SLOT(selectDefaultTemplate()));
    m_templatesMenu->addSeparator();
    m_templatesMenu->addAction(setDefaultAction);

    m_manageTemplatesAction->setEnabled(havePersonal);
}

void VisualiserWidget::setupRecentTransformsMenu()
{
    m_recentTransformsMenu->clear();
    vector<QString> transforms = m_recentTransforms.getRecent();
    for (size_t i = 0; i < transforms.size(); ++i) {
        TransformActionReverseMap::iterator ti =
                m_transformActionsReverse.find(transforms[i]);
        if (ti == m_transformActionsReverse.end()) {
            cerr << "WARNING: VisualiserWidget::setupRecentTransformsMenu: "
                 << "Unknown transform \"" << transforms[i]
                    << "\" in recent transforms list" << endl;
            continue;
        }
        if (i == 0) {
            ti->second->setShortcut(tr("Ctrl+T"));
            m_keyReference->registerShortcut
                    (tr("Repeat Transform"),
                     ti->second->shortcut().toString(),
                     tr("Re-select the most recently run transform"));
        } else {
            ti->second->setShortcut(QString(""));
        }
        m_recentTransformsMenu->addAction(ti->second);
    }
}

void VisualiserWidget::setupExistingLayersMenus()
{
    if (!m_existingLayersMenu) return; // should have been created by setupMenus

    //    cerr << "VisualiserWidget::setupExistingLayersMenu" << endl;

    m_existingLayersMenu->clear();
    m_existingLayerActions.clear();

    m_sliceMenu->clear();
    m_sliceActions.clear();

    IconLoader il;

    vector<Layer *> orderedLayers;
    set<Layer *> observedLayers;
    set<Layer *> sliceableLayers;

    LayerFactory &factory = LayerFactory::getInstance();

    for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {

        Pane *pane = m_paneStack->getPane(i);
        if (!pane) continue;

        for (int j = 0; j < pane->getLayerCount(); ++j) {

            Layer *layer = pane->getLayer(j);
            if (!layer) continue;
            if (observedLayers.find(layer) != observedLayers.end()) {
                //		cerr << "found duplicate layer " << layer << endl;
                continue;
            }

            //	    cerr << "found new layer " << layer << " (name = "
            //		      << layer->getLayerPresentationName() << ")" << endl;

            orderedLayers.push_back(layer);
            observedLayers.insert(layer);

            if (factory.isLayerSliceable(layer)) {
                sliceableLayers.insert(layer);
            }
        }
    }

    map<QString, int> observedNames;

    for (size_t i = 0; i < orderedLayers.size(); ++i) {

        Layer *layer = orderedLayers[i];

        QString name = layer->getLayerPresentationName();
        int n = ++observedNames[name];
        if (n > 1) name = QString("%1 <%2>").arg(name).arg(n);

        QIcon icon = il.load(factory.getLayerIconName(factory.getLayerType(layer)));

        QAction *action = new QAction(icon, name, this);
        connect(action, SIGNAL(triggered()), this, SLOT(addLayer()));
        connect(this, SIGNAL(canAddLayer(bool)), action, SLOT(setEnabled(bool)));
        m_existingLayerActions[action] = layer;

        m_existingLayersMenu->addAction(action);

        if (sliceableLayers.find(layer) != sliceableLayers.end()) {
            action = new QAction(icon, name, this);
            connect(action, SIGNAL(triggered()), this, SLOT(addLayer()));
            connect(this, SIGNAL(canAddLayer(bool)), action, SLOT(setEnabled(bool)));
            m_sliceActions[action] = layer;
            m_sliceMenu->addAction(action);
        }
    }

    m_sliceMenu->setEnabled(!m_sliceActions.empty());
}

void VisualiserWidget::setupToolbars()
{
    IconLoader il;

    // Playback toolbar
    setupPlaybackMenusAndToolbar();
    addToolBar(m_toolbarPlayback);

    // Edit toolbar
    QToolBar *toolbar = addToolBar(tr("Edit Toolbar"));
    CommandHistory::getInstance()->registerToolbar(toolbar);

    // Panes, Layers, Transforms toolbar
    toolbar = addToolBar(tr("Panes Layers Transforms"));
    QToolButton* toolButtonPanes = new QToolButton();
    toolButtonPanes->setText("Panes ");
    toolButtonPanes->setMenu(m_paneMenu);
    toolButtonPanes->setPopupMode(QToolButton::InstantPopup);
    QWidgetAction* toolButtonActionPanes = new QWidgetAction(this);
    toolButtonActionPanes->setDefaultWidget(toolButtonPanes);
    toolbar->addAction(toolButtonActionPanes);

    QToolButton* toolButtonLayers = new QToolButton();
    toolButtonLayers->setText("Layers ");
    toolButtonLayers->setMenu(m_layerMenu);
    toolButtonLayers->setPopupMode(QToolButton::InstantPopup);
    QWidgetAction* toolButtonActionLayers = new QWidgetAction(this);
    toolButtonActionLayers->setDefaultWidget(toolButtonLayers);
    toolbar->addAction(toolButtonActionLayers);

    QToolButton* toolButtonTransforms = new QToolButton();
    toolButtonTransforms->setText("Transforms ");
    toolButtonTransforms->setMenu(m_transformsMenu);
    toolButtonTransforms->setPopupMode(QToolButton::InstantPopup);
    QWidgetAction* toolButtonActionTransforms = new QWidgetAction(this);
    toolButtonActionTransforms->setDefaultWidget(toolButtonTransforms);
    toolbar->addAction(toolButtonActionTransforms);

    // Tools toolbar
    toolbar = addToolBar(tr("Tools Toolbar"));
    QActionGroup *group = new QActionGroup(this);

    m_keyReference->setCategory(tr("Tool Selection"));
    QAction *action = toolbar->addAction(il.load("navigate"), tr("Navigate"));
    action->setCheckable(true);
    action->setChecked(true);
    action->setShortcut(tr("1"));
    action->setStatusTip(tr("Navigate"));
    connect(action, SIGNAL(triggered()), this, SLOT(toolNavigateSelected()));
    connect(this, SIGNAL(replacedDocument()), action, SLOT(trigger()));
    group->addAction(action);
    m_keyReference->registerShortcut(action);
    m_toolActions[ViewManager::NavigateMode] = action;

    m_keyReference->setCategory(tr("Navigate Tool Mouse Actions"));
    m_keyReference->registerShortcut
            (tr("Navigate"), tr("Left"),
             tr("Click left button and drag to move around"));
    m_keyReference->registerShortcut
            (tr("Zoom to Area"), tr("Shift+Left"),
             tr("Shift-click left button and drag to zoom to a rectangular area"));
    m_keyReference->registerShortcut
            (tr("Relocate"), tr("Double-Click Left"),
             tr("Double-click left button to jump to clicked location"));
    m_keyReference->registerShortcut
            (tr("Edit"), tr("Double-Click Left"),
             tr("Double-click left button on an item to edit it"));

    m_keyReference->setCategory(tr("Tool Selection"));
    action = toolbar->addAction(il.load("select"),
                                tr("Select"));
    action->setCheckable(true);
    action->setShortcut(tr("2"));
    action->setStatusTip(tr("Select ranges"));
    connect(action, SIGNAL(triggered()), this, SLOT(toolSelectSelected()));
    group->addAction(action);
    m_keyReference->registerShortcut(action);
    m_toolActions[ViewManager::SelectMode] = action;

    m_keyReference->setCategory
            (tr("Select Tool Mouse Actions"));
    m_keyReference->registerShortcut
            (tr("Select"), tr("Left"),
             tr("Click left button and drag to select region; drag region edge to resize"));
#ifdef Q_OS_MAC
    m_keyReference->registerShortcut
            (tr("Multi Select"), tr("Ctrl+Left"),
             tr("Cmd-click left button and drag to select an additional region"));
#else
    m_keyReference->registerShortcut
            (tr("Multi Select"), tr("Ctrl+Left"),
             tr("Ctrl-click left button and drag to select an additional region"));
#endif
    m_keyReference->registerShortcut
            (tr("Fine Select"), tr("Shift+Left"),
             tr("Shift-click left button and drag to select without snapping to items or grid"));

    m_keyReference->setCategory(tr("Tool Selection"));
    action = toolbar->addAction(il.load("move"),
                                tr("Edit"));
    action->setCheckable(true);
    action->setShortcut(tr("3"));
    action->setStatusTip(tr("Edit items in layer"));
    connect(action, SIGNAL(triggered()), this, SLOT(toolEditSelected()));
    connect(this, SIGNAL(canEditLayer(bool)), action, SLOT(setEnabled(bool)));
    group->addAction(action);
    m_keyReference->registerShortcut(action);
    m_toolActions[ViewManager::EditMode] = action;

    m_keyReference->setCategory
            (tr("Edit Tool Mouse Actions"));
    m_keyReference->registerShortcut
            (tr("Move"), tr("Left"),
             tr("Click left button on an item or selected region and drag to move"));
    m_keyReference->registerShortcut
            (tr("Edit"), tr("Double-Click Left"),
             tr("Double-click left button on an item to edit it"));

    m_keyReference->setCategory(tr("Tool Selection"));
    action = toolbar->addAction(il.load("draw"),
                                tr("Draw"));
    action->setCheckable(true);
    action->setShortcut(tr("4"));
    action->setStatusTip(tr("Draw new items in layer"));
    connect(action, SIGNAL(triggered()), this, SLOT(toolDrawSelected()));
    connect(this, SIGNAL(canEditLayer(bool)), action, SLOT(setEnabled(bool)));
    group->addAction(action);
    m_keyReference->registerShortcut(action);
    m_toolActions[ViewManager::DrawMode] = action;

    m_keyReference->setCategory
            (tr("Draw Tool Mouse Actions"));
    m_keyReference->registerShortcut
            (tr("Draw"), tr("Left"),
             tr("Click left button and drag to create new item"));

    m_keyReference->setCategory(tr("Tool Selection"));
    action = toolbar->addAction(il.load("erase"),
                                tr("Erase"));
    action->setCheckable(true);
    action->setShortcut(tr("5"));
    action->setStatusTip(tr("Erase items from layer"));
    connect(action, SIGNAL(triggered()), this, SLOT(toolEraseSelected()));
    connect(this, SIGNAL(canEditLayer(bool)), action, SLOT(setEnabled(bool)));
    group->addAction(action);
    m_keyReference->registerShortcut(action);
    m_toolActions[ViewManager::EraseMode] = action;

    m_keyReference->setCategory
            (tr("Erase Tool Mouse Actions"));
    m_keyReference->registerShortcut
            (tr("Erase"), tr("Left"),
             tr("Click left button on an item to remove it from the layer"));

    m_keyReference->setCategory(tr("Tool Selection"));
    action = toolbar->addAction(il.load("measure"), tr("Measure"));
    action->setCheckable(true);
    action->setShortcut(tr("6"));
    action->setStatusTip(tr("Make measurements in layer"));
    connect(action, SIGNAL(triggered()), this, SLOT(toolMeasureSelected()));
    connect(this, SIGNAL(canMeasureLayer(bool)), action, SLOT(setEnabled(bool)));
    group->addAction(action);
    m_keyReference->registerShortcut(action);
    m_toolActions[ViewManager::MeasureMode] = action;

    m_keyReference->setCategory
            (tr("Measure Tool Mouse Actions"));
    m_keyReference->registerShortcut
            (tr("Measure Area"), tr("Left"),
             tr("Click left button and drag to measure a rectangular area"));
    m_keyReference->registerShortcut
            (tr("Measure Item"), tr("Double-Click Left"),
             tr("Click left button and drag to measure extents of an item or shape"));
    m_keyReference->registerShortcut
            (tr("Zoom to Area"), tr("Shift+Left"),
             tr("Shift-click left button and drag to zoom to a rectangular area"));

    toolNavigateSelected();

    Pane::registerShortcuts(*m_keyReference);
}

void
VisualiserWidget::connectLayerEditDialog(ModelDataTableDialog *dialog)
{
    VisualiserWindowBase::connectLayerEditDialog(dialog);
    QToolBar *toolbar = dialog->getPlayToolbar();
    if (toolbar) {
        toolbar->addAction(m_rwdStartAction);
        toolbar->addAction(m_rwdAction);
        toolbar->addAction(m_playAction);
        toolbar->addAction(m_ffwdAction);
        toolbar->addAction(m_ffwdEndAction);
    }
}

// protected slot, base class implementation (VisualiserBaseWindow)
void VisualiserWidget::handleOSCMessage(const OSCMessage &)
{
    cerr << "VisualiserWidget::handleOSCMessage: Not implemented" << endl;
}

void
VisualiserWidget::updateMenuStates()
{
    VisualiserWindowBase::updateMenuStates();

    Pane *currentPane = 0;
    Layer *currentLayer = 0;

    if (m_paneStack) currentPane = m_paneStack->getCurrentPane();
    if (currentPane) currentLayer = currentPane->getSelectedLayer();

    bool haveCurrentPane = (currentPane != 0);
    bool haveCurrentLayer = (haveCurrentPane && (currentLayer != 0));
    bool havePlayTarget = (m_playTarget != 0);
    bool haveSelection = (m_viewManager && !m_viewManager->getSelections().empty());
    bool haveCurrentEditableLayer = (haveCurrentLayer && currentLayer->isLayerEditable());
    bool haveCurrentTimeInstantsLayer = (haveCurrentLayer && dynamic_cast<TimeInstantLayer *>(currentLayer));
    bool haveCurrentTimeValueLayer = (haveCurrentLayer && dynamic_cast<TimeValueLayer *>(currentLayer));

    bool alignMode = m_viewManager && m_viewManager->getAlignMode();
    emit canChangeSolo(havePlayTarget && !alignMode);
    emit canAlign(havePlayTarget && m_document && m_document->canAlign());

    emit canChangePlaybackSpeed(true);
    int v = m_playSpeed->value();
    emit canSpeedUpPlayback(v < m_playSpeed->maximum());
    emit canSlowDownPlayback(v > m_playSpeed->minimum());

    if (m_deleteSelectedAction) {
        if (m_viewManager &&
                (m_viewManager->getToolMode() == ViewManager::MeasureMode)) {
            emit canDeleteSelection(haveCurrentLayer);
            m_deleteSelectedAction->setText(tr("&Delete Current Measurement"));
            m_deleteSelectedAction->setStatusTip(tr("Delete the measurement currently under the mouse pointer"));
        } else {
            emit canDeleteSelection(haveSelection && haveCurrentEditableLayer);
            m_deleteSelectedAction->setText(tr("&Delete Selected Items"));
            m_deleteSelectedAction->setStatusTip(tr("Delete items in current selection from the current layer"));
        }
    }

    if (m_ffwdAction && m_rwdAction) {
        if (haveCurrentTimeInstantsLayer) {
            m_ffwdAction->setText(tr("Fast Forward to Next Instant"));
            m_ffwdAction->setStatusTip(tr("Fast forward to the next time instant in the current layer"));
            m_rwdAction->setText(tr("Rewind to Previous Instant"));
            m_rwdAction->setStatusTip(tr("Rewind to the previous time instant in the current layer"));
        } else if (haveCurrentTimeValueLayer) {
            m_ffwdAction->setText(tr("Fast Forward to Next Point"));
            m_ffwdAction->setStatusTip(tr("Fast forward to the next point in the current layer"));
            m_rwdAction->setText(tr("Rewind to Previous Point"));
            m_rwdAction->setStatusTip(tr("Rewind to the previous point in the current layer"));
        } else {
            m_ffwdAction->setText(tr("Fast Forward"));
            m_ffwdAction->setStatusTip(tr("Fast forward"));
            m_rwdAction->setText(tr("Rewind"));
            m_rwdAction->setStatusTip(tr("Rewind"));
        }
    }
}


void
VisualiserWidget::toolNavigateSelected()
{
    m_viewManager->setToolMode(ViewManager::NavigateMode);
}

void
VisualiserWidget::toolSelectSelected()
{
    m_viewManager->setToolMode(ViewManager::SelectMode);
}

void
VisualiserWidget::toolEditSelected()
{
    m_viewManager->setToolMode(ViewManager::EditMode);
}

void
VisualiserWidget::toolDrawSelected()
{
    m_viewManager->setToolMode(ViewManager::DrawMode);
}

void
VisualiserWidget::toolEraseSelected()
{
    m_viewManager->setToolMode(ViewManager::EraseMode);
}

void VisualiserWidget::toolMeasureSelected()
{
    m_viewManager->setToolMode(ViewManager::MeasureMode);
}

void VisualiserWidget::importAudio()
{
    QString path = getOpenFileName(FileFinder::AudioFile);
    if (path != "") {
        if (openAudio(path, ReplaceSession) == FileOpenFailed) {
            QMessageBox::critical(this, tr("Failed to open file"),
                                  tr("<b>File open failed</b><p>Audio file \"%1\" could not be opened").arg(path));
        }
    }
}

void VisualiserWidget::importMoreAudio()
{
    QString path = getOpenFileName(FileFinder::AudioFile);
    if (path != "") {
        if (openAudio(path, CreateAdditionalModel) == FileOpenFailed) {
            QMessageBox::critical(this, tr("Failed to open file"),
                                  tr("<b>File open failed</b><p>Audio file \"%1\" could not be opened").arg(path));
        }
    }
}

void VisualiserWidget::replaceMainAudio()
{
    QString path = getOpenFileName(FileFinder::AudioFile);

    if (path != "") {
        if (openAudio(path, ReplaceMainModel) == FileOpenFailed) {
            QMessageBox::critical(this, tr("Failed to open file"),
                                  tr("<b>File open failed</b><p>Audio file \"%1\" could not be opened").arg(path));
        }
    }
}

void VisualiserWidget::exportAudio()
{
    exportAudio(false);
}

void
VisualiserWidget::exportAudioData() {
    exportAudio(true);
}

void VisualiserWidget::exportAudio(bool asData)
{
    if (!getMainModel()) return;

    RangeSummarisableTimeValueModel *model = getMainModel();
    std::set<RangeSummarisableTimeValueModel *> otherModels;
    RangeSummarisableTimeValueModel *current = model;
    if (m_paneStack) {
        for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {
            Pane *pane = m_paneStack->getPane(i);
            if (!pane) continue;
            for (int j = 0; j < pane->getLayerCount(); ++j) {
                Layer *layer = pane->getLayer(j);
                if (!layer) continue;
                cerr << "layer = " << layer->objectName() << endl;
                Model *m = layer->getModel();
                RangeSummarisableTimeValueModel *wm =
                        dynamic_cast<RangeSummarisableTimeValueModel *>(m);
                if (wm) {
                    cerr << "found: " << wm->objectName() << endl;
                    otherModels.insert(wm);
                    if (pane == m_paneStack->getCurrentPane()) {
                        current = wm;
                    }
                }
            }
        }
    }
    if (!otherModels.empty()) {
        std::map<QString, RangeSummarisableTimeValueModel *> m;
        m[tr("1. %2").arg(model->objectName())] = model;
        int n = 2;
        int c = 0;
        for (std::set<RangeSummarisableTimeValueModel *>::const_iterator i
             = otherModels.begin();
             i != otherModels.end(); ++i) {
            if (*i == model) continue;
            m[tr("%1. %2").arg(n).arg((*i)->objectName())] = *i;
            ++n;
            if (*i == current) c = n-1;
        }
        QStringList items;
        for (std::map<QString, RangeSummarisableTimeValueModel *>
             ::const_iterator i = m.begin();
             i != m.end(); ++i) {
            items << i->first;
        }
        if (items.size() > 1) {
            bool ok = false;
            QString item = QInputDialog::getItem
                    (this, tr("Select audio file to export"),
                     tr("Which audio file do you want to export from?"),
                     items, c, false, &ok);
            if (!ok || item.isEmpty()) return;
            if (m.find(item) == m.end()) {
                cerr << "WARNING: Model " << item
                     << " not found in list!" << endl;
            } else {
                model = m[item];
            }
        }
    }

    QString path;
    if (asData) {
        path = getSaveFileName(FileFinder::CSVFile);
    } else {
        path = getSaveFileName(FileFinder::AudioFile);
    }
    if (path == "") return;

    bool ok = false;
    QString error;

    MultiSelection ms = m_viewManager->getSelection();
    MultiSelection::SelectionList selections = m_viewManager->getSelections();

    bool multiple = false;

    MultiSelection *selectionToWrite = 0;

    if (selections.size() == 1) {

        QStringList items;
        items << tr("Export the selected region only")
              << tr("Export the whole audio file");

        bool ok = false;
        QString item = ListInputDialog::getItem
                (this, tr("Select region to export"),
                 tr("Which region from the original audio file do you want to export?"),
                 items, 0, &ok);

        if (!ok || item.isEmpty()) return;

        if (item == items[0]) selectionToWrite = &ms;

    } else if (selections.size() > 1) {

        if (!asData) { // Multi-file export not supported for data

            QStringList items;
            items << tr("Export the selected regions into a single file")
                  << tr("Export the selected regions into separate files")
                  << tr("Export the whole file");

            QString item = ListInputDialog::getItem
                    (this, tr("Select region to export"),
                     tr("Multiple regions of the original audio file are selected.\nWhat do you want to export?"),
                     items, 0, &ok);

            if (!ok || item.isEmpty()) return;

            if (item == items[0]) {
                selectionToWrite = &ms;
            } else if (item == items[1]) {
                multiple = true;
            }

        } else { // asData
            selectionToWrite = &ms;
        }

        if (multiple) { // Can only happen when asData false

            int n = 1;
            QString base = path;
            base.replace(".wav", "");

            for (MultiSelection::SelectionList::iterator i = selections.begin();
                 i != selections.end(); ++i) {

                MultiSelection subms;
                subms.setSelection(*i);

                QString subpath = QString("%1.%2.wav").arg(base).arg(n);
                ++n;

                if (QFileInfo(subpath).exists()) {
                    error = tr("Fragment file %1 already exists, aborting").arg(subpath);
                    break;
                }

                WavFileWriter subwriter(subpath,
                                        model->getSampleRate(),
                                        model->getChannelCount(),
                                        WavFileWriter::WriteToTemporary);
                subwriter.writeModel(model, &subms);
                ok = subwriter.isOK();

                if (!ok) {
                    error = subwriter.getError();
                    break;
                }
            }
        }
    }

    if (!multiple) {
        if (asData) {
            CSVFileWriter writer(path, model,
                                 ((QFileInfo(path).suffix() == "csv") ?
                                      "," : "\t"));
            if (selectionToWrite) {
                writer.writeSelection(selectionToWrite);
            } else {
                writer.write();
            }
            ok = writer.isOK();
            error = writer.getError();
        } else {
            WavFileWriter writer(path,
                                 model->getSampleRate(),
                                 model->getChannelCount(),
                                 WavFileWriter::WriteToTemporary);
            writer.writeModel(model, selectionToWrite);
            ok = writer.isOK();
            error = writer.getError();
        }
    }

    if (ok) {
        if (multiple) {
            emit activity(tr("Export multiple audio files"));
        } else {
            emit activity(tr("Export audio to \"%1\"").arg(path));
        }
    } else {
        QMessageBox::critical(this, tr("Failed to write file"), error);
    }
}

void VisualiserWidget::importLayer()
{
    Pane *pane = m_paneStack->getCurrentPane();

    if (!pane) {
        // shouldn't happen, as the menu action should have been disabled
        cerr << "WARNING: VisualiserWidget::importLayer: no current pane" << endl;
        return;
    }

    if (!getMainModel()) {
        // shouldn't happen, as the menu action should have been disabled
        cerr << "WARNING: VisualiserWidget::importLayer: No main model -- hence no default sample rate available" << endl;
        return;
    }

    QString path = getOpenFileName(FileFinder::LayerFile);

    if (path != "") {

        FileOpenStatus status = openLayer(path);

        if (status == FileOpenFailed) {
            QMessageBox::critical(this, tr("Failed to open file"),
                                  tr("<b>File open failed</b><p>Layer file %1 could not be opened.").arg(path));
            return;
        } else if (status == FileOpenWrongMode) {
            QMessageBox::critical(this, tr("Failed to open file"),
                                  tr("<b>Audio required</b><p>Unable to load layer data from \"%1\" without an audio file.<br>Please load at least one audio file before importing annotations.").arg(path));
        }
    }
}

void
VisualiserWidget::exportLayer()
{
    Pane *pane = m_paneStack->getCurrentPane();
    if (!pane) return;

    Layer *layer = pane->getSelectedLayer();
    if (!layer) return;

    Model *model = layer->getModel();
    if (!model) return;

    FileFinder::FileType type = FileFinder::LayerFileNoMidi;

    if (dynamic_cast<NoteModel *>(model)) type = FileFinder::LayerFile;

    QString path = getSaveFileName(type);

    if (path == "") return;

    if (QFileInfo(path).suffix() == "") path += ".svl";

    QString suffix = QFileInfo(path).suffix().toLower();

    QString error;

    if (suffix == "xml" || suffix == "svl") {

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            error = tr("Failed to open file %1 for writing").arg(path);
        } else {
            QTextStream out(&file);
            out.setCodec(QTextCodec::codecForName("UTF-8"));
            out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << "<!DOCTYPE sonic-visualiser>\n"
                << "<sv>\n"
                << "  <data>\n";

            model->toXml(out, "    ");

            out << "  </data>\n"
                << "  <display>\n";

            layer->toXml(out, "    ");

            out << "  </display>\n"
                << "</sv>\n";
        }

    } else if (suffix == "mid" || suffix == "midi") {

        NoteModel *nm = dynamic_cast<NoteModel *>(model);

        if (!nm) {
            error = tr("Can't export non-note layers to MIDI");
        } else {
            MIDIFileWriter writer(path, nm, nm->getSampleRate());
            writer.write();
            if (!writer.isOK()) {
                error = writer.getError();
            }
        }

    } else if (suffix == "ttl" || suffix == "n3") {

        if (!RDFExporter::canExportModel(model)) {
            error = tr("Sorry, cannot export this layer type to RDF (supported types are: region, note, text, time instants, time values)");
        } else {
            RDFExporter exporter(path, model);
            exporter.write();
            if (!exporter.isOK()) {
                error = exporter.getError();
            }
        }

    } else {

        CSVFileWriter writer(path, model,
                             ((suffix == "csv") ? "," : "\t"));
        writer.write();

        if (!writer.isOK()) {
            error = writer.getError();
        }
    }

    if (error != "") {
        QMessageBox::critical(this, tr("Failed to write file"), error);
    } else {
        emit activity(tr("Export layer to \"%1\"").arg(path));
    }
}

void
VisualiserWidget::exportVisualisation()
{
    QPointer<ExportVisualisationDialog> d = new ExportVisualisationDialog(m_document, m_paneStack, m_viewManager, m_tiers);
    d->exec();
}

void
VisualiserWidget::newSession()
{
    if (!checkSaveModified()) return;
    closeSession();
    createDocument();
    Pane *pane = m_paneStack->addPane();
    connect(pane, SIGNAL(contextHelpChanged(const QString &)),
            this, SLOT(contextHelpChanged(const QString &)));
    if (!m_timeRulerLayer) {
        m_timeRulerLayer = m_document->createMainModelLayer
                (LayerFactory::Type("TimeRuler"));
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

void
VisualiserWidget::closeSession()
{
    if (!checkSaveModified()) return;
    SimpleVisualiserWidget::closeSession();
    delete m_layerTreeDialog.data();
}

void
VisualiserWidget::openSomething()
{
    QString orig = m_audioFile;
    if (orig == "") orig = ".";
    else orig = QFileInfo(orig).absoluteDir().canonicalPath();
    QString path = getOpenFileName(FileFinder::AnyFile);
    if (path.isEmpty()) return;
    FileOpenStatus status = openPath(path, ReplaceSession);
    if (status == FileOpenFailed) {
        QMessageBox::critical(this, tr("Failed to open file"),
                              tr("<b>File open failed</b><p>File \"%1\" could not be opened").arg(path));
    } else if (status == FileOpenWrongMode) {
        QMessageBox::critical(this, tr("Failed to open file"),
                              tr("<b>Audio required</b><p>Unable to load layer data from \"%1\" without an audio file.<br>Please load at least one audio file before importing annotations.").arg(path));
    }
}

void
VisualiserWidget::openLocation()
{
    QSettings settings;
    settings.beginGroup("VisualiserWidget");
    QString lastLocation = settings.value("lastremote", "").toString();
    bool ok = false;
    QString text = QInputDialog::getText
            (this, tr("Open Location"),
             tr("Please enter the URL of the location to open:"),
             QLineEdit::Normal, lastLocation, &ok);
    if (!ok) return;
    settings.setValue("lastremote", text);
    if (text.isEmpty()) return;
    FileOpenStatus status = openPath(text, AskUser);
    if (status == FileOpenFailed) {
        QMessageBox::critical(this, tr("Failed to open location"),
                              tr("<b>Open failed</b><p>URL \"%1\" could not be opened").arg(text));
    } else if (status == FileOpenWrongMode) {
        QMessageBox::critical(this, tr("Failed to open location"),
                              tr("<b>Audio required</b><p>Unable to load layer data from \"%1\" without an audio file.<br>Please load at least one audio file before importing annotations.").arg(text));
    }
}

void
VisualiserWidget::applyTemplate()
{
    QObject *s = sender();
    QAction *action = qobject_cast<QAction *>(s);

    if (!action) {
        cerr << "WARNING: VisualiserWidget::applyTemplate: sender is not an action"
             << endl;
        return;
    }

    QString n = action->objectName();
    if (n == "") n = action->text();

    if (n == "") {
        cerr << "WARNING: VisualiserWidget::applyTemplate: sender has no name"
             << endl;
        return;
    }

    QString mainModelLocation;
    WaveFileModel *mm = getMainModel();
    if (mm) mainModelLocation = mm->getLocation();
    if (mainModelLocation != "") {
        openAudio(mainModelLocation, ReplaceSession, n);
    } else {
        openSessionTemplate(n);
    }
}

void
VisualiserWidget::saveSessionAsTemplate()
{
    QDialog *d = new QDialog(this);
    d->setWindowTitle(tr("Enter template name"));

    QGridLayout *layout = new QGridLayout;
    d->setLayout(layout);

    layout->addWidget(new QLabel(tr("Please enter a name for the saved template:")),
                      0, 0);
    QLineEdit *lineEdit = new QLineEdit;
    layout->addWidget(lineEdit, 1, 0);
    QCheckBox *makeDefault = new QCheckBox(tr("Set as default template for future audio files"));
    layout->addWidget(makeDefault, 2, 0);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel);
    layout->addWidget(bb, 3, 0);
    connect(bb, SIGNAL(accepted()), d, SLOT(accept()));
    connect(bb, SIGNAL(accepted()), d, SLOT(accept()));
    connect(bb, SIGNAL(rejected()), d, SLOT(reject()));

    if (d->exec() == QDialog::Accepted) {

        QString name = lineEdit->text();
        name.replace(QRegExp("[^\\w\\s\\.\"'-]"), "_");

        ResourceFinder rf;
        QString dir = rf.getResourceSaveDir("templates");
        QString filename = QString("%1/%2.svt").arg(dir).arg(name);
        if (QFile(filename).exists()) {
            if (QMessageBox::warning(this,
                                     tr("Template file exists"),
                                     tr("<b>Template file exists</b><p>The template \"%1\" already exists.<br>Overwrite it?").arg(name),
                                     QMessageBox::Ok | QMessageBox::Cancel,
                                     QMessageBox::Cancel) != QMessageBox::Ok) {
                return;
            }
        }

        if (saveSessionTemplate(filename)) {
            if (makeDefault->isChecked()) {
                setDefaultSessionTemplate(name);
            }
        }
    }
}

void
VisualiserWidget::manageSavedTemplates()
{
    ResourceFinder rf;
    QDesktopServices::openUrl("file:" + rf.getResourceSaveDir("templates"));
}

void VisualiserWidget::paneDropAccepted(Pane *pane, QStringList uriList)
{
    if (pane) m_paneStack->setCurrentPane(pane);
    for (QStringList::iterator i = uriList.begin(); i != uriList.end(); ++i) {
        FileOpenStatus status;
        if (i == uriList.begin()) {
            status = openPath(*i, ReplaceCurrentPane);
        } else {
            status = openPath(*i, CreateAdditionalModel);
        }
        if (status == FileOpenFailed) {
            QMessageBox::critical(this, tr("Failed to open dropped URL"),
                                  tr("<b>Open failed</b><p>Dropped URL \"%1\" could not be opened").arg(*i));
            break;
        } else if (status == FileOpenWrongMode) {
            QMessageBox::critical(this, tr("Failed to open dropped URL"),
                                  tr("<b>Audio required</b><p>Unable to load layer data from \"%1\" without an audio file.<br>Please load at least one audio file before importing annotations.").arg(*i));
            break;
        } else if (status == FileOpenCancelled) {
            break;
        }
    }
}

void VisualiserWidget::selectDefaultTemplate()
{

}

void VisualiserWidget::paneDropAccepted(Pane *pane, QString text)
{
    if (pane) m_paneStack->setCurrentPane(pane);

    QUrl testUrl(text);
    if (testUrl.scheme() == "file" ||
            testUrl.scheme() == "http" ||
            testUrl.scheme() == "ftp") {
        QStringList list;
        list.push_back(text);
        paneDropAccepted(pane, list);
        return;
    }

    //!!! open as text -- but by importing as if a CSV, or just adding
    //to a text layer?
}

void VisualiserWidget::closeEvent(QCloseEvent *e)
{
    //    cerr << "VisualiserWidget::closeEvent" << endl;

    if (m_openingAudioFile) {
        //        cerr << "Busy - ignoring close event" << endl;
        e->ignore();
        return;
    }

    if (!m_abandoning && !checkSaveModified()) {
        //        cerr << "Close refused by user - ignoring close event" << endl;
        e->ignore();
        return;
    }

    QSettings settings;
    settings.beginGroup("VisualiserWidget");
    settings.setValue("maximised", isMaximized());
    if (!isMaximized()) {
        settings.setValue("size", size());
        settings.setValue("position", pos());
    }
    settings.endGroup();

    closeSession();

    e->accept();

    return;
}

bool VisualiserWidget::commitData(bool mayAskUser)
{
    if (mayAskUser) {
        bool rv = checkSaveModified();
        return rv;
    } else {
        if (!m_documentModified) return true;

        // If we can't check with the user first, then we can't save
        // to the original session file (even if we have it) -- have
        // to use a temporary file

        QString svDirBase = ".sv1";
        QString svDir = QDir::home().filePath(svDirBase);

        if (!QFileInfo(svDir).exists()) {
            if (!QDir::home().mkdir(svDirBase)) return false;
        } else {
            if (!QFileInfo(svDir).isDir()) return false;
        }

        // This name doesn't have to be unguessable
#ifndef _WIN32
        QString fname = QString("tmp-%1-%2.sv")
                .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"))
                .arg(QProcess().pid());
#else
        QString fname = QString("tmp-%1.sv")
                .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
#endif
        QString fpath = QDir(svDir).filePath(fname);
        if (saveSessionFile(fpath)) {
            m_recentFiles.addFile(fpath);
            emit activity(tr("Export image to \"%1\"").arg(fpath));
            return true;
        } else {
            return false;
        }
    }
}

bool VisualiserWidget::checkSaveModified()
{
    // Called before some destructive operation (e.g. new session,
    // exit program).  Return true if we can safely proceed, false to
    // cancel.

    if (!m_documentModified) return true;

    int button = QMessageBox::No;
//            QMessageBox::warning(this,
//                                 tr("Session modified"),
//                                 tr("<b>Session modified</b><p>The current session has been modified.<br>Do you want to save it?"),
//                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
//                                 QMessageBox::Yes);

    if (button == QMessageBox::Yes) {
        saveSession();
        if (m_documentModified) { // save failed -- don't proceed!
            return false;
        } else {
            return true; // saved, so it's safe to continue now
        }
    } else if (button == QMessageBox::No) {
        m_documentModified = false; // so we know to abandon it
        return true;
    }

    // else cancel
    return false;
}

bool VisualiserWidget::shouldCreateNewSessionForRDFAudio(bool *cancel)
{
    //!!! This is very similar to part of VisualiserWidgetBase::openAudio --
    //!!! make them a bit more uniform

    QSettings settings;
    settings.beginGroup("VisualiserWidget");
    bool prevNewSession = settings.value("newsessionforrdfaudio", true).toBool();
    settings.endGroup();
    bool newSession = true;

    QStringList items;
    items << tr("Close the current session and create a new one")
          << tr("Add this data to the current session");

    bool ok = false;
    QString item = ListInputDialog::getItem
            (this, tr("Select target for import"),
             tr("<b>Select a target for import</b><p>This RDF document refers to one or more audio files.<br>You already have an audio waveform loaded.<br>What would you like to do with the new data?"),
             items, prevNewSession ? 0 : 1, &ok);

    if (!ok || item.isEmpty()) {
        *cancel = true;
        return false;
    }

    newSession = (item == items[0]);
    settings.beginGroup("VisualiserWidget");
    settings.setValue("newsessionforrdfaudio", newSession);
    settings.endGroup();

    if (newSession) return true;
    else return false;
}

void VisualiserWidget::saveSession()
{
    if (m_sessionFile != "") {
        if (!saveSessionFile(m_sessionFile)) {
            QMessageBox::critical(this, tr("Failed to save file"),
                                  tr("<b>Save failed</b><p>Session file \"%1\" could not be saved.").arg(m_sessionFile));
        } else {
            CommandHistory::getInstance()->documentSaved();
            documentRestored();
        }
    } else {
        saveSessionAs();
    }
}

void VisualiserWidget::saveSessionAs()
{
    QString orig = m_audioFile;
    if (orig == "") orig = ".";
    else orig = QFileInfo(orig).absoluteDir().canonicalPath();

    QString path = getSaveFileName(FileFinder::SessionFile);

    if (path == "") return;

    if (!saveSessionFile(path)) {
        QMessageBox::critical(this, tr("Failed to save file"),
                              tr("<b>Save failed</b><p>Session file \"%1\" could not be saved.").arg(path));
    } else {
        setWindowTitle(tr("%1: %2")
                       .arg(QApplication::applicationName())
                       .arg(QFileInfo(path).fileName()));
        m_sessionFile = path;
        CommandHistory::getInstance()->documentSaved();
        documentRestored();
        m_recentFiles.addFile(path);
        emit activity(tr("Save session as \"%1\"").arg(path));
    }
}

void VisualiserWidget::propertyStacksResized(int width)
{
    // cerr << "VisualiserWidget::propertyStacksResized(" << width << ")" << endl;
    if (!m_playControlsSpacer) return;
    int spacerWidth = width - m_playControlsWidth - 4;
    // cerr << "resizing spacer from " << m_playControlsSpacer->width() << " to " << spacerWidth << endl;
    m_playControlsSpacer->setFixedSize(QSize(spacerWidth, 2));
}

void VisualiserWidget::addPane()
{
    QObject *s = sender();
    QAction *action = dynamic_cast<QAction *>(s);
    cerr << "addPane: sender is " << s << ", action is " << action << ", name " << action->text() << endl;
    if (!action) {
        cerr << "WARNING: VisualiserWidget::addPane: sender is not an action"
             << endl;
        return;
    }
    PaneActionMap::iterator i = m_paneActions.find(action);
    if (i == m_paneActions.end()) {
        cerr << "WARNING: VisualiserWidget::addPane: unknown action "
             << action->objectName() << endl;
        cerr << "known actions are:" << endl;
        for (PaneActionMap::const_iterator i = m_paneActions.begin();
             i != m_paneActions.end(); ++i) {
            cerr << i->first << ", name " << i->first->text() << endl;
        }
        return;
    }
    // Call base class implementation to add the pane
    SimpleVisualiserWidget::addPane(i->second, action->text());
}

void VisualiserWidget::addLayer()
{
    QObject *s = sender();
    QAction *action = dynamic_cast<QAction *>(s);

    if (!action) {
        cerr << "WARNING: VisualiserWidget::addLayer: sender is not an action"
             << endl;
        return;
    }

    Pane *pane = m_paneStack->getCurrentPane();

    if (!pane) {
        cerr << "WARNING: VisualiserWidget::addLayer: no current pane" << endl;
        return;
    }

    ExistingLayerActionMap::iterator ei = m_existingLayerActions.find(action);

    if (ei != m_existingLayerActions.end()) {
        Layer *newLayer = ei->second;
        m_document->addLayerToView(pane, newLayer);
        m_paneStack->setCurrentLayer(pane, newLayer);
        return;
    }

    ei = m_sliceActions.find(action);

    if (ei != m_sliceActions.end()) {
        Layer *newLayer = m_document->createLayer(LayerFactory::Type("Slice"));
        //        document->setModel(newLayer, ei->second->getModel());
        SliceableLayer *source = dynamic_cast<SliceableLayer *>(ei->second);
        SliceLayer *dest = dynamic_cast<SliceLayer *>(newLayer);
        if (source && dest) {
            //!!!???
            dest->setSliceableModel(source->getSliceableModel());
            connect(source, SIGNAL(sliceableModelReplaced(const Model *, const Model *)),
                    dest, SLOT(sliceableModelReplaced(const Model *, const Model *)));
            connect(m_document, SIGNAL(modelAboutToBeDeleted(Model *)),
                    dest, SLOT(modelAboutToBeDeleted(Model *)));
        }
        m_document->addLayerToView(pane, newLayer);
        m_paneStack->setCurrentLayer(pane, newLayer);
        return;
    }

    TransformActionMap::iterator i = m_transformActions.find(action);

    if (i == m_transformActions.end()) {
        LayerActionMap::iterator i = m_layerActions.find(action);

        if (i == m_layerActions.end()) {
            cerr << "WARNING: VisualiserWidget::addLayer: unknown action "
                 << action->objectName() << endl;
            return;
        }

        LayerFactory::LayerType type = i->second.layer;

        LayerFactory::LayerTypeSet emptyTypes =
                LayerFactory::getInstance().getValidEmptyLayerTypes();

        Layer *newLayer = 0;

        if (emptyTypes.find(type) != emptyTypes.end()) {

            newLayer = m_document->createEmptyLayer(type);
            if (newLayer) {
                m_toolActions[ViewManager::DrawMode]->trigger();
            }

        } else {

            Model *model = i->second.sourceModel;

            cerr << "model = "<< model << endl;

            if (!model) {
                if (type == LayerFactory::Type("TimeRuler")) {
                    newLayer = m_document->createMainModelLayer(type);
                } else {
                    // if model is unspecified and this is not a
                    // time-ruler layer, use any plausible model from
                    // the current pane -- this is the case for
                    // right-button menu layer additions
                    Pane::ModelSet ms = pane->getModels();
                    foreach (Model *m, ms) {
                        RangeSummarisableTimeValueModel *r =
                                dynamic_cast<RangeSummarisableTimeValueModel *>(m);
                        if (r) model = m;
                    }
                    if (!model) model = getMainModel();
                }
            }

            if (model) {
                newLayer = m_document->createLayer(type);
                if (m_document->isKnownModel(model)) {
                    m_document->setChannel(newLayer, i->second.channel);
                    m_document->setModel(newLayer, model);
                } else {
                    cerr << "WARNING: VisualiserWidget::addLayer: unknown model "
                         << model
                         << " (\""
                         << model->objectName()
                         << "\") in layer action map"
                         << endl;
                }
            }
        }

        if (newLayer) {
            m_document->addLayerToView(pane, newLayer);
            m_paneStack->setCurrentLayer(pane, newLayer);
        }

        return;
    }

    //!!! want to do something like this, but it's not supported in
    //ModelTransformerFactory yet
    /*
    int channel = -1;
    // pick up the default channel from any existing layers on the same pane
    for (int j = 0; j < pane->getLayerCount(); ++j) {
    int c = LayerFactory::getInstance()->getChannel(pane->getLayer(j));
    if (c != -1) {
        channel = c;
        break;
    }
    }
    */

    // We always ask for configuration, even if the plugin isn't
    // supposed to be configurable, because we need to let the user
    // change the execution context (block size etc).

    QString transformId = i->second;

    addLayer(transformId);
}

void VisualiserWidget::addLayer(QString transformId)
{
    Pane *pane = m_paneStack->getCurrentPane();
    if (!pane) {
        cerr << "WARNING: VisualiserWidget::addLayer: no current pane" << endl;
        return;
    }

    Transform transform = TransformFactory::getInstance()->
            getDefaultTransformFor(transformId);

    std::vector<Model *> candidateInputModels =
            m_document->getTransformInputModels();

    Model *defaultInputModel = 0;

    for (int j = 0; j < pane->getLayerCount(); ++j) {

        Layer *layer = pane->getLayer(j);
        if (!layer) continue;

        if (LayerFactory::getInstance().getLayerType(layer) != LayerFactory::Type("Waveform") &&
                !layer->isLayerOpaque()) continue;

        Model *model = layer->getModel();
        if (!model) continue;

        for (size_t k = 0; k < candidateInputModels.size(); ++k) {
            if (candidateInputModels[k] == model) {
                defaultInputModel = model;
                break;
            }
        }

        if (defaultInputModel) break;
    }

    if (candidateInputModels.size() > 1) {
        // Add an aggregate model as another option
        AggregateWaveModel::ChannelSpecList sl;
        foreach (Model *m, candidateInputModels) {
            RangeSummarisableTimeValueModel *r =
                    qobject_cast<RangeSummarisableTimeValueModel *>(m);
            if (r) {
                sl.push_back(AggregateWaveModel::ModelChannelSpec(r, -1));
            }
        }
        if (!sl.empty()) {
            AggregateWaveModel *aggregate = new AggregateWaveModel(sl);
            aggregate->setObjectName(tr("Multiplex all of the above"));
            candidateInputModels.push_back(aggregate);
            //!!! but it leaks
        }
    }

    sv_frame_t startFrame = 0, duration = 0;
    sv_frame_t endFrame = 0;
    m_viewManager->getSelection().getExtents(startFrame, endFrame);
    if (endFrame > startFrame) duration = endFrame - startFrame;
    else startFrame = 0;

    TransformUserConfigurator configurator;

    ModelTransformer::Input input = ModelTransformerFactory::getInstance()->
            getConfigurationForTransform
            (transform,
             candidateInputModels,
             defaultInputModel,
             m_playSource,
             startFrame,
             duration,
             &configurator);

    if (!input.getModel()) return;

    //    cerr << "VisualiserWidget::addLayer: Input model is " << input.getModel() << " \"" << input.getModel()->objectName() << "\"" << endl << "transform:" << endl << transform.toXmlString() << endl;

    Layer *newLayer = m_document->createDerivedLayer(transform, input);

    if (newLayer) {
        m_document->addLayerToView(pane, newLayer);
        m_document->setChannel(newLayer, input.getChannel());
        m_recentTransforms.add(transformId);
        m_paneStack->setCurrentLayer(pane, newLayer);
    }

    updateMenuStates();
}

void VisualiserWidget::renameCurrentLayer()
{
    Pane *pane = m_paneStack->getCurrentPane();
    if (pane) {
        Layer *layer = pane->getSelectedLayer();
        if (layer) {
            bool ok = false;
            QString newName = QInputDialog::getText
                    (this, tr("Rename Layer"),
                     tr("New name for this layer:"),
                     QLineEdit::Normal, layer->objectName(), &ok);
            if (ok) {
                layer->setPresentationName(newName);
                setupExistingLayersMenus();
            }
        }
    }
}

void VisualiserWidget::findTransform()
{
    TransformFinder *finder = new TransformFinder(this);
    if (!finder->exec()) {
        delete finder;
        return;
    }
    TransformId transform = finder->getTransform();
    delete finder;

    if (getMainModel() != 0 && m_paneStack->getCurrentPane() != 0) {
        addLayer(transform);
    }
}

void VisualiserWidget::alignToggled()
{
    QAction *action = dynamic_cast<QAction *>(sender());

    if (!m_viewManager) return;

    if (action) {
        m_viewManager->setAlignMode(action->isChecked());
    } else {
        m_viewManager->setAlignMode(!m_viewManager->getAlignMode());
    }

    if (m_viewManager->getAlignMode()) {
        m_prevSolo = m_soloAction->isChecked();
        if (!m_soloAction->isChecked()) {
            m_soloAction->setChecked(true);
            VisualiserWindowBase::playSoloToggled();
        }
        m_soloModified = false;
        emit canChangeSolo(false);
        m_document->alignModels();
        m_document->setAutoAlignment(true);
    } else {
        if (!m_soloModified) {
            if (m_soloAction->isChecked() != m_prevSolo) {
                m_soloAction->setChecked(m_prevSolo);
                VisualiserWindowBase::playSoloToggled();
            }
        }
        emit canChangeSolo(true);
        m_document->setAutoAlignment(false);
    }

    for (int i = 0; i < m_paneStack->getPaneCount(); ++i) {

        Pane *pane = m_paneStack->getPane(i);
        if (!pane) continue;

        pane->update();
    }
}

void VisualiserWidget::playSpeedChanged(int position)
{
    SimpleVisualiserWidget::playSpeedChanged(position);
    updateMenuStates();
}

void VisualiserWidget::midiEventsAvailable()
{
    Pane *currentPane = 0;
    NoteLayer *currentNoteLayer = 0;
    TimeValueLayer *currentTimeValueLayer = 0;

    if (m_paneStack) {
        currentPane = m_paneStack->getCurrentPane();
    }

    if (currentPane) {
        currentNoteLayer = dynamic_cast<NoteLayer *>
                (currentPane->getSelectedLayer());
        currentTimeValueLayer = dynamic_cast<TimeValueLayer *>
                (currentPane->getSelectedLayer());
    } else {
        // discard these events
        while (m_midiInput->getEventsAvailable() > 0) {
            (void)m_midiInput->readEvent();
        }
        return;
    }

    // This is called through a serialised signal/slot invocation
    // (across threads).  It could happen quite some time after the
    // event was actually received, which is why event timestamping
    // happens in the MIDI input class and not here.

    while (m_midiInput->getEventsAvailable() > 0) {

        MIDIEvent ev(m_midiInput->readEvent());

        sv_frame_t frame = currentPane->alignFromReference(ev.getTime());

        bool noteOn = (ev.getMessageType() == MIDIConstants::MIDI_NOTE_ON &&
                       ev.getVelocity() > 0);

        bool noteOff = (ev.getMessageType() == MIDIConstants::MIDI_NOTE_OFF ||
                        (ev.getMessageType() == MIDIConstants::MIDI_NOTE_ON &&
                         ev.getVelocity() == 0));

        if (currentNoteLayer) {

            if (!m_playSource || !m_playSource->isPlaying()) continue;

            if (noteOn) {

                currentNoteLayer->addNoteOn(frame,
                                            ev.getPitch(),
                                            ev.getVelocity());

            } else if (noteOff) {

                currentNoteLayer->addNoteOff(frame,
                                             ev.getPitch());

            }

            continue;
        }

        if (currentTimeValueLayer) {

            if (!noteOn) continue;

            if (!m_playSource || !m_playSource->isPlaying()) continue;

            Model *model = static_cast<Layer *>(currentTimeValueLayer)->getModel();
            SparseTimeValueModel *tvm =
                    dynamic_cast<SparseTimeValueModel *>(model);
            if (tvm) {
                SparseTimeValueModel::Point point(frame, ev.getPitch() % 12, "");
                SparseTimeValueModel::AddPointCommand *command =
                        new SparseTimeValueModel::AddPointCommand
                        (tvm, point, tr("Add Point"));
                CommandHistory::getInstance()->addCommand(command);
            }

            continue;
        }

        // This is reached only if !currentNoteLayer and
        // !currentTimeValueLayer, i.e. there is some other sort of
        // layer that may be insertable-into

        if (!noteOn) continue;
        insertInstantAt(ev.getTime());
    }
}

void VisualiserWidget::playStatusChanged(bool)
{
    Pane *currentPane = 0;
    NoteLayer *currentNoteLayer = 0;
    if (m_paneStack) currentPane = m_paneStack->getCurrentPane();
    if (currentPane) {
        currentNoteLayer = dynamic_cast<NoteLayer *>(currentPane->getSelectedLayer());
    }
    if (currentNoteLayer) {
        currentNoteLayer->abandonNoteOns();
    }
}

void VisualiserWidget::layerRemoved(Layer *layer)
{
    setupExistingLayersMenus();
    VisualiserWindowBase::layerRemoved(layer);
}

void VisualiserWidget::layerInAView(Layer *layer, bool inAView)
{
    setupExistingLayersMenus();
    VisualiserWindowBase::layerInAView(layer, inAView);
}

void VisualiserWidget::modelAdded(Model *model)
{
    VisualiserWindowBase::modelAdded(model);
    if (dynamic_cast<DenseTimeValueModel *>(model)) {
        setupPaneAndLayerMenus();
    }
}

void
VisualiserWidget::setInstantsNumbering()
{
    QAction *a = dynamic_cast<QAction *>(sender());
    if (!a) return;

    int type = m_numberingActions[a];

    if (m_labeller) m_labeller->setType(Labeller::ValueType(type));

    QSettings settings;
    settings.beginGroup("VisualiserWidget");
    settings.setValue("labellertype", type);
    settings.endGroup();
}

void
VisualiserWidget::setInstantsCounterCycle()
{
    QAction *a = dynamic_cast<QAction *>(sender());
    if (!a) return;

    int cycle = a->text().toInt();
    if (cycle == 0) return;

    if (m_labeller) m_labeller->setCounterCycleSize(cycle);

    QSettings settings;
    settings.beginGroup("VisualiserWidget");
    settings.setValue("labellercycle", cycle);
    settings.endGroup();
}

void VisualiserWidget::setInstantsCounters()
{
    LabelCounterInputDialog dialog(m_labeller, this);
    dialog.setWindowTitle(tr("Reset Counters"));
    dialog.exec();
}

void VisualiserWidget::resetInstantsCounters()
{
    if (m_labeller) m_labeller->resetCounters();
}

void VisualiserWidget::showLayerTree()
{
    if (!m_layerTreeDialog.isNull()) {
        m_layerTreeDialog->show();
        m_layerTreeDialog->raise();
        return;
    }

    m_layerTreeDialog = new LayerTreeDialog(m_paneStack, this);
    m_layerTreeDialog->setAttribute(Qt::WA_DeleteOnClose); // see below
    m_layerTreeDialog->show();
}

// ====================================================================================================================
// Praaline corpus items interface
// ====================================================================================================================

// public slot
//void VisualiserWidget::newSessionWithCommunication(QPointer<CorpusCommunication> com)
//{
//    if (!com) return;
//    bool first = false;
//    foreach (QPointer<CorpusRecording> rec, com->recordings()) {
//        if (!first) {
//            // Main audio
//            QString path = rec->corpus()->baseMediaPath() + "/" + rec->filename();
//            FileOpenStatus status = openPath(path, ReplaceSession);
//            if (status == FileOpenFailed) {
//                QMessageBox::critical(this, tr("Failed to open file"),
//                                      tr("<b>File open failed</b><p>File \"%1\" could not be opened").arg(path));
//            } else if (status == FileOpenWrongMode) {
//                QMessageBox::critical(this, tr("Failed to open file"),
//                                      tr("<b>Audio required</b><p>Unable to load layer data from \"%1\" without an audio file.<br>Please load at least one audio file before importing annotations.").arg(path));
//            }
//            addPane(LayerConfiguration(LayerFactory::Type("Waveform"), getMainModel()), "Main Waveform");
//            //addPane(LayerConfiguration(LayerFactory::Type("Spectrogram"), getMainModel()), "Main Spectrogram");
//            addProsogramPaneToSession(rec);
//            first = true;
//        }
//        else {
//            // import more audio
//            addRecordingToSession(rec);
//        }
//    }
//}

void VisualiserWidget::setAnnotationTiers(QMap<QString, QPointer<AnnotationTierGroup> > &tiers)
{
    m_tiers = tiers;
    updateAnnotationPanes();
}

void VisualiserWidget::setAnnotationLevelAttributeSelection(const QList<QPair<QString, QString> > &annotationSelection)
{
    m_annotationSelection = annotationSelection;
    updateAnnotationPanes();
}

void VisualiserWidget::addAnnotationPane()
{
    if (!getMainModel()) return;

    QList<QPair<QString, QString> > annotationAttributes;
    QPair<QString, QString> pair;
    foreach (pair, m_annotationSelection) if (pair.first != "tapping") annotationAttributes << pair;

    AnnotationGridModel *model = new AnnotationGridModel(getMainModel()->getSampleRate(), m_tiers, annotationAttributes);
    // Excluded speakers
    QList<QString> excluded;
    excluded << "Emilie_1" << "Emilie_2";
    for (int i = 1; i <=9; ++i) excluded << QString("S0%1").arg(i);
    for (int i = 10; i <=300; ++i) excluded << QString("S%1").arg(i);
    excluded << "DEL" << "CAM" << "AVM" << "GJP" << "KAS" << "DUC" << "MET" << "LAB" << "SCS" << "POC" << "AUA" << "LYC"
             << "BIU" << "SMC" << "PIC" << "NIG" << "GRA" << "LDA" << "FEG" << "JOE" << "PEJ" << "SIC" << "GOR" << "TRV"
             << "GRI" << "HES" << "MAP" << "MIM" << "PRT" << "smooth";
    model->excludeSpeakerIDs(excluded);
    // Create a pane + layer for the annotations
    CommandHistory::getInstance()->startCompoundOperation("Add annotations pane", true);
    AddPaneCommand *command = new AddPaneCommand(this);
    CommandHistory::getInstance()->addCommand(command);
    Pane *pane = command->getPane();
    Layer *newLayer = m_document->createImportedLayer(model);
    qobject_cast<AnnotationGridLayer *>(newLayer)->setPlotStyle(AnnotationGridLayer::PlotBlendedSpeakers);
    qobject_cast<AnnotationGridLayer *>(newLayer)->setBaseColour(ColourDatabase::getInstance()->getColourIndex(tr("Black")));
    m_document->addLayerToView(pane, newLayer);
    m_paneStack->setCurrentPane(pane);
    m_paneStack->setCurrentLayer(pane, newLayer);
    CommandHistory::getInstance()->endCompoundOperation();
    updateMenuStates();

    // addTappingDataPane(m_tiers); // for Emilie
    addTappingDataPane(m_tiers, "tappingAdj_naive", "smooth_naive", "boundary");
    addTappingDataPane(m_tiers, "tappingAdj_expert", "smooth_expert", "boundaryExpert");
}

void VisualiserWidget::updateAnnotationPanes()
{

}

void VisualiserWidget::addProsogramPaneToSession(QPointer<CorpusRecording> rec)
{
    if (!getMainModel()) return;
    ProsogramModel *model = new ProsogramModel(getMainModel()->getSampleRate(), rec);
    CommandHistory::getInstance()->startCompoundOperation("Add Prosogram pane", true);
    AddPaneCommand *command = new AddPaneCommand(this);
    CommandHistory::getInstance()->addCommand(command);
    Pane *pane = command->getPane();
    Layer *newLayer = m_document->createImportedLayer(model);
    newLayer->setProperty("Show Vertical Lines", 0);
    m_document->addLayerToView(pane, newLayer);
    m_paneStack->setCurrentPane(pane);
    m_paneStack->setCurrentLayer(pane, newLayer);
    CommandHistory::getInstance()->endCompoundOperation();
    updateMenuStates();
}

void VisualiserWidget::addLayerTimeInstantsFromIntevalTier(IntervalTier *tier)
{
    Pane *pane = m_paneStack->getPane(0);
    LayerFactory::LayerType type = LayerFactory::Type("TimeInstants");
    Layer *newLayer = m_document->createEmptyLayer(type);
    SparseOneDimensionalModel *model = qobject_cast<SparseOneDimensionalModel *>(newLayer->getModel());
    foreach(Interval *intv, tier->intervals()) {
        SparseOneDimensionalModel::Point point(RealTime::realTime2Frame(intv->tMin(), model->getSampleRate()), intv->text());
        model->addPoint(point);
    }
    m_document->addLayerToView(pane, newLayer);
    m_paneStack->setCurrentLayer(pane, newLayer);
}

Layer * VisualiserWidget::addLayerTimeValuesFromAnnotationTier(
        AnnotationTier *tier, const QString &timeAttributeID, const QString &valueAttributeID, const QString &labelAttributeID)
{
    if (!tier) return 0;
    if (!getMainModel()) return 0;
    CommandHistory::getInstance()->startCompoundOperation("Add Pane", true);
    AddPaneCommand *command = new AddPaneCommand(this);
    CommandHistory::getInstance()->addCommand(command);
    Pane *pane = command->getPane();
    LayerFactory::LayerType type = LayerFactory::Type("TimeValues");
    Layer *newLayer = m_document->createEmptyLayer(type);
    newLayer->setPresentationName(tier->name());
    newLayer->setProperty("Plot Type", 0);
    SparseTimeValueModel *model = qobject_cast<SparseTimeValueModel *>(newLayer->getModel());
    for (int i = 0; i < tier->count(); ++i) {
        RealTime t = RealTime::fromNanoseconds(tier->at(i)->attribute(timeAttributeID).toLongLong());
        SparseTimeValueModel::Point point(RealTime::realTime2Frame(t , model->getSampleRate()),
                                          tier->at(i)->attribute(valueAttributeID).toDouble(),
                                          tier->at(i)->attribute(labelAttributeID).toString());
        model->addPoint(point);
    }
    m_document->addLayerToView(pane, newLayer);
    m_paneStack->setCurrentLayer(pane, newLayer);
    CommandHistory::getInstance()->endCompoundOperation();
    updateMenuStates();
    return newLayer;
}

void VisualiserWidget::addTappingDataPane(QMap<QString, QPointer<AnnotationTierGroup> > &tiers,
                                          const QString &tappingTierName,
                                          const QString &smoothTierName,
                                          const QString &boundaryAttributePrefix)
{
    if (!getMainModel()) return;
    CommandHistory::getInstance()->startCompoundOperation("Add Tapping Pane", true);
    AddPaneCommand *command = new AddPaneCommand(this);
    CommandHistory::getInstance()->addCommand(command);
    Pane *pane = command->getPane();
    // add instants layer
    Layer *layerTapping = m_document->createEmptyLayer(LayerFactory::Type("TimeInstants"));
    m_document->addLayerToView(pane, layerTapping);
    layerTapping->setLayerDormant(pane, true);
    // add smoothed values layer
    Layer *newValuesLayer = m_document->createEmptyLayer(LayerFactory::Type("TimeValues"));
    TimeValueLayer *layerSmooth = qobject_cast<TimeValueLayer *>(newValuesLayer);
    if (layerSmooth) {
        layerSmooth->setPlotStyle(TimeValueLayer::PlotCurve);
        layerSmooth->setDrawSegmentDivisions(false);
        layerSmooth->setDisplayExtents(0.0, 100.0);
    }
    m_document->addLayerToView(pane, newValuesLayer);
    // add instants layer : local maxima
    Layer *newInstantsLayerLocalMax = m_document->createEmptyLayer(LayerFactory::Type("TimeInstants"));
    m_document->addLayerToView(pane, newInstantsLayerLocalMax);
    // add regions layer
    Layer *newRegionsLayer = m_document->createEmptyLayer(LayerFactory::Type("Regions"));
    RegionLayer *layerPPB = qobject_cast<RegionLayer *>(newRegionsLayer);
    if (layerPPB) {
        layerPPB->setVerticalScale(RegionLayer::LinearScale);
        layerPPB->setDisplayExtents(0.0, 100.0);
    }
    m_document->addLayerToView(pane, newRegionsLayer);
    // ---
    m_paneStack->setCurrentPane(pane);
    m_paneStack->setCurrentLayer(pane, newRegionsLayer);
    CommandHistory::getInstance()->endCompoundOperation();
    updateMenuStates();

    SparseOneDimensionalModel *modelInstants = qobject_cast<SparseOneDimensionalModel *>(layerTapping->getModel());
    SparseOneDimensionalModel *modelInstantsLocalMax = qobject_cast<SparseOneDimensionalModel *>(newInstantsLayerLocalMax->getModel());
    SparseTimeValueModel *modelSmooth = qobject_cast<SparseTimeValueModel *>(newValuesLayer->getModel());
    RegionModel *modelRegions = qobject_cast<RegionModel *>(newRegionsLayer->getModel());
    sv_samplerate_t sampleRate = modelInstants->getSampleRate();

    foreach (QString subjectID, tiers.keys()) {
        QPointer<AnnotationTierGroup> tiersSubj = tiers.value(subjectID);
        // Instants model
        IntervalTier *tier_tapping = tiersSubj->getIntervalTierByName(tappingTierName);
        if (tier_tapping) {
            foreach(Interval *intv, tier_tapping->intervals()) {
                if (intv->text() != "x") continue;
                SparseOneDimensionalModel::Point point(RealTime::realTime2Frame(intv->tMin(), modelInstants->getSampleRate()),
                                                       QString(subjectID).replace("Emilie_", ""));
                modelInstants->addPoint(point);
            }
        }
        // Smooth model
        PointTier *tier_smooth = tiersSubj->getPointTierByName(smoothTierName);
        if (tier_smooth) {
            foreach (Point *sp, tier_smooth->points()) {
                SparseTimeValueModel::Point smoothedPoint(RealTime::realTime2Frame(sp->time(), sampleRate),
                                                          sp->attribute("force").toDouble() * 100.0,
                                                          sp->text());
                modelSmooth->addPoint(smoothedPoint);
                if (sp->attribute("localMax").toBool()) {
                    SparseOneDimensionalModel::Point pointLocalMax(RealTime::realTime2Frame(sp->time(), sampleRate), "");
                    modelInstantsLocalMax->addPoint(pointLocalMax);
                }
            }
        }
        // Regions model
        IntervalTier *tier_syll = tiersSubj->getIntervalTierByName("syll");
        if (tier_syll) {
            foreach (Interval *syll, tier_syll->intervals()) {
                double force = syll->attribute(boundaryAttributePrefix + "Force").toDouble() * 100.0;
                if (force < 5.0) continue;
                RealTime tFirst = RealTime::fromSeconds(syll->attribute(boundaryAttributePrefix + "FirstPPB").toDouble());
                RealTime tLast = RealTime::fromSeconds(syll->attribute(boundaryAttributePrefix + "LastPPB").toDouble());
                // frame, value, duration, label
                QString label = QString("%1 F:%2% D:%3 R:%4")
                        .arg((syll->text().isEmpty()) ? "(.)" : syll->text())
                        .arg(QString::number(force, 'f', 1))
                        .arg(QString::number(syll->attribute(boundaryAttributePrefix + "Dispersion").toDouble(), 'f', 2))
                        .arg(QString::number(syll->attribute(boundaryAttributePrefix + "Delay").toDouble(), 'f', 2));

                RegionModel::Point regionPPB(RealTime::realTime2Frame(tFirst, sampleRate), force,
                                             RealTime::realTime2Frame(tLast - tFirst, sampleRate),
                                             label);
                modelRegions->addPoint(regionPPB);
            }
            RegionModel::Point r0(0, 0.0, 0, "");
            modelRegions->addPoint(r0);
            RegionModel::Point r100(0, 100.0, 0, "");
            modelRegions->addPoint(r100);
        }
    }
}

