#ifndef ADVANCEDQUERIESWIDGET_H
#define ADVANCEDQUERIESWIDGET_H

#include <QMainWindow>

namespace Ui {
class AdvancedQueriesWidget;
}

struct AdvancedQueriesWidgetData;

class AdvancedQueriesWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdvancedQueriesWidget(QWidget *parent = 0);
    ~AdvancedQueriesWidget();

private slots:
    void activeCorpusRepositoryChanged(const QString &newActiveCorpusRepositoryID);

private:
    Ui::AdvancedQueriesWidget *ui;
    AdvancedQueriesWidgetData *d;
};

#endif // ADVANCEDQUERIESWIDGET_H
