#include "TidyUpAnnotationsDialog.h"
#include "ui_TidyUpAnnotationsDialog.h"

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "annotation/IntervalTier.h"
#include "annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

#include "CorpusRepositoriesManager.h"

struct TidyUpAnnotationsDialogData {
    TidyUpAnnotationsDialogData() :
        corpusRepositoriesManager(0)
    {}

    QPointer<CorpusRepositoriesManager> corpusRepositoriesManager;
    QString repositoryID;
};

TidyUpAnnotationsDialog::TidyUpAnnotationsDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::TidyUpAnnotationsDialog), d(new TidyUpAnnotationsDialogData)
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
    // Annotation level combo-box
    connect(ui->comboBoxAnnotationLevel, SIGNAL(currentIndexChanged(int)), this, SLOT(annotationLevelChanged(int)));
    // Apply button
    connect(ui->commandApply, SIGNAL(clicked(bool)), this, SLOT(applyChanges()));
    // Initialize
    corpusRepositoryChanged(d->corpusRepositoriesManager->activeCorpusRepositoryID());
}

TidyUpAnnotationsDialog::~TidyUpAnnotationsDialog()
{
    delete d;
    delete ui;
}

void TidyUpAnnotationsDialog::corpusRepositoryChanged(const QString &repositoryID)
{
    if (d->repositoryID == repositoryID) return;
    d->repositoryID = repositoryID;
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID);
    if (!repository) return;
    if (!repository->annotationStructure()) return;
    ui->comboBoxAnnotationLevel->clear();
    foreach (QPointer<AnnotationStructureLevel> level, repository->annotationStructure()->levels()) {
        ui->comboBoxAnnotationLevel->addItem(level->name(), level->ID());
    }
    annotationLevelChanged(0);
}

void TidyUpAnnotationsDialog::annotationLevelChanged(int index)
{
    Q_UNUSED(index)
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID);
    if (!repository) return;
    if (!repository->annotationStructure()) return;
    ui->comboBoxAnnotationAttribute->clear();
    QPointer<AnnotationStructureLevel> level = repository->annotationStructure()->level(ui->comboBoxAnnotationLevel->currentData().toString());
    foreach (QPointer<AnnotationStructureAttribute> attribute, level->attributes()) {
        ui->comboBoxAnnotationAttribute->addItem(attribute->name(), attribute->ID());
    }
}

void TidyUpAnnotationsDialog::applyChanges()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID);
    if (!repository) return;
    QString levelID = ui->comboBoxAnnotationLevel->currentData().toString();
    QString attributeID = ui->comboBoxAnnotationAttribute->currentData().toString();
    // For each annotation
    QList<CorpusObjectInfo> list = repository->metadata()->getCorpusObjectInfoList(CorpusObject::Type_Annotation, MetadataDatastore::Selection());
    int i = 0;
    ui->progressBar->setMaximum(list.count());
    foreach (CorpusObjectInfo annotation, list) {
        QString annotationID = annotation.attribute("annotationID").toString();
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = repository->annotations()->getTiersAllSpeakers(annotationID, QStringList() << levelID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (ui->optionMergeContiguousPauses->isChecked()) {
                IntervalTier *tier = tiers->getIntervalTierByName(levelID);
                if (!tier) continue;
                tier->fillEmptyWith("", "_");
                tier->mergeIdenticalAnnotations("_");
                repository->annotations()->saveTier(annotationID, speakerID, tier);
            }
        }
        i++;
        ui->progressBar->setValue(i);
    }
}
