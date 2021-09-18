#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDebug>
#include "StatisticsModeWidget.h"
#include "ui_StatisticsModeWidget.h"

#include "CorpusRepositoriesManager.h"

#include <QtilitiesExtensionSystem>
using namespace QtilitiesExtensionSystem;
#include "interfaces/IStatisticsPlugin.h"
using namespace Praaline::Plugins;

struct StatisticsModeWidgetData {
    StatisticsModeWidgetData() :
        corpusRepositoriesManager(0), modelStatisticsPlugins(0)
    {}

    CorpusRepositoriesManager *corpusRepositoriesManager;
    QStandardItemModel *modelStatisticsPlugins;
    QList<QWidget *> openDocuments;
};

StatisticsModeWidget::StatisticsModeWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::StatisticsModeWidget), d(new StatisticsModeWidgetData)
{
    ui->setupUi(this);
    // Get CorpusRepositoriesManager from global object list
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    connect(d->corpusRepositoriesManager, &CorpusRepositoriesManager::corpusRepositoryAdded, this, &StatisticsModeWidget::corpusRepositoryAdded);
    connect(d->corpusRepositoriesManager, &CorpusRepositoriesManager::corpusRepositoryRemoved, this, &StatisticsModeWidget::corpusRepositoryRemoved);
    // List of repositories
    ui->comboBoxCorpusRepository->addItems(d->corpusRepositoriesManager->listCorpusRepositoryIDs());
    // Find available statistical analysis plugins
    createStatisticsPluginsTree();
    // Handle "open analyser" double click
    connect(ui->treeviewStatisticsPlugins, &QAbstractItemView::doubleClicked, this, &StatisticsModeWidget::anayserDoubleClicked);
    connect(ui->tabWidgetDocuments, &QTabWidget::tabCloseRequested, this, &StatisticsModeWidget::documentTabCloseRequested);
}

StatisticsModeWidget::~StatisticsModeWidget()
{
    delete ui;
    delete d;
}

void StatisticsModeWidget::corpusRepositoryAdded(const QString &repositoryID)
{
    // Called whenever a new corpus repository is opened: adds the repository to the combobox
    if (!d->corpusRepositoriesManager) return;
    CorpusRepository *repository = d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID);
    if (!repository) return;
    ui->comboBoxCorpusRepository->addItem(repository->ID(), repositoryID);
}

void StatisticsModeWidget::corpusRepositoryRemoved(const QString &repositoryID)
{
    // Called whenever a new corpus repository is closed: removes the repository from the combobox
    if (!d->corpusRepositoriesManager) return;
    int index = ui->comboBoxCorpusRepository->findData(repositoryID);
    if (index >= 0) {
        ui->comboBoxCorpusRepository->removeItem(index);
    }
}

void StatisticsModeWidget::createStatisticsPluginsTree()
{
    d->modelStatisticsPlugins = new QStandardItemModel(this);
    QStandardItem *rootItem = d->modelStatisticsPlugins->invisibleRootItem();
    QList<QObject*> registeredStatisticsPlugins = OBJECT_MANAGER->registeredInterfaces("IStatisticsPlugin");
    foreach (QObject *obj, registeredStatisticsPlugins) {
        IStatisticsPlugin *plugin = dynamic_cast<IStatisticsPlugin *>(obj);
        if (!plugin) continue;
        QStandardItem *itemPlugin = new QStandardItem(plugin->pluginName());
        itemPlugin->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        foreach (QString analyserID, plugin->analyserIDs()) {
            QStandardItem *itemAnalyser = new QStandardItem(plugin->analyserName(analyserID));
            itemAnalyser->setData(plugin->pluginName() + "::" + analyserID);
            itemAnalyser->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            itemPlugin->appendRow(itemAnalyser);
        }
        rootItem->appendRow(itemPlugin);
    }
    d->modelStatisticsPlugins->setHorizontalHeaderLabels(QStringList() << tr("Statistical Analysis Plugins"));
    ui->treeviewStatisticsPlugins->setModel(d->modelStatisticsPlugins);
    ui->treeviewStatisticsPlugins->expandAll();
}

void StatisticsModeWidget::anayserDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    if (!d->modelStatisticsPlugins) return;
    QString corpusRepositoryID = ui->comboBoxCorpusRepository->currentText();
    CorpusRepository *repository = d->corpusRepositoriesManager->corpusRepositoryByID(corpusRepositoryID);
    if (!repository) {
        QMessageBox::warning(this, tr("Statistics"), tr("Please open or connect to a Corpus Repository first."),
                             QMessageBox::Ok);
        return;
    }

    QString id = d->modelStatisticsPlugins->data(index, Qt::UserRole + 1).toString();
    if (id.isEmpty()) return;
    if (!id.contains("::")) return;
    QString pluginName = id.section("::", 0, 0);
    QString analyserID = id.section("::", 1, 1);
    foreach (QObject *obj, OBJECT_MANAGER->registeredInterfaces("IStatisticsPlugin")) {
        IStatisticsPlugin *plugin = dynamic_cast<IStatisticsPlugin *>(obj);
        if (!plugin) continue;
        if (plugin->pluginName() ==  pluginName) {
            QWidget *analyser = plugin->analyser(analyserID, repository, this);
            d->openDocuments.append(analyser);
            ui->tabWidgetDocuments->addTab(analyser, plugin->analyserName(analyserID));
            ui->tabWidgetDocuments->setCurrentWidget(analyser);
        }
    }
}

void StatisticsModeWidget::documentTabCloseRequested(int index)
{
    QWidget *analyser = d->openDocuments.takeAt(index);
    if (analyser) {
        analyser->deleteLater();
        ui->tabWidgetDocuments->removeTab(index);
    }
}
