#include "CorpusItemPreview.h"

#include "MediaPlayerControls.h"
#include "MediaPlayerVideoWidget.h"
#include "MiniTranscriptionWidget.h"

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaService>
#include <QMediaPlaylist>
#include <QVideoWidget>
#include <QMediaMetaData>
#include <QTreeWidget>
#include <QtWidgets>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
using namespace Praaline::Core;

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
    connect(d->recordingsComboBox, SIGNAL(currentIndexChanged(int)), SLOT(recordingsIndexChanged(int)));
    // List of available annotations (for transcription preview)
    d->annotationsComboBox = new QComboBox(this);
    connect(d->annotationsComboBox, SIGNAL(currentIndexChanged(int)), SLOT(annotationsIndexChanged(int)));
    // List of available annotation levels
    d->annotationLevelsComboBox = new QComboBox(this);
    connect(d->annotationLevelsComboBox, SIGNAL(currentIndexChanged(int)), SLOT(annotationLevelsIndexChanged(int)));
    // Status label
    d->labelStatus = new QLabel(this);
    // Media player
    d->player = new QMediaPlayer(this);
    connect(d->player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(d->player, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
    connect(d->player, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));
    connect(d->player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
    connect(d->player, SIGNAL(bufferStatusChanged(int)), this, SLOT(bufferingProgress(int)));
    connect(d->player, SIGNAL(videoAvailableChanged(bool)), this, SLOT(videoAvailableChanged(bool)));
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
    connect(d->slider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));
    // Playback controls
    d->controls = new MediaPlayerControls(this);
    d->controls->setState(d->player->state());
    d->controls->setVolume(d->player->volume());
    d->controls->setMuted(d->controls->isMuted()); // this sets the icon
    // Connections from the controls to the media player and local slots
    connect(d->controls, SIGNAL(play()), d->player, SLOT(play()));
    connect(d->controls, SIGNAL(pause()), d->player, SLOT(pause()));
    connect(d->controls, SIGNAL(stop()), d->player, SLOT(stop()));
    connect(d->controls, SIGNAL(seekForward()), this, SLOT(seekForward()));
    connect(d->controls, SIGNAL(seekBackward()), this, SLOT(seekBackward()));
    connect(d->controls, SIGNAL(changeVolume(int)), d->player, SLOT(setVolume(int)));
    connect(d->controls, SIGNAL(changeMuting(bool)), d->player, SLOT(setMuted(bool)));
    connect(d->controls, SIGNAL(changeRate(qreal)), d->player, SLOT(setPlaybackRate(qreal)));
    // Connection from the controls to the video player
    connect(d->controls, SIGNAL(stop()), d->videoWidget, SLOT(update()));
    // Connections from the media player to the controls
    connect(d->player, SIGNAL(stateChanged(QMediaPlayer::State)),
            d->controls, SLOT(setState(QMediaPlayer::State)));
    connect(d->player, SIGNAL(volumeChanged(int)), d->controls, SLOT(setVolume(int)));
    connect(d->player, SIGNAL(mutedChanged(bool)), d->controls, SLOT(setMuted(bool)));
    // Full screen button
    d->fullScreenButton = new QPushButton(tr("FullScreen"), this);
    d->fullScreenButton->setCheckable(true);
    // Color adjustments button
    d->colorButton = new QPushButton(tr("Color Options..."), this);
    d->colorButton->setEnabled(false);
    connect(d->colorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));
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

void CorpusItemPreview::openCommunication(QPointer<CorpusCommunication> com)
{
    if (d->communication == com) return;
    // Clear state
    d->communication = Q_NULLPTR;
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
    foreach (QPointer<CorpusRecording> rec, com->recordings()) {
        if (!rec) continue;
        d->recordingsIDs.append(rec->ID());
        names.append(rec->name());
    }
    d->recordingsComboBox->insertItems(0, names);
    names.clear();
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        d->annotationIDs.append(annot->ID());
        names.append(annot->name());
    }
    d->annotationsComboBox->insertItems(0, names);
    // Populate combo box with annotation levels (only if necessary, i.e. if corpus changed)
    QPointer<Corpus> corpus = com->corpus();
    if (corpus && corpus->annotationStructure()) {
        QStringList levelIDs = corpus->annotationStructure()->levelIDs();
        if (d->levelIDs != levelIDs) {
            d->annotationLevelsComboBox->clear();
            d->annotationLevelsComboBox->insertItems(0, levelIDs);
            d->levelIDs = levelIDs;
        }
    }
}

