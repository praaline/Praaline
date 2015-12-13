#ifndef TRANSCRIBERWIDGET_H
#define TRANSCRIBERWIDGET_H

#include <QMainWindow>

namespace Ui {
class TranscriberWidget;
}

struct TranscriberWidgetData;

class TranscriberWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit TranscriberWidget(QWidget *parent = 0);
    ~TranscriberWidget();

private:
    Ui::TranscriberWidget *ui;
    TranscriberWidgetData *d;
};

#endif // TRANSCRIBERWIDGET_H
