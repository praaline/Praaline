#include "AddCalculatedAnnotationDialog.h"
#include "ui_AddCalculatedAnnotationDialog.h"

#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

#include "CorpusRepositoriesManager.h"

struct AddCalculatedAnnotationDialogData {
    AddCalculatedAnnotationDialogData() :
        corpusRepositoriesManager(0)
    {}

    QPointer<CorpusRepositoriesManager> corpusRepositoriesManager;
    QString repositoryID;
};

AddCalculatedAnnotationDialog::AddCalculatedAnnotationDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::AddCalculatedAnnotationDialog), d(new AddCalculatedAnnotationDialogData)
{
    ui->setupUi(this);
    // Corpus repositories
    foreach (QObject* obj, OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager")) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    if (!d->corpusRepositoriesManager) return;
    // Repository combobox
    ui->comboBoxRepository->addItems(d->corpusRepositoriesManager->listCorpusRepositoryIDs());
    connect(ui->comboBoxRepository, &QComboBox::currentTextChanged, this, &AddCalculatedAnnotationDialog::corpusRepositoryChanged);
    // Annotation level combo-boxes
    connect(ui->comboBoxAnnotationLevelBase, qOverload<int>(&QComboBox::currentIndexChanged), this, &AddCalculatedAnnotationDialog::annotationLevelBaseChanged);
    connect(ui->comboBoxAnnotationLevelDerived, qOverload<int>(&QComboBox::currentIndexChanged), this, &AddCalculatedAnnotationDialog::annotationLevelDerivedChanged);
    // Apply button
    connect(ui->commandApply, &QPushButton::clicked, this, &AddCalculatedAnnotationDialog::applyCalculation);
    // Initialize
    corpusRepositoryChanged(d->corpusRepositoriesManager->activeCorpusRepositoryID());
}

AddCalculatedAnnotationDialog::~AddCalculatedAnnotationDialog()
{
    delete d;
    delete ui;
}

void AddCalculatedAnnotationDialog::corpusRepositoryChanged(const QString &repositoryID)
{
    if (d->repositoryID == repositoryID) return;
    d->repositoryID = repositoryID;
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID);
    if (!repository) return;
    if (!repository->annotationStructure()) return;
    ui->comboBoxAnnotationLevelBase->clear();
    ui->comboBoxAnnotationLevelDerived->clear();
    foreach (QPointer<AnnotationStructureLevel> level, repository->annotationStructure()->levels()) {
        ui->comboBoxAnnotationLevelBase->addItem(level->name(), level->ID());
        ui->comboBoxAnnotationLevelDerived->addItem(level->name(), level->ID());
    }
    annotationLevelBaseChanged(0);
    annotationLevelDerivedChanged(0);
}

void AddCalculatedAnnotationDialog::annotationLevelBaseChanged(int index)
{
    Q_UNUSED(index)
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID);
    if (!repository) return;
    if (!repository->annotationStructure()) return;
    ui->comboBoxAnnotationAttributeBase->clear();
    QPointer<AnnotationStructureLevel> level = repository->annotationStructure()->level(ui->comboBoxAnnotationLevelBase->currentData().toString());
    foreach (QPointer<AnnotationStructureAttribute> attribute, level->attributes()) {
        ui->comboBoxAnnotationAttributeBase->addItem(attribute->name(), attribute->ID());
    }
}

void AddCalculatedAnnotationDialog::annotationLevelDerivedChanged(int index)
{
    Q_UNUSED(index)
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID);
    if (!repository) return;
    if (!repository->annotationStructure()) return;
    ui->comboBoxAnnotationAttributeDerived->clear();
    QPointer<AnnotationStructureLevel> level = repository->annotationStructure()->level(ui->comboBoxAnnotationLevelDerived->currentData().toString());
    foreach (QPointer<AnnotationStructureAttribute> attribute, level->attributes()) {
        ui->comboBoxAnnotationAttributeDerived->addItem(attribute->name(), attribute->ID());
    }
}

void AddCalculatedAnnotationDialog::applyCalculation()
{

}


