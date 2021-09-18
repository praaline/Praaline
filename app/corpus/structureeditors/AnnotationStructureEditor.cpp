#include <QString>
#include <QToolBar>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QDebug>

#include "AnnotationStructureEditor.h"
#include "ui_AnnotationStructureEditor.h"

#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
#include "PraalineCore/Serialisers/JSON/JSONSerialiserAnnotationStructure.h"
#include "PraalineCore/Serialisers/XML/XMLSerialiserAnnotationStructure.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/AnnotationStructureTreeModel.h"
#include "NameValueListEditor.h"
#include "CorpusRepositoriesManager.h"
#include "AddAttributeDialog.h"
#include "AddLevelDialog.h"

#include "pngui/PraalineUserInterfaceOptions.h"

struct AnnotationStructureEditorData {
    AnnotationStructureEditorData() : corpusRepositoriesManager(0), treemodelAnnotationStructure(0)
    { }

    CorpusRepositoriesManager *corpusRepositoriesManager;

    QAction *actionSaveAnnotationStructure;
    QAction *actionAddAnnotationStructureLevel;
    QAction *actionAddAnnotationStructureAttribute;
    QAction *actionRemoveAnnotationStructureItem;
    QAction *actionImportAnnotationStructure;
    QAction *actionExportAnnotationStructure;
    QAction *actionDuplicateAnnotationLevel;

    QTreeView *treeviewAnnotationStructure;
    QPointer<AnnotationStructureTreeModel> treemodelAnnotationStructure;

    QToolBar *toolbarAnnotationStructure;

    NameValueListEditor *editorNVList;
};

AnnotationStructureEditor::AnnotationStructureEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AnnotationStructureEditor), d(new AnnotationStructureEditorData)
{
    ui->setupUi(this);

    // Get Corpora Manager from global object list
    QList<QObject *> list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    connect(d->corpusRepositoriesManager, &CorpusRepositoriesManager::activeCorpusRepositoryChanged, this, &AnnotationStructureEditor::activeCorpusRepositoryChanged);

    // Toolbars and actions
    d->toolbarAnnotationStructure = new QToolBar("Annotation Structure", this);
    d->toolbarAnnotationStructure->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarAnnotationStructure->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    setupActions();

    // Set up structure editor
    QMainWindow *inner;
    inner = new QMainWindow(this);
    inner->addToolBar(d->toolbarAnnotationStructure);
    d->treeviewAnnotationStructure = new QTreeView(inner);
    inner->setCentralWidget(d->treeviewAnnotationStructure);
    ui->dockAnnotationStructure->setWidget(inner);
    d->treeviewAnnotationStructure->setAlternatingRowColors(true);

    // Name-value list editor
    d->editorNVList = new NameValueListEditor(this);
    ui->dockNameValueLists->setWidget(d->editorNVList);

    // Set proportions
    QList<int> sizes;
    sizes << width() * 0.8 << width() * 0.2;
    ui->splitter->setSizes(sizes);
}

AnnotationStructureEditor::~AnnotationStructureEditor()
{
    delete ui;
    delete d;
}

