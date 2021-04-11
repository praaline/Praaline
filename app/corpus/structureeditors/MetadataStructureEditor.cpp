#include <QString>
#include <QToolBar>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QDebug>

#include "MetadataStructureEditor.h"
#include "ui_MetadataStructureEditor.h"

#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Structure/MetadataStructure.h"
#include "PraalineCore/Serialisers/JSON/JSONSerialiserMetadataStructure.h"
#include "PraalineCore/Serialisers/XML/XMLSerialiserMetadataStructure.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/MetadataStructureTreeModel.h"
#include "NameValueListEditor.h"
#include "CorpusRepositoriesManager.h"
#include "AddAttributeDialog.h"
#include "AddLevelDialog.h"

#include "pngui/PraalineUserInterfaceOptions.h"

struct MetadataStructureEditorData {
    MetadataStructureEditorData() : corpusRepositoriesManager(0), treemodelMetadataStructure(0)
    { }

    CorpusRepositoriesManager *corpusRepositoriesManager;

    QAction *actionSaveMetadataStructure;
    QAction *actionAddMetadataStructureSection;
    QAction *actionAddMetadataStructureAttribute;
    QAction *actionRemoveMetadataStructureItem;
    QAction *actionImportMetadataStructure;
    QAction *actionExportMetadataStructure;

    QTreeView *treeviewMetadataStructure;
    QPointer<MetadataStructureTreeModel> treemodelMetadataStructure;

    QToolBar *toolbarMetadataStructure;

    NameValueListEditor *editorNVList;
};

MetadataStructureEditor::MetadataStructureEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MetadataStructureEditor), d(new MetadataStructureEditorData)
{
    ui->setupUi(this);

    // Get Corpora Manager from global object list
    QList<QObject *> list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    connect(d->corpusRepositoriesManager, SIGNAL(activeCorpusRepositoryChanged(QString)), this, SLOT(activeCorpusRepositoryChanged(QString)));

    // Toolbars and actions
    d->toolbarMetadataStructure = new QToolBar("Metadata Structure", this);
    d->toolbarMetadataStructure->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMetadataStructure->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    setupActions();

    // Set up structure editor
    QMainWindow *inner;
    inner = new QMainWindow(this);
    inner->addToolBar(d->toolbarMetadataStructure);
    d->treeviewMetadataStructure = new QTreeView(inner);
    inner->setCentralWidget(d->treeviewMetadataStructure);
    ui->dockMetadataStructure->setWidget(inner);
    d->treeviewMetadataStructure->setAlternatingRowColors(true);

    // Name-value list editor
    d->editorNVList = new NameValueListEditor(this);
    ui->dockNameValueLists->setWidget(d->editorNVList);

    // Set proportions
    QList<int> sizes;
    sizes << width() * 0.8 << width() * 0.2;
    ui->splitter->setSizes(sizes);
}

MetadataStructureEditor::~MetadataStructureEditor()
{
    delete ui;
    delete d;
}

