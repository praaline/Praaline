#ifndef SCRIPTINGMODEWIDGET_H
#define SCRIPTINGMODEWIDGET_H

#include <QWidget>

namespace Ui {
class ScriptingModeWidget;
}

struct ScriptingModeWidgetData;

class ScriptingModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScriptingModeWidget(QWidget *parent = 0);
    ~ScriptingModeWidget();

signals:
    void activateMode();

private slots:
    void showScriptEditor();
    void showInteractiveConsole();

private:
    Ui::ScriptingModeWidget *ui;
    ScriptingModeWidgetData *d;

    void setupActions();
};

#endif // SCRIPTINGMODEWIDGET_H
