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

#ifndef _MAIN_WINDOW_BASE_H_
#define _MAIN_WINDOW_BASE_H_

#include <QFrame>
#include <QString>
#include <QUrl>
#include <QMainWindow>
#include <QPointer>
#include <QThread>

#include "base/Command.h"
#include "view/ViewManager.h"
#include "base/PropertyContainer.h"
#include "base/RecentFiles.h"
#include "base/FrameTimer.h"
#include "layer/LayerFactory.h"
#include "transform/Transform.h"
#include "SVFileReader.h"
#include "data/fileio/FileFinder.h"
#include "data/fileio/FileSource.h"
#include "data/osc/OSCQueue.h"
#include <map>

class Document;
class PaneStack;
class Pane;
class View;
class Fader;
class Overview;
class Layer;
class WaveformLayer;
class WaveFileModel;
class AudioCallbackPlaySource;
class AudioCallbackPlayTarget;
class CommandHistory;
class QMenu;
class AudioDial;
class QLabel;
class QCheckBox;
class PreferencesDialog;
class QTreeView;
class QPushButton;
class OSCMessage;
class MIDIInput;
class KeyReference;
class Labeller;
class ModelDataTableDialog;
class QSignalMapper;
class QShortcut;

/**
 * The base class for the SV main window.  This includes everything to
 * do with general document and pane stack management, but nothing
 * that involves user interaction -- this doesn't create the widget or
 * menu structures or editing tools, and if a function needs to open a
 * dialog, it shouldn't be in here.  This permits "variations on SV"
 * to use different subclasses retaining the same general structure.
 */

class VisualiserWindowBase : public QMainWindow, public FrameTimer
{
    Q_OBJECT

public:
    VisualiserWindowBase(bool withAudioOutput, bool withMIDIInput);
    virtual ~VisualiserWindowBase();
    
    enum AudioFileOpenMode {
        ReplaceSession,
        ReplaceMainModel,
        CreateAdditionalModel,
        ReplaceCurrentPane,
        AskUser
    };

    enum FileOpenStatus {
        FileOpenSucceeded,
        FileOpenFailed,
        FileOpenCancelled,
        FileOpenWrongMode // attempted to open layer when no main model present
    };

    virtual FileOpenStatus open(FileSource source, AudioFileOpenMode = AskUser);
    virtual FileOpenStatus openPath(QString fileOrUrl, AudioFileOpenMode = AskUser);
    virtual FileOpenStatus openAudio(FileSource source, AudioFileOpenMode = AskUser, QString templateName = "");
    virtual FileOpenStatus openPlaylist(FileSource source, AudioFileOpenMode = AskUser);
    virtual FileOpenStatus openLayer(FileSource source);
    virtual FileOpenStatus openImage(FileSource source);

    virtual FileOpenStatus openSession(FileSource source);
    virtual FileOpenStatus openSessionPath(QString fileOrUrl);
    virtual FileOpenStatus openSessionTemplate(QString templateName);
    virtual FileOpenStatus openSessionTemplate(FileSource source);

    virtual bool saveSessionFile(QString path);
    virtual bool saveSessionTemplate(QString path);

    /// Implementation of FrameTimer interface method
    virtual sv_frame_t getFrame() const;

    void setDefaultFfwdRwdStep(RealTime step) {
        m_defaultFfwdRwdStep = step;
    }

signals:
    // Used to toggle the availability of menu actions
    void canAddPane(bool);
    void canDeleteCurrentPane(bool);
    void canAddLayer(bool);
    void canImportMoreAudio(bool);
    void canReplaceMainAudio(bool);
    void canImportLayer(bool);
    void canChangeSessionTemplate(bool);
    void canExportAudio(bool);
    void canExportLayer(bool);
    void canExportImage(bool);
    void canRenameLayer(bool);
    void canEditLayer(bool);
    void canEditLayerTabular(bool);
    void canMeasureLayer(bool);
    void canSelect(bool);
    void canClearSelection(bool);
    void canEditSelection(bool);
    void canDeleteSelection(bool);
    void canPaste(bool);
    void canInsertInstant(bool);
    void canInsertInstantsAtBoundaries(bool);
    void canInsertItemAtSelection(bool);
    void canRenumberInstants(bool);
    void canDeleteCurrentLayer(bool);
    void canZoom(bool);
    void canScroll(bool);
    void canPlay(bool);
    void canFfwd(bool);
    void canRewind(bool);
    void canPlaySelection(bool);
    void canSpeedUpPlayback(bool);
    void canSlowDownPlayback(bool);
    void canChangePlaybackSpeed(bool);
    void canSelectPreviousPane(bool);
    void canSelectNextPane(bool);
    void canSelectPreviousLayer(bool);
    void canSelectNextLayer(bool);
    void canSave(bool);
    void canSaveAs(bool);
    void hideSplash();
    void sessionLoaded();
    void audioFileLoaded();
    void replacedDocument();
    void activity(QString);

public slots:
    virtual void preferenceChanged(PropertyContainer::PropertyName);
    virtual void resizeConstrained(QSize);

protected slots:
    virtual void zoomIn();
    virtual void zoomOut();
    virtual void zoomToFit();
    virtual void zoomDefault();
    virtual void scrollLeft();
    virtual void scrollRight();
    virtual void jumpLeft();
    virtual void jumpRight();
    virtual void peekLeft();
    virtual void peekRight();