void MetadataStructureEditor::setupActions()
{
    // Context and menu bar
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));

    bool existed;
    Command* command;

    ActionContainer* menu_bar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer* view_menu = ACTION_MANAGER->createMenu(qti_action_VIEW, existed);
    if (!existed) menu_bar->addMenu(view_menu, qti_action_HELP);
    ActionContainer* corpus_menu = ACTION_MANAGER->createMenu("&Corpus", existed);
    if (!existed) menu_bar->addMenu(corpus_menu, qti_action_HELP);

    // ------------------------------------------------------------------------------------------------------
    // STRUCTURE EDITOR
    // ------------------------------------------------------------------------------------------------------
    d->actionSaveMetadataStructure = new QAction(QIcon(":icons/actions/action_save.png"), "Save", this);
    connect(d->actionSaveMetadataStructure, SIGNAL(triggered()), SLOT(saveMetadataStructure()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.SaveMetadataStructure", d->actionSaveMetadataStructure, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataStructure->addAction(d->actionSaveMetadataStructure);

    d->actionAddMetadataStructureSection = new QAction(QIcon(":icons/actions/list_add.png"), "Add Section", this);
    connect(d->actionAddMetadataStructureSection, SIGNAL(triggered()), SLOT(addMetadataStructureSection()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.AddMetadataSection", d->actionAddMetadataStructureSection, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataStructure->addAction(d->actionAddMetadataStructureSection);

    d->actionAddMetadataStructureAttribute = new QAction(QIcon(":icons/actions/list_add.png"), "Add Attribute", this);
    connect(d->actionAddMetadataStructureAttribute, SIGNAL(triggered()), SLOT(addMetadataStructureAttribute()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.AddMetadataAttribute", d->actionAddMetadataStructureAttribute, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataStructure->addAction(d->actionAddMetadataStructureAttribute);

    d->actionRemoveMetadataStructureItem = new QAction(QIcon(":icons/actions/list_remove.png"), "Delete", this);
    connect(d->actionRemoveMetadataStructureItem, SIGNAL(triggered()), SLOT(removeMetadataStructureItem()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.RemoveMetadataItem", d->actionRemoveMetadataStructureItem, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataStructure->addAction(d->actionRemoveMetadataStructureItem);

    d->actionImportMetadataStructure = new QAction(QIcon(":icons/actions/table_import.png"), "Import Metadata Structure", this);
    connect(d->actionImportMetadataStructure, SIGNAL(triggered()), SLOT(importMetadataStructure()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.ImportMetadataStructure", d->actionImportMetadataStructure, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataStructure->addAction(d->actionImportMetadataStructure);

    d->actionExportMetadataStructure = new QAction(QIcon(":icons/actions/table_export.png"), "Export Metadata Structure", this);
    connect(d->actionExportMetadataStructure, SIGNAL(triggered()), SLOT(exportMetadataStructure()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.ExportMetadataStructure", d->actionExportMetadataStructure, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataStructure->addAction(d->actionExportMetadataStructure);
}

// ----------------------------------------------- corpus --------------------------------------------------------

void MetadataStructureEditor::refreshMetadataStructureTreeView(MetadataStructure *structure)
{
    if (!structure) return;
    QPointer<MetadataStructureTreeModel> previousMetadataStructureModel = d->treemodelMetadataStructure;
    // disconnect events from previous model
    if (d->treemodelMetadataStructure) d->treemodelMetadataStructure->disconnect();
    // create new model
    d->treemodelMetadataStructure = new MetadataStructureTreeModel(structure, this);
    // connect signals to new model
    connect(d->treemodelMetadataStructure, SIGNAL(renameMetadataAttribute(Praaline::Core::CorpusObject::Type, QString, QString)),
            this, SLOT(renameMetadataAttribute(Praaline::Core::CorpusObject::Type,QString,QString)));
    // set model
    d->treeviewMetadataStructure->setModel(d->treemodelMetadataStructure);
    d->treeviewMetadataStructure->expandAll();
    for (int i = 0; i< d->treemodelMetadataStructure->columnCount(); ++i)
        d->treeviewMetadataStructure->resizeColumnToContents(i);
    if (previousMetadataStructureModel) delete previousMetadataStructureModel;
}

void MetadataStructureEditor::activeCorpusRepositoryChanged(const QString &repositoryID)
{
    Q_UNUSED(repositoryID);
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) {
        d->treeviewMetadataStructure->setModel(0);
        if (d->treemodelMetadataStructure) delete d->treemodelMetadataStructure;
        d->editorNVList->rebind(0);
    } else {
        refreshMetadataStructureTreeView(repository->metadataStructure());
        d->editorNVList->rebind(repository->metadata());
    }
}

// ----------------------------------------------- metadata --------------------------------------------------------

void MetadataStructureEditor::saveMetadataStructure()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    if (!repository->metadata()) return;
    repository->metadata()->saveMetadataStructure();
}

void MetadataStructureEditor::addMetadataStructureSection()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    if (!d->treemodelMetadataStructure) return;
    QModelIndex index = d->treeviewMetadataStructure->currentIndex();
    if (!index.isValid()) return;

    QObject *item = (QObject *)(index.internalPointer());
    if (!item) return;

    MetadataStructureTreeModelCategory *category = qobject_cast<MetadataStructureTreeModelCategory *>(item);
    MetadataStructureSection *section = qobject_cast<MetadataStructureSection *>(item);
    MetadataStructureAttribute *attribute = qobject_cast<MetadataStructureAttribute *>(item);
    CorpusObject::Type type {CorpusObject::Type_Undefined};
    if (category)
        type = category->type;
    else if (section)
        type = repository->metadataStructure()->corpusObjectTypeOfSection(section);
    else if (attribute) {
        section = qobject_cast<MetadataStructureSection *>(attribute->parent());
        if (!section) return;
        type = repository->metadataStructure()->corpusObjectTypeOfSection(section);
    }
    repository->metadataStructure()->addSection(type, new MetadataStructureSection());
    refreshMetadataStructureTreeView(repository->metadataStructure());
}

void MetadataStructureEditor::addMetadataStructureAttribute()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;

    if (!d->treemodelMetadataStructure) return;
    QModelIndex index = d->treeviewMetadataStructure->currentIndex();
    if (!index.isValid()) return;

    QObject *item = (QObject *)(index.internalPointer());
    if (!item) return;

    MetadataStructureSection *section = qobject_cast<MetadataStructureSection *>(item);
    MetadataStructureAttribute *attribute = qobject_cast<MetadataStructureAttribute *>(item);
    if (attribute) section = qobject_cast<MetadataStructureSection *>(attribute->parent());
    if (!section) return;
    CorpusObject::Type type = repository->metadataStructure()->corpusObjectTypeOfSection(section);

    // Ask for attribute ID and add it
    AddAttributeDialog *dialog = new AddAttributeDialog(this);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected) return;
    QString attributeID = dialog->attributeID();
    if (attributeID.isEmpty()) return;
    MetadataStructureAttribute *newAttribute = new MetadataStructureAttribute(attributeID);
    DataType dt = DataType(dialog->datatype());
    if (dialog->datalength() > 0) dt = DataType(dt.base(), dialog->datalength());
    newAttribute->setDatatype(dt);
    newAttribute->setParent(section);

    if (!repository->metadata()->createMetadataAttribute(type, newAttribute)) {
        return; // failed to create attribute
    }
    section->addAttribute(newAttribute);
    refreshMetadataStructureTreeView(repository->metadataStructure());
}

void MetadataStructureEditor::removeMetadataStructureItem()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    if (!repository->metadataStructure()) return;
    if (!d->treemodelMetadataStructure) return;

    QModelIndex index = d->treeviewMetadataStructure->currentIndex();
    if (!index.isValid()) return;
    QObject *item = (QObject *)(index.internalPointer());
    if (!item) return;

    MetadataStructureSection *section = qobject_cast<MetadataStructureSection *>(item);
    MetadataStructureAttribute *attribute = qobject_cast<MetadataStructureAttribute *>(item);
    if (section) {
        if (QMessageBox::warning(this, "Remove metadata section?",
                                 QString("Do you want to delete all the metadata stored under Section '%1'? "
                                         "This action cannot be reversed!").arg(section->name()),
                                 "&Yes", "&No", QString(), 1, 1) == QMessageBox::No) return;
        CorpusObject::Type type = repository->metadataStructure()->corpusObjectTypeOfSection(section);
        foreach (MetadataStructureAttribute *attribute, section->attributes())
            repository->metadata()->deleteMetadataAttribute(type, attribute->ID());
        repository->metadataStructure()->removeSectionByID(type, section->ID());
        refreshMetadataStructureTreeView(repository->metadataStructure());
    }
    else if (attribute) {
        section = qobject_cast<MetadataStructureSection *>(attribute->parent());
        if (!section) return;
        if (QMessageBox::warning(this, "Remove metadata attribute?",
                                 QString("Do you want to delete all the metadata stored for Attribute '%1'? "
                                         "This action cannot be reversed!").arg(attribute->name()),
                                 "&Yes", "&No", QString(), 1, 1) == QMessageBox::No) return;
        CorpusObject::Type type = repository->metadataStructure()->corpusObjectTypeOfSection(section);
        repository->metadata()->deleteMetadataAttribute(type, attribute->ID());
        section->removeAttributeByID(attribute->ID());
        refreshMetadataStructureTreeView(repository->metadataStructure());
    }
}

void MetadataStructureEditor::renameMetadataAttribute(Praaline::Core::CorpusObject::Type type,
                                                          const QString &oldID, const QString &newID)
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    if (!repository->metadata()) return;
    if (repository->metadata()->renameMetadataAttribute(type, oldID, newID)) {
        repository->metadataStructure()->attribute(type, oldID)->setID(newID);
    }
}

void MetadataStructureEditor::importMetadataStructure()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Metadata Structure"), "",
                                                    tr("XML File (*.xml);;JSON File (*.json);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    MetadataStructure *structure(0);
    if (filename.toLower().endsWith(".json")) {
        structure = JSONSerialiserMetadataStructure::read(filename);
    } else {
        structure = XMLSerialiserMetadataStructure::read(filename);
    }
    if (!structure) {
        QMessageBox::warning(this, tr("Error Reading File"),
                             tr("Error while reading the metadata structure file: %1").arg(filename));
        return;
    }
    repository->importMetadataStructure(structure);
    refreshMetadataStructureTreeView(repository->metadataStructure());
}

void MetadataStructureEditor::exportMetadataStructure()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Metadata Structure"), "",
                                                    tr("XML File (*.xml);;JSON File (*.json);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    if (filename.toLower().endsWith("json")) {
        JSONSerialiserMetadataStructure::write(repository->metadataStructure(), filename);
    } else {
        if (!filename.toLower().endsWith(".xml")) filename = filename.append(".xml");
        XMLSerialiserMetadataStructure::write(repository->metadataStructure(), filename);
    }
}
