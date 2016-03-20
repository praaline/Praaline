#include "AdvancedQueriesWidget.h"
#include "ui_AdvancedQueriesWidget.h"

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

#include "pngui/widgets/gridviewwidget.h"
#include "CorporaManager.h"
#include "sql/SqlTextEdit.h"

struct AdvancedQueriesWidgetData {
    AdvancedQueriesWidgetData() : corporaManager(0), model(0)
    {}

    CorporaManager *corporaManager;
    QAbstractTableModel *model;

    SqlTextEdit *sqlEditor;
    GridViewWidget *tableView;
};

AdvancedQueriesWidget::AdvancedQueriesWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AdvancedQueriesWidget),
    d(new AdvancedQueriesWidgetData)
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
    // Add results grid
    d->tableView = new GridViewWidget(this);
    ui->gridLayoutTable->addWidget(d->tableView);
    d->tableView->tableView()->verticalHeader()->setDefaultSectionSize(20);
    // Add SQL editor
    d->sqlEditor = new SqlTextEdit();
    ui->gridLayoutQuery->addWidget(d->sqlEditor);
}

AdvancedQueriesWidget::~AdvancedQueriesWidget()
{
    delete ui;
    delete d;
}

void AdvancedQueriesWidget::activeCorpusChanged(const QString &newActiveCorpusID)
{

}
