#include "CorpusExplorerTableWidget.h"
#include "ui_CorpusExplorerTableWidget.h"

#include "grid/QAdvancedTableView.h"
#include "grid/QAdvancedHeaderView.h"
#include "grid/QConditionalDecorationDialog.h"
#include "grid/QConditionalDecorationProxyModel.h"
#include "grid/QFilterModel.h"
#include "grid/QFilterModelProxy.h"
#include "grid/QFilterViewConnector.h"
#include "grid/QGroupingProxyModel.h"
#include "grid/QConditionalDecoration.h"
#include "grid/QFixedRowsTableView.h"
#include "grid/QMimeDataUtil.h"
#include "grid/QRangeFilter.h"
#include "grid/QUniqueValuesProxyModel.h"
#include "grid/QSelectionListFilter.h"
#include "grid/QTextFilter.h"
#include "grid/QValueFilter.h"

#include "PraalineCore/Corpus/CorpusObject.h"
#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Structure/MetadataStructure.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/CorpusCommunicationTableModel.h"
#include "pngui/model/corpus/CorpusSpeakerTableModel.h"
#include "pngui/model/corpus/CorpusRecordingTableModel.h"
#include "pngui/model/corpus/CorpusAnnotationTableModel.h"
#include "pngui/model/corpus/CorpusParticipationTableModel.h"
#include "pngui/widgets/SelectionListDataProviderProxy.h"
#include "pngui/widgets/GridViewWidget.h"
#include "pngui/PraalineUserInterfaceOptions.h"

#include "CorpusRepositoriesManager.h"

struct CorpusExplorerTableWidgetData
{
    CorpusExplorerTableWidgetData() :
        corpusRepositoryManager(0), repository(0), corpuObjectType(CorpusObject::Type_Communication), model(0)
    {}

    // Main toolbar
    QToolBar *toolbarCorpusExplorerTable;
    QAction *actionSave;

    CorpusRepositoriesManager *corpusRepositoryManager;
    QPointer<CorpusRepository> repository;
    CorpusObject::Type corpuObjectType;

    QList<CorpusCommunication *> itemsCom;
    QList<CorpusRecording *> itemsRec;
    QList<CorpusAnnotation *> itemsAnnot;
    QList<CorpusSpeaker *> itemsSpk;
    QList<CorpusParticipation *> itemsPart;

    QAbstractTableModel *model;

    GridViewWidget *tableView;

};

CorpusExplorerTableWidget::CorpusExplorerTableWidget(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::CorpusExplorerTableWidget), d(new CorpusExplorerTableWidgetData)
{
    ui->setupUi(this);

    // Get Corpus Repositories Manager from global object list
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoryManager = manager;
    }
    connect(d->corpusRepositoryManager, SIGNAL(activeCorpusRepositoryChanged(QString)), this, SLOT(activeCorpusRepositoryChanged(QString)));
    // Add grid
    d->tableView = new GridViewWidget(this);
    d->tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->gridLayoutTable->addWidget(d->tableView);
    d->tableView->tableView()->verticalHeader()->setDefaultSectionSize(20);
    // Corpus object types
    ui->comboItemType->addItems(QStringList() << tr("Communications") << tr("Speakers") << tr("Recordings") << tr("Annotations") << tr("Participations"));
    ui->comboItemType->setCurrentText(tr("Communications"));
    connect(ui->comboItemType, SIGNAL(currentTextChanged(QString)), this, SLOT(corpusObjectTypeChanged(QString)));
    // Menu and Toolbar actions
    d->toolbarCorpusExplorerTable = new QToolBar("Corpus Explorer Tables", this);
    d->toolbarCorpusExplorerTable->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarCorpusExplorerTable->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    this->addToolBar(d->toolbarCorpusExplorerTable);
    setupActions();
}

CorpusExplorerTableWidget::~CorpusExplorerTableWidget()
{
    delete ui;
    delete d;
}

