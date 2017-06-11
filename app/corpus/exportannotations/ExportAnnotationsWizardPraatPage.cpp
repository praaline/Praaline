#include <QMessageBox>
#include <QStandardItemModel>

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/interfaces/praat/AnnotationInterfacePraat.h"
using namespace Praaline::Core;

#include "pngui/model/CheckableProxyModel.h"
#include "pngui/model/corpus/AnnotationStructureTreeModel.h"

#include "ExportAnnotationsWizardPraatPage.h"
#include "ui_ExportAnnotationsWizardPraatPage.h"


struct ExportAnnotationsWizardPraatPageData {
    ExportAnnotationsWizardPraatPageData () :
        repository(0), modelLevelsAttributes(0), modelTextgridStructure(0)
    {}

    QPointer<CorpusRepository> repository;
    QSharedPointer<AnnotationStructureTreeModel> modelLevelsAttributes;
    QSharedPointer<QStandardItemModel> modelTextgridStructure;
    QString exportPath;
};

ExportAnnotationsWizardPraatPage::ExportAnnotationsWizardPraatPage(QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ExportAnnotationsWizardPraatPage),
    d(new ExportAnnotationsWizardPraatPageData())
{
    ui->setupUi(this);
    ui->editFilenameTemplate->setText("$AnnotationID_$SpeakerID.TextGrid");
    // Move items in the textgrid structure up or down
    connect(ui->buttonMoveUp, SIGNAL(clicked(bool)), this, SLOT(textgridStructureMoveUp()));
    connect(ui->buttonMoveDown, SIGNAL(clicked(bool)), this, SLOT(textgridStructureMoveDown()));
    // Propose different filename templates based on whether different speakers will be included in the same TextGrid or not.
    connect(ui->optionAllSpeakers, SIGNAL(clicked(bool)), this, SLOT(changedSpeakerPolicy()));
    connect(ui->optionOneSpeakerPerFile, SIGNAL(clicked(bool)), this, SLOT(changedSpeakerPolicy()));
}

ExportAnnotationsWizardPraatPage::~ExportAnnotationsWizardPraatPage()
{
    delete ui;
    delete d;
}

void ExportAnnotationsWizardPraatPage::setExportPath(const QString &path)
{
    d->exportPath = path;
}

void ExportAnnotationsWizardPraatPage::setRepository(CorpusRepository *repository)
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
    d->modelTextgridStructure = QSharedPointer<QStandardItemModel>(new QStandardItemModel);
    d->modelTextgridStructure->setColumnCount(3);
    d->modelTextgridStructure->setHorizontalHeaderLabels(QStringList() << tr("Level") << tr("Attribute") << tr("TextGrid Tier"));
    ui->tableviewTextgridStructure->setModel(d->modelTextgridStructure.data());
    ui->tableviewTextgridStructure->verticalHeader()->setDefaultSectionSize(20);
}

void ExportAnnotationsWizardPraatPage::annotationLevelAttributeSelectionChanged(
        const QString &levelID, const QString &attributeID, bool selected)
{
    if (!d->modelTextgridStructure) return;
    if (selected) {
        QList<QStandardItem *> columns;
        QStandardItem *itemLevelID = new QStandardItem(levelID);
        itemLevelID->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        QStandardItem *itemAttributeID = new QStandardItem(attributeID);
        itemAttributeID->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        QStandardItem *itemTierName = new QStandardItem((attributeID.isEmpty()) ? levelID : attributeID);
        columns << itemLevelID << itemAttributeID << itemTierName;
        d->modelTextgridStructure->appendRow(columns);
    } else {
        bool found = false;
        int row(0);
        for (row = 0; row < d->modelTextgridStructure->rowCount(); ++row) {
            if ((d->modelTextgridStructure->item(row, 0)->text() == levelID) && (d->modelTextgridStructure->item(row, 1)->text() == attributeID)) {
                found = true;
                break;
            }
        }
        if (found) d->modelTextgridStructure->removeRow(row);
    }
}

void ExportAnnotationsWizardPraatPage::doExport(const QList<CorpusObjectInfo> &annotations)
{
    QScopedPointer<AnnotationInterfacePraat> interface(new AnnotationInterfacePraat(this));
    // Export path
    interface->setExportPath(d->exportPath);
    // Speaker policy
    if      (ui->optionOneSpeakerPerFile->isChecked())
        interface->setSpeakerPolicy(AnnotationInterfacePraat::SpeakerPolicy_SinglePerFile);
    else if (ui->optionAllSpeakers->isChecked())
        interface->setSpeakerPolicy(AnnotationInterfacePraat::SpeakerPolicy_PrefixTierNames);
    // Tier correspondances
    for (int i = 0; i < d->modelTextgridStructure->rowCount(); ++i) {
        QString levelID = d->modelTextgridStructure->item(i, 0)->text();
        QString attributeID = d->modelTextgridStructure->item(i, 1)->text();
        QString tierName = d->modelTextgridStructure->item(i, 2)->text();
        interface->addCorrepondance(levelID, attributeID, tierName);
    }
    // Export annotations
    foreach (CorpusObjectInfo info, annotations) {
        QString annotationID = info.attribute("annotationID").toString();
        QString filename = ui->editFilenameTemplate->text();
        filename = filename.replace("$CorpusID", info.attribute("corpusID").toString());
        filename = filename.replace("$CommunicationID", info.attribute("communicationID").toString());
        filename = filename.replace("$RecordingID", info.attribute("recordingID").toString());
        filename = filename.replace("$AnnotationID", annotationID);
        interface->setExportFilenameTemplate(filename);
        interface->exportAnnotation(d->repository, annotationID);
    }
}

void ExportAnnotationsWizardPraatPage::textgridStructureMoveUp()
{
    QModelIndexList indexes = ui->tableviewTextgridStructure->selectionModel()->selection().indexes();
    if (indexes.isEmpty()) return;
    int row = indexes.first().row();
    if (row <= 0) return;
    QList<QStandardItem*> items = d->modelTextgridStructure->takeRow(row);
    d->modelTextgridStructure->insertRow(row - 1, items);
    ui->tableviewTextgridStructure->selectRow(row - 1);
}

void ExportAnnotationsWizardPraatPage::textgridStructureMoveDown()
{
    QModelIndexList indexes = ui->tableviewTextgridStructure->selectionModel()->selection().indexes();
    if (indexes.isEmpty()) return;
    int row = indexes.first().row();
    if (row >= d->modelTextgridStructure->rowCount() - 1) return;
    QList<QStandardItem*> items = d->modelTextgridStructure->takeRow(row);
    d->modelTextgridStructure->insertRow(row + 1, items);
    ui->tableviewTextgridStructure->selectRow(row + 1);
}

void ExportAnnotationsWizardPraatPage::changedSpeakerPolicy()
{
    if (ui->optionAllSpeakers->isChecked()) {
        if (ui->editFilenameTemplate->text() == "$AnnotationID_$SpeakerID.TextGrid")
            ui->editFilenameTemplate->setText("$AnnotationID.TextGrid");
    }
    else if (ui->optionOneSpeakerPerFile->isChecked()) {
        if (ui->editFilenameTemplate->text() == "$AnnotationID.TextGrid")
            ui->editFilenameTemplate->setText("$AnnotationID_$SpeakerID.TextGrid");
    }
}
