#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"
#include "pngui/model/annotation/AnnotationElementTableModel.h"
#include "CorpusRepositoriesManager.h"
#include "AnnotationBrowserWidget.h"

#include "pngui/PraalineUserInterfaceOptions.h"

struct AnnotationBrowserWidgetData {
    AnnotationBrowserWidgetData() :
        corpusRepositoriesManager(0), repository(0), model(0)
    {}

    // User interface elements
    CorpusRepositoriesManager *corpusRepositoriesManager;
    QComboBox *comboboxCorpusRepositories;
    QComboBox *comboboxAnnotationLevel;
    GridViewWidget *gridview;
    // Main toolbar
    QToolBar *toolbarMain;
    QAction *actionSave;
    // Data
    QPointer<CorpusRepository> repository;
    QString levelID;
    QStringList attributeIDs;
    QList<AnnotationElement *> elements;
    QPointer<AnnotationElementTableModel> model;
};

AnnotationBrowserWidget::AnnotationBrowserWidget(QWidget *parent) :
    QWidget(parent), d(new AnnotationBrowserWidgetData)
{
    // Get CorpusRepositoriesManager from global object list
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    connect(d->corpusRepositoriesManager, SIGNAL(corpusRepositoryAdded(QString)), this, SLOT(corpusRepositoryAdded(QString)));
    connect(d->corpusRepositoriesManager, SIGNAL(corpusRepositoryRemoved(QString)), this, SLOT(corpusRepositoryRemoved(QString)));
    // Corpus repositories and Annotation Level selection combo-boxes
    d->comboboxCorpusRepositories = new QComboBox(this);
    connect(d->comboboxCorpusRepositories, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedCorpusRepositoryChanged(int)));
    d->comboboxAnnotationLevel = new QComboBox(this);
    QPushButton *commandRefresh = new QPushButton("Load Annotation Data", this);
    connect(commandRefresh, SIGNAL(clicked(bool)), this, SLOT(loadAnnotationData()));
    // Grid view
    d->gridview = new GridViewWidget(this);
    d->gridview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->gridview->tableView()->verticalHeader()->setDefaultSectionSize(20);
    // Main toolbar
    d->toolbarMain = new QToolBar(tr("Manual annotation"), this);
    d->toolbarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMain->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    setupActions();
    // Create layout
    QGroupBox *groupboxSelection = new QGroupBox(tr("Corpus and Annotation Level"), this);
    QHBoxLayout *layoutSelection = new QHBoxLayout(this);
    layoutSelection->addWidget(new QLabel(tr("Corpus:"), this));
    layoutSelection->addWidget(d->comboboxCorpusRepositories);
    layoutSelection->addWidget(new QLabel(tr("Annotation Level:"), this));
    layoutSelection->addWidget(d->comboboxAnnotationLevel);
    layoutSelection->addWidget(commandRefresh);
    layoutSelection->addStretch();
    groupboxSelection->setLayout(layoutSelection);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(d->toolbarMain);
    layout->addWidget(groupboxSelection);
    layout->addWidget(d->gridview);
    this->setLayout(layout);
}

AnnotationBrowserWidget::~AnnotationBrowserWidget()
{
    delete d;
}

void AnnotationBrowserWidget::setupActions()
{
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    Command* command;

    // MAIN TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionSave = new QAction(QIcon(":/icons/actions/action_save.png"), tr("Save Annotations"), this);
    connect(d->actionSave, SIGNAL(triggered()), SLOT(saveAnnotationData()));
    command = ACTION_MANAGER->registerAction("Annotation.AnnotationBrowser.Save", d->actionSave, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMain->addAction(d->actionSave);
    d->actionSave->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
}

void AnnotationBrowserWidget::corpusRepositoryAdded(const QString &repositoryID)
{
    if (!d->corpusRepositoriesManager) return;
    CorpusRepository *repository = d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID);
    if (!repository) return;
    d->comboboxCorpusRepositories->addItem(repository->ID(), repositoryID);
}

void AnnotationBrowserWidget::corpusRepositoryRemoved(const QString &repositoryID)
{
    if (!d->corpusRepositoriesManager) return;
    int index = d->comboboxCorpusRepositories->findData(repositoryID);
    if (index >= 0) {
        if (index == d->comboboxCorpusRepositories->currentIndex()) {
            d->comboboxAnnotationLevel->clear();
        }
        d->comboboxCorpusRepositories->removeItem(index);
    }
}

void AnnotationBrowserWidget::selectedCorpusRepositoryChanged(int index)
{
    if (!d->corpusRepositoriesManager) return;
    CorpusRepository *repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->comboboxCorpusRepositories->itemData(index).toString());
    if (!repository) return;
    // New corpus selected, update the annotation levels
    d->comboboxAnnotationLevel->clear();
    foreach (AnnotationStructureLevel *level, repository->annotationStructure()->levels()) {
        d->comboboxAnnotationLevel->addItem(level->name(), level->ID());
    }
}

void AnnotationBrowserWidget::loadAnnotationData()
{
    if (!d->corpusRepositoriesManager) return;
    // Get corpus repository
    CorpusRepository *repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->comboboxCorpusRepositories->currentData().toString());
    if (!repository) return;
    // Get annotation level
    AnnotationStructureLevel *level = repository->annotationStructure()->level(d->comboboxAnnotationLevel->currentData().toString());
    if (!level) return;
    // Get data
    QList<AnnotationElement *> elements = repository->annotations()->getAnnotationElements(
                AnnotationDatastore::Selection("", "", level->ID()));
    // Set data model
    AnnotationElementTableModel *model = new AnnotationElementTableModel(AnnotationElement::Type_Interval, elements, level->attributeIDs(), this);
    d->gridview->tableView()->setModel(model);
    // Update internal state
    if (d->model) delete d->model;
    qDeleteAll(d->elements);
    d->model = model;
    d->repository = repository;
    d->levelID = level->ID();
    d->attributeIDs = level->attributeIDs();
    d->elements = elements;
}

void AnnotationBrowserWidget::saveAnnotationData()
{
    if (!d->repository) return;
    bool result = d->repository->annotations()->saveAnnotationElements(d->elements, d->levelID, d->attributeIDs);
    if (result && d->model) {
        d->model->modelSavedInDatabase();
    } else {
        QMessageBox::warning(this, tr("Error(s) saving"),
                             tr("One or more annotation elements could not be saved in the database. "
                                "They are indicated as 'modified' or 'new'."));
    }
}
