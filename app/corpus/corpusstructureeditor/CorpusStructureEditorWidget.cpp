#include <QString>
#include <QToolBar>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QDebug>

#include "CorpusStructureEditorWidget.h"
#include "ui_CorpusStructureEditorWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/serialisers/xml/XMLSerialiserMetadataStructure.h"
#include "pncore/serialisers/xml/XMLSerialiserAnnotationStructure.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/MetadataStructureTreeModel.h"
#include "pngui/model/corpus/AnnotationStructureTreeModel.h"
#include "pngui/observers/corpusobserver.h"
#include "CorporaManager.h"
#include "AddAttributeDialog.h"
#include "AddLevelDialog.h"

struct CorpusStructureEditorWidgetData {
    CorpusStructureEditorWidgetData() :
        corporaManager(0), treemodelMetadataStructure(0), treemodelAnnotationStructure(0)
    { }

    CorporaManager *corporaManager;

    QAction *actionAddMetadataStructureSection;
    QAction *actionAddMetadataStructureAttribute;
    QAction *actionRemoveMetadataStructureItem;
    QAction *actionImportMetadataStructure;
    QAction *actionExportMetadataStructure;
    QAction *actionAddAnnotationStructureLevel;
    QAction *actionAddAnnotationStructureAttribute;
    QAction *actionRemoveAnnotationStructureItem;
    QAction *actionImportAnnotationStructure;
    QAction *actionExportAnnotationStructure;
    QAction *actionDuplicateAnnotationLevel;

    QTreeView *treeviewMetadataStructure;
    QTreeView *treeviewAnnotationStructure;
    QPointer<MetadataStructureTreeModel> treemodelMetadataStructure;
    QPointer<AnnotationStructureTreeModel> treemodelAnnotationStructure;

    QToolBar *toolbarMetadataStructure;
    QToolBar *toolbarAnnotationStructure;
};

CorpusStructureEditorWidget::CorpusStructureEditorWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CorpusStructureEditorWidget), d(new CorpusStructureEditorWidgetData)
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
    d->toolbarMetadataStructure = new QToolBar("Metadata Structure", this);
    d->toolbarMetadataStructure->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarAnnotationStructure = new QToolBar("Annotation Structure", this);
    d->toolbarAnnotationStructure->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setupActions();

    // Set up structure editor
    QMainWindow *inner;
    inner = new QMainWindow(this);
    inner->addToolBar(d->toolbarMetadataStructure);
    d->treeviewMetadataStructure = new QTreeView(inner);
    inner->setCentralWidget(d->treeviewMetadataStructure);
    ui->dockMetadataStructure->setWidget(inner);
    d->treeviewMetadataStructure->setAlternatingRowColors(true);

    inner = new QMainWindow(this);
    inner->addToolBar(d->toolbarAnnotationStructure);
    d->treeviewAnnotationStructure = new QTreeView(inner);
    inner->setCentralWidget(d->treeviewAnnotationStructure);
    ui->dockAnnotationStructure->setWidget(inner);
    d->treeviewAnnotationStructure->setAlternatingRowColors(true);
}

CorpusStructureEditorWidget::~CorpusStructureEditorWidget()
{
    delete ui;
    delete d;
}

void CorpusStructureEditorWidget::setupActions()
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

    d->actionImportMetadataStructure = new QAction("Import Metadata Structure", this);
    connect(d->actionImportMetadataStructure, SIGNAL(triggered()), SLOT(importMetadataStructure()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.ImportMetadataStructure", d->actionImportMetadataStructure, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataStructure->addAction(d->actionImportMetadataStructure);

    d->actionExportMetadataStructure = new QAction("Export Metadata Structure", this);
    connect(d->actionExportMetadataStructure, SIGNAL(triggered()), SLOT(exportMetadataStructure()));
    command = ACTION_MANAGER->registerAction("Corpus.Structure.ExportMetadataStructure", d->actionExportMetadataStructure, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataStructure->addAction(d->actionExportMetadataStructure);

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

void CorpusStructureEditorWidget::refreshMetadataStructureTreeView(MetadataStructure *structure)
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

void CorpusStructureEditorWidget::refreshAnnotationStructureTreeView(AnnotationStructure *structure)
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

void CorpusStructureEditorWidget::activeCorpusChanged(const QString &newActiveCorpusID)
{
    Q_UNUSED(newActiveCorpusID);
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) {
        d->treeviewMetadataStructure->setModel(0);
        d->treeviewAnnotationStructure->setModel(0);
        if (d->treemodelMetadataStructure) delete d->treemodelMetadataStructure;
        if (d->treemodelAnnotationStructure) delete d->treemodelAnnotationStructure;
    } else {
        refreshMetadataStructureTreeView(corpus->metadataStructure());
        refreshAnnotationStructureTreeView(corpus->annotationStructure());
    }
}

// ----------------------------------------------- metadata --------------------------------------------------------

void CorpusStructureEditorWidget::addMetadataStructureSection()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;

    if (!d->treemodelMetadataStructure) return;
    QModelIndex index = d->treeviewMetadataStructure->currentIndex();
    if (!index.isValid()) return;

    QObject *item = (QObject *)(index.internalPointer());
    if (!item) return;

    MetadataStructureTreeModelCategory *category = qobject_cast<MetadataStructureTreeModelCategory *>(item);
    MetadataStructureSection *section = qobject_cast<MetadataStructureSection *>(item);
    MetadataStructureAttribute *attribute = qobject_cast<MetadataStructureAttribute *>(item);
    CorpusObject::Type type;
    if (category)
        type = category->type;
    else if (section)
        type = obs->corpus()->metadataStructure()->corpusObjectTypeOfSection(section);
    else if (attribute) {
        section = qobject_cast<MetadataStructureSection *>(attribute->parent());
        if (!section) return;
        type = obs->corpus()->metadataStructure()->corpusObjectTypeOfSection(section);
    }
    obs->corpus()->metadataStructure()->addSection(type, new MetadataStructureSection());
    refreshMetadataStructureTreeView(obs->corpus()->metadataStructure());
}

void CorpusStructureEditorWidget::addMetadataStructureAttribute()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;

    if (!d->treemodelMetadataStructure) return;
    QModelIndex index = d->treeviewMetadataStructure->currentIndex();
    if (!index.isValid()) return;

    QObject *item = (QObject *)(index.internalPointer());
    if (!item) return;

    MetadataStructureSection *section = qobject_cast<MetadataStructureSection *>(item);
    MetadataStructureAttribute *attribute = qobject_cast<MetadataStructureAttribute *>(item);
    if (attribute) section = qobject_cast<MetadataStructureSection *>(attribute->parent());
    if (!section) return;
    CorpusObject::Type type = obs->corpus()->metadataStructure()->corpusObjectTypeOfSection(section);

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

    if (!obs->corpus()->datastoreMetadata()->createMetadataAttribute(type, newAttribute))
        return; // failed to create attribute
    section->addAttribute(newAttribute);
    refreshMetadataStructureTreeView(obs->corpus()->metadataStructure());
}

