#ifndef CORPUSMODEWIDGET_H
#define CORPUSMODEWIDGET_H

#include <QWidget>
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/CorpusRepositoryDefinition.h"

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

public slots:
    void newCorpusRepository();
    void openCorpusRepository();
    void openCorpusRepositoryRecent();
    void editCorpusRepository();
    void closeCorpusRepository();
    void saveCorpusRepository();
    void saveCorpusRepositoryAs();

private slots:
    void showCorpusExplorer();
    void showCorpusExplorerTables();
    void showMetadataStructureEditor();
    void showAnnotationStructureEditor();

private:
    Ui::CorpusModeWidget *ui;
    CorpusModeWidgetData *d;

    void setupActions();
    void setupRecentFilesMenu();

    void openCorpusRepositoryFromDefinition(const QString &filename);
    Praaline::Core::CorpusRepository *openCorpusRepository(const QString &filename, Praaline::Core::CorpusRepositoryDefinition &definition);
    void activateNextCorpusRepository();
};

#endif // CORPUSMODEWIDGET_H
