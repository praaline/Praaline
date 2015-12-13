#include <QString>
#include <QList>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QFont>
#include <QDebug>
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"

#include "automaticannotationwidget.h"
#include "ui_automaticannotationwidget.h"

#include "pngui/observers/corpusobserver.h"
#include "pngui/model/checkableproxymodel.h"

#include <QtilitiesExtensionSystem>
using namespace QtilitiesExtensionSystem;

#include "iannotationplugin.h"
using namespace Praaline::Plugins;

struct AutomaticAnnotationWidgetData {
    AutomaticAnnotationWidgetData() :
        runningPlugins(0), corporaTopLevelNode(0), checkableProxyModelCorpusItems(0), observerWidgetCorpusItems(0),
        modelAnnotationPlugins(0), checkableProxyModelAnnotationPlugins(0), treeviewAnnotationPlugins(0),
        propertyVariantManagerPluginParameters(0), propertyVariantFactoryPluginParameters(0), propertyBrowserPluginParameters(0)
    { }

    QAction *actionAnnotate;
    QAction *actionSaveOutput;
    QAction *actionClearOutput;

    QTextEdit *textResults;
    int runningPlugins;

    QPointer<TreeNode> corporaTopLevelNode;
    CheckableProxyModel *checkableProxyModelCorpusItems;
    ObserverWidget* observerWidgetCorpusItems;

    QStandardItemModel *modelAnnotationPlugins;
    CheckableProxyModel *checkableProxyModelAnnotationPlugins;
    QTreeView *treeviewAnnotationPlugins;

    QtVariantPropertyManager *propertyVariantManagerPluginParameters;
    QtVariantEditorFactory *propertyVariantFactoryPluginParameters;
    QtTreePropertyBrowser *propertyBrowserPluginParameters;
    QHash<QString, QtProperty *> propertiesSelectedPlugins;
    QHash<QString, QHash<QString, QtProperty *> > propertiesParameters;
};

AutomaticAnnotationWidget::AutomaticAnnotationWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AutomaticAnnotationWidget), d(new AutomaticAnnotationWidgetData)
{
    ui->setupUi(this);
    setupActions();

    // Find corpus explorer top-level node in object manager
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("Qtilities::CoreGui::TreeNode");
    foreach (QObject* obj, list) {
        TreeNode *node = qobject_cast<TreeNode *>(obj);
        if (node && node->observerName() == "Corpus Explorer") {
            d->corporaTopLevelNode = node;
        }
    }

    d->textResults = new QTextEdit(this);
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    d->textResults->setFont(fixedFont);
    ui->gridLayoutOutput->addWidget(d->textResults);

    // Create observer widget for communications sub-tree
    d->observerWidgetCorpusItems = new ObserverWidget(Qtilities::TreeView, this);
    d->observerWidgetCorpusItems->setRefreshMode(ObserverWidget::RefreshModeShowTree);
    d->observerWidgetCorpusItems->setGlobalMetaType("Corpus Tree Meta Type");
    d->observerWidgetCorpusItems->setAcceptDrops(false);
    d->observerWidgetCorpusItems->setObserverContext(d->corporaTopLevelNode);
    d->observerWidgetCorpusItems->layout()->setMargin(0);
    d->checkableProxyModelCorpusItems = new CheckableProxyModel(this);
    d->checkableProxyModelCorpusItems->setSourceModel(d->observerWidgetCorpusItems->treeModel());
    d->checkableProxyModelCorpusItems->setDefaultCheckState(false);
    d->observerWidgetCorpusItems->setCustomTreeProxyModel(d->checkableProxyModelCorpusItems);
    d->observerWidgetCorpusItems->initialize();

    ui->gridLayoutCorpusExplorer->addWidget(d->observerWidgetCorpusItems);

    // Plugin parameters property browser
    d->propertyVariantManagerPluginParameters = new QtVariantPropertyManager(this);
    d->propertyVariantFactoryPluginParameters = new QtVariantEditorFactory(this);
    d->propertyBrowserPluginParameters = new QtTreePropertyBrowser(this);
    d->propertyBrowserPluginParameters->setFactoryForManager(d->propertyVariantManagerPluginParameters, d->propertyVariantFactoryPluginParameters);
    d->propertyBrowserPluginParameters->setPropertiesWithoutValueMarked(true);
    d->propertyBrowserPluginParameters->setRootIsDecorated(false);
    d->propertyBrowserPluginParameters->show();
    ui->gridLayoutPluginParameters->addWidget(d->propertyBrowserPluginParameters);

    // Find available annotation plugins
    d->modelAnnotationPlugins = new QStandardItemModel(this);
    QList<QObject*> registeredAnnotationPlugins = OBJECT_MANAGER->registeredInterfaces("IAnnotationPlugin");
    int i = 0;
    foreach (QObject *obj, registeredAnnotationPlugins) {
        IAnnotationPlugin *plugin = dynamic_cast<IAnnotationPlugin *>(obj);
        if (!plugin) continue;
        d->modelAnnotationPlugins->setItem(i, 0, new QStandardItem(plugin->pluginName()));
        i++;
    }
    d->modelAnnotationPlugins->setHorizontalHeaderLabels(QStringList() << "Plugin Name");
    d->checkableProxyModelAnnotationPlugins = new CheckableProxyModel(this);
    d->checkableProxyModelAnnotationPlugins->setSourceModel(d->modelAnnotationPlugins);
    d->checkableProxyModelAnnotationPlugins->setDefaultCheckState(false);
    d->treeviewAnnotationPlugins = new QTreeView(this);
    d->treeviewAnnotationPlugins->setModel(d->checkableProxyModelAnnotationPlugins);
    ui->gridLayoutAnnotationPlugins->addWidget(d->treeviewAnnotationPlugins);
    connect(d->checkableProxyModelAnnotationPlugins, SIGNAL(checkedNodesChanged()), this, SLOT(pluginSelectionChanged()));
}