void CorpusStructureEditorWidget::removeMetadataStructureItem()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;

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
                                 "&Yes", "&No", QString::null, 1, 1) == QMessageBox::No) return;
        CorpusObject::Type type = obs->corpus()->metadataStructure()->corpusObjectTypeOfSection(section);
        foreach (MetadataStructureAttribute *attribute, section->attributes())
            obs->corpus()->datastoreMetadata()->deleteMetadataAttribute(type, attribute->ID());
        obs->corpus()->metadataStructure()->removeSectionByID(type, section->ID());
        refreshMetadataStructureTreeView(obs->corpus()->metadataStructure());
    }
    else if (attribute) {
        section = qobject_cast<MetadataStructureSection *>(attribute->parent());
        if (!section) return;
        if (QMessageBox::warning(this, "Remove metadata attribute?",
                                 QString("Do you want to delete all the metadata stored for Attribute '%1'? "
                                         "This action cannot be reversed!").arg(attribute->name()),
                                 "&Yes", "&No", QString::null, 1, 1) == QMessageBox::No) return;
        CorpusObject::Type type = obs->corpus()->metadataStructure()->corpusObjectTypeOfSection(section);
        obs->corpus()->datastoreMetadata()->deleteMetadataAttribute(type, attribute->ID());
        section->removeAttributeByID(attribute->ID());
        refreshMetadataStructureTreeView(obs->corpus()->metadataStructure());
    }
}

void CorpusStructureEditorWidget::renameMetadataAttribute(Praaline::Core::CorpusObject::Type type,
                                                          const QString &oldID, const QString &newID)
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    if (!obs->corpus()->datastoreMetadata()) return;
    if (obs->corpus()->datastoreMetadata()->renameMetadataAttribute(type, oldID, newID)) {
        obs->corpus()->metadataStructure()->attribute(type, oldID)->setID(newID);
    }
}

void CorpusStructureEditorWidget::importMetadataStructure()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Metadata Structure"), "",
                                                    tr("XML File (*.xml);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    MetadataStructure *structure = XMLSerialiserMetadataStructure::read(filename);
    if (!structure) return;
    obs->corpus()->importMetadataStructure(structure);
    refreshMetadataStructureTreeView(obs->corpus()->metadataStructure());
}

void CorpusStructureEditorWidget::exportMetadataStructure()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Metadata Structure"), "",
                                                    tr("XML File (*.xml);;All Files (*)"), &selectedFilter, options);
    if (filename.isEmpty()) return;
    XMLSerialiserMetadataStructure::write(obs->corpus()->metadataStructure(), filename);
}

// ----------------------------------------------- annotations --------------------------------------------------------

void CorpusStructureEditorWidget::addAnnotationStructureLevel()
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

void CorpusStructureEditorWidget::addAnnotationStructureAttribute()
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

void CorpusStructureEditorWidget::removeAnnotationStructureItem()
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

void CorpusStructureEditorWidget::importAnnotationStructure()
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

void CorpusStructureEditorWidget::exportAnnotationStructure()
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

void CorpusStructureEditorWidget::renameAnnotationLevel(const QString &oldID, const QString &newID)
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    if (!obs->corpus()->datastoreAnnotations()) return;
    if (obs->corpus()->datastoreAnnotations()->renameAnnotationLevel(oldID, newID)) {
        obs->corpus()->annotationStructure()->level(oldID)->setID(newID);
    }
}

void CorpusStructureEditorWidget::renameAnnotationAttribute(const QString &levelID, const QString &oldID, const QString &newID)
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    if (!obs->corpus()->datastoreAnnotations()) return;
    if (obs->corpus()->datastoreAnnotations()->renameAnnotationAttribute(levelID, oldID, newID)) {
        obs->corpus()->annotationStructure()->level(levelID)->attribute(oldID)->setID(newID);
    }
}

void CorpusStructureEditorWidget::cloneAnnotationStructureLevel()
{

}
