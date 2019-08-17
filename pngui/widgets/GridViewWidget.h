#ifndef GRIDVIEWWIDGET_H
#define GRIDVIEWWIDGET_H

#include <QString>
#include <QWidget>
#include <QLabel>
#include <QAbstractItemModel>
#include "grid/QAdvancedTableView.h"

namespace Ui {
class GridViewWidget;
}

struct GridViewWidgetData;

class GridViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GridViewWidget(QWidget *parent = nullptr);
    ~GridViewWidget();

    QAdvancedTableView *tableView();

    void setResultsLabelText(const QString &);
    bool exportToTabSeparated(const QString &);
    bool exportToExcel(const QString &);

    bool toolbarVisible() const;
    void setToolbarVisible(bool);

    QSize toolbarIconSize() const;
    void setToolbarIconSize(QSize);

    bool resultsLabelVisible() const;
    void setResultsLabelVisible(bool);

public slots:
    void resultCountChanged(int filterRows, int unfilteredRows);
    void splitGridActionTriggered();
    void pinRowsToolButtonClicked();
    void clipboardCopySelection();
    void clipboardPaste();
    void findDialog();
    void findNext();
    void exportToFile();
    void toggleFilterRows();

private:
    Ui::GridViewWidget *ui;
    GridViewWidgetData *d;

    bool matches(const QVariant &data, const QString &findWhat, bool matchWholeWordsOnly, bool matchCase, bool useRegularExpressions);
};

#endif // GRIDVIEWWIDGET_H
