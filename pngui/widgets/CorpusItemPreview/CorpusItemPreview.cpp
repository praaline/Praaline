#include <QWidget>
#include <QPointer>
#include <QMediaPlayer>
#include <QMediaService>
#include <QMediaPlaylist>
#include <QVideoWidget>
#include <QMediaMetaData>
#include <QTreeWidget>
#include <QtWidgets>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
using namespace Praaline::Core;

#include "MediaPlayerControls.h"
#include "MediaPlayerVideoWidget.h"
#include "MiniTranscriptionWidget.h"

#include "CorpusItemPreview.h"


struct CorpusItemPreviewData {
    CorpusItemPreviewData() :
        videoWidget(0), slider(0), colorDialog(0), transcriptionWidget(0), communication(0)
    {}

    QComboBox *recordingsComboBox;
    QComboBox *annotationsComboBox;
    QComboBox *annotationLevelsComboBox;
    QLabel *labelStatus;

    QMediaPlayer *player;
    MediaPlayerControls *controls;
    MediaPlayerVideoWidget *videoWidget;
    QSlider *slider;
    QLabel *labelDuration;
    QPushButton *fullScreenButton;
    QPushButton *colorButton;
    QDialog *colorDialog;
    MiniTranscriptionWidget *transcriptionWidget;

    QString trackInfo;
    QString statusInfo;
    qint64 duration;

    QPointer<CorpusCommunication> communication;
    QStringList recordingsIDs;
    QStringList annotationIDs;
    QStringList levelIDs;
};

