#ifndef CORPUSEXPLORERWIDGET_H
#define CORPUSEXPLORERWIDGET_H

#include <QWidget>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "pngui/observers/corpusobserver.h"

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
    void selectionChanged(QList<QObject*>);

    void newCorpus();
    void openCorpusFile();
    void openCorpusRecentFile();
    void openCorpusDbConnection();
    void closeCorpus();
    void saveCorpus();
    void saveCorpusAs();
    void importCorpus();

    void addCommunication();
    void addSpeaker();
    void addRecording();
    void addAnnotation();
    void addParticipation();
    void removeCorpusItems();
    void relinkCorpusItem();

    void addItemsFromFolder();
    void importMetadata();
    void exportMetadata();
    void exportAnnotations();

    void checkMediaFiles();
    void createAnnotationsFromRecordings();
    void createSpeakersFromAnnotations();
    void cleanUpParticipationsFromAnnotations();
    void splitCommunications();

    void attributesAndGroupings();

    void setupRecentFilesMenu();

private:
    Ui::CorpusExplorerWidget *ui;
    CorpusExplorerWidgetData *d;

    void setupActions();

    void openCorpusFile(const QString &filename);
    Corpus *openCorpus(const QString &filename, CorpusDefinition &definition);
    void activateNextCorpus();
    QList<CorpusObject *> selectedCorpusItems();
    void updateMetadataEditorsForCom(CorpusCommunication *communication);
    void updateMetadataEditorsForSpk(CorpusSpeaker *speaker);
};

#endif // CORPUSEXPLORERWIDGET_H
