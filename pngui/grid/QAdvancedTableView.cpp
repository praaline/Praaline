/******************************************************************************
** This file is part of qadvanceditemviews.
**
** Copyright (c) 2011-2012 Martin Hoppe martin@2x2hoppe.de
**
** qadvanceditemviews is free software: you can redistribute it
** and/or modify it under the terms of the GNU Lesser General
** Public License as published by the Free Software Foundation,
** either version 3 of the License, or (at your option) any
** later version.
**
** qadvanceditemviews is distributed in the hope that it will be
** useful, but WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
** See the GNU General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with qadvanceditemviews.
** If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <QContextMenuEvent>
#include <QHeaderView>
#include <QScrollBar>
#include <QTableView>
#include <QTimer>

#include "QAdvancedTableView.h"
#include "ui_QAdvancedTableView.h"

#include "QAdvancedHeaderView.h"
#include "QAdvancedTableView_p.h"

#include "QAbstractFilterProxyModel.h"
#include "QFilterModelProxy.h"
#include "QFilterModel.h"
#include "QFilterViewItemDelegate.h"
#include "QFixedRowsHeaderView.h"
#include "QFixedRowsTableView.h"
#include "QSharedItemSelectionModel.h"
#include "QTextFilter.h"

#define V_CALL(_m_) ui->dataTableView->_m_; \
    ui->fixedRowsTableView->_m_; \
    ui->splittedDataTableView->_m_;


class QAdvancedTableViewPrivate
{
public:
    QAdvancedTableViewPrivate(QAdvancedTableView* tv)
    {
        autoResizeRowsToContents = false;
        autoResizeColumnsToFitView = false;
        defaultFilterType = QTextFilter::Type;
        dataViewProxy = new QFilterModelProxy(tv);
        filterModel = nullptr;
        model = nullptr;
        horizontalHeader = nullptr;
        horizontalScrollBarPolicy = Qt::ScrollBarAsNeeded;
        verticalHeader = nullptr;
        splittedViewSelectionModel = nullptr;
        v = tv;
    }

    ~QAdvancedTableViewPrivate()
    {
    }

    bool autoResizeRowsToContents;
    bool autoResizeColumnsToFitView;
    int defaultFilterType;
    QMap<int, int> columnSpareWidthParts;
    QAbstractFilterProxyModel* dataViewProxy;
    QAbstractFilterModel* filterModel;
    QAbstractItemModel* model;
    QAdvancedHeaderView* horizontalHeader;
    Qt::ScrollBarPolicy horizontalScrollBarPolicy;
    QAdvancedHeaderView* verticalHeader;
    QSharedItemSelectionModel* splittedViewSelectionModel;

    QAdvancedTableView* v;
};


//-----------------------------------------------
// class QAdvancedTableView
//-----------------------------------------------

QAdvancedTableView::QAdvancedTableView(QWidget *parent) :
    QWidget(parent), ui(new Ui::QAdvancedTableView)
{
    d = new QAdvancedTableViewPrivate(this);
    ui->setupUi(this);
    //
    ui->splittedDataTableView->hide();

    // Create header view (model) proxy
    setFilterModel(new QFilterModel(this));
    //d->filterModel = new QFilterModel(this);
    // Create horizontal header view
    d->horizontalHeader = new QAdvancedHeaderView(Qt::Horizontal, this);
    ui->headerTableView->setHorizontalHeader(d->horizontalHeader);
    ui->headerTableView->setItemDelegate(new QFilterViewItemDelegate(this));
    ui->headerTableView->horizontalHeader()->setSectionsMovable(true);
    // Create vertical header views
    d->verticalHeader = new QAdvancedHeaderView(Qt::Vertical, this);
    ui->headerTableView->setVerticalHeader(d->verticalHeader);
    ui->headerTableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // ui->dataTableView->setVerticalHeader(new QFixedRowsHeaderView(Qt::Vertical, this));
    // set models
    d->dataViewProxy->setSourceModel(ui->fixedRowsTableView->decorationProxy());
    ui->dataTableView->setModel(d->dataViewProxy);
    ui->splittedDataTableView->setModel(d->dataViewProxy);

    // set selection models
    ui->splittedDataTableView->setSelectionModel(new QSharedItemSelectionModel(ui->splittedDataTableView->model(), ui->dataTableView->selectionModel(), this));
    ui->fixedRowsTableView->setSelectionModel(new QSharedItemSelectionModel(ui->fixedRowsTableView->model(), ui->dataTableView->selectionModel(), this));

    // data table view
    connect(ui->dataTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &QAdvancedTableView::verticalHeaderSectionClicked);
    connect(ui->dataTableView->horizontalScrollBar(), &QAbstractSlider::sliderMoved, this, &QAdvancedTableView::dataViewHorizontalScrollBarSilderMoved);
    connect(ui->dataTableView->horizontalScrollBar(), &QAbstractSlider::rangeChanged, this, &QAdvancedTableView::dataViewHorizontalScrollBarRangeChanged);
    connect(ui->dataTableView->horizontalScrollBar(), &QAbstractSlider::valueChanged, this, &QAdvancedTableView::dataViewHorizontalScrollBarValueChanged);
    connect(ui->dataTableView->verticalScrollBar(), &QAbstractSlider::rangeChanged, this, &QAdvancedTableView::updateHeaderViewVerticalScrollBar);
    connect(ui->dataTableView->verticalHeader(), &QHeaderView::sectionResized, this, &QAdvancedTableView::verticalHeaderSectionResized);
    connect(ui->dataTableView, &QAdvancedTableViewProxy::focusReceived, this, &QAdvancedTableView::subviewReceivedFocus);

    // fixed rows view
    connect(ui->fixedRowsTableView, &QFixedRowsTableView::focusReceived, this, &QAdvancedTableView::subviewReceivedFocus);

    // splitted data table view view
    connect(ui->splittedDataTableView, &QAdvancedTableViewProxy::focusReceived, this, &QAdvancedTableView::subviewReceivedFocus);
    connect(ui->splittedDataTableView->verticalHeader(), &QHeaderView::sectionResized, this, &QAdvancedTableView::verticalHeaderSectionResized);
    connect(ui->splittedDataTableView->verticalHeader(), &QHeaderView::sectionClicked, this, &QAdvancedTableView::verticalHeaderSectionClicked);

    // filter model
    connect(d->filterModel, &QAbstractItemModel::modelReset, this, &QAdvancedTableView::updateHeaderViewGeometries);
    connect(d->filterModel, &QAbstractItemModel::rowsRemoved, this, &QAdvancedTableView::updateHeaderViewGeometries);

    // data view proxy
    connect(d->dataViewProxy, &QAbstractItemModel::modelReset, this, &QAdvancedTableView::modelReset);
    connect(d->dataViewProxy, &QAbstractItemModel::layoutChanged, this, &QAdvancedTableView::dataModelLayoutChanged);

    // header table view
    connect(ui->headerTableView, &QFilterView::cornerButtonClicked, this, &QAdvancedTableView::selectAll);
    connect(ui->headerTableView, &QFilterView::calcGeometryRequested, this, &QAdvancedTableView::updateHeaderViewGeometries);
    connect(ui->headerTableView, &QFilterView::visibilityChanged, this, &QAdvancedTableView::updateHeaderViewGeometries);
    connect(ui->headerTableView, &QFilterView::focusReceived, this, &QAdvancedTableView::subviewReceivedFocus);
    //
    connect(ui->headerTableView->model(), &QAbstractItemModel::rowsInserted, this, &QAdvancedTableView::filterAdded);
    connect(ui->headerTableView->horizontalHeader(), &QHeaderView::sectionResized, this, &QAdvancedTableView::headerViewSectionResized);
    connect(ui->headerTableView->horizontalHeader(), &QHeaderView::sectionMoved, this, &QAdvancedTableView::horizontalHeaderViewSectionMoved);
    connect(ui->headerTableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &QAdvancedTableView::horizontalHeaderSortIndicatorChanged);
    connect(ui->headerTableView->horizontalScrollBar(), &QAbstractSlider::rangeChanged, this, &QAdvancedTableView::headerViewHorizontalScrollBarRangeChanged);
    connect(ui->headerTableView->horizontalScrollBar(), &QAbstractSlider::sliderMoved, this, &QAdvancedTableView::headerViewHorizontalScrollBarSilderMoved);
    connect(ui->headerTableView->horizontalScrollBar(), &QAbstractSlider::valueChanged, this, &QAdvancedTableView::headerViewHorizontalScrollBarValueChanged);

    //connect(ui->headerTableView, SIGNAL(verticalHeaderWidthChangeRequested(int)), this, SLOT(verticalHeaderWidthChangeRequested(int)));
    // Forward data view signals
    connect(ui->dataTableView, &QAbstractItemView::activated, this, &QAdvancedTableView::viewActivated);
    connect(ui->dataTableView, &QAbstractItemView::clicked, this, &QAdvancedTableView::viewClicked);
    connect(ui->dataTableView, &QWidget::customContextMenuRequested, this, &QWidget::customContextMenuRequested);
    connect(ui->dataTableView, &QAbstractItemView::doubleClicked, this, &QAdvancedTableView::viewDoubleClicked);
    connect(ui->dataTableView, &QAbstractItemView::entered, this, &QAdvancedTableView::viewEntered);
    connect(ui->dataTableView, &QAbstractItemView::pressed, this, &QAdvancedTableView::viewPressed);
    connect(ui->dataTableView, &QAbstractItemView::viewportEntered, this, &QAdvancedTableView::viewportEntered);

    // Forward fixed rows view signals
    connect(ui->fixedRowsTableView, &QAbstractItemView::activated, this, &QAdvancedTableView::viewActivated);
    connect(ui->fixedRowsTableView, &QAbstractItemView::clicked, this, &QAdvancedTableView::viewClicked);
    connect(ui->fixedRowsTableView, &QWidget::customContextMenuRequested, this, &QWidget::customContextMenuRequested);
    connect(ui->fixedRowsTableView, &QAbstractItemView::doubleClicked, this, &QAdvancedTableView::viewDoubleClicked);
    connect(ui->fixedRowsTableView, &QAbstractItemView::entered, this, &QAdvancedTableView::viewEntered);
    connect(ui->fixedRowsTableView, &QAbstractItemView::pressed, this, &QAdvancedTableView::viewPressed);
    connect(ui->fixedRowsTableView, &QAbstractItemView::viewportEntered, this, &QAdvancedTableView::viewportEntered);

    // Forward splitted data table view signals
    connect(ui->splittedDataTableView, &QAbstractItemView::activated, this, &QAdvancedTableView::viewActivated);
    connect(ui->splittedDataTableView, &QAbstractItemView::clicked, this, &QAdvancedTableView::viewClicked);
    connect(ui->splittedDataTableView, &QWidget::customContextMenuRequested, this, &QWidget::customContextMenuRequested);
    connect(ui->splittedDataTableView, &QAbstractItemView::doubleClicked, this, &QAdvancedTableView::viewDoubleClicked);
    connect(ui->splittedDataTableView, &QAbstractItemView::entered, this, &QAdvancedTableView::viewEntered);
    connect(ui->splittedDataTableView, &QAbstractItemView::pressed, this, &QAdvancedTableView::viewPressed);
    connect(ui->splittedDataTableView, &QAbstractItemView::viewportEntered, this, &QAdvancedTableView::viewportEntered);

    // Install event filter
    ui->dataTableView->verticalHeader()->installEventFilter(this);
    updateHeaderViewGeometries();
    //
    setFocusProxy(ui->dataTableView);
}

QAdvancedTableView::~QAdvancedTableView()
{
    delete d;
    delete ui;
}

void QAdvancedTableView::addFilterGroup()
{
    d->filterModel->insertRows(d->filterModel->rowCount(), 1);
}

bool QAdvancedTableView::alternatingRowColors() const
{
    return ui->dataTableView->alternatingRowColors();
}

int QAdvancedTableView::columnAt(int x) const
{
    return ui->dataTableView->columnAt(x);
}

QVariantList QAdvancedTableView::columnsFilterTypes(int column) const
{
    return d->filterModel->index(0, column).data(QAbstractFilterModel::ColumnFilterTypesRole).toList();
}

int QAdvancedTableView::columnWidth(int column) const
{
    return ui->headerTableView->columnWidth(column);
}

void QAdvancedTableView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex i;
    if (ui->dataTableView->viewport()->rect().contains(event->pos())) {
        i = ui->dataTableView->indexAt(ui->dataTableView->viewport()->mapFromParent(event->pos()));
    } else if (ui->fixedRowsTableView->viewport()->rect().contains(event->pos())) {
        i = ui->fixedRowsTableView->indexAt(event->pos());
    }
    QMenu* m = createStandardContextMenu(i);

    m->exec(event->globalPos());
    delete m;
}

QMenu *QAdvancedTableView::createStandardContextMenu(const QModelIndex & index)
{
    Q_UNUSED(index)
    QAction* a;
    QMenu* m = new QMenu();
    m = m->addMenu(tr("View"));
    a = m->addAction(tr("Splitted View"));
    a->setCheckable(true);
    a->setChecked(viewSplitted());
    connect(a, &QAction::toggled, this, &QAdvancedTableView::splitView);

    a = m->addAction(tr("Fixed Rows"));
    a->setCheckable(true);
    a->setChecked(showFixedRows());
    connect(a, &QAction::toggled, this, &QAdvancedTableView::setShowFixedRows);

    m->addSeparator();

    a = m->addAction(tr("Show Filter"));
    a->setCheckable(true);
    a->setChecked(showFilter());
    connect(a, &QAction::toggled, this, &QAdvancedTableView::setShowFilter);

    m->addSeparator();

    a = m->addAction(tr("Show Grid"));
    a->setCheckable(true);
    a->setChecked(showGrid());

    connect(a, &QAction::toggled, this, &QAdvancedTableView::setShowGrid);

    return m;
}

void QAdvancedTableView::clearSelection()
{
    ui->dataTableView->clearSelection();
}

QModelIndex QAdvancedTableView::currentIndex() const
{
    if (ui->fixedRowsTableView->hasFocus()) {
        return mapToSource(ui->fixedRowsTableView->currentIndex());
    } else if (ui->splittedDataTableView->hasFocus()) {
        return mapToSource(ui->splittedDataTableView->currentIndex());
    }
    return mapToSource(ui->dataTableView->currentIndex());
}

int QAdvancedTableView::defaultFilterType(int column) const
{
    return d->filterModel->index(0, column).data(QAbstractFilterModel::DefaultFilterTypeRole).toInt();
}

void QAdvancedTableView::dataViewHorizontalScrollBarRangeChanged(int minimum, int maximum)
{
    ui->headerTableView->horizontalScrollBar()->setRange(minimum, maximum);
    ui->splittedDataTableView->horizontalScrollBar()->setRange(minimum, maximum);
    ui->fixedRowsTableView->horizontalScrollBar()->setRange(minimum, maximum);

}

void QAdvancedTableView::dataViewHorizontalScrollBarSilderMoved(int value)
{
    ui->headerTableView->horizontalScrollBar()->setValue(value);
    ui->splittedDataTableView->horizontalScrollBar()->setValue(value);
    ui->fixedRowsTableView->horizontalScrollBar()->setValue(value);
}

void QAdvancedTableView::dataViewHorizontalScrollBarValueChanged(int value)
{
    ui->headerTableView->horizontalScrollBar()->setValue(value);
    ui->splittedDataTableView->horizontalScrollBar()->setValue(value);
    ui->fixedRowsTableView->horizontalScrollBar()->setValue(value);
}

bool QAdvancedTableView::dragEnabled() const
{
    return ui->dataTableView->dragEnabled();
}

void QAdvancedTableView::edit(const QModelIndex & index)
{
    if (ui->fixedRowsTableView->hasFocus()) {
        ui->fixedRowsTableView->edit(index);
    } else if (ui->splittedDataTableView->hasFocus()) {
        ui->splittedDataTableView->edit(index);
    } else {
        ui->dataTableView->edit(index);
    }
}

QAbstractItemView::EditTriggers QAdvancedTableView::editTriggers() const
{
    return ui->dataTableView->editTriggers();
}

bool QAdvancedTableView::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Hide) {
        if (obj == ui->dataTableView->verticalHeader()) {
            ui->fixedRowsTableView->verticalHeader()->setVisible(false);
            ui->headerTableView->verticalHeader()->setVisible(false);
            ui->splittedDataTableView->verticalHeader()->setVisible(false);
        }
    }
    return QObject::eventFilter(obj, event);
}

QAbstractFilter* QAdvancedTableView::filterAt(int row, int col) const
{
    return d->filterModel->filter(d->filterModel->index(row, col));
}

void QAdvancedTableView::filterAdded(const QModelIndex & parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    updateHeaderViewGeometries();
}

QIcon QAdvancedTableView::filterDisabledIcon() const
{
    return d->filterModel->filterDisabledIcon();
}

QIcon QAdvancedTableView::filterEnabledIcon() const
{
    return d->filterModel->filterEnabledIcon();
}

void QAdvancedTableView::headerViewHorizontalScrollBarRangeChanged(int min, int max)
{
    Q_UNUSED(min)
    Q_UNUSED(max)
    // ui->dataTableView->horizontalScrollBar()->setRange(min, max);
}

QAbstractFilterModel* QAdvancedTableView::filterModel() const
{
    return d->filterModel;
}

QAbstractFilterProxyModel *QAdvancedTableView::filterProxyModel() const
{
    return d->dataViewProxy;
}

bool QAdvancedTableView::showFilter() const
{
    return ui->headerTableView->filterVisible();
}

QModelIndexList QAdvancedTableView::fixedRows(int column) const
{
    QModelIndexList l;
    for (int i = 0; i < ui->fixedRowsTableView->model()->rowCount(); i++){
        l << qSourceIndex(ui->fixedRowsTableView->model()->index(i, column));
    }
    return l;
}

Qt::PenStyle QAdvancedTableView::gridStyle() const
{
    return ui->dataTableView->gridStyle();
}

QSize QAdvancedTableView::iconSize() const
{
    return ui->dataTableView->iconSize();
}

bool QAdvancedTableView::hasAutoScroll() const
{
    return ui->dataTableView->hasAutoScroll();
}

void QAdvancedTableView::headerViewHorizontalScrollBarSilderMoved(int value)
{
    ui->dataTableView->horizontalScrollBar()->setValue(value);
    ui->fixedRowsTableView->horizontalScrollBar()->setValue(value);
    ui->splittedDataTableView->horizontalScrollBar()->setValue(value);
}

void QAdvancedTableView::headerViewHorizontalScrollBarValueChanged(int value)
{
    ui->dataTableView->horizontalScrollBar()->setValue(value);
    ui->fixedRowsTableView->horizontalScrollBar()->setValue(value);
    ui->splittedDataTableView->horizontalScrollBar()->setValue(value);
}

void QAdvancedTableView::headerViewSectionResized(int logicalIndex, int oldSize, int newSize)
{
    if (newSize == 0) {
        ui->dataTableView->horizontalHeader()->hideSection(logicalIndex);
        ui->fixedRowsTableView->horizontalHeader()->hideSection(logicalIndex);
        ui->splittedDataTableView->horizontalHeader()->hideSection(logicalIndex);
    } else if (oldSize == 0) {
        ui->dataTableView->horizontalHeader()->showSection(logicalIndex);
        ui->fixedRowsTableView->horizontalHeader()->showSection(logicalIndex);
        ui->splittedDataTableView->horizontalHeader()->showSection(logicalIndex);
    } else {
        ui->dataTableView->horizontalHeader()->resizeSection(logicalIndex, newSize);
        ui->fixedRowsTableView->horizontalHeader()->resizeSection(logicalIndex, newSize);
        ui->splittedDataTableView->horizontalHeader()->resizeSection(logicalIndex, newSize);
    }
}

void QAdvancedTableView::hideColumn(int column)
{
    ui->headerTableView->horizontalHeader()->hideSection(ui->headerTableView->horizontalHeader()->logicalIndex(column));
}

void QAdvancedTableView::hideColumns(const QStringList & names)
{
    for (int i = 0; i < horizontalHeader()->count(); i++) {
        horizontalHeader()->setSectionHidden(i, names.contains(horizontalHeader()->model()->headerData(i, Qt::Horizontal).toString()));
    }
}

void QAdvancedTableView::hideFilterView()
{
    setShowFilter(false);
}

void QAdvancedTableView::hideRow(int row)
{
    V_CALL(hideRow(row))
}

QHeaderView* QAdvancedTableView::horizontalHeader() const
{
    return ui->headerTableView->horizontalHeader();
}

void QAdvancedTableView::horizontalHeaderViewSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    Q_UNUSED(logicalIndex);
    ui->dataTableView->horizontalHeader()->moveSection(oldVisualIndex, newVisualIndex);
    ui->fixedRowsTableView->horizontalHeader()->moveSection(oldVisualIndex, newVisualIndex);
    ui->splittedDataTableView->horizontalHeader()->moveSection(oldVisualIndex, newVisualIndex);
}

void QAdvancedTableView::horizontalHeaderSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    ui->dataTableView->sortByColumn(logicalIndex, order);
    ui->fixedRowsTableView->sortByColumn(logicalIndex, order);
    ui->splittedDataTableView->sortByColumn(logicalIndex, order);
}

QModelIndex QAdvancedTableView::indexAt(const QPoint & point) const
{
    if (focusProxy() == ui->splittedDataTableView) {
        return mapToSource(ui->splittedDataTableView->indexAt(point));
    } else if (focusProxy() == ui->fixedRowsTableView) {
        return mapToSource(ui->fixedRowsTableView->indexAt(point));
    }
    return mapToSource(ui->dataTableView->indexAt(point));
}

bool QAdvancedTableView::isColumnHidden(int column) const
{
    return ui->dataTableView->isColumnHidden(column);
}

bool QAdvancedTableView::isRowHidden(int row) const
{
    return ui->dataTableView->isRowHidden(row);
}

void QAdvancedTableView::setRowHidden(int row, bool hide) {
    if (focusProxy() == ui->splittedDataTableView) {
        ui->splittedDataTableView->setRowHidden(row, hide);
    } else if (focusProxy() == ui->fixedRowsTableView) {
        ui->fixedRowsTableView->setRowHidden(row, hide);
    } else {
        ui->dataTableView->setRowHidden(row, hide);
    }
}

bool QAdvancedTableView::isSortIndicatorShown() const
{
    return ui->headerTableView->horizontalHeader()->isSortIndicatorShown();
}

bool QAdvancedTableView::isSortingEnabled() const
{
    return ui->dataTableView->isSortingEnabled();
}

bool QAdvancedTableView::viewSplitted() const
{
    return ui->splittedDataTableView->isVisible();
}

QAbstractItemDelegate *QAdvancedTableView::itemDelegate() const
{
    return ui->dataTableView->itemDelegate();
}

QAbstractItemDelegate *QAdvancedTableView::itemDelegateForColumn(int column) const
{
    return ui->dataTableView->itemDelegateForColumn(column);
}

QAbstractItemDelegate *QAdvancedTableView::itemDelegateForRow(int row) const
{
    return ui->dataTableView->itemDelegateForRow(row);
}

QModelIndex QAdvancedTableView::mapToSource(const QModelIndex & index) const
{
    QModelIndex i(index);
    QAbstractProxyModel* p;
    while(i.model() != d->model && (p = qobject_cast<QAbstractProxyModel*>((QAbstractProxyModel*)i.model()))) {
        i = p->mapToSource(i);
    }
    return i;
}

QSize QAdvancedTableView::minimumSizeHint() const
{
    return QSize(QWidget::minimumSizeHint().width(), ui->headerTableView->height() * 2);
}

QAbstractItemModel* QAdvancedTableView::model() const
{
    return d->model;
}

bool QAdvancedTableView::showFixedRows() const
{
    return ui->fixedRowsTableView->decorationProxy()->isEnabled();
}

void QAdvancedTableView::modelReset()
{
    if (d->autoResizeRowsToContents) {
        ui->dataTableView->resizeRowsToContents();
        ui->fixedRowsTableView->resizeRowsToContents();
        ui->splittedDataTableView->resizeRowsToContents();
    }
}

bool QAdvancedTableView::restoreFilter(const QByteArray & data)
{
    QByteArray mData(data);
    QDataStream mStream(&mData, QIODevice::ReadOnly);
    if (mStream.atEnd()) {
        return false;
    }
    qint32 mCol;
    qint32 mRow;
    qint32 mRows;
    QString mName;
    QVariantMap mProperties;
    // Clear current filter model
    d->filterModel->removeRows(0, d->filterModel->rowCount());
    mStream >> mRows;
    d->filterModel->insertRows(0, mRows);
    for (int iRow = 0; iRow < mRows; iRow++) {
        mStream >> mName;
        d->filterModel->setHeaderData(iRow, Qt::Vertical, mName);
    }
    while(!mStream.atEnd()) {
        mStream >> mRow >> mCol >> mProperties;
        if (d->filterModel->rowCount() < mRow){
            d->filterModel->insertRows(d->filterModel->rowCount(), 1);
        }
        d->filterModel->setData(d->filterModel->index(mRow, mCol), mProperties);
    }
    return true;
}

bool QAdvancedTableView::restoreState(const QByteArray & data)
{
    QByteArray d;
    QByteArray ds(data);
    bool splitted;
    bool fixed;
    bool filter;
    bool grid;
    QDataStream s(&ds, QIODevice::ReadOnly);
    if (s.atEnd()) {
        return false;
    }
    s >> d >> splitted >> fixed >> filter >> grid;
    if (ui->headerTableView->horizontalHeader()->restoreState(d)) {
        splitView(splitted);
        setShowFixedRows(fixed);
        setShowFilter(filter);
        setShowGrid(grid);
        updateHorizontalHeaderSectionSize();
        ui->headerTableView->viewport()->update();
        ui->dataTableView->horizontalHeader()->restoreState(d);
        ui->fixedRowsTableView->horizontalHeader()->restoreState(d);
        ui->splittedDataTableView->horizontalHeader()->restoreState(d);
    }
    return false;
}

QModelIndex QAdvancedTableView::rootIndex() const
{
    return ui->dataTableView->rootIndex();
}

int QAdvancedTableView::rowAt(int y) const
{
    if (ui->splittedDataTableView->isVisible()) {
        int r = ui->splittedDataTableView->rowAt(y);
        if (r > -1) {
            return r;
        }
    }
    return ui->dataTableView->rowAt(y);
}

int QAdvancedTableView::rowHeight(int row) const
{
    return ui->dataTableView->rowHeight(row);
}

int QAdvancedTableView::datatableHeight() const
{
    return ui->dataTableView->height();
}

int QAdvancedTableView::datatableWidth() const
{
    return ui->dataTableView->width();
}

int QAdvancedTableView::rowViewportPosition(int row) const
{
    return ui->dataTableView->rowViewportPosition(row);
}

int QAdvancedTableView::columnViewportPosition(int column) const
{
    return ui->dataTableView->columnViewportPosition(column);
}


void QAdvancedTableView::reset()
{
    V_CALL(reset())
}

void QAdvancedTableView::resizeColumnToContents(int column)
{
    ui->headerTableView->resizeColumnToContents(column);
}

void QAdvancedTableView::resizeColumnsToContents()
{
    ui->headerTableView->resizeColumnsToContents();
}

void QAdvancedTableView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    viewLayoutChanged();
}

void QAdvancedTableView::resizeRowToContents(int row)
{
    ui->dataTableView->resizeRowToContents(row);
    ui->fixedRowsTableView->resizeRowToContents(row);
    ui->splittedDataTableView->resizeRowToContents(row);
}

void QAdvancedTableView::resizeRowsToContents()
{
    ui->dataTableView->resizeRowsToContents();
    ui->fixedRowsTableView->resizeRowsToContents();
    ui->splittedDataTableView->resizeRowsToContents();
}

QByteArray QAdvancedTableView::saveFilter() const
{
    QByteArray mData;
    QDataStream mStream(&mData, QIODevice::WriteOnly);
    QVariantMap mProperties;

    mStream << qint32(d->filterModel->rowCount());
    for (int iRow = 0; iRow < d->filterModel->rowCount(); iRow++) {
        mStream << d->filterModel->headerData(iRow, Qt::Vertical).toString();
    }
    for (int iRow = 0; iRow < d->filterModel->rowCount(); iRow++) {
        for (int iCol = 0; iCol < d->filterModel->columnCount(); iCol++){
            mProperties = d->filterModel->index(iRow, iCol).data(Qt::EditRole).toMap();
            if (!mProperties.isEmpty()) {
                mStream << qint32(iRow) << qint32(iCol) << mProperties;
            }

        }
    }
    return mData;
}

QByteArray QAdvancedTableView::saveState()
{
    QByteArray d;
    QDataStream s(&d, QIODevice::WriteOnly);

    s << ui->headerTableView->horizontalHeader()->saveState();
    s << viewSplitted();
    s << showFixedRows();
    s << showFilter();
    s << showGrid();
    return d;
}

void QAdvancedTableView::scrollToBottom()
{
    ui->dataTableView->scrollToBottom();
}

void QAdvancedTableView::scrollToTop()
{
    ui->dataTableView->scrollToTop();
}

void QAdvancedTableView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    ui->dataTableView->scrollTo(index, hint);
}

void QAdvancedTableView::selectAll()
{
    ui->dataTableView->selectAll();
}

void QAdvancedTableView::selectColumn(int colum)
{
    ui->dataTableView->selectColumn(colum);
}

void QAdvancedTableView::selectRow(int row)
{
    ui->dataTableView->selectRow(row);
}

QAbstractItemView::SelectionBehavior QAdvancedTableView::selectionBehavior() const
{
    return ui->dataTableView->selectionBehavior();
}

QAbstractItemView::SelectionMode QAdvancedTableView::selectionMode() const
{
    return ui->dataTableView->selectionMode();
}

QItemSelectionModel* QAdvancedTableView::selectionModel() const
{
    return ui->dataTableView->selectionModel();
}

void QAdvancedTableView::setAlternatingRowColors( bool enable )
{
    V_CALL(setAlternatingRowColors(enable))
}

void QAdvancedTableView::setAutoScroll(bool enable)
{
    V_CALL(setAutoScroll(enable))
}

void QAdvancedTableView::setColumnWidth(int column, int width)
{
    ui->headerTableView->setColumnWidth(column, width);
}

void QAdvancedTableView::setContextMenuPolicy(Qt::ContextMenuPolicy policy)
{
    V_CALL(setContextMenuPolicy(policy))
}

void QAdvancedTableView::setCurrentIndex(const QModelIndex & index)
{
    ui->dataTableView->setCurrentIndex(index);
}

void QAdvancedTableView::setDefaultFilterType(int column, int type)
{
    d->filterModel->setData(d->filterModel->index(0, column), type, QAbstractFilterModel::DefaultFilterTypeRole);
}

void QAdvancedTableView::setDefaultFilterType(const QString & name, int type)
{
    for (int i = 0; i < ui->headerTableView->model()->columnCount(); i++) {
        if (ui->headerTableView->model()->headerData(i, Qt::Horizontal) == name) {
            setDefaultFilterType(i, type);
        }
    }
}
void QAdvancedTableView::setDragEnabled(bool enable)
{
    V_CALL(setDragEnabled(enable))
}

void QAdvancedTableView::setEditTriggers(QAbstractItemView::EditTriggers triggers)
{
    V_CALL(setEditTriggers(triggers))
}

void QAdvancedTableView::setColumnFilterTypes(int column, const QVariantList &types)
{
    d->filterModel->setData(d->filterModel->index(0, column), types, QAbstractFilterModel::ColumnFilterTypesRole);
}

void QAdvancedTableView::setFilterDisabledIcon(const QIcon & icon)
{
    d->filterModel->setFilterDisabledIcon(icon);
}

void QAdvancedTableView::setFilterEnabledIcon(const QIcon & icon)
{
    d->filterModel->setFilterEnabledIcon(icon);
}

void QAdvancedTableView::setFilterEnabled(int row, int column, bool enable)
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(enable)
    // d->filterModel->setFilterEnabled(row, column, enable);
}

void QAdvancedTableView::setItemDelegate(QAbstractItemDelegate* delegate)
{
    V_CALL(setItemDelegate(delegate))
}

void QAdvancedTableView::setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate)
{
    V_CALL(setItemDelegateForColumn(column, delegate))
}

void QAdvancedTableView::setItemDelegateForRow(int row, QAbstractItemDelegate * delegate)
{
    V_CALL(setItemDelegateForRow(row, delegate))
}

void QAdvancedTableView::setGridStyle(Qt::PenStyle style)
{
    ui->headerTableView->setGridStyle(style);
    V_CALL(setGridStyle(style))
}

void QAdvancedTableView::setIconSize(const QSize &size)
{
    V_CALL(setIconSize(size))
}

QWidget* QAdvancedTableView::indexWidget(const QModelIndex &index) const {
    if (focusProxy() == ui->splittedDataTableView) {
        return ui->splittedDataTableView->indexWidget(index);
    } else if (focusProxy() == ui->fixedRowsTableView) {
        return ui->fixedRowsTableView->indexWidget(index);
    }
    return ui->dataTableView->indexWidget(index);
}

void QAdvancedTableView::setIndexWidget(const QModelIndex &index, QWidget *widget) {
    if (focusProxy() == ui->splittedDataTableView) {
        ui->splittedDataTableView->setIndexWidget(index, widget);
    } else if (focusProxy() == ui->fixedRowsTableView) {
        ui->fixedRowsTableView->setIndexWidget(index, widget);
    } else {
        ui->dataTableView->setIndexWidget(index, widget);
    }
}

void QAdvancedTableView::setModel(QAbstractItemModel* model)
{
    d->model = model;
    connect(d->model, &QAbstractItemModel::layoutChanged, this, &QAdvancedTableView::viewLayoutChanged);
    ui->fixedRowsTableView->setModel(d->model);
    d->filterModel->setSourceModel(d->model);
    for(int iCol = 0; iCol < d->horizontalHeader->count(); iCol++) {
        ui->dataTableView->horizontalHeader()->resizeSection(iCol, d->horizontalHeader->sectionSize(iCol));
        ui->dataTableView->horizontalHeader()->moveSection(ui->dataTableView->horizontalHeader()->visualIndex(iCol), d->horizontalHeader->visualIndex(iCol));
    }
    if (horizontalHeader()->stretchLastSection()) {
        if (horizontalHeader()->count() > 0) {
            horizontalHeader()->setSectionResizeMode(horizontalHeader()->count() - 1, QHeaderView::Stretch);
        }
    }
    updateHeaderViewGeometries();
}

void QAdvancedTableView::setRowHeight(int row, int height)
{
    ui->dataTableView->setRowHeight(row, height);
}

void QAdvancedTableView::setRootIndex(const QModelIndex & index)
{
    V_CALL(setRootIndex(index))
}

void QAdvancedTableView::showColumn(int column)
{
    horizontalHeader()->showSection(column);
}

void QAdvancedTableView::showEvent(QShowEvent* event)
{
    ui->fixedRowsTableView->verticalHeader()->setVisible(ui->dataTableView->verticalHeader()->isVisible());
    ui->splittedDataTableView->verticalHeader()->setVisible(ui->dataTableView->verticalHeader()->isVisible());
    d->verticalHeader->setVisible(ui->dataTableView->verticalHeader()->isVisible());
    viewLayoutChanged();
    QWidget::showEvent(event);
}

void QAdvancedTableView::showRow(int row)
{
    V_CALL(showRow(row))
}

void QAdvancedTableView::setColumnsAutoFitParams(QMap<int, int> colSpareWidthParts, bool forceFitSize)
{
    d->columnSpareWidthParts.clear();
    d->columnSpareWidthParts = colSpareWidthParts;
    d->autoResizeColumnsToFitView = forceFitSize;
    autoResizeColumnsToContent();
}

void QAdvancedTableView::autoResizeColumnsToContent()
{
    // MODIFICATION for Praaline
    // This method is way to slow to be used in a grid showing real-time/time-constrained data, such as
    // the annotations of a MultiTierAnnotationEditor.
    // TODO A better solution needs to be developped.
    return;


    if (model()) {
        int headerFullWidth = viewport()->width();
        if (ui->headerTableView->verticalScrollBar()->isVisible()) {
            int scrollBarWidth = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
            headerFullWidth -= scrollBarWidth;
        }

        int columnsCnt = d->horizontalHeader->count();
        QMap<int, int> colFinalWidthList;

        QMap<int, int>::const_iterator it = d->columnSpareWidthParts.constBegin();
        auto end = d->columnSpareWidthParts.constEnd();
        int partCount = 0;
        while (it != end) {
            partCount += it.value();
            if (it.key() >= columnsCnt) {
                return;
            }
            ++it;
        }

        int resizedWidth = 0;
        if (d->autoResizeColumnsToFitView) {
            for (int i = 0; i < columnsCnt; i++) {
                if (!d->columnSpareWidthParts.contains(i)) {
                    ui->dataTableView->resizeColumnToContents(i);
                    int currColWidth = ui->dataTableView->columnWidth(i);
                    colFinalWidthList[i] = currColWidth;
                    resizedWidth += currColWidth;
                }
            }

            int resColumnWidth = headerFullWidth - resizedWidth;
            if (resColumnWidth < 0) {
                for(int iCol = 0; iCol < columnsCnt; iCol++) {
                    ui->dataTableView->horizontalHeader()->resizeSection(iCol, d->horizontalHeader->sectionSize(iCol));
                }
                return;
            }

            // Avoid division by zero
            if (partCount <= 0) partCount = 1;

            it = d->columnSpareWidthParts.constBegin();
            while (it != end) {
                int partWidth = (resColumnWidth / partCount) * it.value();
                colFinalWidthList[it.key()] = partWidth;
                ui->dataTableView->setColumnWidth(it.key(), partWidth);
                ++it;
            }
        } else {
            for (int i = 0; i < columnsCnt; i++) {
                ui->dataTableView->resizeColumnToContents(i);
                int currColWidth = ui->dataTableView->columnWidth(i);
                colFinalWidthList[i] = currColWidth;
                resizedWidth += currColWidth;
            }

            int deltaWidth = headerFullWidth - resizedWidth;
            if (deltaWidth < 0)
                return;

            // Avoid division by zero
            if (partCount <= 0) partCount = 1;

            it = d->columnSpareWidthParts.constBegin();
            while (it != end) {
                int partWidth = (deltaWidth / partCount) * it.value() + ui->dataTableView->columnWidth(it.key());
                colFinalWidthList[it.key()] = partWidth;
                ui->dataTableView->setColumnWidth(it.key(), partWidth);
                ++it;
            }
        }

        for (int j = 0; j < columnsCnt; j++) {
            ui->headerTableView->horizontalHeader()->resizeSection(j, colFinalWidthList.value(j, 1));
        }
        ui->dataTableView->resizeRowsToContents();
    }
}

void QAdvancedTableView::setSelectionBehavior(QAbstractItemView::SelectionBehavior behavior)
{
    V_CALL(setSelectionBehavior(behavior))
}

void QAdvancedTableView::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    V_CALL(setSelectionMode(mode))
}

void QAdvancedTableView::setFilterModel(QAbstractFilterModel* model)
{
    if (d->filterModel) {
        disconnect(d->filterModel);
    }
    d->filterModel = model;
    d->dataViewProxy->setFilterModel(d->filterModel);
    ui->headerTableView->setModel(d->filterModel);
    // filter model
    connect(d->filterModel, &QAbstractItemModel::modelReset, this, &QAdvancedTableView::updateHeaderViewGeometries);
    connect(d->filterModel, &QAbstractItemModel::rowsRemoved, this, &QAdvancedTableView::updateHeaderViewGeometries);
}

void QAdvancedTableView::setFilterProxyModel(QAbstractFilterProxyModel* proxy)
{
    if (d->dataViewProxy) {
        disconnect(d->dataViewProxy);
    }
    d->dataViewProxy = proxy;
    d->dataViewProxy->setFilterModel(d->filterModel);
    d->dataViewProxy->setSourceModel(ui->fixedRowsTableView->decorationProxy());
    ui->dataTableView->setModel(d->dataViewProxy);
    ui->splittedDataTableView->setModel(d->dataViewProxy);

    connect(d->dataViewProxy, &QAbstractItemModel::modelReset, this, &QAdvancedTableView::modelReset);
    connect(d->dataViewProxy, &QAbstractItemModel::layoutChanged, this, &QAdvancedTableView::dataModelLayoutChanged);
    dataModelLayoutChanged();
}

bool QAdvancedTableView::setFilterType(int type, int column, int row)
{
    QVariantMap mProperties;
    mProperties["type"] = type;
    if (d->filterModel->createFilter(d->filterModel->index(row, column), mProperties) != 0){
        return true;
    }
    return false;
}

void QAdvancedTableView::setShowFixedRows(bool show)
{
    ui->fixedRowsTableView->decorationProxy()->setEnabled(show);
}

void QAdvancedTableView::setShowGrid( bool show )
{
    ui->headerTableView->setShowGrid(show);
    V_CALL(setShowGrid(show))
}

void QAdvancedTableView::setSortIndicatorShown( bool show )
{
    ui->headerTableView->horizontalHeader()->setSortIndicatorShown(show);
}

void QAdvancedTableView::setSortingEnabled( bool enable )
{
    V_CALL(setSortingEnabled(enable))
}

void QAdvancedTableView::setSpan(int row, int column, int rowSpanCount, int columnSpanCount) {
    if (focusProxy() == ui->splittedDataTableView) {
        ui->splittedDataTableView->setSpan(row, column, rowSpanCount, columnSpanCount);
    } else if (focusProxy() == ui->fixedRowsTableView) {
        ui->fixedRowsTableView->setSpan(row, column, rowSpanCount, columnSpanCount);
    } else {
        ui->dataTableView->setSpan(row, column, rowSpanCount, columnSpanCount);
    }
}

void QAdvancedTableView::setTextElideMode(Qt::TextElideMode mode)
{
    V_CALL(setTextElideMode(mode))
}

void QAdvancedTableView::setShowFilter(bool show)
{
    ui->headerTableView->setFilterVisible(show);
}

void QAdvancedTableView::setWordWrap(bool wrap)
{
    ui->dataTableView->setWordWrap(wrap);
    ui->fixedRowsTableView->setWordWrap(wrap);
}

bool QAdvancedTableView::showGrid() const
{
    return ui->dataTableView->showGrid();
}

void QAdvancedTableView::subviewReceivedFocus()
{
    QWidget* w = qobject_cast<QWidget*>(sender());
    if (w) {
        setFocusProxy(w);
    }
}

QSize QAdvancedTableView::sizeHint() const
{
    return QSize(QWidget::sizeHint().width(), ui->headerTableView->height() * 2);
}

void QAdvancedTableView::splitView(bool split)
{
    if (split == ui->splittedDataTableView->isVisible()) {
        return;
    }
    if (split) {
        ui->splittedDataTableView->show();
        if (d->autoResizeRowsToContents) {
            ui->splittedDataTableView->resizeRowsToContents();
        }
    } else {
        ui->splittedDataTableView->hide();
    }
    ui->splittedDataTableView->setVerticalScrollBarPolicy(ui->dataTableView->verticalScrollBarPolicy());
}

void QAdvancedTableView::sortByColumn(int column, Qt::SortOrder order)
{
    ui->headerTableView->sortByColumn(column, order);
}

Qt::TextElideMode QAdvancedTableView::textElideMode() const
{
    return ui->dataTableView->textElideMode();
}

void QAdvancedTableView::update(const QModelIndex & index)
{
    ui->dataTableView->update(index);
}

void QAdvancedTableView::dataModelLayoutChanged()
{
    if (d->autoResizeRowsToContents) {
        ui->dataTableView->resizeRowsToContents();
        ui->fixedRowsTableView->resizeRowsToContents();
        ui->splittedDataTableView->resizeRowsToContents();
    }
    for (int i = 0; i < ui->headerTableView->horizontalHeader()->count(); i++) {
        ui->dataTableView->horizontalHeader()->setSectionHidden(i, ui->headerTableView->horizontalHeader()->isSectionHidden(i));
        ui->fixedRowsTableView->horizontalHeader()->setSectionHidden(i, ui->headerTableView->horizontalHeader()->isSectionHidden(i));
        ui->splittedDataTableView->horizontalHeader()->setSectionHidden(i, ui->headerTableView->horizontalHeader()->isSectionHidden(i));
    }
}

void QAdvancedTableView::updateHeaderViewGeometries()
{
    int rows = ui->headerTableView->model()->rowCount();
    if (rows > ui->headerTableView->maxVisibileFilterSets()) {
        rows = ui->headerTableView->maxVisibileFilterSets();
    }
    for (int iRow = 0; iRow < ui->headerTableView->model()->rowCount(); iRow++) {
        ui->headerTableView->verticalHeader()->resizeSection(iRow, ui->headerTableView->verticalHeader()->sizeHint().height());
    }
    int mRowHeight = ui->headerTableView->verticalHeader()->sizeHint().height();
    if (mRowHeight == 0) {
        mRowHeight = ui->headerTableView->horizontalHeader()->sizeHint().height();
    }
    int mHeaderHeight = ui->headerTableView->horizontalHeader()->sizeHint().height();
    if (mHeaderHeight == 0) {
        mHeaderHeight = mRowHeight;
    }
    if (ui->headerTableView->filterVisible()) {
        ui->headerTableView->setFixedHeight(mHeaderHeight + mRowHeight * rows + (1 * rows) + 1);
    } else {
        ui->headerTableView->setFixedHeight(ui->headerTableView->horizontalHeader()->sizeHint().height());
    }

    mRowHeight = ui->dataTableView->verticalHeader()->sizeHint().height();
    // ui->fixedRowsTableView->setFixedHeight();
    if (ui->headerTableView->model()->rowCount() > ui->headerTableView->maxVisibileFilterSets()) {
        ui->headerTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->dataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->splittedDataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }
    if (ui->headerTableView->verticalHeader()->width() > ui->dataTableView->verticalHeader()->width()) {
        ui->dataTableView->verticalHeader()->setFixedWidth(ui->headerTableView->verticalHeader()->width());
        ui->fixedRowsTableView->verticalHeader()->setFixedWidth(ui->headerTableView->verticalHeader()->width());
        ui->splittedDataTableView->verticalHeader()->setFixedWidth(ui->headerTableView->verticalHeader()->width());
    } else {
        ui->headerTableView->verticalHeader()->setFixedWidth(ui->dataTableView->verticalHeader()->width());
    }
}

void QAdvancedTableView::updateHeaderViewVerticalScrollBar(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    if (ui->dataTableView->verticalScrollBar()->maximum() == 0) {
        ui->headerTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->fixedRowsTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        ui->headerTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->fixedRowsTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }
}

void QAdvancedTableView::updateHorizontalHeaderSectionSize()
{
    for (int i = 0; i < ui->headerTableView->horizontalHeader()->count(); i++) {
        ui->dataTableView->horizontalHeader()->resizeSection(i, ui->headerTableView->horizontalHeader()->sectionSize(i));
        ui->fixedRowsTableView->horizontalHeader()->resizeSection(i, ui->headerTableView->horizontalHeader()->sectionSize(i));
    }
}

void QAdvancedTableView::verticalHeaderSectionClicked(int section)
{
    QPoint p = mapFromGlobal(QCursor::pos());
    if (ui->fixedRowsTableView->decorationProxy()->isEnabled()) {
        if (p.x() < ui->fixedRowsTableView->decorationProxy()->iconSize().width()){
            ui->fixedRowsTableView->decorationProxy()->toggleRow(ui->dataTableView->model()->index(section, 0));
        }
    }
}

void QAdvancedTableView::verticalHeaderWidthChangeRequested(int width)
{
    if (sender() == ui->headerTableView) {
        if (ui->dataTableView->verticalHeader()->sectionSize(0) < width) {
            int w = width + 12;
            if (showFixedRows()){
                w += 22;
            }
            ui->dataTableView->verticalHeader()->setFixedWidth(w);
            ui->headerTableView->verticalHeader()->setFixedWidth(w);
            ui->fixedRowsTableView->verticalHeader()->setFixedWidth(w);
            ui->splittedDataTableView->verticalHeader()->setFixedWidth(w);
            updateHeaderViewGeometries();
        }
    }
}

void QAdvancedTableView::verticalHeaderSectionResized(int logicalIndex, int oldSize, int newSize)
{
    Q_UNUSED(oldSize);
    if (sender() == ui->splittedDataTableView->verticalHeader()) {
        ui->dataTableView->verticalHeader()->resizeSection(logicalIndex, newSize);
    } else if (sender() == ui->dataTableView->verticalHeader()) {
        ui->splittedDataTableView->verticalHeader()->resizeSection(logicalIndex, newSize);
    }
}

QHeaderView* QAdvancedTableView::verticalHeader() const
{
    return ui->dataTableView->verticalHeader();
}

void QAdvancedTableView::viewActivated(const QModelIndex & index)
{
    emit activated(mapToSource(index));
}

void QAdvancedTableView::viewClicked(const QModelIndex & index)
{
    emit clicked(mapToSource(index));
}

void QAdvancedTableView::viewDoubleClicked(const QModelIndex & index)
{
    emit doubleClicked(mapToSource(index));
}

void QAdvancedTableView::viewEntered(const QModelIndex & index)
{
    emit entered(mapToSource(index));
}

void QAdvancedTableView::viewLayoutChanged()
{
    QTimer::singleShot(0, this, &QAdvancedTableView::viewLayoutChangedImpl);
}

void QAdvancedTableView::viewLayoutChangedImpl() {
    autoResizeColumnsToContent();
}

void QAdvancedTableView::viewPressed(const QModelIndex & index)
{
    emit pressed(mapToSource(index));
}

QWidget* QAdvancedTableView::viewport() const
{
    if (focusProxy() == ui->splittedDataTableView) {
        return ui->splittedDataTableView->viewport();
    } else if (focusProxy() == ui->fixedRowsTableView) {
        return ui->fixedRowsTableView->viewport();
    }
    return ui->dataTableView->viewport();
}

bool QAdvancedTableView::wordWrap() const
{
    return ui->dataTableView->wordWrap();
}

void QAdvancedTableView::setVerticalHeadersSize(int height)
{
    ui->headerTableView->verticalHeader()->setDefaultSectionSize(height);
    ui->splittedDataTableView->verticalHeader()->setDefaultSectionSize(height);
    ui->fixedRowsTableView->verticalHeader()->setDefaultSectionSize(height);
}

// --------------------------------------------------------------------------------------------------------------------

QAdvancedTableViewProxy::QAdvancedTableViewProxy(QWidget *parent) :
    QTableView(parent)
{
}

QAdvancedTableViewProxy::~QAdvancedTableViewProxy()
{
}

void QAdvancedTableViewProxy::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    if (editor->parent()->parent() == this) {
        QTableView::closeEditor(editor, hint);
    }
}

void QAdvancedTableViewProxy::focusInEvent(QFocusEvent *event)
{
    QTableView::focusInEvent(event);
    emit focusReceived();
}
