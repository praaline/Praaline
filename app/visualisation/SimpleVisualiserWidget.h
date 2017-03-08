#ifndef SIMPLEVISUALISERWIDGET_H
#define SIMPLEVISUALISERWIDGET_H

#include <QWidget>
#include <QPointer>
#include "framework/VisualiserWindowBase.h"

class LayerTreeDialog;
class QFileSystemWatcher;
class QScrollArea;
class KeyReference;
class ActivityLog;

namespace Praaline {
namespace Core {
class CorpusCommunication;
class CorpusRecording;
}
}
using namespace Praaline::Core;

#include "QtilitiesCore/QtilitiesCore"
using namespace QtilitiesCore;

class SimpleVisualiserWidget : public VisualiserWindowBase, public Qtilities::Core::Interfaces::IContext
{
    Q_OBJECT
public:
    SimpleVisualiserWidget(const QString &contextStringID, bool withAudioOutput = true, bool withOSCSupport = true);
    virtual ~SimpleVisualiserWidget();

    // IObjectBase Implementation
    QObject* objectBase() { return this; }
    const QObject* objectBase() const { return this; }
    // IContext implementation
    virtual QString contextString() const { return m_contextStringID; }
    virtual QString contextHelpId() const { return m_contextStringID; }

signals:
    void canChangeSolo(bool);
    void canAlign(bool);

    void userScrolledToTime(RealTime);
    void playbackScrolledToTime(RealTime);

public slots:
    virtual void preferenceChanged(PropertyContainer::PropertyName);
    virtual bool commitData(bool mayAskUser);

    virtual void jumpToTime(const RealTime &time);
    virtual void baseGlobalCentreFrameChanged(sv_frame_t);
    virtual void basePlaybackFrameChanged(sv_frame_t);

    virtual void newSessionWithCommunication(QPointer<CorpusCommunication> com);
    virtual void addRecordingToSession(QPointer<CorpusRecording> rec);

protected slots:
    virtual void importAudio();
    virtual void importMoreAudio();
    virtual void replaceMainAudio();
    virtual void openSomething();
    virtual void openLocation();
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

    virtual void paneDropAccepted(Pane *, QStringList);
    virtual void paneDropAccepted(Pane *, QString);

    virtual void updateMenuStates();
    virtual void updateDescriptionLabel();

    virtual void modelGenerationFailed(QString, QString);
    virtual void modelGenerationWarning(QString, QString);
    virtual void modelRegenerationFailed(QString, QString, QString);
    virtual void modelRegenerationWarning(QString, QString, QString);
    virtual void alignmentFailed(QString, QString);

    virtual void rightButtonMenuRequested(Pane *, QPoint);
    virtual void propertyStacksResized(int);

    virtual void paneAdded(Pane *);
    virtual void paneHidden(Pane *);
    virtual void paneAboutToBeDeleted(Pane *);

    virtual void playSoloToggled();
    virtual void playSpeedChanged(int);
    virtual void speedUpPlayback();
    virtual void slowDownPlayback();
    virtual void restoreNormalPlayback();

    virtual void currentPaneChanged(Pane *);
    virtual void outputLevelsChanged(float, float);

    virtual void mainModelChanged(WaveFileModel *);
    virtual void mainModelGainChanged(float);
    virtual void modelAboutToBeDeleted(Model *);

    virtual void mouseEnteredWidget();
    virtual void mouseLeftWidget();

    virtual void handleOSCMessage(const OSCMessage &);
    virtual void midiEventsAvailable();

protected:
    QString                  m_contextStringID;

    QFrame                  *m_visualiserFrame;
    Overview                *m_overview;
    Fader                   *m_fader;
    AudioDial               *m_playSpeed;
    WaveformLayer           *m_panLayer;
    QScrollArea             *m_mainScroll;

    QMenu                   *m_rightButtonMenu;
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
    QAction                 *m_zoomInAction;
    QAction                 *m_zoomOutAction;
    QAction                 *m_zoomFitAction;
    QAction                 *m_scrollLeftAction;
    QAction                 *m_scrollRightAction;
    QAction                 *m_showPropertyBoxesAction;

    bool                     m_soloModified;
    bool                     m_prevSolo;

    QLabel                  *m_descriptionLabel;
    QLabel                  *m_currentLabel;

    ActivityLog             *m_activityLog;
    KeyReference            *m_keyReference;

    QToolBar                *m_toolbarPlayback;
    QToolBar                *m_toolbarPlayMode;

    virtual void setupMenus();
    virtual void setupEditMenu();
    virtual void setupViewMenu();
    virtual void setupPlaybackMenusAndToolbar();

    virtual bool checkSaveModified();
    virtual void exportAudio(bool asData);
    virtual void updateVisibleRangeDisplay(Pane *p) const;
    virtual void updatePositionStatusDisplays() const;

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

    virtual void addPane(const LayerConfiguration &configuration, QString text);
};

#endif // SIMPLEVISUALISERWIDGET_H
