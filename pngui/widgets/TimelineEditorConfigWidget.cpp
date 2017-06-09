#include "TimelineEditorConfigWidget.h"
#include "ui_TimelineEditorConfigWidget.h"

#include <QString>
#include <QList>
#include <QPair>
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QTableView>
#include <QTreeView>
#include <QStandardItemModel>

#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

struct TimelineEditorConfigWidgetData {
    TimelineEditorConfigWidgetData() :
        modelLevelsAttributes(0), modelSpeakers(0)
    {}

    // Controls for level and attribute selection
    QMainWindow *innerwindowLevelsAttributes;
    QToolBar *toolbarLevelsAttributes;
    QAction *actionAddLevelAttribute;
    QAction *actionRemoveLevelAttribute;
    QAction *actionUpdateEditor;
    QAction *actionSaveConfiguration;
    QTableView *tableviewLevelsAttributes;

    // Controls for speaker selection
    QTreeView *treeviewSpeakers;

    // Data
    QStandardItemModel *modelSpeakers;
    QStandardItemModel *modelLevelsAttributes;
};


TimelineEditorConfigWidget::TimelineEditorConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TimelineEditorConfigWidget), d(new TimelineEditorConfigWidgetData)
{
    ui->setupUi(this);

    // Levels/Attributes: toolbar and actions
    d->toolbarLevelsAttributes = new QToolBar("Levels and Attributes", this);
    d->toolbarLevelsAttributes->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setupActions();
    // Levels and attributes selection
    d->modelLevelsAttributes = new QStandardItemModel;
    d->modelLevelsAttributes->setColumnCount(4);
    d->modelLevelsAttributes->setHorizontalHeaderLabels(QStringList() << "Level" << "Attribute" << "Function" << "Parameters");
    //addLevelAttribute("syll", "boundary");
    //addLevelAttribute("syll", "contour");

//    addLevelAttribute("tok_min", "");
//    addLevelAttribute("tok_min", "pos_min");
//    addLevelAttribute("tok_min", "disfluency");
//    addLevelAttribute("tok_mwu", "");
//    addLevelAttribute("tok_mwu", "pos_mwu");
//    addLevelAttribute("tok_mwu", "discourse");
//    addLevelAttribute("tok_mwu", "DM_discourse");
//    addLevelAttribute("tok_mwu", "DM_optionality");

//    addLevelAttribute("chunk", "");
//    addLevelAttribute("chunk", "", "group", "tok_mwu");
//    addLevelAttribute("tok_min", "liaison");
//    addLevelAttribute("tok_min", "liaison_nsyll");
//    addLevelAttribute("tok_min", "liaison_realisation");
//    addLevelAttribute("tok_min", "liaison_consonant");
//    addLevelAttribute("tok_min", "liaison_context");
//    addLevelAttribute("tok_min", "liaison_forbidden");
//    addLevelAttribute("tok_min", "", "context", "");

    QString configFilename = QCoreApplication::applicationDirPath() + "/timelineConfig.txt";
    QFile fileConfig(configFilename);
    if (fileConfig.open( QIODevice::ReadOnly | QIODevice::Text )) {
        QTextStream stream(&fileConfig);
        do {
            QString line = stream.readLine().trimmed();
            if (line.startsWith("#")) continue;
            QStringList fields = line.split("\t");
            QString levelID = (fields.count() >= 1) ? fields.at(0) : "";
            QString attributeID = (fields.count() >= 2) ? fields.at(1) : "";
            QString function = (fields.count() >= 3) ? fields.at(2) : "";
            QString parameters = (fields.count() >= 4) ? fields.at(3) : "";
            if (!levelID.isEmpty()) addLevelAttribute(levelID, attributeID, function, parameters);
        } while (!stream.atEnd());
        fileConfig.close();
    }

    d->innerwindowLevelsAttributes = new QMainWindow(this);
    d->innerwindowLevelsAttributes->addToolBar(d->toolbarLevelsAttributes);
    d->tableviewLevelsAttributes = new QTableView(this);
    d->tableviewLevelsAttributes->verticalHeader()->setDefaultSectionSize(20);
    d->tableviewLevelsAttributes->setAlternatingRowColors(true);
    d->innerwindowLevelsAttributes->setCentralWidget(d->tableviewLevelsAttributes);
    ui->gridLayoutLevelsAttributes->setMargin(0);
    ui->gridLayoutLevelsAttributes->addWidget(d->innerwindowLevelsAttributes);

    d->tableviewLevelsAttributes->setModel(d->modelLevelsAttributes);
    for (int i = 0; i < d->tableviewLevelsAttributes->horizontalHeader()->count(); ++i) {
        d->tableviewLevelsAttributes->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }

    // Speakers
    d->treeviewSpeakers = new QTreeView();
    ui->gridLayoutSpeakers->setMargin(0);
    ui->gridLayoutSpeakers->addWidget(d->treeviewSpeakers);

}