void AnnotationStructureEditor::setupActions()
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

    d->actionSaveAnnotationStructure = new QAction(QIcon(":icons/actions/action_save.png"), "Save", this);
    connect(d->actionSaveAnnotationStructure, &QAction::triggered, this, &AnnotationStructureEditor::saveAnnotationStructure);
    command = ACTION_MANAGER->registerAction("Corpus.Structure.SaveAnnotationStructure", d->actionSaveAnnotationStructure, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarAnnotationStructure->addAction(d->actionSaveAnnotationStructure);

    d->actionAddAnnotationStructureLevel = new QAction(QIcon(":icons/actions/list_add.png"), "Add Level", this);
    connect(d->actionAddAnnotationStructureLevel, &QAction::triggered, this, &AnnotationStructureEditor::addAnnotationStructureLevel);
    command = ACTION_MANAGER->registerAction("Corpus.Structure.AddAnnotationLevel", d->actionAddAnnotationStructureLevel, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarAnnotationStructure->addAction(d->actionAddAnnotationStructureLevel);

    d->actionAddAnnotationStructureAttribute = new QAction(QIcon(":icons/actions/list_add.png"), "Add Attribute", this);
    connect(d->actionAddAnnotationStructureAttribute, &QAction::triggered, this, &AnnotationStructureEditor::addAnnotationStructureAttribute);
    command = ACTION_MANAGER->registerAction("Corpus.Structure.AddAnnotationAttribute", d->actionAddAnnotationStructureAttribute, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarAnnotationStructure->addAction(d->actionAddAnnotationStructureAttribute);

    d->actionRemoveAnnotationStructureItem = new QAction(QIcon(":icons/actions/list_remove.png"), "Delete", this);
    connect(d->actionRemoveAnnotationStructureItem, &QAction::triggered, this, &AnnotationStructureEditor::removeAnnotationStructureItem);
    command = ACTION_MANAGER->registerAction("Corpus.Structure.RemoveAnnotationItem", d->actionRemoveAnnotationStructureItem, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarAnnotationStructure->addAction(d->actionRemoveAnnotationStructureItem);

    d->actionImportAnnotationStructure = new QAction(QIcon(":icons/actions/table_import.png"), "Import Annotation Structure", this);
    connect(d->actionImportAnnotationStructure, &QAction::triggered, this, &AnnotationStructureEditor::importAnnotationStructure);
    command = ACTION_MANAGER->registerAction("Corpus.Structure.ImportAnnotationStructure", d->actionImportAnnotationStructure, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarAnnotationStructure->addAction(d->actionImportAnnotationStructure);

    d->actionExportAnnotationStructure = new QAction(QIcon(":icons/actions/table_export.png"), "Export Annotation Structure", this);
    connect(d->actionExportAnnotationStructure, &QAction::triggered, this, &AnnotationStructureEditor::exportAnnotationStructure);
    command = ACTION_MANAGER->registerAction("Corpus.Structure.ExportAnnotationStructure", d->actionExportAnnotationStructure, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarAnnotationStructure->addAction(d->actionExportAnnotationStructure);
}

// ----------------------------------------------- corpus --------------------------------------------------------

void AnnotationStructureEditor::refreshAnnotationStructureTreeView(AnnotationStructure *structure)
{
    if (!structure) return;
    QPointer<AnnotationStructureTreeModel> previousAnnotationStructureModel = d->treemodelAnnotationStructure;
    // disconnect events from previous model
    if (d->treemodelAnnotationStructure) d->treemodelAnnotationStructure->disconnect();
    // create new model
    d->treemodelAnnotationStructure = new AnnotationStructureTreeModel(structure, false, false, this);
    // connect signals to new model
    connect(d->treemodelAnnotationStructure.data(), &AnnotationStructureTreeModel::renameAnnotationLevel,
            this, &AnnotationStructureEditor::renameAnnotationLevel);
    connect(d->treemodelAnnotationStructure.data(), &AnnotationStructureTreeModel::renameAnnotationAttribute,
            this, &AnnotationStructureEditor::renameAnnotationAttribute);
    // set model
    d->treeviewAnnotationStructure->setModel(d->treemodelAnnotationStructure);
    d->treeviewAnnotationStructure->expandAll();
    for (int i = 0; i< d->treemodelAnnotationStructure->columnCount(); ++i)
        d->treeviewAnnotationStructure->resizeColumnToContents(i);
    if (previousAnnotationStructureModel) delete previousAnnotationStructureModel;
}

void AnnotationStructureEditor::activeCorpusRepositoryChanged(const QString &repositoryID)
{
    Q_UNUSED(repositoryID);
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) {
        d->treeviewAnnotationStructure->setModel(nullptr);
        if (d->treemodelAnnotationStructure) delete d->treemodelAnnotationStructure;
        d->editorNVList->rebind(nullptr);
    } else {
        refreshAnnotationStructureTreeView(repository->annotationStructure());
        d->editorNVList->rebind(repository->annotations());
    }
}

// ----------------------------------------------- annotations --------------------------------------------------------

void AnnotationStructureEditor::saveAnnotationStructure()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    if (!repository->annotations()) return;
    repository->annotations()->saveAnnotationStructure();
}

void AnnotationStructureEditor::addAnnotationStructureLevel()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    if (!repository->annotationStructure()) return;
    if (!d->treemodelAnnotationStructure) return;

    // Ask user for level definition
    AddLevelDialog *dialog = new AddLevelDialog(repository->annotationStructure(), this);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected) return;
    QString levelID = dialog->levelID();
    if (levelID.isEmpty()) return;
    AnnotationStructureLevel *newLevel = new AnnotationStructureLevel(levelID);
    newLevel->setLevelType(dialog->levelType());
    newLevel->setParentLevelID(dialog->parentLevelID());
    DataType dt = DataType(dialog->datatype());
    if (dialog->datalength() > 0) dt = DataType(dt.base(), dialog->datalength());
    newLevel->setDatatype(dt);

    // Create level
    if (!repository->annotations()->createAnnotationLevel(newLevel))
        return; // failed to create level
    repository->annotationStructure()->addLevel(newLevel);
    refreshAnnotationStructureTreeView(repository->annotationStructure());
}

void AnnotationStructureEditor::addAnnotationStructureAttribute()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    if (!repository->annotationStructure()) return;
    if (!d->treemodelAnnotationStructure) return;

    QModelIndex index = d->treeviewAnnotationStructure->currentIndex();
    if (!index.isValid()) return;
    QObject *item = (QObject *)(index.internalPointer());
    if (!item) return;

    AnnotationStructureLevel *level = qobject_cast<AnnotationStructureLevel *>(item);
    AnnotationStructureAttribute *attr = qobject_cast<AnnotationStructureAttribute *>(item);
    if (attr) level = qobject_cast<AnnotationStructureLevel *>(attr->parent());
    if (!level) return;

    // Ask for attribute ID and add it
    AddAttributeDialog *dialog = new AddAttributeDialog(false, this);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected) return;
    QString attributeID = dialog->attributeID();
    if (attributeID.isEmpty()) return;
    AnnotationStructureAttribute *newAttribute = new AnnotationStructureAttribute(attributeID);
    DataType dt = DataType(dialog->datatype());
    if (dialog->datalength() > 0) dt = DataType(dt.base(), dialog->datalength());
    newAttribute->setDatatype(dt);

    if (!repository->annotations()->createAnnotationAttribute(level->ID(), newAttribute))
        return; // failed to create attribute
    level->addAttribute(newAttribute);
    refreshAnnotationStructureTreeView(repository->annotationStructure());
}

