#include <QString>
#include <QPointer>
#include <QList>
#include <QMessageBox>
#include <QMap>
#include <QDebug>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QModelIndex>

#include "CorpusItemSelectorWidget.h"
#include "ui_CorpusItemSelectorWidget.h"

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Serialisers/XML/XMLSerialiserCorpusBookmark.h"
using namespace Praaline::Core;

#include "pngui/observers/CorpusObserver.h"
#include "pngui/model/corpus/CorpusBookmarkModel.h"

struct CorpusItemSelectorWidgetData {
    CorpusItemSelectorWidgetData() :
        corporaTopLevelNode(nullptr), observerWidgetCorpusItems(nullptr),
        innerwindowBookmarks(nullptr), toolbarBookmarks(nullptr), actionOpenBookmarks(nullptr), actionSaveBookmarks(nullptr),
        treeviewBookmarks(nullptr), modelBookmarks(nullptr)
    {}

    // Corpus
    QPointer<TreeNode> corporaTopLevelNode;
    QPointer<ObserverWidget> observerWidgetCorpusItems;

    // Bookmarks
    QMainWindow *innerwindowBookmarks;
    QToolBar *toolbarBookmarks;
    QAction *actionOpenBookmarks;
    QAction *actionSaveBookmarks;
    QTreeView *treeviewBookmarks;
    QList<CorpusBookmark *> bookmarks;
    CorpusBookmarkModel *modelBookmarks;
    QString bookmarksFilename;
};

CorpusItemSelectorWidget::CorpusItemSelectorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CorpusItemSelectorWidget), d(new CorpusItemSelectorWidgetData)
{
    ui->setupUi(this);

    // Find corpus explorer top-level node in object manager
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("Qtilities::CoreGui::TreeNode");
    foreach (QObject* obj, list) {
        TreeNode *node = qobject_cast<TreeNode *>(obj);
        if (node && node->observerName() == "Corpus Explorer") {
            d->corporaTopLevelNode = node;
        }
    }

    // Create observer widget for communications sub-tree
    d->observerWidgetCorpusItems = new ObserverWidget(Qtilities::TreeView);
    d->observerWidgetCorpusItems->setRefreshMode(ObserverWidget::RefreshModeShowTree);
    d->observerWidgetCorpusItems->setGlobalMetaType("Corpus Tree Meta Type");
    d->observerWidgetCorpusItems->setAcceptDrops(false);
    connect(d->observerWidgetCorpusItems, SIGNAL(doubleClickRequest(QObject*, Observer*)),
            this, SLOT(doubleClickRequest(QObject*, Observer*)));
    connect(d->observerWidgetCorpusItems, SIGNAL(selectedObjectsChanged(QList<QObject*>)),
            this, SLOT(selectionChanged(QList<QObject*>)));
    d->observerWidgetCorpusItems->setObserverContext(d->corporaTopLevelNode);
    d->observerWidgetCorpusItems->layout()->setMargin(0);
    d->observerWidgetCorpusItems->initialize();
    ui->gridLayoutCorpusExplorer->addWidget(d->observerWidgetCorpusItems);

    // Toolbars
    // Bookmarks toolbar
    d->toolbarBookmarks = new QToolBar("Corpus Bookmarks", this);
    d->toolbarBookmarks->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // Bookmarks
    d->innerwindowBookmarks = new QMainWindow(this);
    d->innerwindowBookmarks->addToolBar(d->toolbarBookmarks);
    d->treeviewBookmarks = new QTreeView(this);
    d->innerwindowBookmarks->setCentralWidget(d->treeviewBookmarks);
    ui->gridLayoutBookmarks->setMargin(0);
    ui->gridLayoutBookmarks->addWidget(d->innerwindowBookmarks);
    connect(d->treeviewBookmarks, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(bookmarkSelected(QModelIndex)));

    setupActions();
}

CorpusItemSelectorWidget::~CorpusItemSelectorWidget()
{
    delete ui;
    delete d;
}

