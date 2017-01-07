#include <QString>
#include <QToolBar>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QDebug>

#include "AnnotationStructureEditor.h"
#include "ui_AnnotationStructureEditor.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/serialisers/xml/XMLSerialiserMetadataStructure.h"
#include "pncore/serialisers/xml/XMLSerialiserAnnotationStructure.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/AnnotationStructureTreeModel.h"
#include "pngui/observers/CorpusObserver.h"
#include "NameValueListEditor.h"
#include "CorporaManager.h"
#include "AddAttributeDialog.h"
#include "AddLevelDialog.h"

struct AnnotationStructureEditorData {
    AnnotationStructureEditorData() :
        corporaManager(0), treemodelAnnotationStructure(0)
    { }

    CorporaManager *corporaManager;

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
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorporaManager");
    foreach (QObject* obj, list) {
        CorporaManager *manager = qobject_cast<CorporaManager *>(obj);
        if (manager) d->corporaManager = manager;
    }
    connect(d->corporaManager, SIGNAL(activeCorpusChanged(QString)), this, SLOT(activeCorpusChanged(QString)));

    // Toolbars and actions
    d->toolbarAnnotationStructure = new QToolBar("Annotation Structure", this);
    d->toolbarAnnotationStructure->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarAnnotationStructure->setIconSize(QSize(24, 24));
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
    ui->splitter->setSizes(QList<int>() << 100 << 100);
    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 1);
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

    d->actionAddAnnotationStructureLevel = new QAction(QIcon(":icons/actions/list_add.png"), "Add Level", this);
    connect(d->actionAddAnnotationStructureLevel, SIGNAL(triggered()), SLOT(addAnnotationStructureLevel()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.AddAnnotationLevel", d->actionAddAnnotationStructureLevel, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarAnnotationStructure->addAction(d->actionAddAnnotationStructureLevel);

    d->actionAddAnnotationStructureAttribute = new QAction(QIcon(":icons/actions/list_add.png"), "Add Attribute", this);
    connect(d->actionAddAnnotationStructureAttribute, SIGNAL(triggered()), SLOT(addAnnotationStructureAttribute()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.AddAnnotationAttribute", d->actionAddAnnotationStructureAttribute, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarAnnotationStructure->addAction(d->actionAddAnnotationStructureAttribute);

    d->actionRemoveAnnotationStructureItem = new QAction(QIcon(":icons/actions/list_remove.png"), "Delete", this);
    connect(d->actionRemoveAnnotationStructureItem, SIGNAL(triggered()), SLOT(removeAnnotationStructureItem()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.RemoveAnnotationItem", d->actionRemoveAnnotationStructureItem, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarAnnotationStructure->addAction(d->actionRemoveAnnotationStructureItem);

    d->actionImportAnnotationStructure = new QAction("Import Annotation Structure", this);
    connect(d->actionImportAnnotationStructure, SIGNAL(triggered()), SLOT(importAnnotationStructure()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.ImportAnnotationStructure", d->actionImportAnnotationStructure, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarAnnotationStructure->addAction(d->actionImportAnnotationStructure);

    d->actionExportAnnotationStructure = new QAction("Export Annotation Structure", this);
    connect(d->actionExportAnnotationStructure, SIGNAL(triggered()), SLOT(exportAnnotationStructure()));
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
    connect(d->treemodelAnnotationStructure, SIGNAL(renameAnnotationLevel(QString, QString)),
            this, SLOT(renameAnnotationLevel(QString,QString)));
    connect(d->treemodelAnnotationStructure, SIGNAL(renameAnnotationAttribute(QString, QString, QString)),
            this, SLOT(renameAnnotationAttribute(QString,QString,QString)));
    // set model
    d->treeviewAnnotationStructure->setModel(d->treemodelAnnotationStructure);
    d->treeviewAnnotationStructure->expandAll();
    for (int i = 0; i< d->treemodelAnnotationStructure->columnCount(); ++i)
        d->treeviewAnnotationStructure->resizeColumnToContents(i);
    if (previousAnnotationStructureModel) delete previousAnnotationStructureModel;
}

void AnnotationStructureEditor::activeCorpusChanged(const QString &newActiveCorpusID)
{
    Q_UNUSED(newActiveCorpusID);
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) {
        d->treeviewAnnotationStructure->setModel(0);
        if (d->treemodelAnnotationStructure) delete d->treemodelAnnotationStructure;
    } else {
        refreshAnnotationStructureTreeView(corpus->annotationStructure());
    }
}

// ----------------------------------------------- annotations --------------------------------------------------------

void AnnotationStructureEditor::addAnnotationStructureLevel()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    if (!d->treemodelAnnotationStructure) return;
    if (!obs->corpus()->annotationStructure()) return;

    // Ask user for level definition
    AddLevelDialog *dialog = new AddLevelDialog(obs->corpus()->annotationStructure(), this);
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
    if (!obs->corpus()->datastoreAnnotations()->createAnnotationLevel(newLevel))
        return; // failed to create level
    obs->corpus()->annotationStructure()->addLevel(newLevel);
    refreshAnnotationStructureTreeView(obs->corpus()->annotationStructure());
}

void AnnotationStructureEditor::addAnnotationStructureAttribute()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    if (!d->treemodelAnnotationStructure) return;
    if (!obs->corpus()->annotationStructure()) return;

    QModelIndex index = d->treeviewAnnotationStructure->currentIndex();
    if (!index.isValid()) return;
    QObject *item = (QObject *)(index.internalPointer());
    if (!item) return;

    AnnotationStructureLevel *level = qobject_cast<AnnotationStructureLevel *>(item);
    AnnotationStructureAttribute *attr = qobject_cast<AnnotationStructureAttribute *>(item);
    if (attr) level = qobject_cast<AnnotationStructureLevel *>(attr->parent());
    if (!level) return;

    // Ask for attribute ID and add it
    AddAttributeDialog *dialog = new AddAttributeDialog(this);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected) return;
    QString attributeID = dialog->attributeID();
    if (attributeID.isEmpty()) return;
    AnnotationStructureAttribute *newAttribute = new AnnotationStructureAttribute(attributeID);
    DataType dt = DataType(dialog->datatype());
    if (dialog->datalength() > 0) dt = DataType(dt.base(), dialog->datalength());
    newAttribute->setDatatype(dt);

    if (!obs->corpus()->datastoreAnnotations()->createAnnotationAttribute(level->ID(), newAttribute))
        return; // failed to create attribute
    level->addAttribute(newAttribute);
    refreshAnnotationStructureTreeView(obs->corpus()->annotationStructure());
}

void AnnotationStructureEditor::removeAnnotationStructureItem()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    if (!d->treemodelAnnotationStructure) return;
    if (!obs->corpus()->annotationStructure()) return;

    QModelIndex index = d->treeviewAnnotationStructure->currentIndex();
    if (!index.isValid()) return;
    QObject *item = (QObject *)(index.internalPointer());
    if (!item) return;

    AnnotationStructureLevel *level = qobject_cast<AnnotationStructureLevel *>(item);
    AnnotationStructureAttribute *attr = qobject_cast<AnnotationStructureAttribute *>(item);
    if (level) {
        if (QMessageBox::warning(this, "Remove annotation level?",
                                 QString("Do you want to delete all the annotations stored on Level '%1'? This action cannot be reversed!")
                                 .arg(level->name()), "&Yes", "&No", QString::null, 1, 1) == QMessageBox::No) return;
        obs->corpus()->datastoreAnnotations()->deleteAnnotationLevel(level->ID());
        obs->corpus()->annotationStructure()->removeLevelByID(level->ID());
        refreshAnnotationStructureTreeView(obs->corpus()->annotationStructure());
    }
    else {
        level = qobject_cast<AnnotationStructureLevel *>(attr->parent());
        if (!level) return;
        if (QMessageBox::warning(this, "Remove annotation attribute?",
                                 QString("Do you want to delete all the annotations stored for Attribute '%1'? This action cannot be reversed!")
                                 .arg(attr->name()), "&Yes", "&No", QString::null, 1, 1) == QMessageBox::No) return;
        obs->corpus()->datastoreAnnotations()->deleteAnnotationAttribute(level->ID(), attr->ID());
        level->removeAttributeByID(attr->ID());
        refreshAnnotationStructureTreeView(obs->corpus()->annotationStructure());
    }
}

void AnnotationStructureEditor::importAnnotationStructure()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Annotation Structure"), "",
                                                    tr("XML File (*.xml);;All Files (*)"), &selectedFilter, options);
    if (filename.isEmpty()) return;
    AnnotationStructure *structure = XMLSerialiserAnnotationStructure::read(filename);
    if (!structure) return;
    obs->corpus()->importAnnotationStructure(structure);
    refreshAnnotationStructureTreeView(obs->corpus()->annotationStructure());
}

void AnnotationStructureEditor::exportAnnotationStructure()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Annotation Structure"), "",
                                                    tr("XML File (*.xml);;All Files (*)"), &selectedFilter, options);
    if (filename.isEmpty()) return;
    XMLSerialiserAnnotationStructure::write(obs->corpus()->annotationStructure(), filename);
}

void AnnotationStructureEditor::renameAnnotationLevel(const QString &oldID, const QString &newID)
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    if (!obs->corpus()->datastoreAnnotations()) return;
    if (obs->corpus()->datastoreAnnotations()->renameAnnotationLevel(oldID, newID)) {
        obs->corpus()->annotationStructure()->level(oldID)->setID(newID);
    }
}

void AnnotationStructureEditor::renameAnnotationAttribute(const QString &levelID, const QString &oldID, const QString &newID)
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    if (!obs->corpus()->datastoreAnnotations()) return;
    if (obs->corpus()->datastoreAnnotations()->renameAnnotationAttribute(levelID, oldID, newID)) {
        obs->corpus()->annotationStructure()->level(levelID)->attribute(oldID)->setID(newID);
    }
}

void AnnotationStructureEditor::cloneAnnotationStructureLevel()
{

}
