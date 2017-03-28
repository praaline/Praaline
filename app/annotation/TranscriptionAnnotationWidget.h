#ifndef TRANSCRIPTIONANNOTATIONWIDGET_H
#define TRANSCRIPTIONANNOTATIONWIDGET_H

#include <QWidget>

namespace Ui {
class TranscriptionAnnotationWidget;
}

class TranscriptionAnnotationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TranscriptionAnnotationWidget(QWidget *parent = 0);
    ~TranscriptionAnnotationWidget();

private:
    Ui::TranscriptionAnnotationWidget *ui;
};

#endif // TRANSCRIPTIONANNOTATIONWIDGET_H