void CorpusItemSelectorWidget::setupActions()
{
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    Command* command;

    // BOOKMARKS TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionOpenBookmarks = new QAction(QIcon(":/icons/actions/action_open.png"), "Open Bookmarks", this);
    connect(d->actionOpenBookmarks, SIGNAL(triggered()), this, SLOT(openBookmarks()));
    command = ACTION_MANAGER->registerAction("CorpusItemsSelector.OpenBookmarks", d->actionOpenBookmarks, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarBookmarks->addAction(d->actionOpenBookmarks);

    d->actionSaveBookmarks = new QAction(QIcon(":/icons/actions/action_save.png"), "Save Bookmarks", this);
    connect(d->actionSaveBookmarks, SIGNAL(triggered()), this, SLOT(saveBookmarks()));
    command = ACTION_MANAGER->registerAction("CorpusItemsSelector.SaveBookmarks", d->actionSaveBookmarks, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarBookmarks->addAction(d->actionSaveBookmarks);
}

// private
Corpus *CorpusItemSelectorWidget::findCorpus(QString corpusID)
{
    QList<QObject *> listCorpusObservers;
    listCorpusObservers = OBJECT_MANAGER->registeredInterfaces("CorpusObserver");
    foreach (QObject* obj, listCorpusObservers) {
        CorpusObserver *obs = qobject_cast<CorpusObserver *>(obj);
        if (obs && obs->nodeRepository()) {
            foreach (QObject *child, obs->nodeRepository()->treeChildren()) {
                CorpusExplorerTreeNodeCorpus *node = qobject_cast<CorpusExplorerTreeNodeCorpus *>(child);
                if (node && node->corpus() && node->corpus()->ID() == corpusID) return node->corpus();
            }
        }
    }
    return nullptr;
}

void CorpusItemSelectorWidget::doubleClickRequest(QObject* object, Observer* parent_observer)
{
    Q_UNUSED(parent_observer)

    QPointer<Corpus> corpus(nullptr);
    QPointer<CorpusCommunication> com(nullptr);
    QPointer<CorpusRecording> rec(nullptr);
    QPointer<CorpusAnnotation> annot(nullptr);

    qDebug() << object;
    CorpusExplorerTreeNodeCommunication *nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(object);
    if (nodeCom && nodeCom->communication) {
        corpus = nodeCom->communication->corpus();
        if (!corpus) return;
        com = nodeCom->communication;
        if (!com) return;
        emit selectedCorpusCommunication(corpus, com);
        return;
    }
    CorpusExplorerTreeNodeRecording *nodeRec = qobject_cast<CorpusExplorerTreeNodeRecording *>(object);
    if (nodeRec && nodeRec->recording) {
        corpus = nodeRec->recording->corpus();
        if (!corpus) return;
        com = corpus->communication(nodeRec->recording->communicationID());
        if (!com) return;
        rec = nodeRec->recording;
        if (!rec) return;
        emit selectedCorpusRecording(corpus, com, rec);
        return;
    }
    CorpusExplorerTreeNodeAnnotation *nodeAnnot = qobject_cast<CorpusExplorerTreeNodeAnnotation *>(object);
    if (nodeAnnot && nodeAnnot->annotation) {
        corpus = nodeAnnot->annotation->corpus();
        if (!corpus) return;
        com = corpus->communication(nodeAnnot->annotation->communicationID());
        if (!com) return;
        annot = nodeAnnot->annotation;
        if (!annot) return;
        emit selectedCorpusAnnotation(corpus, com, annot);
    }
}


void CorpusItemSelectorWidget::selectionChanged(QList<QObject*> selected)
{
    QPointer<Corpus> corpus(nullptr);
    QPointer<CorpusCommunication> com(nullptr);
    QPointer<CorpusRecording> rec(nullptr);
    QPointer<CorpusAnnotation> annot(nullptr);

    if (selected.isEmpty()) return;

    QObject *object = selected.first();
    CorpusExplorerTreeNodeCommunication *nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(object);
    if (nodeCom && nodeCom->communication) {
        corpus = nodeCom->communication->corpus();
        if (!corpus) return;
        com = nodeCom->communication;
        if (!com) return;
        emit selectedCorpusCommunication(corpus, com);
        return;
    }
    CorpusExplorerTreeNodeRecording *nodeRec = qobject_cast<CorpusExplorerTreeNodeRecording *>(object);
    if (nodeRec && nodeRec->recording) {
        corpus = nodeRec->recording->corpus();
        if (!corpus) return;
        com = corpus->communication(nodeRec->recording->communicationID());
        if (!com) return;
        rec = nodeRec->recording;
        if (!rec) return;
        emit selectedCorpusRecording(corpus, com, rec);
        return;
    }
    CorpusExplorerTreeNodeAnnotation *nodeAnnot = qobject_cast<CorpusExplorerTreeNodeAnnotation *>(object);
    if (nodeAnnot && nodeAnnot->annotation) {
        corpus = nodeAnnot->annotation->corpus();
        if (!corpus) return;
        com = corpus->communication(nodeAnnot->annotation->communicationID());
        if (!com) return;
        annot = nodeAnnot->annotation;
        if (!annot) return;
        emit selectedCorpusAnnotation(corpus, com, annot);
    }
}

void CorpusItemSelectorWidget::openBookmarks()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Bookmarks"), "",
                                                    tr("Praaline Bookmarks file (*.xml);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    QList<CorpusBookmark *> newBookmarks;
    bool result = XMLSerialiserCorpusBookmark::loadCorpusBookmarks(newBookmarks, filename);
    if (!result) {
        QMessageBox::warning(this, "Cannot open Bookmarks",
                             QString("Cannot open bookmarks file (%1). Is it a valid Praaline XML bookmarks file?").arg(filename), QMessageBox::Ok);
        return;
    }
    if (d->modelBookmarks) delete d->modelBookmarks;
    qDeleteAll(d->bookmarks);
    d->bookmarks = newBookmarks;
    d->modelBookmarks = new CorpusBookmarkModel(d->bookmarks, this);
    d->treeviewBookmarks->setModel(d->modelBookmarks);
    d->bookmarksFilename = filename;
}

void CorpusItemSelectorWidget::saveBookmarks()
{
    if (d->bookmarksFilename.isEmpty())
        saveBookmarksAs();
    else
        XMLSerialiserCorpusBookmark::saveCorpusBookmarks(d->bookmarks, d->bookmarksFilename);
}


void CorpusItemSelectorWidget::saveBookmarksAs()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Bookmarks As..."), d->bookmarksFilename,
                                                    tr("Praaline Bookmarks File (*.xml);;All Files (*)"), &selectedFilter, options);
    if (filename.isEmpty()) return;
    bool result = XMLSerialiserCorpusBookmark::saveCorpusBookmarks(d->bookmarks, filename);
    if (result) d->bookmarksFilename = filename;
}