    virtual void showNoOverlays();
    virtual void showMinimalOverlays();
    virtual void showAllOverlays();

    virtual void toggleTimeRulers();
    virtual void toggleZoomWheels();
    virtual void togglePropertyBoxes();
    virtual void toggleStatusBar();
    virtual void toggleCentreLine();

    virtual void play();
    virtual void ffwd();
    virtual void ffwdEnd();
    virtual void rewind();
    virtual void rewindStart();
    virtual void stop();

    virtual void ffwdSimilar();
    virtual void rewindSimilar();

    virtual void deleteCurrentPane();
    virtual void deleteCurrentLayer();
    virtual void editCurrentLayer();

    virtual void previousPane();
    virtual void nextPane();
    virtual void previousLayer();
    virtual void nextLayer();

    virtual void playLoopToggled();
    virtual void playSelectionToggled();
    virtual void playSoloToggled();

    virtual void sampleRateMismatch(sv_samplerate_t, sv_samplerate_t, bool) = 0;
    virtual void audioOverloadPluginDisabled() = 0;
    virtual void audioTimeStretchMultiChannelDisabled() = 0;

    virtual void playbackFrameChanged(sv_frame_t);
    virtual void globalCentreFrameChanged(sv_frame_t);
    virtual void viewCentreFrameChanged(View *, sv_frame_t);
    virtual void viewZoomLevelChanged(View *, int, bool);
    virtual void outputLevelsChanged(float, float) = 0;

    virtual void currentPaneChanged(Pane *);
    virtual void currentLayerChanged(Pane *, Layer *);

    virtual void selectAll();
    virtual void selectToStart();
    virtual void selectToEnd();
    virtual void selectVisible();
    virtual void clearSelection();

    virtual void cut();
    virtual void copy();
    virtual void paste();
    virtual void pasteAtPlaybackPosition();
    virtual void pasteRelative(sv_frame_t offset);
    virtual void deleteSelected();

    virtual void insertInstant();
    virtual void insertInstantAt(sv_frame_t);
    virtual void insertInstantsAtBoundaries();
    virtual void insertItemAtSelection();
    virtual void insertItemAt(sv_frame_t, sv_frame_t);
    virtual void renumberInstants();

    virtual void documentModified();
    virtual void documentRestored();

    virtual void layerAdded(Layer *);
    virtual void layerRemoved(Layer *);
    virtual void layerAboutToBeDeleted(Layer *);
    virtual void layerInAView(Layer *, bool);

    virtual void mainModelChanged(WaveFileModel *);
    virtual void modelAdded(Model *);
    virtual void modelAboutToBeDeleted(Model *);

    virtual void updateMenuStates();
    virtual void updateDescriptionLabel() = 0;

    virtual void modelGenerationFailed(QString, QString) = 0;
    virtual void modelGenerationWarning(QString, QString) = 0;
    virtual void modelRegenerationFailed(QString, QString, QString) = 0;
    virtual void modelRegenerationWarning(QString, QString, QString) = 0;
    virtual void alignmentFailed(QString, QString) = 0;

    virtual void rightButtonMenuRequested(Pane *, QPoint point) = 0;

    virtual void paneAdded(Pane *) = 0;
    virtual void paneHidden(Pane *) = 0;
    virtual void paneAboutToBeDeleted(Pane *) = 0;
    virtual void paneDropAccepted(Pane *, QStringList) = 0;
    virtual void paneDropAccepted(Pane *, QString) = 0;
    virtual void paneDeleteButtonClicked(Pane *);

    virtual void oscReady();
    virtual void pollOSC();
    virtual void handleOSCMessage(const OSCMessage &) = 0;

    virtual void contextHelpChanged(const QString &);
    virtual void inProgressSelectionChanged();

    virtual FileOpenStatus openSessionFromRDF(FileSource source);
    virtual FileOpenStatus openLayersFromRDF(FileSource source);

    virtual void closeSession() = 0;

    virtual void newerVersionAvailable(QString) { }

    virtual void menuActionMapperInvoked(QObject *);

protected:
    QString                  m_sessionFile;
    QString                  m_audioFile;
    Document                *m_document;

