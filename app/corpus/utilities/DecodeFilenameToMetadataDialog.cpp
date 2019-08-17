#include <QString>
#include <QPointer>
#include <QStandardItemModel>
#include <QFileInfo>

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/datastore/MetadataDatastore.h"
using namespace Praaline::Core;

#include "DecodeFilenameToMetadataDialog.h"
#include "ui_DecodeFilenameToMetadataDialog.h"

struct DecodeFilenameOp {
    DecodeFilenameOp() : operation(0), from(0), to(0) {}
    int operation;
    QString delimiter;
    int from;
    int to;
    QString description;
    QString metadataAttributeName;
    QString metadataAttributeID;
};

struct DecodeFilenameToMetadataDialogData {
    QPointer<Corpus> corpus;
    QList<DecodeFilenameOp> operations;
    QPointer<QStandardItemModel> modelCorrespondances;
};

DecodeFilenameToMetadataDialog::DecodeFilenameToMetadataDialog(Corpus *corpus, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DecodeFilenameToMetadataDialog),
    d(new DecodeFilenameToMetadataDialogData())
{
    ui->setupUi(this);
    d->corpus = corpus;
    connect(ui->commandClose, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->commandAdd, SIGNAL(clicked(bool)), this, SLOT(addCorrespondance()));
    connect(ui->commandRemove, SIGNAL(clicked(bool)), this, SLOT(removeCorrespondance()));
    connect(ui->commandUpdateMetadata, SIGNAL(clicked(bool)), this, SLOT(updateMetadata()));
    connect(ui->comboBoxOperation, SIGNAL(currentIndexChanged(int)), this, SLOT(operationChanged(int)));
    ui->spinBoxFrom->setMinimum(1);
    ui->spinBoxTo->setMinimum(1);
    // Load metadata attributes in current corpus
    if (corpus->repository()) {
        foreach (MetadataStructureAttribute *attr, corpus->repository()->metadataStructure()->attributes(CorpusObject::Type_Communication)){
            ui->comboBoxMetadataAttribute->addItem(attr->name(), attr->ID());
        }
    }
    // Model corespondances
    d->modelCorrespondances = new QStandardItemModel(0, 2, this);
    d->modelCorrespondances->setHorizontalHeaderLabels(QStringList() << "Description" << "Metadata Attribute");
    ui->treeViewCorrespondances->setModel(d->modelCorrespondances);
}

DecodeFilenameToMetadataDialog::~DecodeFilenameToMetadataDialog()
{
    delete ui;
    delete d;
}

void DecodeFilenameToMetadataDialog::operationChanged(int operation)
{
    // 0: delimited section
    // 1: characters from left
    // 2: characters from right
    // 3: characters between
    if (operation == 0) {
        ui->labelDelimiter->setVisible(true);   ui->editDelimiter->setVisible(true);
        ui->labelFrom->setVisible(true);        ui->labelFrom->setText(tr("From:"));    ui->spinBoxFrom->setVisible(true);
        ui->labelTo->setVisible(true);          ui->labelTo->setText(tr("To:"));        ui->spinBoxTo->setVisible(true);
    }
    else if ((operation == 1) || (operation == 2)) {
        ui->labelDelimiter->setVisible(false);  ui->editDelimiter->setVisible(false);
        ui->labelFrom->setVisible(true);        ui->labelFrom->setText(tr("Number:"));  ui->spinBoxFrom->setVisible(true);
        ui->labelTo->setVisible(false);                                                 ui->spinBoxTo->setVisible(false);
    }
    else if (operation == 3) {
        ui->labelDelimiter->setVisible(false);  ui->editDelimiter->setVisible(false);
        ui->labelFrom->setVisible(true);        ui->labelFrom->setText(tr("From:"));    ui->spinBoxFrom->setVisible(true);
        ui->labelTo->setVisible(true);          ui->labelTo->setText(tr("To:"));        ui->spinBoxTo->setVisible(true);
    }
}

void DecodeFilenameToMetadataDialog::addCorrespondance()
{
    DecodeFilenameOp op;
    op.operation = ui->comboBoxOperation->currentIndex();
    op.delimiter = ui->editDelimiter->text();
    op.from = ui->spinBoxFrom->value();
    op.to = ui->spinBoxTo->value();
    op.metadataAttributeID = ui->comboBoxMetadataAttribute->currentData().toString();
    op.metadataAttributeName = ui->comboBoxMetadataAttribute->currentText();
    if (op.operation == 0) {
        if (op.delimiter.isEmpty()) return;
        if (op.from > op.to) return;
        op.description = tr("Part(s) %1 to %2 of the filename, delimited by %3").arg(op.from).arg(op.to).arg(op.delimiter);
    }
    else if (op.operation == 1) {
        op.description = tr("Leftmost %1 characters of the filename").arg(op.from);
    }
    else if (op.operation == 2) {
        op.description = tr("Rightmost %1 characters of the filename").arg(op.from);
    }
    else if (op.operation == 3) {
        if (op.from > op.to) return;
        op.description = tr("Characters between position %1 and %2 of the filename").arg(op.from).arg(op.to);
    }
    else return;
    d->operations << op;
    QList<QStandardItem *> columns;
    QStandardItem *itemDescription = new QStandardItem(op.description);
    itemDescription->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    QStandardItem *itemMetadataAttr = new QStandardItem(op.metadataAttributeName);
    itemMetadataAttr->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    columns << itemDescription << itemMetadataAttr;
    d->modelCorrespondances->appendRow(columns);
}

void DecodeFilenameToMetadataDialog::removeCorrespondance()
{
    if (!ui->treeViewCorrespondances->selectionModel()) return;
    QModelIndexList selectedIndexes = ui->treeViewCorrespondances->selectionModel()->selectedIndexes();
    QList<int> rows;
    foreach (QModelIndex index, selectedIndexes)
        rows << index.row();
    for (int i = rows.count() - 1; i >= 0; --i) {
        d->modelCorrespondances->removeRow(rows.at(i));
        d->operations.removeAt(rows.at(i));
    }
}

void DecodeFilenameToMetadataDialog::updateMetadata()
{
    // Checks
    if (!d->corpus) return;
    if (d->operations.isEmpty()) return;
    if (d->corpus->communicationsCount() == 0) return;

    bool overwrite = ui->radioButtonOverwrite->isChecked();
    ui->progressBar->setMaximum(d->corpus->communicationsCount());
    ui->progressBar->setValue(0);
    int i = 0;
    foreach (CorpusCommunication *com, d->corpus->communications()) {
        if (!com) continue;
        if (com->recordingsCount() == 0) continue;
        CorpusRecording *rec = com->recordings().first();
        if (!rec) continue;
        QString filename = QFileInfo(rec->filename()).baseName();
        foreach (DecodeFilenameOp op, d->operations) {
            QString data;
            if (op.operation == 0)
                data = filename.section(op.delimiter, op.from - 1, op.to - 1);
            else if (op.operation == 1)
                data = filename.left(op.from);
            else if (op.operation == 2)
                data = filename.right(op.to);
            else if (op.operation == 3)
                data = filename.mid(op.from - 1, op.to - op.from + 1);
            if (overwrite)
                com->setProperty(op.metadataAttributeID, data);
            else {
                QString check = com->property(op.metadataAttributeID).toString();
                if (check.isEmpty() || check == "0" || check == "0.0")
                    com->setProperty(op.metadataAttributeID, data);
            }
        }
        ++i;
        ui->progressBar->setValue(i);
        if (i % 10 == 0) QApplication::processEvents();
    }
}
