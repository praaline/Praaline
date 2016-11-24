#include <QDebug>
#include <QHash>
#include <QPair>
#include <QHash>
#include <QFileInfo>
#include <QMultiHash>
#include <QStandardItemModel>
#include "pncore/interfaces/InterfaceTextFile.h"
#include "ImportCorpusItemsWizardCorrespondancesPage.h"
#include "ui_ImportCorpusItemsWizardCorrespondancesPage.h"

ImportCorpusItemsWizardCorrespondancesPage::ImportCorpusItemsWizardCorrespondancesPage(
        QPointer<Corpus> corpus,
        QMultiHash<QString, TierCorrespondance> &tierCorrespondances,
        QSet<QString> &tierNamesCommon,
        QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ImportCorpusItemsWizardCorrespondancesPage),
    m_corpus(corpus), m_tierCorrespondances(tierCorrespondances), m_tierNamesCommon(tierNamesCommon)
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

    connect(ui->commandBatchUpdate, SIGNAL(clicked()), this, SLOT(batchUpdate()));
}

ImportCorpusItemsWizardCorrespondancesPage::~ImportCorpusItemsWizardCorrespondancesPage()
{
    delete ui;
    delete m_modelTiers;
}

void ImportCorpusItemsWizardCorrespondancesPage::initializePage()
{
    int i;
    QStringList labels;

    QList<QString> tierNamesAllSorted;
    QList<QString> tierNamesCommonSorted;

    if (!m_tierNamesCommon.isEmpty()) tierNamesCommonSorted = m_tierNamesCommon.toList();
    qSort(tierNamesCommonSorted);

    // Find all tiers (common and not common)
    QHash<QString, QStringList> tierNamesPresence;
    QMultiHash<QString, TierCorrespondance>::iterator iter;
    for (iter = m_tierCorrespondances.begin(); iter != m_tierCorrespondances.end(); ++iter) {
        QString tierName = iter.value().tierName;
        if (!tierNamesAllSorted.contains(tierName)) tierNamesAllSorted << tierName;
        QFileInfo finfo(iter.value().filename);
        tierNamesPresence[tierName].append(finfo.baseName());
    }
    qSort(tierNamesAllSorted);

    // Model for tiers
    m_modelTiers = new QStandardItemModel(tierNamesAllSorted.count(), 4, this);
    i = 0;
    foreach (QString tier, tierNamesAllSorted) {
        m_modelTiers->setItem(i, 0, new QStandardItem(tier));
        m_modelTiers->setItem(i, 1, new QStandardItem());
        m_modelTiers->setItem(i, 2, new QStandardItem());
        if (m_tierNamesCommon.contains(tier))
            m_modelTiers->setItem(i, 3, new QStandardItem(tr("All files")));
        else {
            m_modelTiers->setItem(i, 3, new QStandardItem(tierNamesPresence[tier].join(", ")));
        }
        i++;
    }
    labels << tr("Tier Name") << tr("Annotation Level") << tr("Annotation Attribute") << tr("Presence");
    m_modelTiers->setHorizontalHeaderLabels(labels);
    ui->tableviewTiers->setModel(m_modelTiers);

    guessCorrespondances();
}

bool filterTierName(QString tierName, QString filterOperator, QString filter)
{
    if (filterOperator == tr("no filter")) {
        return true;
    }
    else if (filterOperator == tr("contains")) {
        if (tierName.contains(filter, Qt::CaseInsensitive)) return true;
    }
    else if (filterOperator == tr("does not contain")) {
        if (!tierName.contains(filter, Qt::CaseInsensitive)) return true;
    }
    else if (filterOperator == tr("starts with")) {
        if (tierName.startsWith(filter, Qt::CaseInsensitive)) return true;
    }
    else if (filterOperator == tr("does not start with")) {
        if (!tierName.startsWith(filter, Qt::CaseInsensitive)) return true;
    }
    else if (filterOperator == tr("ends with")) {
        if (tierName.endsWith(filter, Qt::CaseInsensitive)) return true;
    }
    else if (filterOperator == tr("does not end with")) {
        if (!tierName.endsWith(filter, Qt::CaseInsensitive)) return true;
    }
    return false;
}

