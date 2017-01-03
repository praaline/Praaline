#ifndef MEDIAPLAYERCONTROLS_H
#define MEDIAPLAYERCONTROLS_H

#include <QMediaPlayer>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QAbstractSlider;
class QComboBox;
QT_END_NAMESPACE

struct MediaPlayerControlsData;

class MediaPlayerControls : public QWidget
{
    Q_OBJECT
public:
    explicit MediaPlayerControls(QWidget *parent = 0);
    virtual ~MediaPlayerControls();

    QMediaPlayer::State state() const;
    int volume() const;
    bool isMuted() const;
    qreal playbackRate() const;

public slots:
    void setState(QMediaPlayer::State state);
    void setVolume(int volume);
    void setMuted(bool muted);
    void setPlaybackRate(float rate);

signals:
    void play();
    void pause();
    void stop();
    void seekForward();
    void seekBackward();
    void changeVolume(int volume);
    void changeMuting(bool muting);
    void changeRate(qreal rate);

private slots:
    void playClicked();
    void muteClicked();
    void updateRate();

private:
    MediaPlayerControlsData *d;
};

#endif // MEDIAPLAYERCONTROLS_H