void CorpusItemSelectorWidget::bookmarkSelected(QModelIndex index)
{
    if (!d->modelBookmarks) return;
    int bookmarkRow = index.row();
    QString corpusID = d->modelBookmarks->data(d->modelBookmarks->index(bookmarkRow, 0), Qt::DisplayRole).toString();
    QString communicationID = d->modelBookmarks->data(d->modelBookmarks->index(bookmarkRow, 1), Qt::DisplayRole).toString();
    QString annotationID = d->modelBookmarks->data(d->modelBookmarks->index(bookmarkRow, 2), Qt::DisplayRole).toString();
    RealTime time = RealTime::fromSeconds(d->modelBookmarks->data(d->modelBookmarks->index(bookmarkRow, 3), Qt::DisplayRole).toDouble());

    QPointer<Corpus> corpus = findCorpus(corpusID);
    if (!corpus) return;
    CorpusCommunication *com = corpus->communication(communicationID);
    if (!com) return;
    CorpusAnnotation *annot = com->annotation(annotationID);
    if (!annot) return;

    emit moveToAnnotationTime(corpus, com, annot, time);
}

// public slot
void CorpusItemSelectorWidget::addBookmark(const QString &corpusID, const QString &communicationID, const QString &annotationID,
                                           const RealTime &timeStart, const RealTime &timeEnd, const QString &name)
{
    Q_UNUSED(corpusID)
    Q_UNUSED(communicationID)
    Q_UNUSED(annotationID)
    Q_UNUSED(timeStart)
    Q_UNUSED(timeEnd)
    Q_UNUSED(name)
}

