#include <QString>
#include <QToolBar>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QDebug>

#include "MetadataStructureEditor.h"
#include "ui_MetadataStructureEditor.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/serialisers/xml/XMLSerialiserMetadataStructure.h"
#include "pncore/serialisers/xml/XMLSerialiserAnnotationStructure.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/MetadataStructureTreeModel.h"
#include "pngui/model/corpus/AnnotationStructureTreeModel.h"
#include "pngui/observers/CorpusObserver.h"
#include "NameValueListEditor.h"
#include "CorporaManager.h"
#include "AddAttributeDialog.h"
#include "AddLevelDialog.h"

struct MetadataStructureEditorData {
    MetadataStructureEditorData() : corporaManager(0), treemodelMetadataStructure(0)
    { }

    CorporaManager *corporaManager;

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
    d->toolbarMetadataStructure->setIconSize(QSize(24, 24));
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
    ui->splitter->setSizes(QList<int>() << 100 << 100);
    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 1);
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

void MetadataStructureEditor::activeCorpusChanged(const QString &newActiveCorpusID)
{
    Q_UNUSED(newActiveCorpusID);
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) {
        d->treeviewMetadataStructure->setModel(0);
        if (d->treemodelMetadataStructure) delete d->treemodelMetadataStructure;
        d->editorNVList->rebind(0);
    } else {
        refreshMetadataStructureTreeView(corpus->metadataStructure());
        d->editorNVList->rebind(corpus->datastoreMetadata());
    }
}

// ----------------------------------------------- metadata --------------------------------------------------------

void MetadataStructureEditor::addMetadataStructureSection()
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

void MetadataStructureEditor::addMetadataStructureAttribute()
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

void MetadataStructureEditor::removeMetadataStructureItem()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    if (!d->treemodelMetadataStructure) return;
    if (!obs->corpus()->metadataStructure()) return;

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

void MetadataStructureEditor::renameMetadataAttribute(Praaline::Core::CorpusObject::Type type,
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

void MetadataStructureEditor::importMetadataStructure()
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

void MetadataStructureEditor::exportMetadataStructure()
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
