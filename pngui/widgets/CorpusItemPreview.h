#ifndef CORPUSITEMPREVIEW_H
#define CORPUSITEMPREVIEW_H

#include <QPointer>
#include <QWidget>
#include <QMediaPlayer>

struct CorpusItemPreviewData;
namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class CorpusItemPreview : public QWidget
{
    Q_OBJECT
public:
    explicit CorpusItemPreview(QWidget *parent = 0);
    virtual ~CorpusItemPreview();

    bool isPlayerAvailable() const;
    void openCommunication(QPointer<Praaline::Core::CorpusCommunication> com);

signals:
    void fullScreenChanged(bool fullScreen);

private slots:
    void recordingsIndexChanged(int);
    void annotationsIndexChanged(int);
    void annotationLevelsIndexChanged(int);

    void durationChanged(qint64 duration);
    void positionChanged(qint64 progress);

    void seekForward();
    void seekBackward();
    void seek(int seconds);

    void statusChanged(QMediaPlayer::MediaStatus status);
    void bufferingProgress(int progress);
    void videoAvailableChanged(bool available);

    void displayErrorMessage();
    void showColorDialog();

private:
    void setStatusInfo(const QString &info);
    void handleCursor(QMediaPlayer::MediaStatus status);
    void updateDurationInfo(qint64 currentInfo);

    CorpusItemPreviewData *d;
};

#endif // CORPUSITEMPREVIEW_H