#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>
#include <QModelIndex>
#include <QItemSelection>
#include <QGridLayout>
#include <QWidget>
#include "pncore/base/RealTime.h"

#include "grid/QTextFilter.h"
#include "grid/QFilterModel.h"
#include "grid/QFilterModelProxy.h"

#include "TimelineEditorWidgetBase.h"

TimelineEditorWidgetBase::TimelineEditorWidgetBase(QWidget *parent)
    : QWidget(parent)
{
    // Grid layout
    QGridLayout *gridLayout;
    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(0);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);
    // GridViewWidget
    m_view = new GridViewWidget(this);
    gridLayout->addWidget(m_view);
    // make grid a little tighter
    m_view->tableView()->verticalHeader()->setDefaultSectionSize(20);
    m_view->tableView()->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive); // StretchLastSection(true);
    // Connect signals
    connect(m_view->tableView()->filterProxyModel(), SIGNAL(resultCountChanged(int, int)),
            this, SLOT(resultChanged(int, int)));
    connect(m_view->tableView()->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(selectionChanged(QItemSelection, QItemSelection)));
    connect(m_view->tableView()->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(currentChanged(QModelIndex,QModelIndex)));
    m_view->tableView()->horizontalHeader()->setSectionsClickable(true);
    m_view->tableView()->setDefaultFilterType(0, QTextFilter::Type);
}

TimelineEditorWidgetBase::~TimelineEditorWidgetBase()
{
    clearModel();
}

void TimelineEditorWidgetBase::addTierGroup(const QString &speakerID, QPointer<Praaline::Core::AnnotationTierGroup> tierGroup)
{
    clearModel();
    m_tiers.insert(speakerID, tierGroup);
    initModel();
}

void TimelineEditorWidgetBase::removeTierGroup(const QString &speakerID)
{
    if (!m_tiers.contains(speakerID)) return;
    clearModel();
    m_tiers.remove(speakerID);
    initModel();
}

const QMap<QString, QPointer<Praaline::Core::AnnotationTierGroup> > &TimelineEditorWidgetBase::tierGroups() const
{
    return m_tiers;
}

void TimelineEditorWidgetBase::resultChanged(int filterRows, int unfilteredRows)
{
    if (unfilteredRows - filterRows > 0)
        m_view->setResultsLabelText(tr("Result: %1 of %2").arg(filterRows).arg(unfilteredRows));
    else
        m_view->setResultsLabelText(tr("All intervals: %1").arg(filterRows));
}

void TimelineEditorWidgetBase::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    QList<int> rowsSelected;
    foreach (QModelIndex index, m_view->tableView()->selectionModel()->selectedIndexes()) {
        if (!rowsSelected.contains(index.row()))
            rowsSelected << index.row();
    }
    m_selectedRows = rowsSelected;
    emit selectedRowsChanged(rowsSelected);
}

void TimelineEditorWidgetBase::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit currentIndexChanged(current, previous);
}

void TimelineEditorWidgetBase::contextMenuRequested(const QPoint &point)
{

}

void TimelineEditorWidgetBase::moveToRow(TimelineTableModelBase *model, int row)
{
    if (model->rowCount() == 0) return;
    int height = m_view->tableView()->datatableHeight();
    int rh =  height / m_view->tableView()->rowHeight(0);
    int pos = m_view->tableView()->rowViewportPosition(row);
    int padding = rh * 5;
    if (pos < padding || pos >= height - padding) {
        int topRow = row + rh/5;
        if (topRow < 0) topRow = 0;
        m_view->tableView()->scrollTo(m_view->tableView()->filterProxyModel()->index(topRow, 0));
    }
    bool haveRowSelected = false;
    for (int i = 0; i < model->columnCount(); ++i) {
        if (m_view->tableView()->selectionModel()->isSelected(model->index(row, i))) {
            haveRowSelected = true;
            break;
        }
    }
    if (!haveRowSelected) {
        m_view->tableView()->selectionModel()->setCurrentIndex(model->index(row, 0),
                                                               QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}

void TimelineEditorWidgetBase::moveToColumn(TimelineTableModelBase *model, int column)
{
    int width = m_view->tableView()->datatableWidth();
    int pos = m_view->tableView()->columnViewportPosition(column);
    int padding = width / 5;
    if (pos < padding || pos >= width - padding) {
        int leftColumn = column + 4;
        if (leftColumn < 0) leftColumn = 0;
        m_view->tableView()->scrollTo(m_view->tableView()->filterProxyModel()->index(0, leftColumn));
    }
    bool haveColumnSelected = false;
    for (int i = 0; i < model->rowCount(); ++i) {
        if (m_view->tableView()->selectionModel()->isSelected(model->index(i, column))) {
            haveColumnSelected = true;
            break;
        }
    }
    if (!haveColumnSelected) {
        m_view->tableView()->selectionModel()->setCurrentIndex(model->index(0, column),
                                                               QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Columns);
    }
}

void TimelineEditorWidgetBase::resizeColumnsToContents()
{
    if (m_view && m_view->tableView())
        m_view->tableView()->resizeColumnsToContents();
}