AutomaticAnnotationWidget::~AutomaticAnnotationWidget()
{
    delete ui;
    delete d;
}

void AutomaticAnnotationWidget::setupActions()
{
    ui->toolbarOutput->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(ui->commandAnnotate, SIGNAL(clicked()), this, SLOT(actionAnnotate()));
    d->actionSaveOutput = ui->toolbarOutput->addAction(QIcon(":/icons/actions/action_save.png"), "Save output");
    connect(d->actionSaveOutput, SIGNAL(triggered()), this, SLOT(actionSaveOutput()));
    d->actionClearOutput = ui->toolbarOutput->addAction(QIcon(":/icons/actions/clear.png"), "Clear output");
    connect(d->actionClearOutput, SIGNAL(triggered()), this, SLOT(actionClearOutput()));
}

void AutomaticAnnotationWidget::pluginSelectionChanged()
{
    QList<IAnnotationPlugin *> plugins = selectedPlugins();

    // Remove properties of unselected plugins
    foreach (QString pluginFilename, d->propertiesSelectedPlugins.keys()) {
        bool selected = false;
        foreach (IAnnotationPlugin *plugin, plugins) {
            if (plugin->pluginFileName() == pluginFilename) selected = true;
        }
        if (!selected) {
            d->propertyBrowserPluginParameters->removeProperty(d->propertiesSelectedPlugins.value(pluginFilename));
            d->propertiesParameters.remove(pluginFilename);
            d->propertiesSelectedPlugins.remove(pluginFilename);
        }
    }
    // Add properties of selected plugins
    foreach (IAnnotationPlugin *plugin, plugins) {
        if (d->propertiesSelectedPlugins.contains(plugin->pluginFileName())) continue;
        QList<IAnnotationPlugin::PluginParameter> parameters = plugin->pluginParameters();
        if (parameters.isEmpty()) continue;
        QtProperty *group = d->propertyVariantManagerPluginParameters->addProperty(QtVariantPropertyManager::groupTypeId(),
                                                                                   plugin->pluginName());
        QtVariantProperty *item;
        foreach (IAnnotationPlugin::PluginParameter parameter, parameters) {
            if (!parameter.listOptions.isEmpty()) {
                item = d->propertyVariantManagerPluginParameters->addProperty(QtVariantPropertyManager::enumTypeId(),
                                                                              parameter.name);
                item->setAttribute(QLatin1String("enumNames"), parameter.listOptions);
                item->setValue(parameter.defaultValue);
            } else {
                item = d->propertyVariantManagerPluginParameters->addProperty(parameter.propertyType, parameter.name);
                item->setValue(parameter.defaultValue);
            }
            group->addSubProperty(item);
            d->propertiesParameters[plugin->pluginFileName()].insert(parameter.ID, item);
        }
        d->propertyBrowserPluginParameters->addProperty(group);
        d->propertiesSelectedPlugins.insert(plugin->pluginFileName(), group);
    }
}

void AutomaticAnnotationWidget::actionSaveOutput()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Automatic Annotation Output"),
                                "praaline_report.txt", tr("Text File (*.txt);;All Files (*)"),
                                &selectedFilter, options);
    if (fileName.isEmpty()) return;
    QFile fileOut(fileName);
    if (! fileOut.open(QFile::WriteOnly | QFile::Text)) return;
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    out << d->textResults->document()->toPlainText();
    fileOut.close();
}

