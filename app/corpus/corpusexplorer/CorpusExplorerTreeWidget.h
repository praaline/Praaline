#ifndef CORPUSEXPLORERTREEWIDGET_H
#define CORPUSEXPLORERTREEWIDGET_H

#include <QWidget>

namespace Ui {
class CorpusExplorerTreeWidget;
}

struct CorpusExplorerTreeWidgetData;

namespace Praaline {
namespace Core {
class CorpusObject;
class CorpusCommunication;
class CorpusSpeaker;
class Corpus;
}
}

class CorpusExplorerTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CorpusExplorerTreeWidget(QWidget *parent = nullptr);
    ~CorpusExplorerTreeWidget();

private slots:
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
    // Metadata
    void saveMetadata();

    // Presentation
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
