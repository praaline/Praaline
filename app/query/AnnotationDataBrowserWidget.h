#ifndef ANNOTATIONDATABROWSERWIDGET_H
#define ANNOTATIONDATABROWSERWIDGET_H

#include <QMainWindow>

namespace Ui {
class AnnotationDataBrowserWidget;
}

class AnnotationDataBrowserWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit AnnotationDataBrowserWidget(QWidget *parent = 0);
    ~AnnotationDataBrowserWidget();

private:
    Ui::AnnotationDataBrowserWidget *ui;
};

#endif // ANNOTATIONDATABROWSERWIDGET_H