CorpusItemPreview::CorpusItemPreview(QWidget *parent) :
    QWidget(parent), d (new CorpusItemPreviewData)
{
    // List of available recordings (media files) to preview
    d->recordingsComboBox = new QComboBox(this);
    connect(d->recordingsComboBox, &QComboBox::currentTextChanged, this, &CorpusItemPreview::recordingChanged);
    // List of available annotations (for transcription preview)
    d->annotationsComboBox = new QComboBox(this);
    connect(d->annotationsComboBox, &QComboBox::currentTextChanged, this, &CorpusItemPreview::annotationChanged);
    // List of available annotation levels
    d->annotationLevelsComboBox = new QComboBox(this);
    connect(d->annotationLevelsComboBox, &QComboBox::currentTextChanged, this, &CorpusItemPreview::annotationLevelChanged);
    // Status label
    d->labelStatus = new QLabel(this);
    // Media player
    d->player = new QMediaPlayer(this);
    connect(d->player, &QMediaPlayer::durationChanged, this, &CorpusItemPreview::durationChanged);
    connect(d->player, &QMediaPlayer::positionChanged, this, &CorpusItemPreview::positionChanged);
    connect(d->player, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));
    connect(d->player, &QMediaPlayer::mediaStatusChanged,
            this, &CorpusItemPreview::statusChanged);
    connect(d->player, &QMediaPlayer::bufferStatusChanged, this, &CorpusItemPreview::bufferingProgress);
    connect(d->player, &QMediaPlayer::videoAvailableChanged, this, &CorpusItemPreview::videoAvailableChanged);
    connect(d->player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(displayErrorMessage()));
    // Video player widget
    d->videoWidget = new MediaPlayerVideoWidget(this);
    d->player->setVideoOutput(d->videoWidget);
    d->videoWidget->setVisible(false); // only becomes visible when there is a video to show
    // Slider (position in currently playing media file)
    d->slider = new QSlider(Qt::Horizontal, this);
    d->slider->setRange(0, d->player->duration() / 1000);
    // Duration label
    d->labelDuration = new QLabel(this);
    connect(d->slider, &QAbstractSlider::sliderMoved, this, &CorpusItemPreview::seek);
    // Playback controls
    d->controls = new MediaPlayerControls(this);
    d->controls->setState(d->player->state());
    d->controls->setVolume(d->player->volume());
    d->controls->setMuted(d->controls->isMuted()); // this sets the icon
    // Connections from the controls to the media player and local slots
    connect(d->controls, &MediaPlayerControls::play, d->player, &QMediaPlayer::play);
    connect(d->controls, &MediaPlayerControls::pause, d->player, &QMediaPlayer::pause);
    connect(d->controls, &MediaPlayerControls::stop, d->player, &QMediaPlayer::stop);
    connect(d->controls, &MediaPlayerControls::seekForward, this, &CorpusItemPreview::seekForward);
    connect(d->controls, &MediaPlayerControls::seekBackward, this, &CorpusItemPreview::seekBackward);
    connect(d->controls, &MediaPlayerControls::changeVolume, d->player, &QMediaPlayer::setVolume);
    connect(d->controls, &MediaPlayerControls::changeMuting, d->player, &QMediaPlayer::setMuted);
    connect(d->controls, &MediaPlayerControls::changeRate, d->player, &QMediaPlayer::setPlaybackRate);
    // Connection from the controls to the video player
    connect(d->controls, SIGNAL(stop()), d->videoWidget, SLOT(update()));
    // Connections from the media player to the controls
    connect(d->player, &QMediaPlayer::stateChanged,
            d->controls, &MediaPlayerControls::setState);
    connect(d->player, &QMediaPlayer::volumeChanged, d->controls, &MediaPlayerControls::setVolume);
    connect(d->player, &QMediaPlayer::mutedChanged, d->controls, &MediaPlayerControls::setMuted);
    // Full screen button
    d->fullScreenButton = new QPushButton(tr("FullScreen"), this);
    d->fullScreenButton->setCheckable(true);
    // Color adjustments button
    d->colorButton = new QPushButton(tr("Color Options..."), this);
    d->colorButton->setEnabled(false);
    connect(d->colorButton, &QAbstractButton::clicked, this, &CorpusItemPreview::showColorDialog);
    // Transcription preview widget
    d->transcriptionWidget = new MiniTranscriptionWidget(this);
    // Create layout
    QBoxLayout *selectionLayout = new QHBoxLayout;
    selectionLayout->addWidget(new QLabel(tr("Media file: "), this));
    selectionLayout->addWidget(d->recordingsComboBox, 1);
    selectionLayout->addWidget(new QLabel(tr("Annotation: "), this));
    selectionLayout->addWidget(d->annotationsComboBox, 1);
    selectionLayout->addWidget(new QLabel(tr("Preview level: "), this));
    selectionLayout->addWidget(d->annotationLevelsComboBox, 1);
    selectionLayout->addWidget(d->labelStatus, 2);

    QBoxLayout *displayLayout = new QHBoxLayout;
    displayLayout->addWidget(d->videoWidget);
    displayLayout->addWidget(d->transcriptionWidget, 2);

    QHBoxLayout *sliderLayout = new QHBoxLayout;
    sliderLayout->addWidget(d->slider);
    sliderLayout->addWidget(d->labelDuration);

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addStretch(1);
    controlLayout->addWidget(d->controls);
    controlLayout->addStretch(1);
    controlLayout->addWidget(d->fullScreenButton);
    controlLayout->addWidget(d->colorButton);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(selectionLayout);
    layout->addLayout(displayLayout);
    layout->addLayout(sliderLayout);
    layout->addLayout(controlLayout);
    setLayout(layout);

    if (!isPlayerAvailable()) {
        QMessageBox::warning(this, tr("Service not available"),
                             tr("The QMediaPlayer object does not have a valid service.\n"\
                                "Please check the media service plugins are installed."));
        d->controls->setEnabled(false);
        d->colorButton->setEnabled(false);
        d->fullScreenButton->setEnabled(false);
    }
}

CorpusItemPreview::~CorpusItemPreview()
{
    delete d;
}

bool CorpusItemPreview::isPlayerAvailable() const
{
    return d->player->isAvailable();
}

void CorpusItemPreview::clear()
{
    // Clear state
    d->communication = nullptr;
    d->recordingsIDs.clear();
    d->annotationIDs.clear();
    d->recordingsComboBox->clear();
    d->annotationsComboBox->clear();
    d->annotationLevelsComboBox->clear();
    d->player->stop();
    d->transcriptionWidget->clear();
}

