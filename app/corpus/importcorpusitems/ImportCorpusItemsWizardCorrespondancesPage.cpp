#include <QDebug>
#include <QPointer>
#include <QHash>
#include <QPair>
#include <QHash>
#include <QFileInfo>
#include <QMultiHash>
#include <QStandardItemModel>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
#include "PraalineCore/Interfaces/InterfaceTextFile.h"
#include "PraalineCore/Interfaces/ImportAnnotations.h"
using namespace Praaline::Core;

#include "ImportCorpusItemsWizardCorrespondancesPage.h"
#include "ui_ImportCorpusItemsWizardCorrespondancesPage.h"

struct ImportCorpusItemsWizardCorrespondancesPageData {
    ImportCorpusItemsWizardCorrespondancesPageData(CorpusRepository *repository,
                                                   QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
                                                   QSet<QString> &tierNamesCommon) :
        repository(repository), tierCorrespondances(tierCorrespondances), tierNamesCommon(tierNamesCommon)
    {}

    CorpusRepository *repository;
    QMultiHash<QString, TierCorrespondance> &tierCorrespondances;
    QSet<QString> &tierNamesCommon;
    QPointer<QStandardItemModel> modelTiers;
};

ImportCorpusItemsWizardCorrespondancesPage::ImportCorpusItemsWizardCorrespondancesPage(
        CorpusRepository *repository,
        QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
        QSet<QString> &tierNamesCommon,
        QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ImportCorpusItemsWizardCorrespondancesPage),
    d(new ImportCorpusItemsWizardCorrespondancesPageData(repository, tierCorrespondances, tierNamesCommon))
{
    ui->setupUi(this);
    setTitle(tr("Correspondances between annotation files and Annotation Levels/Attributes"));
    setSubTitle(tr("In this step you can specify how the different annotation tiers found in the files to be imported correspond to the "
                   "Annotation Levels and Attributes defined for your Corpus. You may also select the policy used to indicate the current speaker."));
    ui->tableviewTiers->verticalHeader()->setDefaultSectionSize(20);

    QStringList filters;
    filters << tr("no filter") << tr("contains") << tr("does not contain") << tr("starts with") << tr("does not start with")
            << tr("ends with") << tr("does not end with");
    ui->comboFilter->addItems(filters);

    QStringList encodings;
    encodings << tr("UTF-8") << tr("ISO 8859-1");
    ui->comboEncoding->addItems(encodings);

    connect(ui->commandBatchUpdate, &QAbstractButton::clicked, this, &ImportCorpusItemsWizardCorrespondancesPage::batchUpdate);
}

ImportCorpusItemsWizardCorrespondancesPage::~ImportCorpusItemsWizardCorrespondancesPage()
{
    delete ui;
    if (d->modelTiers) delete d->modelTiers;
    delete d;
}

void ImportCorpusItemsWizardCorrespondancesPage::initializePage()
{
    int i;
    QStringList labels;

    QList<QString> tierNamesAllSorted;
    QList<QString> tierNamesCommonSorted;

    if (!d->tierNamesCommon.isEmpty()) tierNamesCommonSorted = d->tierNamesCommon.values();
    std::sort(tierNamesCommonSorted.begin(), tierNamesCommonSorted.end());

    // Find all tiers (common and not common)
    QHash<QString, QStringList> tierNamesPresence;
    QMultiHash<QString, TierCorrespondance>::iterator iter;
    for (iter = d->tierCorrespondances.begin(); iter != d->tierCorrespondances.end(); ++iter) {
        QString tierName = iter.value().tierName;
        if (!tierNamesAllSorted.contains(tierName)) tierNamesAllSorted << tierName;
        QFileInfo finfo(iter.value().filename);
        tierNamesPresence[tierName].append(finfo.baseName());
    }
    std::sort(tierNamesAllSorted.begin(), tierNamesAllSorted.end());

    // Model for tiers
    d->modelTiers = new QStandardItemModel(tierNamesAllSorted.count(), 4, this);
    i = 0;
    foreach (QString tier, tierNamesAllSorted) {
        d->modelTiers->setItem(i, 0, new QStandardItem(tier));
        d->modelTiers->setItem(i, 1, new QStandardItem());
        d->modelTiers->setItem(i, 2, new QStandardItem());
        if (d->tierNamesCommon.contains(tier))
            d->modelTiers->setItem(i, 3, new QStandardItem(tr("All files")));
        else {
            d->modelTiers->setItem(i, 3, new QStandardItem(tierNamesPresence[tier].join(", ")));
        }
        i++;
    }
    labels << tr("Tier Name") << tr("Annotation Level") << tr("Annotation Attribute") << tr("Presence");
    d->modelTiers->setHorizontalHeaderLabels(labels);
    ui->tableviewTiers->setModel(d->modelTiers);

    guessCorrespondances();
}

bool filterTierName(QString tierName, QString filterOperator, QString filter)
{
    if (filterOperator == QObject::tr("no filter")) {
        return true;
    }
    else if (filterOperator == QObject::tr("contains")) {
        if (tierName.contains(filter, Qt::CaseInsensitive)) return true;
    }
    else if (filterOperator == QObject::tr("does not contain")) {
        if (!tierName.contains(filter, Qt::CaseInsensitive)) return true;
    }
    else if (filterOperator == QObject::tr("starts with")) {
        if (tierName.startsWith(filter, Qt::CaseInsensitive)) return true;
    }
    else if (filterOperator == QObject::tr("does not start with")) {
        if (!tierName.startsWith(filter, Qt::CaseInsensitive)) return true;
    }
    else if (filterOperator == QObject::tr("ends with")) {
        if (tierName.endsWith(filter, Qt::CaseInsensitive)) return true;
    }
    else if (filterOperator == QObject::tr("does not end with")) {
        if (!tierName.endsWith(filter, Qt::CaseInsensitive)) return true;
    }
    return false;
}

