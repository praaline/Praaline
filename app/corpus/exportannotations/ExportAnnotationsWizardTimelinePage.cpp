#include <QDebug>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QProgressDialog>

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/interfaces/praat/AnnotationInterfacePraat.h"
using namespace Praaline::Core;

#include "pngui/model/CheckableProxyModel.h"
#include "pngui/model/corpus/AnnotationStructureTreeModel.h"
#include "pngui/model/annotation/AnnotationMultiTierTableModel.h"

#include "pngui/xlsx/xlsxdocument.h"
#include "pngui/xlsx/xlsxformat.h"

#include "ExportAnnotationsWizardTimelinePage.h"
#include "ui_ExportAnnotationsWizardTimelinePage.h"


struct ExportAnnotationsWizardTimelinePageData {
    ExportAnnotationsWizardTimelinePageData() :
        repository(0), modelLevelsAttributes(0), modelTimelineStructure(0)
    {}

    QPointer<CorpusRepository> repository;
    QSharedPointer<AnnotationStructureTreeModel> modelLevelsAttributes;
    QSharedPointer<QStandardItemModel> modelTimelineStructure;
    QString exportPath;
};

ExportAnnotationsWizardTimelinePage::ExportAnnotationsWizardTimelinePage(QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ExportAnnotationsWizardTimelinePage), d(new ExportAnnotationsWizardTimelinePageData)
{
    ui->setupUi(this);
    ui->editFilenameTemplateAll->setText("$CorpusID.xlsx");
    ui->editFilenameTemplateEach->setText("$AnnotationID.xlsx");
    // Edit the timeline structure
    connect(ui->buttonAdd, SIGNAL(clicked(bool)), this, SLOT(timelineStructureAdd()));
    connect(ui->buttonDelete, SIGNAL(clicked(bool)), this, SLOT(timelineStructureDelete()));
    connect(ui->buttonMoveUp, SIGNAL(clicked(bool)), this, SLOT(timelineStructureMoveUp()));
    connect(ui->buttonMoveDown, SIGNAL(clicked(bool)), this, SLOT(timelineStructureMoveDown()));
}

ExportAnnotationsWizardTimelinePage::~ExportAnnotationsWizardTimelinePage()
{
    delete ui;
    delete d;
}

void ExportAnnotationsWizardTimelinePage::setExportPath(const QString &path)
{
    d->exportPath = path;
}

void ExportAnnotationsWizardTimelinePage::setRepository(CorpusRepository *repository)
{
    if (!repository) return;
    if (!repository->annotationStructure()) return;
    d->repository = repository;
    // Get annotation structure tree model for the current repository and display it in the treeview
    d->modelLevelsAttributes = QSharedPointer<AnnotationStructureTreeModel>(
                new AnnotationStructureTreeModel(repository->annotationStructure(), true, true, this));
    ui->treeviewLevelsAttributes->setModel(d->modelLevelsAttributes.data());
    // Whenever the user clicks (selects or deselects) an item (level/attribute) of this treeview, we need to
    // add it to or remove it from the table showing the structure of the exported textgrids
    connect(d->modelLevelsAttributes.data(), SIGNAL(annotationLevelAttributeSelectionChanged(QString,QString,bool)),
            this, SLOT(annotationLevelAttributeSelectionChanged(QString,QString,bool)));
    // Reset textgrid structure
    d->modelTimelineStructure = QSharedPointer<QStandardItemModel>(new QStandardItemModel);
    d->modelTimelineStructure->setColumnCount(4);
    d->modelTimelineStructure->setHorizontalHeaderLabels(QStringList() << tr("Level") << tr("Attribute") << tr("Function") << tr("Parameters"));
    ui->tableviewTimelineStructure->setModel(d->modelTimelineStructure.data());
    ui->tableviewTimelineStructure->verticalHeader()->setDefaultSectionSize(20);
}

