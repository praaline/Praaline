#include <QDebug>
#include <QStandardItemModel>
#include "CorpusExplorerOptionsDialog.h"
#include "ui_CorpusExplorerOptionsDialog.h"

struct CorpusExplorerOptionsDialogData {
    CorpusExplorerOptionsDialogData() :
        structure(0), modelAttributesDisplayCom(0), modelAttributesGroupCom(0), modelAttributesDisplaySpk(0), modelAttributesGroupSpk(0)
    {}

    QPointer<MetadataStructure> structure;
    QPointer<QStandardItemModel> modelAttributesDisplayCom;
    QPointer<QStandardItemModel> modelAttributesGroupCom;
    QPointer<QStandardItemModel> modelAttributesDisplaySpk;
    QPointer<QStandardItemModel> modelAttributesGroupSpk;
};


CorpusExplorerOptionsDialog::CorpusExplorerOptionsDialog(MetadataStructure *mstructure, QWidget *parent) :
    QDialog(parent), ui(new Ui::CorpusExplorerOptionsDialog), d(new CorpusExplorerOptionsDialogData)
{
    ui->setupUi(this);

    connect(ui->commandComGroup, SIGNAL(clicked(bool)), this, SLOT(commandGroup()));
    connect(ui->commandComUngroup, SIGNAL(clicked(bool)), this, SLOT(commandUngroup()));
    connect(ui->commandComUp, SIGNAL(clicked(bool)), this, SLOT(commandGroupMoveUp()));
    connect(ui->commandComDown, SIGNAL(clicked(bool)), this, SLOT(commandGroupMoveDown()));
    connect(ui->commandSpkGroup, SIGNAL(clicked(bool)), this, SLOT(commandGroup()));
    connect(ui->commandSpkUngroup, SIGNAL(clicked(bool)), this, SLOT(commandUngroup()));
    connect(ui->commandSpkUp, SIGNAL(clicked(bool)), this, SLOT(commandGroupMoveUp()));
    connect(ui->commandSpkDown, SIGNAL(clicked(bool)), this, SLOT(commandGroupMoveDown()));

    // Initialise lists
    if (!mstructure) return;
    d->structure = mstructure;
    d->modelAttributesDisplayCom = createAttributeModel(CorpusObject::Type_Communication, true);
    d->modelAttributesGroupCom = new QStandardItemModel(this);
    d->modelAttributesDisplaySpk = createAttributeModel(CorpusObject::Type_Speaker, true);
    d->modelAttributesGroupSpk = new QStandardItemModel(this);
    ui->treeViewComDisplay->setModel(d->modelAttributesDisplayCom);
    ui->treeViewComDisplay->setHeaderHidden(true);
    ui->treeViewComGroup->setModel(d->modelAttributesGroupCom);
    ui->treeViewComGroup->setHeaderHidden(true);
    ui->treeViewSpkDisplay->setModel(d->modelAttributesDisplaySpk);
    ui->treeViewSpkDisplay->setHeaderHidden(true);
    ui->treeViewSpkGroup->setModel(d->modelAttributesGroupSpk);
    ui->treeViewSpkGroup->setHeaderHidden(true);
}

// private
QPointer<QStandardItemModel> CorpusExplorerOptionsDialog::createAttributeModel(CorpusObject::Type type, bool checkable)
{
    QPointer<QStandardItemModel> model = new QStandardItemModel(this);
    model->setColumnCount(1);
    int i = 0;
    foreach (MetadataStructureAttribute *attribute, d->structure->attributes(type)) {
        if (!attribute) continue;
        QStandardItem *item = new QStandardItem(attribute->name());
        item->setData(attribute->ID());
        item->setCheckable(checkable);
        model->setItem(i, item);
        ++i;
    }
    return model;
}

CorpusExplorerOptionsDialog::~CorpusExplorerOptionsDialog()
{
    delete ui;
    delete d;
}

QStringList CorpusExplorerOptionsDialog::groupAttributesForCommunications() const
{
    QStringList ret;
    if (!d->modelAttributesGroupCom) return ret;
    for (int i = 0; i < d->modelAttributesGroupCom->rowCount(); ++i) {
        QStandardItem *item = d->modelAttributesGroupCom->item(i, 0);
        QString attributeID = item->data().toString();
        ret << attributeID;
    }
    return ret;
}

QStringList CorpusExplorerOptionsDialog::groupAttributesForSpeakers() const
{
    QStringList ret;
    if (!d->modelAttributesGroupSpk) return ret;
    for (int i = 0; i < d->modelAttributesGroupSpk->rowCount(); ++i) {
        QStandardItem *item = d->modelAttributesGroupSpk->item(i, 0);
        QString attributeID = item->data().toString();
        ret << attributeID;
    }
    return ret;
}

QStringList CorpusExplorerOptionsDialog::displayAttributesForCommunications() const
{
    QStringList ret;
    if (!d->modelAttributesDisplayCom) return ret;
    for (int i = 0; i < d->modelAttributesDisplayCom->rowCount(); ++i) {
        QStandardItem *item = d->modelAttributesDisplayCom->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            QString attributeID = item->data().toString();
            ret << attributeID;
        }
    }
    return ret;
}

QStringList CorpusExplorerOptionsDialog::displayAttributesForSpeakers() const
{
    QStringList ret;
    if (!d->modelAttributesDisplaySpk) return ret;
    for (int i = 0; i < d->modelAttributesDisplaySpk->rowCount(); ++i) {
        QStandardItem *item = d->modelAttributesDisplaySpk->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            QString attributeID = item->data().toString();
            ret << attributeID;
        }
    }
    return ret;
}

void CorpusExplorerOptionsDialog::commandGroup()
{
    QObject *s = sender();
    QStandardItemModel *source(0);
    QStandardItemModel *destination(0);
    QModelIndexList selectedIndices;
    if (ui->commandComGroup == qobject_cast<QPushButton *>(s)) {
        source = d->modelAttributesDisplayCom;
        destination = d->modelAttributesGroupCom;
        selectedIndices = ui->treeViewComDisplay->selectionModel()->selectedIndexes();
    }
    else if (ui->commandSpkGroup == qobject_cast<QPushButton *>(s)) {
        source = d->modelAttributesDisplaySpk;
        destination = d->modelAttributesGroupSpk;
        selectedIndices = ui->treeViewSpkDisplay->selectionModel()->selectedIndexes();
    }
    if (!source || !destination) return;
    foreach (QModelIndex selectedIndex, selectedIndices) {
        QStandardItem *selected = source->itemFromIndex(selectedIndex);
        if (!selected) continue;
        QString attributeID = selected->data().toString();
        QString attributeName = selected->text();
        bool add = true;
        for (int i = 0; i < destination->rowCount(); ++i) {
            if (destination->item(i, 0)->data().toString() == attributeID) add = false;
        }
        if (add) {
            QStandardItem *added = new QStandardItem(attributeName);
            added->setData(attributeID);
            destination->appendRow(added);
        }
    }
}

void CorpusExplorerOptionsDialog::commandUngroup()
{

}

void CorpusExplorerOptionsDialog::commandGroupMoveUp()
{

}

void CorpusExplorerOptionsDialog::commandGroupMoveDown()
{

}
