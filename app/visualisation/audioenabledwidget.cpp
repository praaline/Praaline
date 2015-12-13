#include <QSettings>
#include <QApplication>
#include <QMessageBox>
#include "base/Preferences.h"
#include "base/PlayParameterRepository.h"
#include "audioio/AudioCallbackPlaySource.h"
#include "audioio/AudioCallbackPlayTarget.h"
#include "audioio/AudioTargetFactory.h"
#include "audioio/PlaySpeedRangeMapper.h"
#include "data/fileio/DataFileReaderFactory.h"
#include "data/fileio/FileSource.h"
#include "data/fileio/AudioFileReaderFactory.h"
#include "data/model/WaveFileModel.h"
#include "widgets/ProgressDialog.h"

#include "audioenabledwidget.h"

AudioEnabledWidget::AudioEnabledWidget(QWidget *parent) :
    QWidget(parent), m_model(0), m_viewManager(0), m_playSource(0), m_playTarget(0)
{
    m_viewManager = new ViewManager();
    m_playSource = new AudioCallbackPlaySource(m_viewManager, QApplication::applicationName());
}

AudioEnabledWidget::~AudioEnabledWidget()
{
    if (m_playTarget) m_playTarget->shutdown();
    delete m_playSource;
}

AudioEnabledWidget::FileOpenStatus AudioEnabledWidget::openAudio(QString fileOrUrl)
{
    ProgressDialog dialog(tr("Opening file or URL..."), true, 2000, this);
    FileSource source = FileSource(fileOrUrl, &dialog);

    if (!source.isAvailable()) return FileOpenFailed;
    source.waitForData();

    if (!AudioFileReaderFactory::getKnownExtensions().contains(source.getExtension().toLower()))
        return FileOpenFailed;

    size_t rate = 0;

    if (Preferences::getInstance()->getFixedSampleRate() != 0) {
        rate = Preferences::getInstance()->getFixedSampleRate();
    } else if (Preferences::getInstance()->getResampleOnLoad()) {
        rate = m_playSource->getSourceSampleRate();
    }

    WaveFileModel *newModel = new WaveFileModel(source, rate);

    if (m_model) {
        m_playSource->removeModel(m_model);
        PlayParameterRepository::getInstance()->removePlayable(m_model);
    }
    PlayParameterRepository::getInstance()->addPlayable(newModel);
    m_model = newModel;

    mainModelChanged(newModel);

    return FileOpenSucceeded;
}

void AudioEnabledWidget::createPlayTarget()
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
        if (factory->isAutoCallbackTarget(targetName)) {
            QMessageBox::warning
                    (this, tr("Couldn't open audio device"),
                     tr("<b>No audio available</b><p>Could not open an audio device for playback.<p>Automatic audio device detection failed. Audio playback will not be available during this session.</p>"),
                     QMessageBox::Ok);
        } else {
            QMessageBox::warning
                    (this, tr("Couldn't open audio device"),
                     tr("<b>No audio available</b><p>Failed to open your preferred audio device (\"%1\").<p>Audio playback will not be available during this session.</p>")
                     .arg(factory->getCallbackTargetDescription(targetName)), QMessageBox::Ok);
        }
    }
}

void AudioEnabledWidget::mainModelChanged(WaveFileModel *model)
{
    if (model) m_viewManager->setMainModelSampleRate(model->getSampleRate());
    if (model && !m_playTarget) {
        createPlayTarget();
    }
}

void AudioEnabledWidget::play()
{
    if (m_playSource->isPlaying()) {
        stop();
    } else {
        // playbackFrameChanged(m_viewManager->getPlaybackFrame());
        m_playSource->play(m_viewManager->getPlaybackFrame());
    }
}

void AudioEnabledWidget::ffwd()
{

}

void AudioEnabledWidget::ffwdEnd()
{

}

void AudioEnabledWidget::rewind()
{

}

void AudioEnabledWidget::rewindStart()
{

}

void AudioEnabledWidget::stop()
{

}


