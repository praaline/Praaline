#ifndef STATISTICSMODEWIDGET_H
#define STATISTICSMODEWIDGET_H

#include <QWidget>

namespace Ui {
class StatisticsModeWidget;
}

struct StatisticsModeWidgetData;

class StatisticsModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsModeWidget(QWidget *parent = nullptr);
    ~StatisticsModeWidget();

signals:
    void activateMode();

protected slots:
    void corpusRepositoryAdded(const QString &repositoryID);
    void corpusRepositoryRemoved(const QString &repositoryID);
    void anayserDoubleClicked(const QModelIndex &index);
    void documentTabCloseRequested(int index);

private:
    Ui::StatisticsModeWidget *ui;
    StatisticsModeWidgetData *d;

    void createStatisticsPluginsTree();
};

#endif // STATISTICSMODEWIDGET_H