void ExportAnnotationsWizardTimelinePage::annotationLevelAttributeSelectionChanged(const QString &levelID, const QString &attributeID, bool selected)
{
    if (!d->modelTimelineStructure) return;
    if (selected) {
        QList<QStandardItem *> columns;
        QStandardItem *itemLevelID = new QStandardItem(levelID);
        itemLevelID->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        QStandardItem *itemAttributeID = new QStandardItem(attributeID);
        itemAttributeID->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        QStandardItem *itemFunction = new QStandardItem();
        QStandardItem *itemParameters = new QStandardItem();
        columns << itemLevelID << itemAttributeID << itemFunction << itemParameters;
        d->modelTimelineStructure->appendRow(columns);
    } else {
        bool found = false;
        int row(0);
        for (row = 0; row < d->modelTimelineStructure->rowCount(); ++row) {
            if ((d->modelTimelineStructure->item(row, 0)->text() == levelID) && (d->modelTimelineStructure->item(row, 1)->text() == attributeID)) {
                found = true;
                break;
            }
        }
        if (found) d->modelTimelineStructure->removeRow(row);
    }
}

void ExportAnnotationsWizardTimelinePage::timelineStructureAdd()
{
    QList<QStandardItem *> items;
    items << new QStandardItem("") << new QStandardItem("") << new QStandardItem("") << new QStandardItem("") ;
    d->modelTimelineStructure->insertRow(d->modelTimelineStructure->rowCount(), items);
    ui->tableviewTimelineStructure->setCurrentIndex(d->modelTimelineStructure->index(d->modelTimelineStructure->rowCount() - 1, 0));
    ui->tableviewTimelineStructure->scrollToBottom();
}

void ExportAnnotationsWizardTimelinePage::timelineStructureDelete()
{
    QModelIndexList selectedIndexes = ui->tableviewTimelineStructure->selectionModel()->selectedIndexes();
    QList<int> rows;
    foreach (QModelIndex index, selectedIndexes)
        rows << index.row();
    for (int i = rows.count() - 1; i >= 0; --i)
        d->modelTimelineStructure->removeRow(rows.at(i));
}

void ExportAnnotationsWizardTimelinePage::timelineStructureMoveUp()
{
    QModelIndexList indexes = ui->tableviewTimelineStructure->selectionModel()->selection().indexes();
    if (indexes.isEmpty()) return;
    int row = indexes.first().row();
    if (row <= 0) return;
    QList<QStandardItem*> items = d->modelTimelineStructure->takeRow(row);
    d->modelTimelineStructure->insertRow(row - 1, items);
    ui->tableviewTimelineStructure->selectRow(row - 1);
}

void ExportAnnotationsWizardTimelinePage::timelineStructureMoveDown()
{
    QModelIndexList indexes = ui->tableviewTimelineStructure->selectionModel()->selection().indexes();
    if (indexes.isEmpty()) return;
    int row = indexes.first().row();
    if (row >= d->modelTimelineStructure->rowCount() - 1) return;
    QList<QStandardItem*> items = d->modelTimelineStructure->takeRow(row);
    d->modelTimelineStructure->insertRow(row + 1, items);
    ui->tableviewTimelineStructure->selectRow(row + 1);
}

QString ExportAnnotationsWizardTimelinePage::interpretFilenameTemplate(const QString &filenameTemplate, const CorpusObjectInfo &info)
{
    QString filename = filenameTemplate;
    filename = filename.replace("$CorpusID", info.attribute("corpusID").toString());
    filename = filename.replace("$CommunicationID", info.attribute("communicationID").toString());
    filename = filename.replace("$RecordingID", info.attribute("recordingID").toString());
    filename = filename.replace("$AnnotationID", info.attribute("annotationID").toString());
    if (filename.startsWith(".")) filename = QString("export") + filename;
    return filename;
}

