#ifndef CORPUSEXPLORERWIDGET_H
#define CORPUSEXPLORERWIDGET_H

#include <QWidget>

#include "pncore/corpus/Corpus.h"

using namespace Praaline::Core;

#include "pngui/observers/CorpusObserver.h"

namespace Ui {
class CorpusExplorerWidget;
}

struct CorpusExplorerWidgetData;

class CorpusExplorerWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit CorpusExplorerWidget(QWidget *parent = 0);
    ~CorpusExplorerWidget();

private slots:
    void corpusRepositoryAdded(const QString &repositoryID);
    void corpusRepositoryRemoved(const QString &repositoryID);
    void corporaObserverWidgetSelectedObjectsChanged(QList<QObject*>);
    void corporaObserverWidgetDoubleClickRequest(QObject*, Observer*);
    // Corpora
    void createCorpus();
    void openCorpus();
    void saveMetadata();
    void deleteCorpus();
    // Corpus items
    void addCommunication();
    void addSpeaker();
    void addRecording();
    void addAnnotation();
    void addParticipation();
    void removeCorpusItems();
    void relinkCorpusItem();
    // Import-export
    void addItemsFromFolder();
    void importMetadata();
    void exportMetadata();
    void importAnnotations();
    void exportAnnotations();
    // Tools
    void checkMediaFiles();
    void createAnnotationsFromRecordings();
    void createSpeakersFromAnnotations();
    void cleanUpParticipationsFromAnnotations();
    void splitCommunications();
    // Presentation
    void attributesAndGroupings();
    void metadataEditorPrimaryStyleTree();
    void metadataEditorPrimaryStyleGroupBox();
    void metadataEditorPrimaryStyleButton();
    void metadataEditorSecondaryStyleTree();
    void metadataEditorSecondaryStyleGroupBox();
    void metadataEditorSecondaryStyleButton();

private:
    Ui::CorpusExplorerWidget *ui;
    CorpusExplorerWidgetData *d;

    void setupActions();
    void setupMetadataEditorsStylingMenu();

    QList<CorpusObject *> selectedCorpusItems();
    void updateMetadataEditorsForCom(CorpusCommunication *communication);
    void updateMetadataEditorsForSpk(CorpusSpeaker *speaker);
    void updateMetadataEditorsForCorpus(Corpus *corpus);
};

#endif // CORPUSEXPLORERWIDGET_H
