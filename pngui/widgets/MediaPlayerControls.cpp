#include "MediaPlayerControls.h"

#include <QBoxLayout>
#include <QSlider>
#include <QStyle>
#include <QToolButton>
#include <QComboBox>

struct MediaPlayerControlsData {
    MediaPlayerControlsData() :
        playerState(QMediaPlayer::StoppedState), playerMuted(false),
        playButton(0), stopButton(0), seekForwardButton(0), seekBackwardButton(0),
        muteButton(0), volumeSlider(0), rateBox(0)
    {}

    QMediaPlayer::State playerState;
    bool playerMuted;
    QAbstractButton *playButton;
    QAbstractButton *stopButton;
    QAbstractButton *seekForwardButton;
    QAbstractButton *seekBackwardButton;
    QAbstractButton *muteButton;
    QAbstractSlider *volumeSlider;
    QComboBox *rateBox;
};

MediaPlayerControls::MediaPlayerControls(QWidget *parent) :
    QWidget(parent), d(new MediaPlayerControlsData)
{
    d->playButton = new QToolButton(this);
    d->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(d->playButton, SIGNAL(clicked()), this, SLOT(playClicked()));

    d->stopButton = new QToolButton(this);
    d->stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    d->stopButton->setEnabled(false);
    connect(d->stopButton, SIGNAL(clicked()), this, SIGNAL(stop()));

    d->seekForwardButton = new QToolButton(this);
    d->seekForwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    connect(d->seekForwardButton, SIGNAL(clicked()), this, SIGNAL(seekForward()));

    d->seekBackwardButton = new QToolButton(this);
    d->seekBackwardButton->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    connect(d->seekBackwardButton, SIGNAL(clicked()), this, SIGNAL(seekBackward()));

    d->muteButton = new QToolButton(this);
    d->muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    connect(d->muteButton, SIGNAL(clicked()), this, SLOT(muteClicked()));

    d->volumeSlider = new QSlider(Qt::Horizontal, this);
    d->volumeSlider->setRange(0, 100);
    connect(d->volumeSlider, SIGNAL(sliderMoved(int)), this, SIGNAL(changeVolume(int)));

    d->rateBox = new QComboBox(this);
    d->rateBox->addItem("0.5x", QVariant(0.5));
    d->rateBox->addItem("1.0x", QVariant(1.0));
    d->rateBox->addItem("2.0x", QVariant(2.0));
    d->rateBox->setCurrentIndex(1);
    connect(d->rateBox, SIGNAL(activated(int)), SLOT(updateRate()));

    QBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->addWidget(d->stopButton);
    layout->addWidget(d->seekBackwardButton);
    layout->addWidget(d->playButton);
    layout->addWidget(d->seekForwardButton);
    layout->addWidget(d->muteButton);
    layout->addWidget(d->volumeSlider);
    layout->addWidget(d->rateBox);
    setLayout(layout);
}

MediaPlayerControls::~MediaPlayerControls()
{
    delete d;
}


QMediaPlayer::State MediaPlayerControls::state() const
{
    return d->playerState;
}

void MediaPlayerControls::setState(QMediaPlayer::State state)
{
    if (state != d->playerState) {
        d->playerState = state;
        switch (state) {
        case QMediaPlayer::StoppedState:
            d->stopButton->setEnabled(false);
            d->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
        case QMediaPlayer::PlayingState:
            d->stopButton->setEnabled(true);
            d->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            break;
        case QMediaPlayer::PausedState:
            d->stopButton->setEnabled(true);
            d->playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
        }
    }
}

int MediaPlayerControls::volume() const
{
    return d->volumeSlider ? d->volumeSlider->value() : 0;
}

void MediaPlayerControls::setVolume(int volume)
{
    if (d->volumeSlider)
        d->volumeSlider->setValue(volume);
}

bool MediaPlayerControls::isMuted() const
{
    return d->playerMuted;
}

void MediaPlayerControls::setMuted(bool muted)
{
    if (muted != d->playerMuted) {
        d->playerMuted = muted;
        d->muteButton->setIcon(style()->standardIcon(muted
                ? QStyle::SP_MediaVolumeMuted
                : QStyle::SP_MediaVolume));
    }
}

void MediaPlayerControls::playClicked()
{
    switch (d->playerState) {
    case QMediaPlayer::StoppedState:
    case QMediaPlayer::PausedState:
        emit play();
        break;
    case QMediaPlayer::PlayingState:
        emit pause();
        break;
    }
}

void MediaPlayerControls::muteClicked()
{
    emit changeMuting(!d->playerMuted);
}

qreal MediaPlayerControls::playbackRate() const
{
    return d->rateBox ? d->rateBox->itemData(d->rateBox->currentIndex()).toDouble() : 1.0;
}

void MediaPlayerControls::setPlaybackRate(float rate)
{
    for (int i = 0; i < d->rateBox->count(); ++i) {
        if (qFuzzyCompare(rate, float(d->rateBox->itemData(i).toDouble()))) {
            d->rateBox->setCurrentIndex(i);
            return;
        }
    }

    d->rateBox->addItem(QString("%1x").arg(rate), QVariant(rate));
    d->rateBox->setCurrentIndex(d->rateBox->count() - 1);
}

void MediaPlayerControls::updateRate()
{
    emit changeRate(playbackRate());
}
