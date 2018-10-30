#ifndef CORPUSEXPLORERTREEWIDGET_H
#define CORPUSEXPLORERTREEWIDGET_H

#include <QWidget>

namespace Ui {
class CorpusExplorerTreeWidget;
}

class CorpusExplorerTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CorpusExplorerTreeWidget(QWidget *parent = nullptr);
    ~CorpusExplorerTreeWidget();

private:
    Ui::CorpusExplorerTreeWidget *ui;
};

#endif // CORPUSEXPLORERTREEWIDGET_H
