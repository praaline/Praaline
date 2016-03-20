#ifndef AUDIOENABLEDWIDGET_H
#define AUDIOENABLEDWIDGET_H

#include <QWidget>
#include "data/model/WaveFileModel.h"
#include "view/ViewManager.h"
#include "audioio/AudioCallbackPlaySource.h"
#include "audioio/AudioCallbackPlayTarget.h"


class AudioEnabledWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AudioEnabledWidget(QWidget *parent = 0);
    ~AudioEnabledWidget();

    enum FileOpenStatus {
        FileOpenSucceeded,
        FileOpenFailed,
        FileOpenCancelled
    };

    virtual FileOpenStatus openAudio(QString fileOrUrl);

        virtual void play();

signals:

public slots:

protected slots:
    virtual void mainModelChanged(WaveFileModel *);
    virtual void ffwd();
    virtual void ffwdEnd();
    virtual void rewind();
    virtual void rewindStart();
    virtual void stop();

protected:
    WaveFileModel           *m_model;
    ViewManager             *m_viewManager;
    AudioCallbackPlaySource *m_playSource;
    AudioCallbackPlayTarget *m_playTarget;

    virtual void createPlayTarget();
};

#endif // AUDIOENABLEDWIDGET_H