void CorpusExplorerTableWidget::setupActions()
{
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    Command* command;

    // MAIN TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionSave = new QAction(QIcon(":/icons/actions/action_save.png"), tr("Save"), this);
    connect(d->actionSave, SIGNAL(triggered()), SLOT(save()));
    command = ACTION_MANAGER->registerAction("Annotation.AnnotationBrowser.Save", d->actionSave, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarCorpusExplorerTable->addAction(d->actionSave);
    d->actionSave->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
}

void CorpusExplorerTableWidget::corpusObjectTypeChanged(const QString &text)
{
    if (text == tr("Communications"))
        d->corpuObjectType = CorpusObject::Type_Communication;
    else if (text == tr("Speakers"))
        d->corpuObjectType = CorpusObject::Type_Speaker;
    else if (text == tr("Recordings"))
        d->corpuObjectType = CorpusObject::Type_Recording;
    else if (text == tr("Annotations"))
        d->corpuObjectType = CorpusObject::Type_Annotation;
    else if (text == tr("Participations"))
        d->corpuObjectType = CorpusObject::Type_Participation;
    refreshModel();
}

void CorpusExplorerTableWidget::refreshModel()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoryManager->activeCorpusRepository();
    if (!repository) {
        d->tableView->tableView()->setModel(0);
        if (d->model) delete d->model;
        d->model = 0;
        return;
    }
    d->repository = repository;
    // otherwise, another corpus has been selected
    QAbstractTableModel *newModel = 0;
    if (d->corpuObjectType == CorpusObject::Type_Communication) {
        QList<MetadataStructureAttribute *> attributes = repository->metadataStructure()->attributes(CorpusObject::Type_Communication);
        QList<CorpusCommunication *> items = repository->metadata()->getCommunications(MetadataDatastore::Selection("", "", ""));
        newModel = new CorpusCommunicationTableModel(items, attributes, true);
        d->itemsCom = items;
    }
    else if (d->corpuObjectType == CorpusObject::Type_Speaker) {
        QList<MetadataStructureAttribute *> attributes = repository->metadataStructure()->attributes(CorpusObject::Type_Speaker);
        QList<CorpusSpeaker *> items = repository->metadata()->getSpeakers(MetadataDatastore::Selection("", "", ""));
        newModel = new CorpusSpeakerTableModel(items, attributes, true);
        d->itemsSpk = items;
    }
    else if (d->corpuObjectType == CorpusObject::Type_Recording) {
        QList<MetadataStructureAttribute *> attributes = repository->metadataStructure()->attributes(CorpusObject::Type_Recording);
        QList<CorpusRecording *> items = repository->metadata()->getRecordings(MetadataDatastore::Selection("", "", ""));
        newModel = new CorpusRecordingTableModel(items, attributes, true);
        d->itemsRec = items;
    }
    else if (d->corpuObjectType == CorpusObject::Type_Annotation) {
        QList<MetadataStructureAttribute *> attributes = repository->metadataStructure()->attributes(CorpusObject::Type_Annotation);
        QList<CorpusAnnotation *> items = repository->metadata()->getAnnotations(MetadataDatastore::Selection("", "", ""));
        newModel = new CorpusAnnotationTableModel(items, attributes, true);
        d->itemsAnnot = items;
    }
    else if (d->corpuObjectType == CorpusObject::Type_Participation) {
        QList<MetadataStructureAttribute *> attributes = repository->metadataStructure()->attributes(CorpusObject::Type_Participation);
        QList<CorpusParticipation *> items = repository->metadata()->getParticipations(MetadataDatastore::Selection("", "", ""));
        newModel = new CorpusParticipationTableModel(items, attributes, true);
        d->itemsPart = items;
    }
    // try to update model, if not possible show an empty table
    if (newModel) {
        d->tableView->tableView()->setModel(newModel);
        d->tableView->tableView()->horizontalHeader()->setSectionsClickable(true);
        d->tableView->tableView()->setDefaultFilterType(0, QTextFilter::Type);
        connect(d->tableView->tableView()->filterProxyModel(), SIGNAL(resultCountChanged(int,int)),
                this, SLOT(resultChanged(int,int)));
        connect(d->tableView->tableView()->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                this, SLOT(selectionChanged(QItemSelection, QItemSelection)));

        if (d->model) delete d->model;
        d->model = newModel;
    }
    else {
        d->tableView->tableView()->setModel(0);
        if (d->model) delete d->model;
        d->model = 0;
    }
}

void CorpusExplorerTableWidget::activeCorpusRepositoryChanged(const QString &repositoryID)
{
    Q_UNUSED(repositoryID);
    refreshModel();
}

void CorpusExplorerTableWidget::resultChanged(int filterRows, int unfilteredRows)
{
    if (unfilteredRows - filterRows > 0)
        d->tableView->setResultsLabelText(tr("Result: %1 of %2").arg(filterRows).arg(unfilteredRows));
    else
        d->tableView->setResultsLabelText(tr("All items: %1").arg(filterRows));
}

void CorpusExplorerTableWidget::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{

}

void CorpusExplorerTableWidget::contextMenuRequested(const QPoint & point)
{

}

void CorpusExplorerTableWidget::save()
{
    if (!d->repository) return;
    bool result(false);
    if      (d->corpuObjectType == CorpusObject::Type_Communication)
        result = d->repository->metadata()->saveCommunications(d->itemsCom);
    else if (d->corpuObjectType == CorpusObject::Type_Speaker)
        result = d->repository->metadata()->saveSpeakers(d->itemsSpk);
    else if (d->corpuObjectType == CorpusObject::Type_Recording)
        result = d->repository->metadata()->saveRecordings(d->itemsRec);
    else if (d->corpuObjectType == CorpusObject::Type_Annotation)
        result = d->repository->metadata()->saveAnnotations(d->itemsAnnot);
    else if (d->corpuObjectType == CorpusObject::Type_Participation)
        result = d->repository->metadata()->saveParticipations(d->itemsPart);

    if (result && d->model) {
        refreshModel(); // d->model->modelSavedInDatabase();
    } else {
        QMessageBox::warning(this, tr("Error(s) saving"),
                             tr("One or more metadata items could not be saved in the database. "
                                "They are indicated as 'modified' or 'new'."));
    }
}
