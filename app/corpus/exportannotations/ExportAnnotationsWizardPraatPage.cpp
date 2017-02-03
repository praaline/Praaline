#include <QMessageBox>
#include <QStandardItemModel>

#include "ExportAnnotationsWizardPraatPage.h"
#include "ui_ExportAnnotationsWizardPraatPage.h"

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/interfaces/praat/AnnotationInterfacePraat.h"
using namespace Praaline::Core;

#include "pngui/model/CheckableProxyModel.h"
#include "pngui/model/corpus/AnnotationStructureTreeModel.h"


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
    d->modelLevelsAttributes = QSharedPointer<AnnotationStructureTreeModel>(
                new AnnotationStructureTreeModel(repository->annotationStructure(), true, true, this));
    ui->treeviewLevelsAttributes->setModel(d->modelLevelsAttributes.data());
    connect(d->modelLevelsAttributes.data(), SIGNAL(annotationLevelAttributeSelectionChanged(QString,QString,bool)),
            this, SLOT(annotationLevelAttributeSelectionChanged(QString,QString,bool)));
    // Reset textgrid structure
    d->modelTextgridStructure = QSharedPointer<QStandardItemModel>(new QStandardItemModel);
    d->modelTextgridStructure->setColumnCount(3);
    ui->tableviewTextgridStructure->setModel(d->modelTextgridStructure.data());
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