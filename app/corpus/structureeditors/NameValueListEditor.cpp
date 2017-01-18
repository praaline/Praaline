#include <QDebug>
#include <QString>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QAction>
#include <QToolBar>
#include <QTableView>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>

#include "pncore/structure/NameValueList.h"
#include "pncore/datastore/NameValueListDatastore.h"
using namespace Praaline::Core;

#include "pngui/model/NameValueListTableModel.h"
#include "NameValueListEditor.h"
#include "AddAttributeDialog.h"

struct NameValueListEditorData {
    NameValueListEditorData() :
        comboboxLists(0), toolbarLists(0), toolbarCurrentList(0), tableviewCurrentList(0),
        datastore(0), currentList(0), model(0)
    {}

    QComboBox *comboboxLists;
    QLineEdit *editListName;
    QPlainTextEdit *textListDescription;
    QLabel *labelDataType;

    QToolBar *toolbarLists;
    QToolBar *toolbarCurrentList;
    QTableView *tableviewCurrentList;

    QAction *actionNewList;
    QAction *actionSaveList;
    QAction *actionDeleteList;
    QAction *actionImportLists;
    QAction *actionExportLists;
    QAction *actionAddItem;
    QAction *actionRemoveItem;
    QAction *actionMoveItemUp;
    QAction *actionMoveItemDown;

    NameValueListDatastore *datastore;
    NameValueList *currentList;
    NameValueListTableModel *model;
};

NameValueListEditor::NameValueListEditor(QWidget *parent) :
    QWidget(parent), d(new NameValueListEditorData)
{
    // List selection combobox
    d->comboboxLists = new QComboBox(this);
    connect(d->comboboxLists, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedListChanged(int)));
    d->comboboxLists->setEditable(false);
    // Editing basic list definition
    d->editListName = new QLineEdit(this);
    d->textListDescription = new QPlainTextEdit(this);
    d->labelDataType = new QLabel(this);
    // Toolbars
    d->toolbarLists = new QToolBar("Lists", this);
    d->toolbarCurrentList = new QToolBar("Current List", this);
    setupActions();
    // Table view for editing the list
    d->tableviewCurrentList = new QTableView(this);
    // Layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(d->toolbarLists);    
    QFormLayout *layoutEditing = new QFormLayout(this);
    layoutEditing->addWidget(new QLabel(tr("Select name-value list: "), this));
    layoutEditing->addWidget(d->comboboxLists);
    layoutEditing->addWidget(new QLabel(tr("Name: "), this));
    layoutEditing->addWidget(d->editListName);
    layoutEditing->addWidget(new QLabel(tr("Description: "), this));
    layoutEditing->addWidget(d->textListDescription);
    layoutEditing->addWidget(new QLabel(tr("Data type: "), this));
    layoutEditing->addWidget(d->labelDataType);
    layout->addLayout(layoutEditing, 1);
    layout->addWidget(d->toolbarCurrentList);
    layout->addWidget(d->tableviewCurrentList, 4);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(3);
    this->setLayout(layout);
}

NameValueListEditor::~NameValueListEditor()
{
    delete d;
}

void NameValueListEditor::setupActions()
{
    if ((!d->toolbarLists) || (!d->toolbarCurrentList)) return;

    d->actionNewList = new QAction(QIcon(":icons/actions/list_add.png"), "New", this);
    connect(d->actionNewList, SIGNAL(triggered()), SLOT(newList()));
    d->toolbarLists->addAction(d->actionNewList);

    d->actionSaveList = new QAction(QIcon(":icons/actions/action_save.png"), "Save", this);
    connect(d->actionSaveList, SIGNAL(triggered()), SLOT(saveList()));
    d->toolbarLists->addAction(d->actionSaveList);

    d->actionDeleteList = new QAction(QIcon(":icons/actions/list_remove.png"), "Delete", this);
    connect(d->actionDeleteList, SIGNAL(triggered()), SLOT(deleteList()));
    d->toolbarLists->addAction(d->actionDeleteList);

    d->actionImportLists = new QAction(QIcon(":icons/actions/action_import.png"), "Import", this);
    connect(d->actionImportLists, SIGNAL(triggered()), SLOT(importLists()));
    d->toolbarLists->addAction(d->actionImportLists);

    d->actionExportLists = new QAction(QIcon(":icons/actions/action_export.png"), "Export", this);
    connect(d->actionExportLists, SIGNAL(triggered()), SLOT(exportLists()));
    d->toolbarLists->addAction(d->actionExportLists);

    d->actionAddItem = new QAction(QIcon(":icons/actions/list_add.png"), "Add", this);
    connect(d->actionAddItem, SIGNAL(triggered()), SLOT(addItem()));
    d->toolbarCurrentList->addAction(d->actionAddItem);

    d->actionRemoveItem = new QAction(QIcon(":icons/actions/list_remove.png"), "Remove", this);
    connect(d->actionRemoveItem, SIGNAL(triggered()), SLOT(removeItem()));
    d->toolbarCurrentList->addAction(d->actionRemoveItem);

    d->actionMoveItemUp = new QAction(QIcon(":icons/actions/move_up.png"), "Move Up", this);
    connect(d->actionMoveItemUp, SIGNAL(triggered()), SLOT(moveItemUp()));
    d->toolbarCurrentList->addAction(d->actionMoveItemUp);

    d->actionMoveItemDown = new QAction(QIcon(":icons/actions/move_down.png"), "Move Down", this);
    connect(d->actionMoveItemDown, SIGNAL(triggered()), SLOT(moveItemDown()));
    d->toolbarCurrentList->addAction(d->actionMoveItemDown);
}