void AutomaticAnnotationWidget::actionClearOutput()
{
    d->textResults->clear();
}

void AutomaticAnnotationWidget::logAnnotationMessage(QString message)
{
    d->textResults->append(message);
    d->textResults->moveCursor(QTextCursor::End);
    QCoreApplication::processEvents();
}

void AutomaticAnnotationWidget::pluginMadeProgress(int percentage)
{
    if (d->runningPlugins == 0) { ui->progressBar->setValue(0); return; }
    int percentageAdjusted = percentage / d->runningPlugins;
    ui->progressBar->setValue(percentageAdjusted);
    QCoreApplication::processEvents();
}

void addNodeToListRecursive(QList<QObject *> &list, ObserverTreeItem *treeItem) {
    if (!treeItem) return;
    list << treeItem->getObject();
    if ((treeItem->itemType() == ObserverTreeItem::TreeNode) || (treeItem->itemType() == ObserverTreeItem::CategoryItem)) {
        foreach (QPointer<ObserverTreeItem> childItem, treeItem->childItemReferences()) {
            if (!childItem) continue;
            addNodeToListRecursive(list, childItem);
        }
    }
}

QList<QPointer<CorpusCommunication> > AutomaticAnnotationWidget::selectedCommunications()
{
    QList<QPointer<CorpusCommunication> > listCom;
    QModelIndexList listBranch;
    d->checkableProxyModelCorpusItems->checkedState().checkedBranchSourceModelIndexes(listBranch);
    QList<QObject *> nodesSelected;
    foreach (QModelIndex index, listBranch) {
        ObserverTreeItem *treeItem = d->observerWidgetCorpusItems->treeModel()->getItem(index);
        addNodeToListRecursive(nodesSelected, treeItem);
    }
    foreach (QObject *obj, nodesSelected) {
        CorpusExplorerTreeNodeCommunication *nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(obj);
        if (nodeCom && nodeCom->communication) listCom << nodeCom->communication;
    }
    return listCom;
}

QList<IAnnotationPlugin *> AutomaticAnnotationWidget::selectedPlugins()
{
    QList<IAnnotationPlugin *> listPlugins;
    QModelIndexList list;
    d->checkableProxyModelAnnotationPlugins->checkedState().checkedLeafSourceModelIndexes(list);
    foreach (QModelIndex index, list) {
        QString pluginName = d->modelAnnotationPlugins->data(index).toString();
        IPlugin *p = EXTENSION_SYSTEM->findPlugin(pluginName);
        IAnnotationPlugin *plugin = dynamic_cast<IAnnotationPlugin *>(p);
        if (plugin) listPlugins << plugin;
    }
    return listPlugins;
}

void AutomaticAnnotationWidget::actionAnnotate()
{
    QList<QPointer<CorpusCommunication> > communications = selectedCommunications();
    QList<IAnnotationPlugin *> plugins = selectedPlugins();

    if (communications.isEmpty() || plugins.isEmpty()) return;
    Corpus *corpus = qobject_cast<Corpus *>(selectedCommunications().first()->parent());
    ui->progressBar->setValue(0);
    d->runningPlugins = plugins.count();

    foreach (IAnnotationPlugin *plugin, plugins) {
        // Connect signals
        connect(dynamic_cast<QObject *>(plugin), SIGNAL(printMessage(QString)), this, SLOT(logAnnotationMessage(QString)));
        connect(dynamic_cast<QObject *>(plugin), SIGNAL(madeProgress(int)), this, SLOT(pluginMadeProgress(int)));
        // Pass parameters
        QHash<QString, QVariant> parameterValues;
        if (d->propertiesParameters.contains(plugin->pluginFileName())) {
            QHash<QString, QtProperty *> parameters = d->propertiesParameters.value(plugin->pluginFileName());
            foreach (QString ID, parameters.keys()) {
                parameterValues.insert(ID, d->propertyVariantManagerPluginParameters->value(parameters.value(ID)));
            }
        }
        // Send parameters to plugin
        plugin->setParameters(parameterValues);
        // Launch annotation plugin
        plugin->process(corpus, communications);
        // Disconnect signals
        disconnect(dynamic_cast<QObject *>(plugin), SIGNAL(printMessage(QString)), this, SLOT(logAnnotationMessage(QString)));
        disconnect(dynamic_cast<QObject *>(plugin), SIGNAL(madeProgress(int)), this, SLOT(pluginMadeProgress(int)));
    }
    d->runningPlugins = 0;
}