void CorpusItemPreview::openCommunication(CorpusCommunication *com)
{
    if (d->communication == com) return;
    // Clear state
    d->communication = nullptr;
    d->recordingsIDs.clear();
    d->annotationIDs.clear();
    d->recordingsComboBox->clear();
    d->annotationsComboBox->clear();
    d->player->stop();
    // Check that this is a valid communication
    if (!com) return;
    // Populate combo boxes (recording/annotation names) and internal state (recording/annotation IDs)
    d->communication = com;
    QStringList names;
    foreach (CorpusRecording *rec, com->recordings()) {
        if (!rec) continue;
        d->recordingsIDs.append(rec->ID());
        names.append(rec->name());
    }
    d->recordingsComboBox->insertItems(0, names);
    names.clear();
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        d->annotationIDs.append(annot->ID());
        names.append(annot->name());
    }
    d->annotationsComboBox->insertItems(0, names);
    // Populate combo box with annotation levels (only if necessary, i.e. if corpus changed)
    QPointer<Corpus> corpus = com->corpus();
    if (corpus && corpus->repository() && corpus->repository()->annotationStructure()) {
        QStringList levelIDs = corpus->repository()->annotationStructure()->levelIDs();
        if (d->levelIDs != levelIDs) {
            d->annotationLevelsComboBox->clear();
            d->annotationLevelsComboBox->insertItems(0, levelIDs);
            d->levelIDs = levelIDs;
            annotationLevelChanged("");
        }
    }
}

void CorpusItemPreview::recordingChanged(const QString &recordingID)
{
    if (!d->communication) { d->player->setMedia(QUrl()); return; }
    CorpusRecording *rec = d->communication->recording(recordingID);
    if (!rec) { d->player->setMedia(QUrl()); return; }
    d->player->setMedia(rec->mediaUrl());
}

void CorpusItemPreview::annotationChanged(const QString &annotationID)
{
    if (!d->communication) { d->transcriptionWidget->setAnnotation(nullptr); return; }
    CorpusAnnotation *annot = d->communication->annotation(annotationID);
    if (!annot) { d->transcriptionWidget->setAnnotation(nullptr); return; }
    d->transcriptionWidget->setAnnotation(annot);
}

void CorpusItemPreview::annotationLevelChanged(const QString &levelID)
{
    d->transcriptionWidget->setTranscriptionLevelID(levelID);
}

void CorpusItemPreview::durationChanged(qint64 duration)
{
    d->duration = duration / 1000;
    d->slider->setMaximum(duration / 1000);
}

void CorpusItemPreview::positionChanged(qint64 progress)
{
    if (!d->slider->isSliderDown()) {
        d->slider->setValue(progress / 1000);
    }
    updateDurationInfo(progress / 1000);
}

void CorpusItemPreview::seekBackward()
{
    qint64 pos = d->player->position();
    pos = pos - 2500; if(pos < 0) pos = 0;
    d->player->setPosition(pos);
}

void CorpusItemPreview::seekForward()
{
    qint64 pos = d->player->position();
    pos = pos + 2500; if(pos > d->player->duration()) pos = d->player->duration();
    d->player->setPosition(pos);
}

void CorpusItemPreview::seek(int seconds)
{
    d->player->setPosition(seconds * 1000);
}

void CorpusItemPreview::statusChanged(QMediaPlayer::MediaStatus status)
{
    handleCursor(status);
    // handle status message
    switch (status) {
    case QMediaPlayer::UnknownMediaStatus:
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
        setStatusInfo(QString());
        break;
    case QMediaPlayer::LoadingMedia:
        setStatusInfo(tr("Loading..."));
        break;
    case QMediaPlayer::StalledMedia:
        setStatusInfo(tr("Media Stalled"));
        break;
    case QMediaPlayer::EndOfMedia:
        QApplication::alert(this);
        break;
    case QMediaPlayer::InvalidMedia:
        displayErrorMessage();
        break;
    }
}

void CorpusItemPreview::handleCursor(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::LoadingMedia ||
        status == QMediaPlayer::BufferingMedia ||
        status == QMediaPlayer::StalledMedia)
        setCursor(QCursor(Qt::BusyCursor));
    else
        unsetCursor();
}

void CorpusItemPreview::bufferingProgress(int progress)
{
    setStatusInfo(tr("Buffering %4%").arg(progress));
}