void ExportAnnotationsWizardTimelinePage::doExport(const QList<CorpusObjectInfo> &annotations)
{
    if (annotations.isEmpty()) return;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    AnnotationMultiTierTableModel *model(0);
    QList<QPair<QString, QString> > selectedLevelsAttributes;
    // Tier correspondances
    for (int i = 0; i < d->modelTimelineStructure->rowCount(); ++i) {
        QString levelID = d->modelTimelineStructure->item(i, 0)->text();
        QString attributeID = d->modelTimelineStructure->item(i, 1)->text();
        QString function = d->modelTimelineStructure->item(i, 2)->text();
        QString parameters = d->modelTimelineStructure->item(i, 3)->text();
        QString attr = attributeID;
        if (function == "context") attr = "_context";
        if (function == "group") attr = attr.prepend(QString("_group:%1").arg(parameters));
        selectedLevelsAttributes << QPair<QString, QString>(levelID, attr);
    }
    // Format for the header
    QXlsx::Format format_header;
    format_header.setFontBold(true);
    format_header.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    // Create an Excel document
    QXlsx::Document xlsxAllAnnotations;
    int rowOffset(0);
    // Create a process dialog while writing data
    int count = annotations.count();
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, count);
    progressDialog.setWindowTitle(tr("Exporting..."));
    // Export
    int iterAnnotation(1);
    foreach (CorpusObjectInfo info, annotations) {
        QString annotationID = info.attribute("annotationID").toString();
        // Update progress dialog
        if (progressDialog.wasCanceled()) break;
        progressDialog.setValue(iterAnnotation);
        progressDialog.setLabelText(tr("Annotation %1 of %2 (%3)...").arg(iterAnnotation).arg(annotations.count())
                                    .arg(annotationID));
        qApp->processEvents();
        // Create timeline model
        tiersAll = d->repository->annotations()->getTiersAllSpeakers(annotationID);
        model = new AnnotationMultiTierTableModel(tiersAll, selectedLevelsAttributes.first().first, selectedLevelsAttributes, Qt::Vertical);
        // Create an Excel document
        QXlsx::Document xlsxAnnotation;
        // Save at the appropriate place
        // column headers
        xlsxAllAnnotations.write(1, 1, "AnnotationID", format_header);
        for (int col = 0; col < model->columnCount(); ++col) {
            xlsxAllAnnotations.write(1, col + 2, model->headerData(col, Qt::Horizontal, Qt::DisplayRole), format_header);
            xlsxAnnotation.write(1, col + 1, model->headerData(col, Qt::Horizontal, Qt::DisplayRole), format_header);
        }
        // data
        for (int row = 0; row < model->rowCount(); ++ row) {
            if (ui->optionExportAll->isChecked()) {
                xlsxAllAnnotations.write(rowOffset + row + 2, 1, annotationID);
            }
            for (int col = 0; col < model->columnCount(); ++col) {
                if (ui->optionExportAll->isChecked()) {
                    xlsxAllAnnotations.write(rowOffset + row + 2, col + 2, model->data(model->index(row, col), Qt::DisplayRole));
                }
                if (ui->optionExportEach->isChecked()) {
                    xlsxAnnotation.write(row + 2, col + 1, model->data(model->index(row, col), Qt::DisplayRole));
                }
            }
        }
        rowOffset = rowOffset + model->rowCount();
        // If saving one file per annotation
        if (ui->optionExportEach->isChecked()) {
            QString filenameEach = interpretFilenameTemplate(ui->editFilenameTemplateEach->text(), info);
            xlsxAnnotation.saveAs(d->exportPath + "/" + filenameEach);
        }
        delete model;
        iterAnnotation++;
    }
    // If saving one big file with all annotations
    if (ui->optionExportAll->isChecked()) {
        progressDialog.setLabelText(tr("Exporting global file..."));
        qApp->processEvents();
        QString filenameAll = interpretFilenameTemplate(ui->editFilenameTemplateAll->text(), annotations.first());
        xlsxAllAnnotations.saveAs(d->exportPath + "/" + filenameAll);
    }
}
