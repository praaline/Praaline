#include "CorpusExplorerTableWidget.h"
#include "ui_corpusexplorertablewidget.h"

#include "grid/qadvancedtableview.h"
#include "grid/qadvancedheaderview.h"
#include "grid/qconditionaldecorationdialog.h"
#include "grid/qconditionaldecorationproxymodel.h"
#include "grid/qfiltermodel.h"
#include "grid/qfiltermodelproxy.h"
#include "grid/qfilterviewconnector.h"
#include "grid/qgroupingproxymodel.h"
#include "grid/qconditionaldecoration.h"
#include "grid/qfixedrowstableview.h"
#include "grid/qmimedatautil.h"
#include "grid/qrangefilter.h"
#include "grid/quniquevaluesproxymodel.h"
#include "grid/qselectionlistfilter.h"
#include "grid/qtextfilter.h"
#include "grid/qvaluefilter.h"

#include "pngui/model/corpus/corpuscommunicationtablemodel.h"
#include "pngui/model/corpus/corpusspeakertablemodel.h"
#include "pngui/model/corpus/corpusrecordingtablemodel.h"
#include "pngui/model/corpus/corpusannotationtablemodel.h"
#include "pngui/widgets/selectionlistdataproviderproxy.h"
#include "pngui/widgets/gridviewwidget.h"

#include "CorporaManager.h"

struct CorpusExplorerTableWidgetData
{
    CorpusExplorerTableWidgetData() :
        corporaManager(0), corpuObjectType(CorpusObject::Type_Communication), model(0)
    {}

    CorporaManager *corporaManager;
    CorpusObject::Type corpuObjectType;
    QAbstractTableModel *model;

    GridViewWidget *tableView;
//    QToolBar *toolbarCorpusExplorerTable;
};

CorpusExplorerTableWidget::CorpusExplorerTableWidget(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::CorpusExplorerTableWidget), d(new CorpusExplorerTableWidgetData)
{
    ui->setupUi(this);

    // Get Corpora Manager from global object list
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorporaManager");
    foreach (QObject* obj, list) {
        CorporaManager *manager = qobject_cast<CorporaManager *>(obj);
        if (manager) d->corporaManager = manager;
    }
    connect(d->corporaManager, SIGNAL(activeCorpusChanged(QString)), this, SLOT(activeCorpusChanged(QString)));
    // Add grid
    d->tableView = new GridViewWidget(this);
    ui->gridLayoutTable->addWidget(d->tableView);
    d->tableView->tableView()->verticalHeader()->setDefaultSectionSize(20);
    // Corpus object types
    ui->comboItemType->addItems(QStringList() << "Communications" << "Speakers" << "Recordings" << "Annotations");
    ui->comboItemType->setCurrentText("Communications");
    connect(ui->comboItemType, SIGNAL(currentTextChanged(QString)), this, SLOT(corpusObjectTypeChanged(QString)));
    // Menu and Toolbar actions
//    d->toolbarCorpusExplorerTable = new QToolBar("Corpus Explorer Tables", this);
//    d->toolbarCorpusExplorerTable->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//    this->addToolBar(d->toolbarCorpusExplorerTable);
}

CorpusExplorerTableWidget::~CorpusExplorerTableWidget()
{
    delete ui;
    delete d;
}

void CorpusExplorerTableWidget::corpusObjectTypeChanged(const QString &text)
{
    if (text == "Communications")
        d->corpuObjectType = CorpusObject::Type_Communication;
    else if (text == "Speakers")
        d->corpuObjectType = CorpusObject::Type_Speaker;
    else if (text == "Recordings")
        d->corpuObjectType = CorpusObject::Type_Recording;
    else if (text == "Annotations")
        d->corpuObjectType = CorpusObject::Type_Annotation;
    refreshModel();
}

void CorpusExplorerTableWidget::refreshModel()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) {
        d->tableView->tableView()->setModel(0);
        if (d->model) delete d->model;
        d->model = 0;
        return;
    }
    // otherwise, another corpus has been selected
    QAbstractTableModel *newModel = 0;
    if (d->corpuObjectType == CorpusObject::Type_Communication) {
        newModel = new CorpusCommunicationTableModel(corpus->communicationsList(),
                                                     corpus->metadataStructure()->attributes(CorpusObject::Type_Communication),
                                                     corpus, true, this);
    }
    else if (d->corpuObjectType == CorpusObject::Type_Speaker) {
        newModel = new CorpusSpeakerTableModel(corpus->speakersList(),
                                               corpus->metadataStructure()->attributes(CorpusObject::Type_Speaker),
                                               corpus, true, this);
    }
    else if (d->corpuObjectType == CorpusObject::Type_Recording) {
        newModel = new CorpusRecordingTableModel(corpus->recordingsList(),
                                                 corpus->metadataStructure()->attributes(CorpusObject::Type_Recording),
                                                 true, this);
    }
    else if (d->corpuObjectType == CorpusObject::Type_Annotation) {
        newModel = new CorpusAnnotationTableModel(corpus->annotationsList(),
                                                  corpus->metadataStructure()->attributes(CorpusObject::Type_Annotation),
                                                  true, this);
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

void CorpusExplorerTableWidget::activeCorpusChanged(const QString &newActiveCorpusID)
{
    Q_UNUSED(newActiveCorpusID);
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