void AnnotationStructureEditor::removeAnnotationStructureItem()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    if (!repository->annotationStructure()) return;
    if (!d->treemodelAnnotationStructure) return;

    QModelIndex index = d->treeviewAnnotationStructure->currentIndex();
    if (!index.isValid()) return;
    QObject *item = (QObject *)(index.internalPointer());
    if (!item) return;

    AnnotationStructureLevel *level = qobject_cast<AnnotationStructureLevel *>(item);
    AnnotationStructureAttribute *attr = qobject_cast<AnnotationStructureAttribute *>(item);
    if (level) {
        if (QMessageBox::warning(this, "Remove annotation level?",
                                 QString("Do you want to delete all the annotations stored on Level '%1'? This action cannot be reversed!")
                                 .arg(level->name()), "&Yes", "&No", QString(), 1, 1) == QMessageBox::No) return;
        if (repository->annotations()->deleteAnnotationLevel(level->ID()))
            repository->annotationStructure()->removeLevelByID(level->ID());
        refreshAnnotationStructureTreeView(repository->annotationStructure());
    }
    else {
        level = qobject_cast<AnnotationStructureLevel *>(attr->parent());
        if (!level) return;
        if (QMessageBox::warning(this, "Remove annotation attribute?",
                                 QString("Do you want to delete all the annotations stored for Attribute '%1'? This action cannot be reversed!")
                                 .arg(attr->name()), "&Yes", "&No", QString(), 1, 1) == QMessageBox::No) return;
        if (repository->annotations()->deleteAnnotationAttribute(level->ID(), attr->ID()))
            level->removeAttributeByID(attr->ID());
        refreshAnnotationStructureTreeView(repository->annotationStructure());
    }
}

void AnnotationStructureEditor::importAnnotationStructure()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;

    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Annotation Structure"), "",
                                                    tr("XML File (*.xml);;JSON File (*.json);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    AnnotationStructure *structure(0);
    if (filename.endsWith(".json", Qt::CaseInsensitive)) {
        structure = JSONSerialiserAnnotationStructure::read(filename);
    } else {
        structure = XMLSerialiserAnnotationStructure::read(filename);
    }
    if (!structure) return;
    repository->importAnnotationStructure(structure);
    refreshAnnotationStructureTreeView(repository->annotationStructure());
}

void AnnotationStructureEditor::exportAnnotationStructure()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;

    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Annotation Structure"), "",
                                                    tr("XML File (*.xml);;JSON File (*.json);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    if (filename.endsWith("json", Qt::CaseInsensitive)) {
        JSONSerialiserAnnotationStructure::write(repository->annotationStructure(), filename);
    } else {
        if (!filename.endsWith(".xml", Qt::CaseInsensitive)) filename = filename.append(".xml");
        XMLSerialiserAnnotationStructure::write(repository->annotationStructure(), filename);
    }
}

void AnnotationStructureEditor::renameAnnotationLevel(const QString &oldID, const QString &newID)
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    if (!repository->annotations()) return;
    if (repository->annotations()->renameAnnotationLevel(oldID, newID)) {
        repository->annotationStructure()->level(oldID)->setID(newID);
    }
}

void AnnotationStructureEditor::renameAnnotationAttribute(const QString &levelID, const QString &oldID, const QString &newID)
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    if (!repository->annotations()) return;
    if (repository->annotations()->renameAnnotationAttribute(levelID, oldID, newID)) {
        repository->annotationStructure()->level(levelID)->attribute(oldID)->setID(newID);
    }
}

void AnnotationStructureEditor::cloneAnnotationStructureLevel()
{

}