void ImportCorpusItemsWizardCorrespondancesPage::batchUpdate()
{
    QString filterOperator = ui->comboFilter->currentText();
    QString filter = ui->editFilter->text();
    // Run through tiers
    for (int i = 0; i < d->modelTiers->rowCount(); ++i) {
        QString tierName = d->modelTiers->item(i, 0)->data(Qt::DisplayRole).toString();
        if (filterTierName(tierName, filterOperator, filter)) {
            d->modelTiers->setData(d->modelTiers->index(i, 1), ui->editLevel->text());
            d->modelTiers->setData(d->modelTiers->index(i, 2), ui->editAttribute->text());
        }
    }
}

void ImportCorpusItemsWizardCorrespondancesPage::guessCorrespondance(int i, QString tierName)
{
    // Get annotation structure of the current corpus
    if (!d->repository) return;
    if (!d->repository->annotationStructure()) return;
    // Best match
    foreach (QString levelID, d->repository->annotationStructure()->levelIDs()) {
        if ((tierName == levelID) || (tierName.replace("-", "_") == levelID)) {
            d->modelTiers->setData(d->modelTiers->index(i, 1), levelID);
            return;
        }
        else {
            foreach (QString attributeID, d->repository->annotationStructure()->level(levelID)->attributeIDs()) {
                if ((tierName == attributeID) || (tierName.replace("-", "_") == attributeID)) {
                    d->modelTiers->setData(d->modelTiers->index(i, 1), levelID);
                    d->modelTiers->setData(d->modelTiers->index(i, 2), attributeID);
                    return;
                }
            }
        }
    }
    // Next best match
    foreach (QString levelID, d->repository->annotationStructure()->levelIDs()) {
        if (tierName.startsWith(levelID) || tierName.replace("-", "_").startsWith(levelID)) {
            d->modelTiers->setData(d->modelTiers->index(i, 1), levelID);
            return;
        }
        else {
            foreach (QString attributeID, d->repository->annotationStructure()->level(levelID)->attributeIDs()) {
                if (tierName.startsWith(attributeID) || tierName.replace("-", "_").startsWith(attributeID)) {
                    d->modelTiers->setData(d->modelTiers->index(i, 1), levelID);
                    d->modelTiers->setData(d->modelTiers->index(i, 2), attributeID);
                    return;
                }
            }
        }
    }
}

void ImportCorpusItemsWizardCorrespondancesPage::guessCorrespondances()
{
    // Get annotation structure of the current corpus
    if (!d->repository) return;
    if (!d->repository->annotationStructure()) return;

    for (int i = 0; i < d->modelTiers->rowCount(); ++i) {
        QString tierName = d->modelTiers->item(i, 0)->data(Qt::DisplayRole).toString();
        guessCorrespondance(i, tierName);
    }
}

bool ImportCorpusItemsWizardCorrespondancesPage::validatePage()
{
    // Get annotation structure of the current corpus
    if (!d->repository) return false;
    if (!d->repository->annotationStructure()) return false;

    QHash<QString, QPair<QString, QString> > correspondances;
    bool structureChanges = false;

    for (int i = 0; i < d->modelTiers->rowCount(); ++i) {
        QString tier = d->modelTiers->item(i, 0)->data(Qt::DisplayRole).toString();
        QString annotationLevelID = d->modelTiers->item(i, 1)->data(Qt::DisplayRole).toString();
        QString annotationAttributeID = d->modelTiers->item(i, 2)->data(Qt::DisplayRole).toString();
        correspondances.insert(tier, QPair<QString, QString>(annotationLevelID, annotationAttributeID));
        // Create levels and attributes if they do not exist
        // IMPROVE this: should lead to a next wizard page asking to define the data types of the new level/attributes
        // for now, just use varchar(1024)
        if (d->repository && ui->optionCreateLevelsAttributes->isChecked()) {
            if ((!annotationLevelID.isEmpty()) && (!d->repository->annotationStructure()->hasLevel(annotationLevelID))) {
                QPointer<AnnotationStructureLevel> level =
                        new AnnotationStructureLevel(annotationLevelID, AnnotationStructureLevel::IndependentIntervalsLevel, annotationLevelID);
                if (d->repository->annotations()->createAnnotationLevel(level)) {
                    d->repository->annotationStructure()->addLevel(level);
                    structureChanges = true;
                }
            }
            AnnotationStructureLevel *level = d->repository->annotationStructure()->level(annotationLevelID);
            if (level && (!annotationAttributeID.isEmpty()) && (!level->hasAttribute(annotationAttributeID))) {
                QPointer<AnnotationStructureAttribute> attr = new AnnotationStructureAttribute(annotationAttributeID, annotationAttributeID);
                if (d->repository->annotations()->createAnnotationAttribute(level->ID(), attr)) {
                    level->addAttribute(attr);
                    structureChanges = true;
                }
            }
        }
    }
    QMultiHash<QString, TierCorrespondance>::iterator i;
    for (i = d->tierCorrespondances.begin(); i != d->tierCorrespondances.end(); ++i) {
        QString tierName = i.value().tierName;
        if (correspondances.contains(tierName)) {
            i.value().annotationLevelID = correspondances.value(tierName).first;
            i.value().annotationAttributeID = correspondances.value(tierName).second;
        }
    }

    // Update encoding setting that will be used in the final stage
    if (!ui->comboEncoding->currentText().isEmpty())
        InterfaceTextFile::setDefaultEncoding(ui->comboEncoding->currentText());

    return true;
}
