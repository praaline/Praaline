#include <QObject>
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include "model/annotation/AnnotationTierModel.h"
#include "AnnotationTimelineEditor.h"
#include "ui_annotationtimelineeditor.h"

#include "grid/qadvancedtableview.h"
#include "grid/qadvancedheaderview.h"
#include "grid/qconditionaldecorationdialog.h"
#include "grid/qconditionaldecorationproxymodel.h"
#include "grid/qfiltermodel.h"
#include "grid/qfiltermodelproxy.h"
#include "grid/qfilterviewconnector.h"
#include "grid/qgroupingproxymodel.h"
#include "grid/qconditionaldecoration.h"
#include "grid/qfixedrowstableview.h"
#include "grid/qmimedatautil.h"
#include "grid/qrangefilter.h"
#include "grid/quniquevaluesproxymodel.h"
#include "grid/qselectionlistfilter.h"
#include "grid/qtextfilter.h"
#include "grid/qvaluefilter.h"

#include "GridViewWidget.h"

using namespace Praaline::Core;


struct AnnotationTimelineEditorData {
    AnnotationTimelineEditorData() :
        orientation(Qt::Vertical), model(0), view(0)
    {}

    Qt::Orientation orientation;
    QMap<QString, QPointer<AnnotationTierGroup> > tiers;
    QList<QPair<QString, QString> > columns;
    AnnotationTierModel *model;

    GridViewWidget *view;
    QList<int> selectedRows;
};

AnnotationTimelineEditor::AnnotationTimelineEditor(QWidget *parent) :
    QWidget(parent),
    d(new AnnotationTimelineEditorData),
    ui(new Ui::AnnotationTimelineEditor)
{
    ui->setupUi(this);
    d->view = new GridViewWidget(this);
    ui->gridLayout->addWidget(d->view);
    // make grid a little tighter
    d->view->tableView()->verticalHeader()->setDefaultSectionSize(20);
    d->view->tableView()->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // StretchLastSection(true);
    // d->view->tableView()->setItemDelegateForColumn(1);
}

AnnotationTimelineEditor::~AnnotationTimelineEditor()
{
    delete ui;
    if (d->model) delete d->model;
}

void AnnotationTimelineEditor::annotationsSplit(const RealTime &time)
{
    if (!d->model) return;
    d->model->splitAnnotations(d->view->tableView()->selectionModel()->currentIndex(), time);
}

void AnnotationTimelineEditor::annotationsSplit()
{
    if (!d->model) return;
    d->model->splitAnnotations(d->view->tableView()->selectionModel()->currentIndex());
}

void AnnotationTimelineEditor::annotationsMerge()
{
    if (!d->model) return;
    d->model->mergeAnnotations(d->view->tableView()->selectionModel()->currentIndex().column(), d->selectedRows);
}

void AnnotationTimelineEditor::removeSorting()
{
    if (!d->model) return;
    d->view->tableView()->sortByColumn(-1, Qt::AscendingOrder);
}

void AnnotationTimelineEditor::setData(QMap<QString, QPointer<AnnotationTierGroup> > &tierGroups,
                                       const QList<QPair<QString, QString> > &columns)
{
    if (d->model) { delete d->model; d->model = 0; }
    d->tiers = tierGroups;
    d->columns = columns;
    initModel();
    initAdvancedTableView();
}

void AnnotationTimelineEditor::addTierGroup(QString speakerID, QPointer<AnnotationTierGroup> tierGroup)
{
    if (d->model) { delete d->model; d->model = 0; }
    d->tiers.insert(speakerID, tierGroup);
    initModel();
    initAdvancedTableView();
}

void AnnotationTimelineEditor::removeTierGroup(QString speakerID)
{
    if (!d->tiers.contains(speakerID)) return;
    if (d->model) { delete d->model; d->model = 0; }
    d->tiers.remove(speakerID);
    initModel();
    initAdvancedTableView();
}

const QMap<QString, QPointer<AnnotationTierGroup> > & AnnotationTimelineEditor::tierGroups() const
{
    return d->tiers;
}

void AnnotationTimelineEditor::setColumns(const QList<QPair<QString, QString> > &columns)
{
    if (d->model) { delete d->model; d->model = 0; }
    d->columns = columns;
    initModel();
    initAdvancedTableView();
}

const QList<QPair<QString, QString> > & AnnotationTimelineEditor::columns() const
{
    return d->columns;
}

AnnotationTierModel *AnnotationTimelineEditor::model()
{
    return d->model;
}

void AnnotationTimelineEditor::initModel()
{
    if (d->columns.isEmpty()) return;
    if (d->tiers.isEmpty()) return;
    QString minimalLevel = d->columns.first().first;
    d->model = new AnnotationTierModel(d->tiers, minimalLevel, d->columns, d->orientation, this);
}

void AnnotationTimelineEditor::initAdvancedTableView()
{
    if (!d->model) return;
    connect(d->view->tableView()->filterProxyModel(), SIGNAL(resultCountChanged(int,int)),
            this, SLOT(resultChanged(int,int)));
    connect(d->view->tableView()->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(selectionChanged(QItemSelection, QItemSelection)));
    connect(d->view->tableView()->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentChanged(QModelIndex,QModelIndex)));

    SelectionListDataProviderProxy* p = new SelectionListDataProviderProxy(this);
    p->setSourceModel(d->model);
    d->view->tableView()->setModel(p);
    d->view->tableView()->horizontalHeader()->setSectionsClickable(true);
    d->view->tableView()->setDefaultFilterType(0, QTextFilter::Type);    
    updateGeometry();
}

