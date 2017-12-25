#ifndef STATUSMESSAGESWIDGET_H
#define STATUSMESSAGESWIDGET_H

#include <QWidget>

struct StatusMessagesWidgetData;

class StatusMessagesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StatusMessagesWidget(QWidget *parent = nullptr);
    ~StatusMessagesWidget();

    void appendMessage(const QString &);

signals:

public slots:
    void actionSaveOutput();
    void actionClearOutput();
    void setToolbarVisible(bool);

private:
    StatusMessagesWidgetData *d;
};

#endif // STATUSMESSAGESWIDGET_H
