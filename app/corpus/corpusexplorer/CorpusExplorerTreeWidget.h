#ifndef CORPUSEXPLORERTREEWIDGET_H
#define CORPUSEXPLORERTREEWIDGET_H

#include <QWidget>
#include <QMainWindow>

namespace Ui {
class CorpusExplorerTreeWidget;
}

namespace Praaline {
namespace Core {
class CorpusObject;
class CorpusCommunication;
class CorpusSpeaker;
class Corpus;
}
}

#include "pngui/observers/CorpusObserver.h"

class CorpusModeWidget;

struct CorpusExplorerTreeWidgetData;

class CorpusExplorerTreeWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit CorpusExplorerTreeWidget(CorpusModeWidget *widgetCorpusMode, QWidget *parent = nullptr);
    ~CorpusExplorerTreeWidget();

private slots:
    // Respond to changes in the corpus repository manager
    void corpusRepositoryAdded(const QString &repositoryID);
    void corpusRepositoryRemoved(const QString &repositoryID);
    // Respond to user interaction with the corpus items tree
    void corporaObserverWidgetSelectedObjectsChanged(QList<QObject*>);
    void corporaObserverWidgetDoubleClickRequest(QObject*, Observer*);
    // Save changes to metadata
    void saveCorpusMetadata();
    // Corpora
    void createCorpus();
    void openCorpus();
    void deleteCorpus();
    // Corpus items
    void addCommunication();
    void addSpeaker();
    void addRecording();
    void addAnnotation();
    void addParticipation();
    void removeCorpusItems();
    void relinkCorpusItem();
    // Presentation
    void attributesAndGroupings();
    void metadataEditorPrimaryStyleTree();
    void metadataEditorPrimaryStyleGroupBox();
    void metadataEditorPrimaryStyleButton();
    void metadataEditorSecondaryStyleTree();
    void metadataEditorSecondaryStyleGroupBox();
    void metadataEditorSecondaryStyleButton();

private:
    Ui::CorpusExplorerTreeWidget *ui;
    CorpusExplorerTreeWidgetData *d;

    void setupActions();
    void setupMetadataEditorsStylingMenu();

    QList<Praaline::Core::CorpusObject *> selectedCorpusItems();
    void updateMetadataEditorsForCom(Praaline::Core::CorpusCommunication *communication);
    void updateMetadataEditorsForSpk(Praaline::Core::CorpusSpeaker *speaker);
    void updateMetadataEditorsForCorpus(Praaline::Core::Corpus *corpus);

    bool checkForActiveCorpus();
};

#endif // CORPUSEXPLORERTREEWIDGET_H
