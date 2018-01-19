#ifndef WEBDESIGNERMODEWIDGET_H
#define WEBDESIGNERMODEWIDGET_H

#include <QWidget>

namespace Ui {
class WebDesignerModeWidget;
}

class WebDesignerModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WebDesignerModeWidget(QWidget *parent = 0);
    ~WebDesignerModeWidget();

private:
    Ui::WebDesignerModeWidget *ui;
};

#endif // WEBDESIGNERMODEWIDGET_H
