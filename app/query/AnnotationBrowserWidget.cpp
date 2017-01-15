#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QMessageBox>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"
#include "pngui/model/annotation/AnnotationTableModel.h"
#include "CorporaManager.h"
#include "AnnotationBrowserWidget.h"


struct AnnotationBrowserWidgetData {
    AnnotationBrowserWidgetData() :
        corporaManager(0), corpus(0), model(0)
    {}

    // User interface elements
    CorporaManager *corporaManager;
    QComboBox *comboboxCorpus;
    QComboBox *comboboxAnnotationLevel;
    GridViewWidget *gridview;
    // Main toolbar
    QToolBar *toolbarMain;
    QAction *actionSave;
    // Data
    QPointer<Corpus> corpus;
    QString levelID;
    QStringList attributeIDs;
    QList<AnnotationElement *> elements;
    AnnotationTableModel *model;
};

AnnotationBrowserWidget::AnnotationBrowserWidget(QWidget *parent) :
    QWidget(parent), d(new AnnotationBrowserWidgetData)
{
    // Get Corpora Manager from global object list
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorporaManager");
    foreach (QObject* obj, list) {
        CorporaManager *manager = qobject_cast<CorporaManager *>(obj);
        if (manager) d->corporaManager = manager;
    }
    connect(d->corporaManager, SIGNAL(corpusAdded(QString)), this, SLOT(corpusAdded(QString)));
    connect(d->corporaManager, SIGNAL(corpusRemoved(QString)), this, SLOT(corpusRemoved(QString)));
    // Corpus and Annotation Level selection combo-boxes
    d->comboboxCorpus = new QComboBox(this);
    connect(d->comboboxCorpus, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedCorpusChanged(int)));
    d->comboboxAnnotationLevel = new QComboBox(this);
    QPushButton *commandRefresh = new QPushButton("Load Annotation Data", this);
    connect(commandRefresh, SIGNAL(clicked(bool)), this, SLOT(loadAnnotationData()));
    // Grid view
    d->gridview = new GridViewWidget(this);
    d->gridview->tableView()->verticalHeader()->setDefaultSectionSize(20);
    // Main toolbar
    d->toolbarMain = new QToolBar(tr("Manual annotation"), this);
    d->toolbarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMain->setIconSize(QSize(24, 24));
    setupActions();
    // Create layout
    QGroupBox *groupboxSelection = new QGroupBox(tr("Corpus and Annotation Level"), this);
    QHBoxLayout *layoutSelection = new QHBoxLayout(this);
    layoutSelection->addWidget(new QLabel(tr("Corpus:"), this));
    layoutSelection->addWidget(d->comboboxCorpus);
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

void AnnotationBrowserWidget::corpusAdded(QString corpusID)
{
    if (!d->corporaManager) return;
    Corpus *corpus = d->corporaManager->corpusByID(corpusID);
    if (!corpus) return;
    d->comboboxCorpus->addItem(corpus->name(), corpusID);
}

void AnnotationBrowserWidget::corpusRemoved(QString corpusID)
{
    if (!d->corporaManager) return;
    int index = d->comboboxCorpus->findData(corpusID);
    if (index >= 0) {
        if (index == d->comboboxCorpus->currentIndex()) {
            d->comboboxAnnotationLevel->clear();
        }
        d->comboboxCorpus->removeItem(index);
    }
}

void AnnotationBrowserWidget::selectedCorpusChanged(int index)
{
    if (!d->corporaManager) return;
    Corpus *corpus = d->corporaManager->corpusByID(d->comboboxCorpus->itemData(index).toString());
    if (!corpus) return;
    // New corpus selected, update the annotation levels
    d->comboboxAnnotationLevel->clear();
    foreach (AnnotationStructureLevel *level, corpus->annotationStructure()->levels()) {
        d->comboboxAnnotationLevel->addItem(level->name(), level->ID());
    }
}

void AnnotationBrowserWidget::loadAnnotationData()
{
    if (!d->corporaManager) return;
    // Get corpus
    Corpus *corpus = d->corporaManager->corpusByID(d->comboboxCorpus->currentData().toString());
    if (!corpus) return;
    // Get annotation level
    AnnotationStructureLevel *level = corpus->annotationStructure()->level(d->comboboxAnnotationLevel->currentData().toString());
    if (!level) return;
    // Get data
    QList<AnnotationElement *> elements = corpus->datastoreAnnotations()->getAnnotationElements(
                AnnotationDatastore::Selection("", "", level->ID()));
    // Set data model
    AnnotationTableModel *model = new AnnotationTableModel(AnnotationElement::Type_Interval, elements, level->attributeIDs(), this);
    d->gridview->tableView()->setModel(model);
    // Update internal state
    if (d->model) delete d->model;
    qDeleteAll(d->elements);
    d->model = model;
    d->corpus = corpus;
    d->levelID = level->ID();
    d->attributeIDs = level->attributeIDs();
    d->elements = elements;
}

void AnnotationBrowserWidget::saveAnnotationData()
{
    if (!d->corpus) return;
    bool result = d->corpus->datastoreAnnotations()->saveAnnotationElements(d->elements, d->levelID, d->attributeIDs);
    if (result && d->model) {
        d->model->modelSavedInDatabase();
    } else {
        QMessageBox::warning(this, tr("Error(s) saving"),
                             tr("One or more annotation elements could not be saved in the database. "
                                "They are indicated as 'modified' or 'new'."));
    }
}