void NameValueListEditor::rebind(NameValueListDatastore *datastore)
{
    d->datastore = datastore;
    if (!datastore) {
        d->tableviewCurrentList->setModel(Q_NULLPTR);
        if (d->model) { delete d->model; d->model = 0; }
        d->comboboxLists->clear();
    }
    else {
        d->comboboxLists->clear();
        QStringList listIDs = datastore->getAllNameValueListIDs();
        d->comboboxLists->addItems(listIDs);
    }
}

void NameValueListEditor::rebindList(Praaline::Core::NameValueList *list)
{
    if (d->currentList == list) return;
    if (d->model) {
        d->tableviewCurrentList->setModel(0);
        delete d->model; d->model = 0;
    }
    if (list){
        NameValueListTableModel *model = new NameValueListTableModel(list, this);
        d->tableviewCurrentList->setModel(model);
        d->model = model;
        d->editListName->setText(list->name());
        d->textListDescription->setPlainText(list->description());
        QString datatypeDesc = list->datatypeString();
        if (list->datatypePrecision() > 0) datatypeDesc.append(QString(" (%1)").arg(list->datatypePrecision()));
        d->labelDataType->setText(datatypeDesc);
    } else {
        d->tableviewCurrentList->setModel(0);
        d->model = 0;
        d->editListName->clear();
        d->textListDescription->clear();
        d->labelDataType->clear();
    }
    d->currentList = list;
}

void NameValueListEditor::selectedListChanged(int index)
{
    Q_UNUSED(index)
    if (!d->datastore) return;
    rebindList(d->datastore->getNameValueList(d->comboboxLists->currentText()));
}

void NameValueListEditor::newList()
{
    if (!d->datastore) return;
    // Ask for the new list ID and add it
    AddAttributeDialog *dialog = new AddAttributeDialog(this);
    dialog->exec();
    if (dialog->result() == QDialog::Rejected) return;
    QString listID = dialog->attributeID();
    if (listID.isEmpty()) return;
    DataType dt = DataType(dialog->datatype());
    if (dialog->datalength() > 0) dt = DataType(dt.base(), dialog->datalength());
    NameValueList *newList = new NameValueList(this);
    newList->setID(listID);
    newList->setDatatype(dt);
    // Update database
    if (!d->datastore->createNameValueList(newList)) {
        QMessageBox::warning(this, tr("Error deleting"),
                             tr("The name-value list was not created due to a database error."));
        return;
    }
    // Update combobox
    d->comboboxLists->clear();
    QStringList listIDs = d->datastore->getAllNameValueListIDs();
    d->comboboxLists->addItems(listIDs);
    d->comboboxLists->setCurrentText(listID);
}

void NameValueListEditor::saveList()
{
    if (!d->datastore) return;
    if (!d->currentList) return;
    d->currentList->setName(d->editListName->text());
    d->currentList->setDescription(d->textListDescription->toPlainText());
    d->datastore->updateNameValueList(d->currentList);
}

void NameValueListEditor::deleteList()
{
    if (!d->datastore) return;
    if (!d->currentList) return;
    if (QMessageBox::warning(this, tr("Delete name-value list?"),
                             QString(tr("Do you want to delete the currently selected Name-Value List ('%1')? "
                                        "This action cannot be reversed!")).arg(d->currentList->name()),
                             tr("&Yes"), tr("&No"), QString::null, 1, 1) == QMessageBox::No) return;
    QString listID = d->currentList->ID();
    if (d->datastore->deleteNameValueList(listID)) {
        d->comboboxLists->removeItem(d->comboboxLists->findText(listID));
    } else {
        QMessageBox::warning(this, tr("Error deleting"),
                             tr("The name-value list was not deleted due to a database error."));
    }
}

void NameValueListEditor::importLists()
{
    if (!d->datastore) return;
}

void NameValueListEditor::exportLists()
{
    if (!d->datastore) return;
}

void NameValueListEditor::addItem()
{
    if (!d->datastore) return;
}

void NameValueListEditor::removeItem()
{
    if (!d->datastore) return;
}

void NameValueListEditor::moveItemUp()
{
    if (!d->datastore) return;
}

void NameValueListEditor::moveItemDown()
{
    if (!d->datastore) return;
}

