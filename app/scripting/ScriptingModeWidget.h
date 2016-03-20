#ifndef SCRIPTINGMODEWIDGET_H
#define SCRIPTINGMODEWIDGET_H

#include <QWidget>

namespace Ui {
class ScriptingModeWidget;
}

class ScriptingModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScriptingModeWidget(QWidget *parent = 0);
    ~ScriptingModeWidget();

private:
    Ui::ScriptingModeWidget *ui;
};

#endif // SCRIPTINGMODEWIDGET_H
