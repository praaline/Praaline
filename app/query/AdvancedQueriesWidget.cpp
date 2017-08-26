#include "AdvancedQueriesWidget.h"
#include "ui_AdvancedQueriesWidget.h"

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

#include "pngui/widgets/GridViewWidget.h"
#include "CorpusRepositoriesManager.h"
#include "sql/SqlTextEdit.h"

struct AdvancedQueriesWidgetData {
    AdvancedQueriesWidgetData() : corpusRepositoriesManager(0), model(0)
    {}

    CorpusRepositoriesManager *corpusRepositoriesManager;
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
    // Get Corpus Repositories Manager from global object list
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    connect(d->corpusRepositoriesManager, SIGNAL(activeCorpusRepositoryChanged(QString)),
            this, SLOT(activeCorpusRepositoryChanged(QString)));
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

void AdvancedQueriesWidget::activeCorpusRepositoryChanged(const QString &newActiveCorpusRepositoryID)
{

}
