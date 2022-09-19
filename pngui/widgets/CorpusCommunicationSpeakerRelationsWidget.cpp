#include <QString>
#include <QPointer>
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QTableView>
#include <QHeaderView>

#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusCommunicationSpeakerRelation.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/SpeakerRelationTableModel.h"
#include "PraalineUserInterfaceOptions.h"

#include "CorpusCommunicationSpeakerRelationsWidget.h"
#include "ui_CorpusCommunicationSpeakerRelationsWidget.h"

struct CorpusCommunicationSpeakerRelationsWidgetData
{
    QPointer<CorpusCommunication> communication;

    QMainWindow *innerWindow;
    QToolBar *toolbar;
    QAction *actionAddSpeakerRelation;
    QAction *actionRemoveSpeakerRelation;

    QTableView *tableviewSpeakerRelations;

    SpeakerRelationTableModel *modelSpeakerRelations;

    CorpusCommunicationSpeakerRelationsWidgetData() :
        communication(nullptr), innerWindow(nullptr), toolbar(nullptr),
        actionAddSpeakerRelation(nullptr), actionRemoveSpeakerRelation(nullptr),
        tableviewSpeakerRelations(nullptr), modelSpeakerRelations(nullptr)
    {}
};


CorpusCommunicationSpeakerRelationsWidget::CorpusCommunicationSpeakerRelationsWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::CorpusCommunicationSpeakerRelationsWidget),
    d(new CorpusCommunicationSpeakerRelationsWidgetData())
{
    ui->setupUi(this);

    // Levels/Attributes: toolbar and actions
    d->toolbar = new QToolBar("Levels and Attributes", this);
    d->toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbar->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    setupActions();

    d->innerWindow = new QMainWindow(this);
    d->innerWindow->addToolBar(d->toolbar);
    d->tableviewSpeakerRelations = new QTableView(this);
    d->tableviewSpeakerRelations->setAlternatingRowColors(true);
    d->innerWindow->setCentralWidget(d->tableviewSpeakerRelations);
    ui->gridLayoutSpeakerRelations->setMargin(0);
    ui->gridLayoutSpeakerRelations->addWidget(d->innerWindow);
}

CorpusCommunicationSpeakerRelationsWidget::~CorpusCommunicationSpeakerRelationsWidget()
{
    delete d;
    delete ui;
}


void CorpusCommunicationSpeakerRelationsWidget::setupActions()
{
    if (!d->toolbar) return;

    d->actionAddSpeakerRelation = new QAction(QIcon(":icons/actions/list_add.png"), "Add", this);
    connect(d->actionAddSpeakerRelation, &QAction::triggered, this, &CorpusCommunicationSpeakerRelationsWidget::addSpeakerRelation);
    d->toolbar->addAction(d->actionAddSpeakerRelation);

    d->actionRemoveSpeakerRelation = new QAction(QIcon(":icons/actions/list_remove.png"), "Remove", this);
    connect(d->actionRemoveSpeakerRelation, &QAction::triggered, this, &CorpusCommunicationSpeakerRelationsWidget::removeSpeakerRelation);
    d->toolbar->addAction(d->actionRemoveSpeakerRelation);
}

void CorpusCommunicationSpeakerRelationsWidget::clear()
{
    // Clear state
    d->communication = nullptr;
    // Reset model
    if (d->modelSpeakerRelations) {
        d->tableviewSpeakerRelations->setModel(nullptr);
        delete d->modelSpeakerRelations;
        d->modelSpeakerRelations = nullptr;
    }
}

void CorpusCommunicationSpeakerRelationsWidget::openCommunication(Praaline::Core::CorpusCommunication *com)
{
    if (d->communication == com) return;
    // Clear state
    clear();
    // Check that this is a valid communication
    if (!com) return;
    // Set model
    d->modelSpeakerRelations = new SpeakerRelationTableModel(com);
    d->tableviewSpeakerRelations->setModel(d->modelSpeakerRelations);
}

void CorpusCommunicationSpeakerRelationsWidget::addSpeakerRelation()
{
    if (!d->modelSpeakerRelations) return;
    d->modelSpeakerRelations->insertRow(d->modelSpeakerRelations->rowCount());
}

void CorpusCommunicationSpeakerRelationsWidget::removeSpeakerRelation()
{
    if (!d->modelSpeakerRelations) return;
    QModelIndexList selectedIndexes = d->tableviewSpeakerRelations->selectionModel()->selectedIndexes();
    QList<int> rows;
    foreach (QModelIndex index, selectedIndexes)
        rows << index.row();
    for (int i = rows.count() - 1; i >= 0; --i)
        d->modelSpeakerRelations->removeRow(rows.at(i));
}

