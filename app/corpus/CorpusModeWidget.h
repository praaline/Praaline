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
    // Corpus Repository Management
    void newCorpusRepository();
    void openCorpusRepository();
    void openCorpusRepositoryRecent();
    void openCorpusRepositoryFromDefinition(const QString &filename);
    void editCorpusRepository();
    void closeCorpusRepository();
    void saveCorpusRepository();
    void saveCorpusRepositoryAs();
    // Import Corpus Items from other software formats
    void addItemsFromFolder();
    // Import / Export Metadata
    void importMetadata();
    void exportMetadata();
    // Repository Integrity
    void checkMediaFiles();
    void createAnnotationsFromRecordings();
    void createSpeakersFromAnnotations();
    void cleanUpParticipationsFromAnnotations();
    // Utilities
    void utilitiesSplitCommunications();
    void utilitiesMergeCommunications();
    void utilitiesDecodeFilenameToMetadata();
    void utilitiesMergeCorpora();

private slots:
    // Corpus Mode Widgets
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
    bool checkForActiveCorpusRepository();
};

#endif // CORPUSMODEWIDGET_H
