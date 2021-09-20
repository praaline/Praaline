#include <QString>
#include <QList>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QDir>
#include <QFileDialog>

#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Structure/MetadataStructure.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/CorpusAnnotationTableModel.h"

#include "CorpusRepositoriesManager.h"

#include "ExportAnnotationsWizard.h"
#include "ui_ExportAnnotationsWizard.h"

#include "ExportAnnotationsWizardPraatPage.h"
#include "ExportAnnotationsWizardTimelinePage.h"

struct ExportAnnotationsWizardData {
    ExportAnnotationsWizardData()
    {}

    QPointer<CorpusRepositoriesManager> corpusRepositoriesManager;
    QString repositoryID;
    QSharedPointer<QStandardItemModel> modelAnnotations;
    QSharedPointer<ExportAnnotationsWizardPraatPage> pagePraat;
    QSharedPointer<ExportAnnotationsWizardTimelinePage> pageTimelines;
};

ExportAnnotationsWizard::ExportAnnotationsWizard(QWidget *parent) :
    QWizard(parent), ui(new Ui::ExportAnnotationsWizard), d(new ExportAnnotationsWizardData)
{
    ui->setupUi(this);

    // Corpus repositories
    foreach (QObject* obj, OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager")) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    if (!d->corpusRepositoriesManager) this->reject();
    // Repository combobox
    ui->comboBoxRepository->addItems(d->corpusRepositoriesManager->listCorpusRepositoryIDs());
    connect(ui->comboBoxRepository, &QComboBox::currentTextChanged, this, &ExportAnnotationsWizard::corpusRepositoryChanged);
    corpusRepositoryChanged(d->corpusRepositoriesManager->activeCorpusRepositoryID());
    // Export formats
    ui->comboBoxExportFormat->addItem(tr("Praat TextGrids"), Page_Praat);
    ui->comboBoxExportFormat->addItem(tr("Multi-tier timeline to Excel spreadsheet(s)"), Page_Timelines);
    ui->comboBoxExportFormat->setCurrentIndex(0);
    connect(ui->comboBoxExportFormat, qOverload<int>(&QComboBox::currentIndexChanged), this, &ExportAnnotationsWizard::exportFormatChanged);
    // Select file or folder button
    ui->editFileFolder->setText(QDir::homePath());
    connect(ui->commandSelectFileFolder, &QAbstractButton::clicked, this, &ExportAnnotationsWizard::selectFileFolder);
    // Presentation
    ui->treeViewAnnotations->setHeaderHidden(false);
    // Pages for export formats
    d->pagePraat = QSharedPointer<ExportAnnotationsWizardPraatPage>(new ExportAnnotationsWizardPraatPage());
    d->pageTimelines = QSharedPointer<ExportAnnotationsWizardTimelinePage>(new ExportAnnotationsWizardTimelinePage());
    setPage(Page_Praat, d->pagePraat.data());
    setPage(Page_Timelines, d->pageTimelines.data());
}

ExportAnnotationsWizard::~ExportAnnotationsWizard()
{
    delete ui;
    delete d;
}

void ExportAnnotationsWizard::selectFileFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Export Annotations"));
    if (directory.isEmpty()) return;
    ui->editFileFolder->setText(directory);
}

void ExportAnnotationsWizard::exportFormatChanged(int)
{
}

void ExportAnnotationsWizard::corpusRepositoryChanged(const QString &repositoryID)
{
    if (d->repositoryID == repositoryID) return;
    d->repositoryID = repositoryID;
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID);
    if (!repository) return;
    if (!repository->metadata()) return;
    if (!repository->metadataStructure()) return;
    // Update the list of annotations
    QList<CorpusObjectInfo> list = repository->metadata()->getCorpusObjectInfoList(CorpusObject::Type_Annotation, MetadataDatastore::Selection());
    d->modelAnnotations = QSharedPointer<QStandardItemModel>(new QStandardItemModel(this));
    QStringList attributeIDs = repository->metadataStructure()->allAttributeIDs(CorpusObject::Type_Annotation);
    QStringList attributeNames = repository->metadataStructure()->allAttributeNames(CorpusObject::Type_Annotation);
    d->modelAnnotations->setColumnCount(attributeIDs.count());
    // Headers
    for (int column = 0; column < attributeNames.count(); ++column) {
        d->modelAnnotations->setHeaderData(column, Qt::Horizontal, attributeNames.at(column));
    }
    // Data
    for (int row = 0; row < list.count(); ++row) {
        for (int column = 0; column < attributeIDs.count(); ++column) {
            QStandardItem *item = new QStandardItem(list.at(row).attribute(attributeIDs.at(column)).toString());
            if (column == 0) {
                item->setCheckable(true);
                item->setCheckState(Qt::Checked);
                item->setData(QVariant::fromValue(list.at(row)));
            }
            d->modelAnnotations->setItem(row, column, item);
        }
    }
    ui->treeViewAnnotations->setModel(d->modelAnnotations.data());
}

int ExportAnnotationsWizard::nextId() const
{
    if (currentId() == 0) {
        if      (ui->comboBoxExportFormat->currentData() == QVariant(Page_Praat))       return Page_Praat;
        else if (ui->comboBoxExportFormat->currentData() == QVariant(Page_Timelines))   return Page_Timelines;
    }
    return -1;
}

bool ExportAnnotationsWizard::validateCurrentPage()
{
    if (currentId() == 0) {
        QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID);
        if (!repository) return false;
        // Update annotation structure on next page, depending on the export format
        if      (ui->comboBoxExportFormat->currentData() == QVariant(Page_Praat))
            d->pagePraat->setRepository(repository);
        else if (ui->comboBoxExportFormat->currentData() == QVariant(Page_Timelines))
            d->pageTimelines->setRepository(repository);
    }
    else if (currentId() >= 1)
        doExport();
    return true;
}

void ExportAnnotationsWizard::doExport()
{
    // Export selected annotations
    QList<CorpusObjectInfo> annotations;
    for (int i = 0; i < d->modelAnnotations->rowCount(); ++i) {
        if (d->modelAnnotations->item(i, 0)->checkState() != Qt::Checked) continue;
        QVariant data = d->modelAnnotations->item(i, 0)->data();
        CorpusObjectInfo info = data.value<CorpusObjectInfo>();
        annotations << info;
    }
    if (currentId() == Page_Praat) {
        d->pagePraat->setExportPath(ui->editFileFolder->text());
        d->pagePraat->doExport(annotations);
    }
    else if (currentId() == Page_Timelines) {
        d->pageTimelines->setExportPath(ui->editFileFolder->text());
        d->pageTimelines->doExport(annotations);
    }
}
