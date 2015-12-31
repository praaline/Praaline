#include <QMessageBox>
#include "SimpleVisualiserWidget.h"

SimpleVisualiserWidget::SimpleVisualiserWidget(const QString &contextStringID, bool withAudioOutput, bool withOSCSupport) :
    VisualiserWindowBase(withAudioOutput, withOSCSupport), m_contextStringID(contextStringID),
    m_overview(0)
{

}

SimpleVisualiserWidget::~SimpleVisualiserWidget()
{

}

// public slot
void SimpleVisualiserWidget::preferenceChanged(PropertyContainer::PropertyName)
{

}

// public slot
bool SimpleVisualiserWidget::commitData(bool mayAskUser)
{

}

// public slot
void SimpleVisualiserWidget::goFullScreen()
{

}

// public slot
void SimpleVisualiserWidget::endFullScreen()
{

}

// public slot
void SimpleVisualiserWidget::jumpToTime(const RealTime &time)
{

}

// public slot
void SimpleVisualiserWidget::baseGlobalCentreFrameChanged(sv_frame_t)
{

}

// public slot
void SimpleVisualiserWidget::basePlaybackFrameChanged(sv_frame_t)
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

void SimpleVisualiserWidget::sampleRateMismatch(sv_samplerate_t, sv_samplerate_t, bool)
{

}

void SimpleVisualiserWidget::audioOverloadPluginDisabled()
{

}

void SimpleVisualiserWidget::audioTimeStretchMultiChannelDisabled()
{

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

}

void SimpleVisualiserWidget::documentRestored()
{

}

void SimpleVisualiserWidget::documentReplaced()
{

}

void SimpleVisualiserWidget::updateMenuStates()
{

}

void SimpleVisualiserWidget::updateDescriptionLabel()
{

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

void SimpleVisualiserWidget::rightButtonMenuRequested(Pane *, QPoint point)
{

}

void SimpleVisualiserWidget::propertyStacksResized(int)
{

}

void SimpleVisualiserWidget::paneAdded(Pane *)
{

}

void SimpleVisualiserWidget::paneHidden(Pane *)
{

}

void SimpleVisualiserWidget::paneAboutToBeDeleted(Pane *)
{

}

void SimpleVisualiserWidget::paneDropAccepted(Pane *, QStringList)
{

}

void SimpleVisualiserWidget::paneDropAccepted(Pane *, QString)
{

}

void SimpleVisualiserWidget::playSpeedChanged(int)
{

}

void SimpleVisualiserWidget::playSoloToggled()
{

}

void SimpleVisualiserWidget::speedUpPlayback()
{

}

void SimpleVisualiserWidget::slowDownPlayback()
{

}

void SimpleVisualiserWidget::restoreNormalPlayback()
{

}

void SimpleVisualiserWidget::currentPaneChanged(Pane *)
{

}

void SimpleVisualiserWidget::outputLevelsChanged(float, float)
{

}

void SimpleVisualiserWidget::layerRemoved(Layer *)
{

}

void SimpleVisualiserWidget::layerInAView(Layer *, bool)
{

}

void SimpleVisualiserWidget::mainModelChanged(WaveFileModel *)
{

}

void SimpleVisualiserWidget::mainModelGainChanged(float)
{

}

void SimpleVisualiserWidget::modelAdded(Model *)
{

}

void SimpleVisualiserWidget::modelAboutToBeDeleted(Model *)
{

}

void SimpleVisualiserWidget::mouseEnteredWidget()
{

}

void SimpleVisualiserWidget::mouseLeftWidget()
{

}

void SimpleVisualiserWidget::handleOSCMessage(const OSCMessage &)
{

}

void SimpleVisualiserWidget::midiEventsAvailable()
{

}

void SimpleVisualiserWidget::playStatusChanged(bool)
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

void SimpleVisualiserWidget::setupToolbars()
{

}

void SimpleVisualiserWidget::addPane(const LayerConfiguration &configuration, QString text)
{

}

void SimpleVisualiserWidget::exportAudio(bool asData)
{

}

void SimpleVisualiserWidget::updateVisibleRangeDisplay(Pane *p) const
{

}

void SimpleVisualiserWidget::updatePositionStatusDisplays() const
{

}

