#include <QString>
#include <QList>
#include <QSet>
#include <QHash>
#include <QAction>
#include <QToolBar>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QProgressDialog>

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/FileDatastore.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/CorpusExplorerTreeModel.h"
#include "pngui/widgets/SelectionDialog.h"
#include "pngui/widgets/MetadataEditorWidget.h"
#include "pngui/widgets/CorpusItemPreview.h"
#include "pngui/observers/CorpusObserver.h"

#include "pngui/PraalineUserInterfaceOptions.h"
#include "CorpusRepositoriesManager.h"

#include "CorpusExplorerTreeWidget.h"
#include "ui_CorpusExplorerTreeWidget.h"

struct CorpusExplorerTreeWidgetData {
    CorpusExplorerTreeWidgetData()
    {}

    // Corpora
    QAction *actionCreateCorpus;
    QAction *actionOpenCorpus;
    QAction *actionSaveMetadata;
    QAction *actionDeleteCorpus;
    // Corpus items
    QAction *actionAddCommunication;
    QAction *actionAddSpeaker;
    QAction *actionAddRecording;
    QAction *actionAddAnnotation;
    QAction *actionAddParticipation;
    QAction *actionRemoveCorpusItems;
    QAction *actionRelinkCorpusItem;
    // Presentation
    QAction *actionToggleSearchBox;
    QAction *actionMetadataEditorPrimaryStyleTree;
    QAction *actionMetadataEditorPrimaryStyleGroupBox;
    QAction *actionMetadataEditorPrimaryStyleButton;
    QAction *actionMetadataEditorSecondaryStyleTree;
    QAction *actionMetadataEditorSecondaryStyleGroupBox;
    QAction *actionMetadataEditorSecondaryStyleButton;

    CorpusRepositoriesManager *corpusRepositoriesManager;
    QHash<QString, CorpusObserver *> observersForCorpusRepositories;
    QPointer<TreeNode> corporaTopLevelNode;
    QPointer<Corpus> activeCorpus;

    ObserverWidget* corporaObserverWidget;
    MetadataEditorWidget *metadataEditorPrimary;
    MetadataEditorWidget *metadataEditorSecondary;

    QToolBar *toolbarCorpusExplorer;
    QMenu *menuMetadataEditorStyles;

    CorpusItemPreview *preview;

};

CorpusExplorerTreeWidget::CorpusExplorerTreeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CorpusExplorerTreeWidget)
{
    ui->setupUi(this);
}

CorpusExplorerTreeWidget::~CorpusExplorerTreeWidget()
{
    delete ui;
}

void CorpusExplorerTreeWidget::setupActions()
{

}

void CorpusExplorerTreeWidget::setupMetadataEditorsStylingMenu()
{

}


void CorpusExplorerTreeWidget::createCorpus()
{

}

void CorpusExplorerTreeWidget::openCorpus()
{

}

void CorpusExplorerTreeWidget::deleteCorpus()
{

}

void CorpusExplorerTreeWidget::addCommunication()
{

}

void CorpusExplorerTreeWidget::addSpeaker()
{

}

void CorpusExplorerTreeWidget::addRecording()
{

}

void CorpusExplorerTreeWidget::addAnnotation()
{

}

void CorpusExplorerTreeWidget::addParticipation()
{

}

void CorpusExplorerTreeWidget::removeCorpusItems()
{

}

void CorpusExplorerTreeWidget::relinkCorpusItem()
{

}

void CorpusExplorerTreeWidget::saveMetadata()
{

}

QList<CorpusObject *> CorpusExplorerTreeWidget::selectedCorpusItems()
{
    QList<CorpusObject *> ret;
    return ret;
}

void CorpusExplorerTreeWidget::updateMetadataEditorsForCom(CorpusCommunication *communication)
{

}

void CorpusExplorerTreeWidget::updateMetadataEditorsForSpk(CorpusSpeaker *speaker)
{

}

void CorpusExplorerTreeWidget::updateMetadataEditorsForCorpus(Corpus *corpus)
{

}

bool CorpusExplorerTreeWidget::checkForActiveCorpus()
{

}

// ==============================================================================================================================
// Metadata editors (primary and secondary) styling
// ==============================================================================================================================

void CorpusExplorerTreeWidget::metadataEditorPrimaryStyleTree()
{
    d->metadataEditorPrimary->setEditorStyle(MetadataEditorWidget::TreeStyle);
}

void CorpusExplorerTreeWidget::metadataEditorPrimaryStyleGroupBox()
{
    d->metadataEditorPrimary->setEditorStyle(MetadataEditorWidget::GroupBoxStyle);
}

void CorpusExplorerTreeWidget::metadataEditorPrimaryStyleButton()
{
    d->metadataEditorPrimary->setEditorStyle(MetadataEditorWidget::ButtonStyle);
}

void CorpusExplorerTreeWidget::metadataEditorSecondaryStyleTree()
{
    d->metadataEditorSecondary->setEditorStyle(MetadataEditorWidget::TreeStyle);
}

void CorpusExplorerTreeWidget::metadataEditorSecondaryStyleGroupBox()
{
    d->metadataEditorSecondary->setEditorStyle(MetadataEditorWidget::GroupBoxStyle);
}

void CorpusExplorerTreeWidget::metadataEditorSecondaryStyleButton()
{
    d->metadataEditorSecondary->setEditorStyle(MetadataEditorWidget::ButtonStyle);
}
