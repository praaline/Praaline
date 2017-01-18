#ifndef BATCHEDITWIDGET_H
#define BATCHEDITWIDGET_H

#include <QWidget>

namespace Ui {
class BatchEditWidget;
}

struct BatchEditWidgetData;

class BatchEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BatchEditorWidget(QWidget *parent = 0);
    ~BatchEditorWidget();


private slots:
    void activeCorpusRepositoryChanged(const QString &repositoryID);
    void levelChanged(const QString &text);

    void actionGetDistinctValues();
    void actionUpdateValues();

private:
    Ui::BatchEditWidget *ui;
    BatchEditWidgetData *d;
};

#endif // BATCHEDITWIDGET_H
