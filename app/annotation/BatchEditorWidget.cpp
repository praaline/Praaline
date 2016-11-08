#include <QVariant>
#include <QPair>
#include <QStandardItemModel>
#include <QDebug>
#include "BatchEditorWidget.h"
#include "ui_BatchEditorWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/AnnotationTier.h"
using namespace Praaline::Core;

#include "pngui/observers/corpusobserver.h"
#include "pngui/widgets/gridviewwidget.h"
#include "CorporaManager.h"

struct BatchEditWidgetData {
    BatchEditWidgetData() : corporaManager(0), tableDistinctValues(0), modelDistinctValues(0) {}

    CorporaManager *corporaManager;
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
    list = OBJECT_MANAGER->registeredInterfaces("CorporaManager");
    foreach (QObject* obj, list) {
        CorporaManager *manager = qobject_cast<CorporaManager *>(obj);
        if (manager) d->corporaManager = manager;
    }
    connect(d->corporaManager, SIGNAL(activeCorpusChanged(QString)), this, SLOT(activeCorpusChanged(QString)));
    connect(ui->comboBoxLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(levelChanged(QString)));

    // Manual selection model - table
    d->tableDistinctValues = new GridViewWidget(this);
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

void BatchEditorWidget::activeCorpusChanged(const QString &corpusID)
{
    Q_UNUSED(corpusID)
    ui->comboBoxLevel->clear();
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    foreach (AnnotationStructureLevel *level, corpus->annotationStructure()->levels()) {
        ui->comboBoxLevel->addItem(level->name(), level->ID());
    }
    levelChanged("");
}

void BatchEditorWidget::levelChanged(const QString &text)
{
    Q_UNUSED(text)
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QString levelID = ui->comboBoxLevel->currentData().toString();
    AnnotationStructureLevel *level = corpus->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxAttributes->clear();
    ui->comboBoxAttributes->insertItem(0, "(text)", true);
    int i = 1;
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxAttributes->insertItem(i, attribute->name(), false);
        i++;
    }
    ui->comboBoxAttributeToUpdate->clear();
    ui->comboBoxAttributeToUpdate->addItem("(text)", "");
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxAttributeToUpdate->addItem(attribute->name(), attribute->ID());
    }
}

// ====================================================================================================================

void BatchEditorWidget::actionGetDistinctValues()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QString levelID = ui->comboBoxLevel->currentData().toString();
    AnnotationStructureLevel *level = corpus->annotationStructure()->level(levelID);
    if (!level) return;
    QStringList attributeIDs;
    QStringList headers;
    d->modelAttributeIDs.clear();
    for (int i = 0; i < ui->comboBoxAttributes->count(); ++i) {
        if (ui->comboBoxAttributes->itemData(i).toBool() == true) {
            if (ui->comboBoxAttributes->itemText(i) == "(text)") {
                attributeIDs << "";
                headers << "(text)";
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
    headers << "Count" << "Update?" << "New Value";

    QList<QPair<QList<QVariant>, long> > data = corpus->datastoreAnnotations()->getDistinctLabels(levelID, attributeIDs);
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
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QString levelID = ui->comboBoxLevel->currentData().toString();
    AnnotationStructureLevel *level = corpus->annotationStructure()->level(levelID);
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
            corpus->datastoreAnnotations()->batchUpdate(level->ID(), attributeID, newValue, criteria);
        }
    }

    if (changesMade) {
        actionGetDistinctValues();
    }
}