void CorpusItemPreview::videoAvailableChanged(bool available)
{
    if (!available) {
        disconnect(d->fullScreenButton, &QAbstractButton::clicked, d->videoWidget, &QVideoWidget::setFullScreen);
        disconnect(d->videoWidget, &QVideoWidget::fullScreenChanged, d->fullScreenButton, &QAbstractButton::setChecked);
        d->videoWidget->setFullScreen(false);
    } else {
        connect(d->fullScreenButton, &QAbstractButton::clicked, d->videoWidget, &QVideoWidget::setFullScreen);
        connect(d->videoWidget, &QVideoWidget::fullScreenChanged, d->fullScreenButton, &QAbstractButton::setChecked);
        if (d->fullScreenButton->isChecked())
            d->videoWidget->setFullScreen(true);
    }
    d->colorButton->setEnabled(available);
}

void CorpusItemPreview::setStatusInfo(const QString &info)
{
    Q_UNUSED(info)
//    statusInfo = info;
//    if (!statusInfo.isEmpty())
//        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
//    else
//        setWindowTitle(trackInfo);
}

void CorpusItemPreview::displayErrorMessage()
{
    setStatusInfo(d->player->errorString());
}

void CorpusItemPreview::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || d->duration) {
        QTime currentTime((currentInfo/3600)%60, (currentInfo/60)%60, currentInfo%60, (currentInfo*1000)%1000);
        QTime totalTime((d->duration/3600)%60, (d->duration/60)%60, d->duration%60, (d->duration*1000)%1000);
        QString format = "mm:ss";
        if (d->duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    d->labelDuration->setText(tStr);
}

void CorpusItemPreview::showColorDialog()
{
    if (!d->colorDialog) {
        QSlider *brightnessSlider = new QSlider(Qt::Horizontal);
        brightnessSlider->setRange(-100, 100);
        brightnessSlider->setValue(d->videoWidget->brightness());
        connect(brightnessSlider, &QAbstractSlider::sliderMoved, d->videoWidget, &QVideoWidget::setBrightness);
        connect(d->videoWidget, &QVideoWidget::brightnessChanged, brightnessSlider, &QAbstractSlider::setValue);

        QSlider *contrastSlider = new QSlider(Qt::Horizontal);
        contrastSlider->setRange(-100, 100);
        contrastSlider->setValue(d->videoWidget->contrast());
        connect(contrastSlider, &QAbstractSlider::sliderMoved, d->videoWidget, &QVideoWidget::setContrast);
        connect(d->videoWidget, &QVideoWidget::contrastChanged, contrastSlider, &QAbstractSlider::setValue);

        QSlider *hueSlider = new QSlider(Qt::Horizontal);
        hueSlider->setRange(-100, 100);
        hueSlider->setValue(d->videoWidget->hue());
        connect(hueSlider, &QAbstractSlider::sliderMoved, d->videoWidget, &QVideoWidget::setHue);
        connect(d->videoWidget, &QVideoWidget::hueChanged, hueSlider, &QAbstractSlider::setValue);

        QSlider *saturationSlider = new QSlider(Qt::Horizontal);
        saturationSlider->setRange(-100, 100);
        saturationSlider->setValue(d->videoWidget->saturation());
        connect(saturationSlider, &QAbstractSlider::sliderMoved, d->videoWidget, &QVideoWidget::setSaturation);
        connect(d->videoWidget, &QVideoWidget::saturationChanged, saturationSlider, &QAbstractSlider::setValue);

        QFormLayout *layout = new QFormLayout;
        layout->addRow(tr("Brightness"), brightnessSlider);
        layout->addRow(tr("Contrast"), contrastSlider);
        layout->addRow(tr("Hue"), hueSlider);
        layout->addRow(tr("Saturation"), saturationSlider);

        QPushButton *button = new QPushButton(tr("Close"));
        layout->addRow(button);

        d->colorDialog = new QDialog(this);
        d->colorDialog->setWindowTitle(tr("Color Options"));
        d->colorDialog->setLayout(layout);

        connect(button, &QAbstractButton::clicked, d->colorDialog, &QWidget::close);
    }
    d->colorDialog->show();
}
