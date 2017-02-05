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

    // Corpora manager
    QList<QObject *> list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }

    // Find available statistical analysis plugins
    createStatisticsPluginsTree();

    // Handle "open analyser" double click
    connect(ui->treeviewStatisticsPlugins, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(anayserDoubleClicked(QModelIndex)));
    connect(ui->tabWidgetDocuments, SIGNAL(tabCloseRequested(int)), this, SLOT(documentTabCloseRequested(int)));
}

StatisticsModeWidget::~StatisticsModeWidget()
{
    delete ui;
    delete d;
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
    CorpusRepository *repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;

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