void CorpusItemPreview::recordingsIndexChanged(int index)
{
    if ((index < 0) || (index >= d->recordingsIDs.count())) return;
    if (!d->communication) { d->player->setMedia(QUrl()); return; }
    QString recordingID = d->recordingsIDs.at(index);
    QPointer<CorpusRecording> rec = d->communication->recording(recordingID);
    if (!rec) { d->player->setMedia(QUrl()); return; }
    d->player->setMedia(rec->mediaUrl());
}

void CorpusItemPreview::annotationsIndexChanged(int index)
{
    if ((index < 0) || (index >= d->annotationIDs.count())) return;
    if (!d->communication) { d->transcriptionWidget->setAnnotation(Q_NULLPTR); return; }
    QString annotationID = d->annotationIDs.at(index);
    QPointer<CorpusAnnotation> annot = d->communication->annotation(annotationID);
    if (!annot) { d->transcriptionWidget->setAnnotation(Q_NULLPTR); return; }
    d->transcriptionWidget->setAnnotation(annot);
}

void CorpusItemPreview::annotationLevelsIndexChanged(int)
{
    d->transcriptionWidget->setTranscriptionLevelID(d->annotationLevelsComboBox->currentText());
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
        disconnect(d->fullScreenButton, SIGNAL(clicked(bool)), d->videoWidget, SLOT(setFullScreen(bool)));
        disconnect(d->videoWidget, SIGNAL(fullScreenChanged(bool)), d->fullScreenButton, SLOT(setChecked(bool)));
        d->videoWidget->setFullScreen(false);
    } else {
        connect(d->fullScreenButton, SIGNAL(clicked(bool)), d->videoWidget, SLOT(setFullScreen(bool)));
        connect(d->videoWidget, SIGNAL(fullScreenChanged(bool)), d->fullScreenButton, SLOT(setChecked(bool)));
        if (d->fullScreenButton->isChecked())
            d->videoWidget->setFullScreen(true);
    }
    d->colorButton->setEnabled(available);
}

void CorpusItemPreview::setStatusInfo(const QString &info)
{
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
        connect(brightnessSlider, SIGNAL(sliderMoved(int)), d->videoWidget, SLOT(setBrightness(int)));
        connect(d->videoWidget, SIGNAL(brightnessChanged(int)), brightnessSlider, SLOT(setValue(int)));

        QSlider *contrastSlider = new QSlider(Qt::Horizontal);
        contrastSlider->setRange(-100, 100);
        contrastSlider->setValue(d->videoWidget->contrast());
        connect(contrastSlider, SIGNAL(sliderMoved(int)), d->videoWidget, SLOT(setContrast(int)));
        connect(d->videoWidget, SIGNAL(contrastChanged(int)), contrastSlider, SLOT(setValue(int)));

        QSlider *hueSlider = new QSlider(Qt::Horizontal);
        hueSlider->setRange(-100, 100);
        hueSlider->setValue(d->videoWidget->hue());
        connect(hueSlider, SIGNAL(sliderMoved(int)), d->videoWidget, SLOT(setHue(int)));
        connect(d->videoWidget, SIGNAL(hueChanged(int)), hueSlider, SLOT(setValue(int)));

        QSlider *saturationSlider = new QSlider(Qt::Horizontal);
        saturationSlider->setRange(-100, 100);
        saturationSlider->setValue(d->videoWidget->saturation());
        connect(saturationSlider, SIGNAL(sliderMoved(int)), d->videoWidget, SLOT(setSaturation(int)));
        connect(d->videoWidget, SIGNAL(saturationChanged(int)), saturationSlider, SLOT(setValue(int)));

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

        connect(button, SIGNAL(clicked()), d->colorDialog, SLOT(close()));
    }
    d->colorDialog->show();
}
