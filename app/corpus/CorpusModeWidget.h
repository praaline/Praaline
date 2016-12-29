#ifndef CORPUSMODEWIDGET_H
#define CORPUSMODEWIDGET_H

#include <QWidget>

namespace Ui {
class CorpusModeWidget;
}

struct CorpusModeWidgetData;

class CorpusModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CorpusModeWidget( QWidget *parent = 0);
    ~CorpusModeWidget();

signals:
    void activateMode();

private:
    Ui::CorpusModeWidget *ui;
    CorpusModeWidgetData *d;

    void setupActions();

private slots:
    void showCorpusExplorer();
    void showCorpusExplorerTables();
    void showMetadataStructureEditor();
    void showAnnotationStructureEditor();
};

#endif // CORPUSMODEWIDGET_H
