#ifndef MEDIAPLAYERVIDEOWIDGET_H
#define MEDIAPLAYERVIDEOWIDGET_H

#include <QVideoWidget>

class MediaPlayerVideoWidget : public QVideoWidget
{
    Q_OBJECT

public:
    MediaPlayerVideoWidget(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
};

#endif // MEDIAPLAYERVIDEOWIDGET_H
