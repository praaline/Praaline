#include "CreateSequenceAnnotationDialog.h"
#include "ui_CreateSequenceAnnotationDialog.h"

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/structure/AnnotationStructure.h"
#include "annotation/AnnotationTier.h"
#include "annotation/SequenceTier.h"
#include "annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

#include "CorpusRepositoriesManager.h"

struct CreateSequenceAnnotationDialogData {
    CreateSequenceAnnotationDialogData() :
        corpusRepositoriesManager(0)
    {}

    QPointer<CorpusRepositoriesManager> corpusRepositoriesManager;
    QString repositoryID;
};


CreateSequenceAnnotationDialog::CreateSequenceAnnotationDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::CreateSequenceAnnotationDialog), d(new CreateSequenceAnnotationDialogData)
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
    // Apply button
    connect(ui->commandCreateSequences, SIGNAL(clicked(bool)), this, SLOT(createSequences()));
    // Initialize
    corpusRepositoryChanged(d->corpusRepositoriesManager->activeCorpusRepositoryID());
}

CreateSequenceAnnotationDialog::~CreateSequenceAnnotationDialog()
{
    delete d;
    delete ui;
}

void CreateSequenceAnnotationDialog::corpusRepositoryChanged(const QString &repositoryID)
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
        if (level->levelType() == AnnotationStructureLevel::SequencesLevel)
            ui->comboBoxAnnotationLevelDerived->addItem(level->name(), level->ID());
    }
    annotationLevelBaseChanged(0);
}

void CreateSequenceAnnotationDialog::annotationLevelBaseChanged(int index)
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

void CreateSequenceAnnotationDialog::createSequences()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(d->repositoryID);
    if (!repository) return;
    QString baseLevelID = ui->comboBoxAnnotationLevelBase->currentData().toString();
    QString baseAttributeID = ui->comboBoxAnnotationAttributeBase->currentData().toString();
    QString derivedLevelID = ui->comboBoxAnnotationLevelDerived->currentData().toString();
    QRegularExpression reBegin(ui->editBeginSequence->text());
    QRegularExpression reEnd(ui->editEndSequence->text());
    // For each annotation
    QList<CorpusObjectInfo> list = repository->metadata()->getCorpusObjectInfoList(CorpusObject::Type_Annotation, MetadataDatastore::Selection());
    int i = 0;
    ui->progressBar->setMaximum(list.count());
    foreach (CorpusObjectInfo annotation, list) {
        QString annotationID = annotation.attribute("annotationID").toString();
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = repository->annotations()->getTiersAllSpeakers(annotationID, QStringList() << baseLevelID << derivedLevelID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            AnnotationTier *tier_base = tiers->tier(baseLevelID);
            SequenceTier *tier_sequences = qobject_cast<SequenceTier *>(tiers->tier(derivedLevelID));
            if ((!tier_base) || (!tier_sequences)) continue;
            int indexFrom(-1), indexTo(-1);
            bool seenStart(false), seenEnd(false);
            for (int index = 0; index < tier_base->count(); ++index) {
                QString value = (baseAttributeID.isEmpty()) ? tier_base->at(index)->text() :
                                                              tier_base->at(index)->attribute(baseAttributeID).toString();
                QRegularExpressionMatch matchBegin = reBegin.match(value);
                QRegularExpressionMatch matchEnd = reEnd.match(value);
                if (!seenStart && matchBegin.hasMatch()) {
                    seenStart = true;
                    seenEnd = false;
                    indexFrom = index;
                    indexTo = index;
                }
                else if (seenStart && matchEnd.hasMatch()) {
                    seenEnd = true;
                    indexTo = index;
                }
                else if (seenEnd && !matchEnd.hasMatch()) {
                    if ((indexFrom > 0) && (indexTo > 0) && (indexFrom < indexTo))
                        tier_sequences->addSequence(new Sequence(indexFrom, indexTo, ""));
                    seenStart = false;
                    seenEnd = false;
                    indexFrom = indexTo = -1;
                }
            }
            repository->annotations()->saveTier(annotationID, speakerID, tier_sequences);
        }
        i++;
        ui->progressBar->setValue(i);
    }
}