Qt::Orientation AnnotationTimelineEditor::orientation() const
{
    return d->orientation;
}

void AnnotationTimelineEditor::setOrientation(Qt::Orientation orientation)
{
    if (d->orientation == orientation) return;
    // update interal state and views
    d->orientation = orientation;
    if (orientation == Qt::Horizontal) {
        d->view->tableView()->setShowFilter(false);
        d->view->tableView()->setSortingEnabled(false);
    } else {
        d->view->tableView()->setShowFilter(true);
        d->view->tableView()->setSortingEnabled(true);
    }
    // update model, if available
    if (!d->model) return;
    d->model->setOrientation(orientation);
}

void AnnotationTimelineEditor::toggleOrientation()
{
    // this is a public slot
    if (d->orientation == Qt::Vertical)
        setOrientation(Qt::Horizontal);
    else
        setOrientation(Qt::Vertical);
}

bool AnnotationTimelineEditor::exportToTabSeparated(const QString &filename)
{
    return d->view->exportToTabSeparated(filename);
}

void AnnotationTimelineEditor::exportToTabSeparated()
{
    QString filename;
    if (filename.isEmpty()) {
        QFileDialog::Options options;
        QString selectedFilter;
        filename = QFileDialog::getSaveFileName(this, tr("Export to tab-separated file"),
                                                "praaline_annotations.txt", tr("Text File (*.txt);;All Files (*)"),
                                                &selectedFilter, options);
    }
    if (filename.isEmpty()) return;
    exportToTabSeparated(filename);
}

void AnnotationTimelineEditor::resultChanged(int filterRows, int unfilteredRows)
{
    if (d->orientation == Qt::Vertical) {
        if (unfilteredRows - filterRows > 0)
            d->view->setResultsLabelText(tr("Result: %1 of %2").arg(filterRows).arg(unfilteredRows));
        else
            d->view->setResultsLabelText(tr("All intervals: %1").arg(filterRows));
    } else {
        d->view->setResultsLabelText("");
    }
}

void AnnotationTimelineEditor::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    QList<int> rowsSelected;
    foreach (QModelIndex index, d->view->tableView()->selectionModel()->selectedIndexes()) {
        if (!rowsSelected.contains(index.row()))
            rowsSelected << index.row();
    }
    d->selectedRows = rowsSelected;
    emit selectedRowsChanged(rowsSelected);
}

void AnnotationTimelineEditor::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit currentIndexChanged(current, previous);
}

void AnnotationTimelineEditor::contextMenuRequested(const QPoint & point)
{

}

RealTime AnnotationTimelineEditor::currentTime() const
{
    int index = -1;
    if (d->model->orientation() == Qt::Vertical) {
        index = d->view->tableView()->currentIndex().row();
    } else {
        index = d->view->tableView()->currentIndex().column();
    }
    return d->model->timeAtTimelineIndex(index);
}

void AnnotationTimelineEditor::moveToTime(const RealTime &time)
{
    int index = d->model->timelineIndexAtTime(time);
    if (d->model->orientation() == Qt::Vertical)
        moveToRow(index);
    else
        moveToColumn(index);
}

void AnnotationTimelineEditor::moveToTime(const RealTime &time, double &tMin_msec, double &tMax_msec)
{
    int index = d->model->timelineIndexAtTime(time, tMin_msec, tMax_msec);
    if (d->model->orientation() == Qt::Vertical)
        moveToRow(index);
    else
        moveToColumn(index);
}

void AnnotationTimelineEditor::moveToRow(int row)
{
    if (d->model->rowCount() == 0) return;
    int height = d->view->tableView()->datatableHeight();
    int rh =  height / d->view->tableView()->rowHeight(0);
    int pos = d->view->tableView()->rowViewportPosition(row);
    int padding = rh * 5;
    if (pos < padding || pos >= height - padding) {
        int topRow = row + rh/5;
        if (topRow < 0) topRow = 0;
        d->view->tableView()->scrollTo(d->view->tableView()->filterProxyModel()->index(topRow, 0));
    }
    bool haveRowSelected = false;
    for (int i = 0; i < d->model->columnCount(); ++i) {
        if (d->view->tableView()->selectionModel()->isSelected(d->model->index(row, i))) {
            haveRowSelected = true;
            break;
        }
    }
    if (!haveRowSelected) {
        d->view->tableView()->selectionModel()->setCurrentIndex
                (d->model->index(row, 0),
                 QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}

void AnnotationTimelineEditor::moveToColumn(int column)
{
    int width = d->view->tableView()->datatableWidth();
    int cw =  width / d->view->tableView()->columnWidth(0);
    int pos = d->view->tableView()->columnViewportPosition(column);
    int padding = width / 5;
    if (pos < padding || pos >= width - padding) {
        int leftColumn = column + 4;
        if (leftColumn < 0) leftColumn = 0;
        d->view->tableView()->scrollTo(d->view->tableView()->filterProxyModel()->index(0, leftColumn));
    }
    bool haveColumnSelected = false;
    for (int i = 0; i < d->model->rowCount(); ++i) {
        if (d->view->tableView()->selectionModel()->isSelected(d->model->index(i, column))) {
            haveColumnSelected = true;
            break;
        }
    }
    if (!haveColumnSelected) {
        d->view->tableView()->selectionModel()->setCurrentIndex
                (d->model->index(0, column),
                 QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Columns);
    }
}

