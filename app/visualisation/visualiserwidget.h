#ifndef VISUALISERWIDGET_H
#define VISUALISERWIDGET_H

#include <QPointer>
#include "framework/VisualiserWindowBase.h"

class LayerTreeDialog;
class ActivityLog;
class UnitConverter;

class QFileSystemWatcher;
class QScrollArea;

class CorpusCommunication;
class CorpusRecording;
class AnnotationTierModel;
class IntervalTier;
class AnnotationTierGroup;

class VisualiserWidget : public VisualiserWindowBase
{
    Q_OBJECT

public:
    VisualiserWidget(bool withAudioOutput = true, bool withOSCSupport = true);
    virtual ~VisualiserWidget();

    void addLayerTimeInstantsFromIntevalTier(IntervalTier *tier);

signals:
    void canChangeSolo(bool);
    void canAlign(bool);

    void userScrolledToTime(RealTime);
    void playbackScrolledToTime(RealTime);

public slots:
    virtual void preferenceChanged(PropertyContainer::PropertyName);
    virtual bool commitData(bool mayAskUser);

    void goFullScreen();
    void endFullScreen();

    void newSessionWithCommunication(QPointer<CorpusCommunication> com);
    void addRecordingToSession(QPointer<CorpusRecording> rec);
    void addAnnotationPaneToSession(QMap<QString, QPointer<AnnotationTierGroup> > &tiers,
                                    const QList<QPair<QString, QString> > &attributes);
    void addProsogramPaneToSession(QPointer<CorpusRecording> rec);
    void addTappingDataPane(QMap<QString, QPointer<AnnotationTierGroup> > &tiers);

    void jumpToTime(const RealTime &time);

    void baseGlobalCentreFrameChanged(sv_frame_t);
    void basePlaybackFrameChanged(sv_frame_t);

protected slots:
    virtual void importAudio();
    virtual void importMoreAudio();
    virtual void replaceMainAudio();
    virtual void openSomething();
    virtual void openLocation();
    virtual void applyTemplate();
    virtual void exportAudio();
    virtual void exportAudioData();
    virtual void importLayer();
    virtual void exportLayer();
    virtual void exportImage();
    virtual void saveSession();
    virtual void saveSessionAs();
    virtual void newSession();
    virtual void closeSession();

    virtual void sampleRateMismatch(sv_samplerate_t, sv_samplerate_t, bool);
    virtual void audioOverloadPluginDisabled();
    virtual void audioTimeStretchMultiChannelDisabled();

    virtual void toolNavigateSelected();
    virtual void toolSelectSelected();
    virtual void toolEditSelected();
    virtual void toolDrawSelected();
    virtual void toolEraseSelected();
    virtual void toolMeasureSelected();

    virtual void documentModified();
    virtual void documentRestored();
    virtual void documentReplaced();

    virtual void updateMenuStates();
    virtual void updateDescriptionLabel();

    virtual void setInstantsNumbering();
    virtual void setInstantsCounterCycle();
    virtual void setInstantsCounters();
    virtual void resetInstantsCounters();

    virtual void modelGenerationFailed(QString, QString);
    virtual void modelGenerationWarning(QString, QString);
    virtual void modelRegenerationFailed(QString, QString, QString);
    virtual void modelRegenerationWarning(QString, QString, QString);
    virtual void alignmentFailed(QString, QString);

    virtual void rightButtonMenuRequested(Pane *, QPoint point);

    virtual void propertyStacksResized(int);

    virtual void addPane();
    virtual void addLayer();
    virtual void addLayer(QString transformId);
    virtual void renameCurrentLayer();

    virtual void findTransform();

    virtual void paneAdded(Pane *);
    virtual void paneHidden(Pane *);
    virtual void paneAboutToBeDeleted(Pane *);
    virtual void paneDropAccepted(Pane *, QStringList);
    virtual void paneDropAccepted(Pane *, QString);

    virtual void setupRecentTransformsMenu();
    virtual void setupTemplatesMenu();

    virtual void playSpeedChanged(int);
    virtual void playSoloToggled();
    virtual void alignToggled();

    virtual void currentPaneChanged(Pane *);

    virtual void speedUpPlayback();
    virtual void slowDownPlayback();
    virtual void restoreNormalPlayback();

    virtual void outputLevelsChanged(float, float);

    virtual void layerRemoved(Layer *);
    virtual void layerInAView(Layer *, bool);

    virtual void mainModelChanged(WaveFileModel *);
    virtual void mainModelGainChanged(float);
    virtual void modelAdded(Model *);
    virtual void modelAboutToBeDeleted(Model *);

    virtual void showLayerTree();
    virtual void showActivityLog();
    virtual void showUnitConverter();

