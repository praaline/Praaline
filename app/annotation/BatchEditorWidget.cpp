#include <QVariant>
#include <QPair>
#include <QStandardItemModel>
#include <QDebug>
#include "BatchEditorWidget.h"
#include "ui_BatchEditorWidget.h"

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/AnnotationTier.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "pngui/observers/CorpusObserver.h"
#include "pngui/widgets/GridViewWidget.h"
#include "CorpusRepositoriesManager.h"

struct BatchEditWidgetData {
    BatchEditWidgetData() : corpusRepositoriesManager(0), tableDistinctValues(0), modelDistinctValues(0) {}

    CorpusRepositoriesManager *corpusRepositoriesManager;
    GridViewWidget *tableDistinctValues;
    QStandardItemModel *modelDistinctValues;
    QStringList modelAttributeIDs;

};

BatchEditorWidget::BatchEditorWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::BatchEditWidget), d(new BatchEditWidgetData)
{
    ui->setupUi(this);

    // Corpora manager
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    connect(d->corpusRepositoriesManager, SIGNAL(activeCorpusRepositoryChanged(QString)), this, SLOT(activeCorpusRepositoryChanged(QString)));
    connect(ui->comboBoxLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(levelChanged(QString)));

    // Manual selection model - table
    d->tableDistinctValues = new GridViewWidget(this);
    d->tableDistinctValues->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->tableDistinctValues->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutResults->addWidget(d->tableDistinctValues );

    // Actions
    connect(ui->commandGetDistinctValues, SIGNAL(clicked()), this, SLOT(actionGetDistinctValues()));
    connect(ui->commandUpdateValues, SIGNAL(clicked()), this, SLOT(actionUpdateValues()));
}

BatchEditorWidget::~BatchEditorWidget()
{
    delete ui;
    delete d;
}

void BatchEditorWidget::activeCorpusRepositoryChanged(const QString &repositoryID)
{
    Q_UNUSED(repositoryID)
    ui->comboBoxLevel->clear();
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    foreach (AnnotationStructureLevel *level, repository->annotationStructure()->levels()) {
        ui->comboBoxLevel->addItem(level->name(), level->ID());
    }
    levelChanged("");
}

void BatchEditorWidget::levelChanged(const QString &text)
{
    Q_UNUSED(text)
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    QString levelID = ui->comboBoxLevel->currentData().toString();
    AnnotationStructureLevel *level = repository->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxAttributes->clear();
    ui->comboBoxAttributes->insertItem(0, tr("(text)"), true);
    int i = 1;
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxAttributes->insertItem(i, attribute->name(), false);
        i++;
    }
    ui->comboBoxAttributeToUpdate->clear();
    ui->comboBoxAttributeToUpdate->addItem(tr("(text)"), "");
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxAttributeToUpdate->addItem(attribute->name(), attribute->ID());
    }
}

// ====================================================================================================================

void BatchEditorWidget::actionGetDistinctValues()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    QString levelID = ui->comboBoxLevel->currentData().toString();
    AnnotationStructureLevel *level = repository->annotationStructure()->level(levelID);
    if (!level) return;
    QStringList attributeIDs;
    QStringList headers;
    d->modelAttributeIDs.clear();
    for (int i = 0; i < ui->comboBoxAttributes->count(); ++i) {
        if (ui->comboBoxAttributes->itemData(i).toBool() == true) {
            if (ui->comboBoxAttributes->itemText(i) == tr("(text)")) {
                attributeIDs << "";
                headers << tr("(text)");
                d->modelAttributeIDs << "";
            } else {
                // (i - 1) is taken because the first item is always the "(text)" item
                if ((i - 1) < level->attributesCount()) {
                    attributeIDs << level->attribute(i - 1)->ID();
                    headers << level->attribute(i - 1)->name();
                    d->modelAttributeIDs << level->attribute(i - 1)->ID();
                }
            }
        }
    }
    headers << tr("Count") << tr("Update?") << tr("New Value");

    QList<QPair<QList<QVariant>, long> > data = repository->annotations()->getDistinctLabels(levelID, attributeIDs);
    QPair<QList<QVariant>, long> rowData;

    QPointer<QStandardItemModel> model = new QStandardItemModel();

    int row = 0;
    foreach (rowData, data) {
        QList<QVariant> values = rowData.first;
        int count = rowData.second;
        QList<QStandardItem *> items;
        for (int col = 0; col < values.count() && col < attributeIDs.count(); ++col) {
            QStandardItem *item = new QStandardItem();
            item->setData(values.at(col), Qt::EditRole);
            items << item;
        }
        QStandardItem *item = new QStandardItem();
        item->setData(count, Qt::EditRole);
        items << item;
        item = new QStandardItem();   // update?
        item->setData(false,     Qt::EditRole);
        items << item;
        items << new QStandardItem(); // new value
        model->insertRow(row, items);
        ++row;
    }
    model->setHorizontalHeaderLabels(headers);

    d->tableDistinctValues->tableView()->setModel(model);
    if (d->modelDistinctValues) { d->modelDistinctValues->clear(); delete d->modelDistinctValues; }
    d->modelDistinctValues = model;
}

void BatchEditorWidget::actionUpdateValues()
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    QString levelID = ui->comboBoxLevel->currentData().toString();
    AnnotationStructureLevel *level = repository->annotationStructure()->level(levelID);
    if (!level) return;
    QString attributeID = ui->comboBoxAttributeToUpdate->currentData().toString();
    if ((!attributeID.isEmpty()) && (!level->hasAttribute(attributeID))) return;
    if (!d->modelDistinctValues) return;

    bool changesMade = false;
    int columnCount = d->modelDistinctValues->columnCount();
    for (int i = 0; i < d->modelDistinctValues->rowCount(); ++i) {
        if (d->modelDistinctValues->data(d->modelDistinctValues->index(i, columnCount - 2)).toBool()) {
            QVariant newValue = d->modelDistinctValues->data(d->modelDistinctValues->index(i, columnCount - 1));
            QList<QPair<QString, QVariant> > criteria;
            for (int j = 0; j < d->modelAttributeIDs.count(); ++j) {
                QPair<QString, QVariant> crit;
                crit.first = d->modelAttributeIDs.at(j);
                crit.second = d->modelDistinctValues->data(d->modelDistinctValues->index(i, j));
                criteria << crit;
            }
            changesMade = true;
            repository->annotations()->batchUpdate(level->ID(), attributeID, newValue, criteria);
        }
    }

    if (changesMade) {
        actionGetDistinctValues();
    }
}


