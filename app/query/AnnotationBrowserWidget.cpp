#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"
#include "pngui/model/annotation/AnnotationTableModel.h"
#include "CorporaManager.h"
#include "AnnotationBrowserWidget.h"


struct AnnotationBrowserWidgetData {
    AnnotationBrowserWidgetData() :
        corporaManager(0), model(0)
    {}

    CorporaManager *corporaManager;
    QComboBox *comboboxCorpus;
    QComboBox *comboboxAnnotationLevel;
    GridViewWidget *gridview;

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
    layout->addWidget(groupboxSelection);
    layout->addWidget(d->gridview);
    this->setLayout(layout);
}

AnnotationBrowserWidget::~AnnotationBrowserWidget()
{
    delete d;
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
    d->gridview->tableView()->horizontalHeader()->setSectionsClickable(true);
    if (d->model) delete d->model;
    qDeleteAll(d->elements);
    //d->model = model;
    d->elements = elements;
}