    virtual void mouseEnteredWidget();
    virtual void mouseLeftWidget();

    virtual void handleOSCMessage(const OSCMessage &);
    virtual void midiEventsAvailable();
    virtual void playStatusChanged(bool);

    virtual void saveSessionAsTemplate();
    virtual void manageSavedTemplates();

    virtual void keyReference();

protected:
    Overview                *m_overview;
    Fader                   *m_fader;
    AudioDial               *m_playSpeed;
    WaveformLayer           *m_panLayer;

    QScrollArea             *m_mainScroll;

    bool                     m_mainMenusCreated;
    QMenu                   *m_paneMenu;
    QMenu                   *m_layerMenu;
    QMenu                   *m_transformsMenu;
    QMenu                   *m_playbackMenu;
    QMenu                   *m_existingLayersMenu;
    QMenu                   *m_sliceMenu;
    QMenu                   *m_recentTransformsMenu;
    QMenu                   *m_templatesMenu;
    QMenu                   *m_rightButtonMenu;
    QMenu                   *m_rightButtonLayerMenu;
    QMenu                   *m_rightButtonTransformsMenu;
    QMenu                   *m_rightButtonPlaybackMenu;

    QAction                 *m_deleteSelectedAction;
    QAction                 *m_soloAction;
    QAction                 *m_rwdStartAction;
    QAction                 *m_rwdSimilarAction;
    QAction                 *m_rwdAction;
    QAction                 *m_ffwdAction;
    QAction                 *m_ffwdSimilarAction;
    QAction                 *m_ffwdEndAction;
    QAction                 *m_playAction;
    QAction                 *m_playSelectionAction;
    QAction                 *m_playLoopAction;
    QAction                 *m_manageTemplatesAction;
    QAction                 *m_zoomInAction;
    QAction                 *m_zoomOutAction;
    QAction                 *m_zoomFitAction;
    QAction                 *m_scrollLeftAction;
    QAction                 *m_scrollRightAction;
    QAction                 *m_showPropertyBoxesAction;

    bool                     m_soloModified;
    bool                     m_prevSolo;

    QFrame                  *m_playControlsSpacer;
    int                      m_playControlsWidth;

    QLabel                  *m_descriptionLabel;
    QLabel                  *m_currentLabel;

    QPointer<LayerTreeDialog>   m_layerTreeDialog;

    ActivityLog             *m_activityLog;
    UnitConverter           *m_unitConverter;
    KeyReference            *m_keyReference;

    QFileSystemWatcher      *m_templateWatcher;

    struct LayerConfiguration {
        LayerConfiguration(LayerFactory::LayerType _layer
                           = LayerFactory::Type("TimeRuler"),
                           Model *_source = 0,
                           int _channel = -1) :
            layer(_layer), sourceModel(_source), channel(_channel) { }
        LayerFactory::LayerType layer;
        Model *sourceModel;
        int channel;
    };

    typedef std::map<QAction *, LayerConfiguration> PaneActionMap;
    PaneActionMap m_paneActions;

    typedef std::map<QAction *, LayerConfiguration> LayerActionMap;
    LayerActionMap m_layerActions;

    typedef std::map<QAction *, TransformId> TransformActionMap;
    TransformActionMap m_transformActions;

    typedef std::map<TransformId, QAction *> TransformActionReverseMap;
    TransformActionReverseMap m_transformActionsReverse;

    typedef std::map<QAction *, Layer *> ExistingLayerActionMap;
    ExistingLayerActionMap m_existingLayerActions;
    ExistingLayerActionMap m_sliceActions;

    typedef std::map<ViewManager::ToolMode, QAction *> ToolActionMap;
    ToolActionMap m_toolActions;

    typedef std::map<QAction *, int> NumberingActionMap;
    NumberingActionMap m_numberingActions;

    virtual void setupMenus();
    virtual void setupFileMenu();
    virtual void setupEditMenu();
    virtual void setupViewMenu();
    virtual void setupPaneAndLayerMenus();
    virtual void setupTransformsMenu();
    virtual void setupHelpMenu();
    virtual void setupExistingLayersMenus();
    virtual void setupToolbars();

    virtual void addPane(const LayerConfiguration &configuration, QString text);

    virtual void closeEvent(QCloseEvent *e);
    virtual bool checkSaveModified();

    virtual void exportAudio(bool asData);

    virtual void updateVisibleRangeDisplay(Pane *p) const;
    virtual void updatePositionStatusDisplays() const;

    virtual bool shouldCreateNewSessionForRDFAudio(bool *cancel);

    virtual void connectLayerEditDialog(ModelDataTableDialog *);
};

#endif // VISUALISERWIDGET_H