TimelineEditorConfigWidget::~TimelineEditorConfigWidget()
{
    delete ui;
    delete d;
}


void TimelineEditorConfigWidget::setupActions()
{
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    Command* command;

    // LEVELS & ATTRIBUTES TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionAddLevelAttribute = new QAction(QIcon(":/icons/actions/list_add.png"), "Add", this);
    connect(d->actionAddLevelAttribute, SIGNAL(triggered()), this, SLOT(addLevelAttribute()));
    command = ACTION_MANAGER->registerAction("Annotation.VerticalTimelineEditor.AddLevelAttribute", d->actionAddLevelAttribute, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarLevelsAttributes->addAction(d->actionAddLevelAttribute);

    d->actionRemoveLevelAttribute = new QAction(QIcon(":/icons/actions/list_remove.png"), "Remove", this);
    connect(d->actionRemoveLevelAttribute, SIGNAL(triggered()), this, SLOT(removeLevelAttribute()));
    command = ACTION_MANAGER->registerAction("Annotation.VerticalTimelineEditor.RemoveLevelAttribute", d->actionRemoveLevelAttribute, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarLevelsAttributes->addAction(d->actionRemoveLevelAttribute);

    d->actionUpdateEditor = new QAction(QIcon(":/icons/actions/view_refresh.png"), "Update editor", this);
    connect(d->actionUpdateEditor, SIGNAL(triggered()), this, SLOT(updateEditor()));
    command = ACTION_MANAGER->registerAction("Annotation.VerticalTimelineEditor.UpdateEditor", d->actionUpdateEditor, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarLevelsAttributes->addAction(d->actionUpdateEditor);

    d->actionSaveConfiguration = new QAction(QIcon(":/icons/actions/action_save.png"), "Save config", this);
    connect(d->actionSaveConfiguration, SIGNAL(triggered()), this, SLOT(saveConfiguration()));
    command = ACTION_MANAGER->registerAction("Annotation.VerticalTimelineEditor.SaveConfig", d->actionSaveConfiguration, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarLevelsAttributes->addAction(d->actionSaveConfiguration);
}

void TimelineEditorConfigWidget::addLevelAttribute(QString levelID, QString attributeID, QString function, QString parameters)
{
    int row = d->modelLevelsAttributes->rowCount();
    d->modelLevelsAttributes->setItem(row, 0, new QStandardItem(levelID));
    d->modelLevelsAttributes->setItem(row, 1, new QStandardItem(attributeID));
    d->modelLevelsAttributes->setItem(row, 2, new QStandardItem(function));
    d->modelLevelsAttributes->setItem(row, 3, new QStandardItem(parameters));
}

void TimelineEditorConfigWidget::addLevelAttribute()
{
    QList<QStandardItem *> items;
    items << new QStandardItem("") << new QStandardItem("") << new QStandardItem("") << new QStandardItem("") ;
    d->modelLevelsAttributes->insertRow(d->modelLevelsAttributes->rowCount(), items);
    d->tableviewLevelsAttributes->setCurrentIndex(d->modelLevelsAttributes->index(d->modelLevelsAttributes->rowCount() - 1, 0));
    d->tableviewLevelsAttributes->scrollToBottom();
}

void TimelineEditorConfigWidget::removeLevelAttribute()
{
    QModelIndexList selectedIndexes = d->tableviewLevelsAttributes->selectionModel()->selectedIndexes();
    QList<int> rows;
    foreach (QModelIndex index, selectedIndexes)
        rows << index.row();
    for (int i = rows.count() - 1; i >= 0; --i)
        d->modelLevelsAttributes->removeRow(rows.at(i));
}

QList<QPair<QString, QString> > TimelineEditorConfigWidget::selectedLevelsAttributes() const {
    // Translate model to columns parameter
    QList<QPair<QString, QString> > selection;
    for (int i = 0; i < d->modelLevelsAttributes->rowCount(); ++i) {
        QString levelID = d->modelLevelsAttributes->item(i, 0)->data(Qt::DisplayRole).toString();
        QString attributeID = d->modelLevelsAttributes->item(i, 1)->data(Qt::DisplayRole).toString();
        QString function = d->modelLevelsAttributes->item(i, 2)->data(Qt::DisplayRole).toString();
        QString parameters = d->modelLevelsAttributes->item(i, 3)->data(Qt::DisplayRole).toString();
        QString attr = attributeID;
        if (function == "context") attr = "_context";
        if (function == "group") attr = attr.prepend(QString("_group:%1").arg(parameters));
        selection << QPair<QString, QString>(levelID, attr);
    }
    return selection;
}

void TimelineEditorConfigWidget::updateEditor()
{
    emit selectedLevelsAttributesChanged();
}

void TimelineEditorConfigWidget::saveConfiguration()
{
    QString configFilename = QCoreApplication::applicationDirPath() + "/timelineConfig.txt";
    QFile fileConfig(configFilename);
    if ( !fileConfig.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
    QTextStream out(&fileConfig);
    out.setCodec("UTF-8");
    out << "# Praaline timeline configuration\n";
    for (int i = 0; i < d->modelLevelsAttributes->rowCount(); ++i) {
        QString levelID = d->modelLevelsAttributes->item(i, 0)->data(Qt::DisplayRole).toString();
        QString attributeID = d->modelLevelsAttributes->item(i, 1)->data(Qt::DisplayRole).toString();
        QString function = d->modelLevelsAttributes->item(i, 2)->data(Qt::DisplayRole).toString();
        QString parameters = d->modelLevelsAttributes->item(i, 3)->data(Qt::DisplayRole).toString();
        out << levelID << "\t" << attributeID << "\t" << function << "\t" << parameters << "\n";
    }
}

void TimelineEditorConfigWidget::updateSpeakerList(QStringList speakerIDs) {
    // Speakers list
    if (d->modelSpeakers) {
        disconnect(d->modelSpeakers, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(speakerSelectionChanged(QStandardItem*)));
        delete d->modelSpeakers;
    }
    d->modelSpeakers = new QStandardItemModel;
    d->modelSpeakers->setHorizontalHeaderLabels(QStringList() << "Speaker ID");
    int i = 0;
    foreach (QString speakerID, speakerIDs) {
        QStandardItem *item = new QStandardItem(speakerID);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        d->modelSpeakers->setItem(i, item);
        i++;
    }
    d->modelSpeakers->setVerticalHeaderLabels(QStringList() << "Speaker ID");
    d->treeviewSpeakers->setModel(d->modelSpeakers);
    connect(d->modelSpeakers, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(speakerSelectionChanged(QStandardItem*)));
}

void TimelineEditorConfigWidget::speakerSelectionChanged(QStandardItem *item)
{
    if (!d->modelSpeakers) return;
    QString speakerID = item->data(Qt::DisplayRole).toString();
    if (item->checkState() == Qt::Checked) {
        emit speakerAdded(speakerID);
    }
    else if (item->checkState() == Qt::Unchecked) {
        emit speakerRemoved(speakerID);
    }
}
