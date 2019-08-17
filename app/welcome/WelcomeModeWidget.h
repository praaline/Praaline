#ifndef WELCOMEMODEWIDGET_H
#define WELCOMEMODEWIDGET_H

#include <QWidget>

namespace Ui {
class WelcomeModeWidget;
}

class WelcomeModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeModeWidget(QWidget *parent = nullptr);
    ~WelcomeModeWidget();

private:
    Ui::WelcomeModeWidget *ui;
};

#endif // WELCOMEMODEWIDGET_H