void ImportCorpusItemsWizardCorrespondancesPage::batchUpdate()
{
    QString filterOperator = ui->comboFilter->currentText();
    QString filter = ui->editFilter->text();
    // Run through tiers
    for (int i = 0; i < m_modelTiers->rowCount(); ++i) {
        QString tierName = m_modelTiers->item(i, 0)->data(Qt::DisplayRole).toString();
        if (filterTierName(tierName, filterOperator, filter)) {
            m_modelTiers->setData(m_modelTiers->index(i, 1), ui->editLevel->text());
            m_modelTiers->setData(m_modelTiers->index(i, 2), ui->editAttribute->text());
        }
    }
}

void ImportCorpusItemsWizardCorrespondancesPage::guessCorrespondance(int i, QString tierName)
{
    // Best match
    foreach (QString levelID, m_corpus->annotationStructure()->levelIDs()) {
        if ((tierName == levelID) || (tierName.replace("-", "_") == levelID)) {
            m_modelTiers->setData(m_modelTiers->index(i, 1), levelID);
            return;
        }
        else {
            foreach (QString attributeID, m_corpus->annotationStructure()->level(levelID)->attributeIDs()) {
                if ((tierName == attributeID) || (tierName.replace("-", "_") == attributeID)) {
                    m_modelTiers->setData(m_modelTiers->index(i, 1), levelID);
                    m_modelTiers->setData(m_modelTiers->index(i, 2), attributeID);
                    return;
                }
            }
        }
    }
    // Next best match
    foreach (QString levelID, m_corpus->annotationStructure()->levelIDs()) {
        if (tierName.startsWith(levelID) || tierName.replace("-", "_").startsWith(levelID)) {
            m_modelTiers->setData(m_modelTiers->index(i, 1), levelID);
            return;
        }
        else {
            foreach (QString attributeID, m_corpus->annotationStructure()->level(levelID)->attributeIDs()) {
                if (tierName.startsWith(attributeID) || tierName.replace("-", "_").startsWith(attributeID)) {
                    m_modelTiers->setData(m_modelTiers->index(i, 1), levelID);
                    m_modelTiers->setData(m_modelTiers->index(i, 2), attributeID);
                    return;
                }
            }
        }
    }
}

void ImportCorpusItemsWizardCorrespondancesPage::guessCorrespondances()
{
    if (!m_corpus) return;
    if (!m_corpus->annotationStructure()) return;

    for (int i = 0; i < m_modelTiers->rowCount(); ++i) {
        QString tierName = m_modelTiers->item(i, 0)->data(Qt::DisplayRole).toString();
        guessCorrespondance(i, tierName);
    }
}

bool ImportCorpusItemsWizardCorrespondancesPage::validatePage()
{
    QHash<QString, QPair<QString, QString> > correspondances;
    bool structureChanges = false;

    for (int i = 0; i < m_modelTiers->rowCount(); ++i) {
        QString tier = m_modelTiers->item(i, 0)->data(Qt::DisplayRole).toString();
        QString annotationLevelID = m_modelTiers->item(i, 1)->data(Qt::DisplayRole).toString();
        QString annotationAttributeID = m_modelTiers->item(i, 2)->data(Qt::DisplayRole).toString();
        correspondances.insert(tier, QPair<QString, QString>(annotationLevelID, annotationAttributeID));
        // Create levels and attributes if they do not exist
        // IMPROVE this: should lead to a next wizard page asking to define the data types of the new level/attributes
        // for now, just use varchar(1024)
        if (m_corpus && ui->optionCreateLevelsAttributes->isChecked()) {
            if (!m_corpus->annotationStructure()->hasLevel(annotationLevelID)) {
                QPointer<AnnotationStructureLevel> level = new AnnotationStructureLevel(annotationLevelID, AnnotationStructureLevel::IndependentLevel, annotationLevelID);
                if (m_corpus->datastoreAnnotations()->createAnnotationLevel(level)) {
                    m_corpus->annotationStructure()->addLevel(level);
                    structureChanges = true;
                }
            }
            AnnotationStructureLevel *level = m_corpus->annotationStructure()->level(annotationLevelID);
            if (level && (!annotationAttributeID.isEmpty()) && (!level->hasAttribute(annotationAttributeID))) {
                QPointer<AnnotationStructureAttribute> attr = new AnnotationStructureAttribute(annotationAttributeID, annotationAttributeID);
                if (m_corpus->datastoreAnnotations()->createAnnotationAttribute(level->ID(), attr)) {
                    level->addAttribute(attr);
                    structureChanges = true;
                }
            }
        }
    }
    if (m_corpus && structureChanges) {
        m_corpus->save();
    }
    QMultiHash<QString, TierCorrespondance>::iterator i;
    for (i = m_tierCorrespondances.begin(); i != m_tierCorrespondances.end(); ++i) {
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
