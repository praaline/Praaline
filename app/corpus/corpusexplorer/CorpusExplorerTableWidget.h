#ifndef CORPUSEXPLORERTABLEWIDGET_H
#define CORPUSEXPLORERTABLEWIDGET_H

#include <QMainWindow>
#include <QItemSelection>

namespace Ui {
class CorpusExplorerTableWidget;
}

struct CorpusExplorerTableWidgetData;

class CorpusExplorerTableWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit CorpusExplorerTableWidget(QWidget *parent = 0);
    ~CorpusExplorerTableWidget();

private slots:
    void activeCorpusRepositoryChanged(const QString &newActiveCorpusID);
    void corpusObjectTypeChanged(const QString &text);

    void resultChanged(int filterRows, int unfilteredRows);
    void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void contextMenuRequested(const QPoint & point);

    void save();

private:
    Ui::CorpusExplorerTableWidget *ui;
    CorpusExplorerTableWidgetData *d;

    void setupActions();
    void refreshModel();
};

#endif // CORPUSEXPLORERTABLEWIDGET_H