    PaneStack               *m_paneStack;
    ViewManager             *m_viewManager;
    Layer                   *m_timeRulerLayer;

    bool                     m_audioOutput;
    AudioCallbackPlaySource *m_playSource;
    AudioCallbackPlayTarget *m_playTarget;

    class OSCQueueStarter : public QThread
    {
    public:
        OSCQueueStarter(VisualiserWindowBase *mwb) : QThread(mwb), m_mwb(mwb) { }
        virtual void run() {
            OSCQueue *queue = new OSCQueue(); // can take a long time
            m_mwb->m_oscQueue = queue;
        }
    private:
        VisualiserWindowBase *m_mwb;
    };

    OSCQueue                *m_oscQueue;
    OSCQueueStarter         *m_oscQueueStarter;
    void startOSCQueue();

    MIDIInput               *m_midiInput;

    RecentFiles              m_recentFiles;
    RecentFiles              m_recentTransforms;

    bool                     m_documentModified;
    bool                     m_openingAudioFile;
    bool                     m_abandoning;

    Labeller                *m_labeller;

    int                      m_lastPlayStatusSec;
    mutable QString          m_myStatusMessage;

    bool                     m_initialDarkBackground;

    RealTime                 m_defaultFfwdRwdStep;

    mutable QLabel *m_statusLabel;
    QLabel *getStatusLabel() const;

    WaveFileModel *getMainModel();
    const WaveFileModel *getMainModel() const;
    void createDocument();

    Pane *addPaneToStack();
    Layer *getSnapLayer() const;

    typedef std::map<Layer *, QPointer<ModelDataTableDialog> > LayerDataDialogMap;
    typedef std::set<QPointer<ModelDataTableDialog> > DataDialogSet;
    typedef std::map<View *, DataDialogSet> ViewDataDialogMap;

    LayerDataDialogMap m_layerDataDialogMap;
    ViewDataDialogMap m_viewDataDialogMap;

    void removeLayerEditDialog(Layer *);

    class PaneCallback : public SVFileReaderPaneCallback
    {
    public:
    PaneCallback(VisualiserWindowBase *mw) : m_mw(mw) { }
	virtual Pane *addPane() { return m_mw->addPaneToStack(); }
	virtual void setWindowSize(int width, int height) {
	    m_mw->resizeConstrained(QSize(width, height));
	}
	virtual void addSelection(sv_frame_t start, sv_frame_t end) {
	    m_mw->m_viewManager->addSelectionQuietly(Selection(start, end));
	}
    protected:
    VisualiserWindowBase *m_mw;
    };

    class AddPaneCommand : public SVCommand
    {
    public:
    AddPaneCommand(VisualiserWindowBase *mw);
	virtual ~AddPaneCommand();
	
	virtual void execute();
	virtual void unexecute();
	virtual QString getName() const;

	Pane *getPane() { return m_pane; }

    protected:
    VisualiserWindowBase *m_mw;
	Pane *m_pane; // Main window owns this, but I determine its lifespan
	Pane *m_prevCurrentPane; // I don't own this
	bool m_added;
    };

    class RemovePaneCommand : public SVCommand
    {
    public:
    RemovePaneCommand(VisualiserWindowBase *mw, Pane *pane);
	virtual ~RemovePaneCommand();
	
	virtual void execute();
	virtual void unexecute();
	virtual QString getName() const;

    protected:
    VisualiserWindowBase *m_mw;
	Pane *m_pane; // Main window owns this, but I determine its lifespan
	Pane *m_prevCurrentPane; // I don't own this
	bool m_added;
    };

    virtual bool checkSaveModified() = 0;

    virtual QString getOpenFileName(FileFinder::FileType type);
    virtual QString getSaveFileName(FileFinder::FileType type);
    virtual void registerLastOpenedFilePath(FileFinder::FileType type, QString path);

    virtual QString getDefaultSessionTemplate() const;
    virtual void setDefaultSessionTemplate(QString);

    virtual void createPlayTarget();
    virtual void openHelpUrl(QString url);

    virtual void setupMenus() = 0;
    virtual void updateVisibleRangeDisplay(Pane *p) const = 0;
    virtual void updatePositionStatusDisplays() const = 0;

    // Call this after setting up the menu bar, to fix up single-key
    // shortcuts on OS/X
    virtual void finaliseMenus();
    virtual void finaliseMenu(QMenu *);

    // Only used on OS/X to work around a Qt/Cocoa bug, see finaliseMenus
    QSignalMapper *m_menuShortcutMapper;
    QList<QShortcut *> m_appShortcuts;

    virtual bool shouldCreateNewSessionForRDFAudio(bool *) { return true; }

    virtual void connectLayerEditDialog(ModelDataTableDialog *dialog);

    virtual void toXml(QTextStream &stream, bool asTemplate);
};


#endif
