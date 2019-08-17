#ifndef HELPMODEWIDGET_H
#define HELPMODEWIDGET_H

#include <QMainWindow>

struct HelpModeWidgetData;

class HelpModeWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit HelpModeWidget(QWidget *parent = nullptr);
    ~HelpModeWidget();

    bool eventFilter(QObject *object, QEvent *event);

signals:
    void activateMode();

public slots:
    //! Toggles the visibility of the dynamic help dock widget.
    void toggleDock(bool toggle);
    //! Handles a new help widget. This function makes the necessary connections.
    void handleNewHelpWidget(QWidget* widget);
    //! Display the page at \p url.
    void handleUrl(const QUrl& url);
    //! Handle requests from the help manager to display the url.
    void handleUrlRequest(const QUrl& url, bool ensure_visible);
    //! Handle home page changed.
    void handleHomePageChanged(const QUrl& url);

private:
    HelpModeWidgetData *d;
};

#endif // HELPMODEWIDGET_H
