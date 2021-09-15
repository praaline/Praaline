#ifndef CORPUSMODEWIDGET_H
#define CORPUSMODEWIDGET_H

#include <QWidget>
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/CorpusRepositoryDefinition.h"

namespace Ui {
class CorpusModeWidget;
}

struct CorpusModeWidgetData;

class CorpusModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CorpusModeWidget( QWidget *parent = nullptr);
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

    void openCorpusRepositoryFromDefinition(const QString &filename);

private slots:
    void showCorpusExplorer();
    void showCorpusExplorerTables();
    void showMetadataStructureEditor();
    void showAnnotationStructureEditor();
    void setupRecentFilesMenu();

private:
    Ui::CorpusModeWidget *ui;
    CorpusModeWidgetData *d;

    void setupActions();

    Praaline::Core::CorpusRepository *openCorpusRepository(const QString &filename, Praaline::Core::CorpusRepositoryDefinition &definition);
    void activateNextCorpusRepository();
};

#endif // CORPUSMODEWIDGET_H
