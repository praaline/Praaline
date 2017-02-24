#include "AddCalculatedAnnotationDialog.h"
#include "ui_AddCalculatedAnnotationDialog.h"

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "annotation/IntervalTier.h"
#include "annotation/AnnotationTierGroup.h"
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
    connect(ui->comboBoxRepository, SIGNAL(currentTextChanged(QString)), this, SLOT(corpusRepositoryChanged(QString)));
    // Annotation level combo-boxes
    connect(ui->comboBoxAnnotationLevelBase, SIGNAL(currentIndexChanged(int)), this, SLOT(annotationLevelBaseChanged(int)));
    connect(ui->comboBoxAnnotationLevelDerived, SIGNAL(currentIndexChanged(int)), this, SLOT(annotationLevelDerivedChanged(int)));
    // Apply button
    connect(ui->commandApply, SIGNAL(clicked(bool)), this, SLOT(createSequences()));
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

void AddCalculatedAnnotationDialog::applyMeasure()
{

}


