#include <QObject>
#include <QPointer>
#include <QMap>
#include <QGridLayout>
#include <QFileDialog>
#include <QSortFilterProxyModel>
#include "model/annotation/AnnotationMultiTierTableModel.h"
#include "AnnotationMultiTierEditorWidget.h"

#include "grid/QAdvancedTableView.h"
#include "grid/QAdvancedHeaderView.h"
#include "grid/QConditionalDecorationDialog.h"
#include "grid/QConditionalDecorationProxyModel.h"
#include "grid/QFilterModel.h"
#include "grid/QFilterModelProxy.h"
#include "grid/QFilterViewConnector.h"
#include "grid/QGroupingProxyModel.h"
#include "grid/QConditionalDecoration.h"
#include "grid/QFixedRowsTableView.h"
#include "grid/QMimeDataUtil.h"
#include "grid/QRangeFilter.h"
#include "grid/QUniqueValuesProxyModel.h"
#include "grid/QSelectionListFilter.h"

#include "grid/QValueFilter.h"

#include "GridViewWidget.h"

using namespace Praaline::Core;


struct AnnotationMultiTierEditorWidgetData {
    AnnotationMultiTierEditorWidgetData() :
        orientation(Qt::Vertical), model(0)
    {}

    Qt::Orientation orientation;
    QList<QPair<QString, QString> > columns;
    AnnotationMultiTierTableModel *model;
};

AnnotationMultiTierEditorWidget::AnnotationMultiTierEditorWidget(QWidget *parent) :
    TimelineEditorWidgetBase(parent), d(new AnnotationMultiTierEditorWidgetData)
{
}

AnnotationMultiTierEditorWidget::~AnnotationMultiTierEditorWidget()
{
    delete d;
}

// TimelineEditorWidgetBase implementation
void AnnotationMultiTierEditorWidget::clearModel()
{
    if (d->model) {
        d->model = nullptr;
    }
}

void AnnotationMultiTierEditorWidget::initModel()
{
    if (d->columns.isEmpty()) return;
    if (m_tiers.isEmpty()) return;
    QString minimalLevel = d->columns.first().first;
    d->model = new AnnotationMultiTierTableModel(m_tiers, minimalLevel, d->columns, d->orientation, this);
    SelectionListDataProviderProxy* p = new SelectionListDataProviderProxy(this);
    p->setSourceModel(d->model);
    m_view->tableView()->setModel(p);
    updateGeometry();
}


void AnnotationMultiTierEditorWidget::annotationsSplit(const RealTime &time)
{
    if (!d->model) return;
    d->model->splitAnnotations(m_view->tableView()->selectionModel()->currentIndex(), time);
}

void AnnotationMultiTierEditorWidget::annotationsSplit()
{
    if (!d->model) return;
    d->model->splitAnnotations(m_view->tableView()->selectionModel()->currentIndex());
}

void AnnotationMultiTierEditorWidget::annotationsMerge()
{
    if (!d->model) return;
    d->model->mergeAnnotations(m_view->tableView()->selectionModel()->currentIndex().column(), m_selectedRows);
}

void AnnotationMultiTierEditorWidget::removeSorting()
{
    if (!d->model) return;
    m_view->tableView()->sortByColumn(1, Qt::AscendingOrder);
    m_view->tableView()->sortByColumn(-1, Qt::AscendingOrder);
}

void AnnotationMultiTierEditorWidget::setData(SpeakerAnnotationTierGroupMap &tierGroups,
                                              const QList<QPair<QString, QString> > &columns)
{
    if (d->model) { delete d->model; d->model = 0; }
    m_tiers = tierGroups;
    d->columns = columns;
    initModel();
}

void AnnotationMultiTierEditorWidget::setColumns(const QList<QPair<QString, QString> > &columns)
{
    if (d->model) { delete d->model; d->model = 0; }
    d->columns = columns;
    initModel();
}

const QList<QPair<QString, QString> > & AnnotationMultiTierEditorWidget::columns() const
{
    return d->columns;
}

AnnotationMultiTierTableModel *AnnotationMultiTierEditorWidget::model()
{
    return d->model;
}

Qt::Orientation AnnotationMultiTierEditorWidget::orientation() const
{
    return d->orientation;
}

void AnnotationMultiTierEditorWidget::setOrientation(Qt::Orientation orientation)
{
    if (d->orientation == orientation) return;
    // update interal state and views
    d->orientation = orientation;
    if (orientation == Qt::Horizontal) {
        m_view->tableView()->setShowFilter(false);
        m_view->tableView()->setSortingEnabled(false);
    } else {
        m_view->tableView()->setShowFilter(true);
        m_view->tableView()->setSortingEnabled(true);
    }
    // update model, if available
    if (!d->model) return;
    d->model->setOrientation(orientation);
}

void AnnotationMultiTierEditorWidget::toggleOrientation()
{
    // this is a public slot
    if (d->orientation == Qt::Vertical)
        setOrientation(Qt::Horizontal);
    else
        setOrientation(Qt::Vertical);
}

void AnnotationMultiTierEditorWidget::resultChanged(int filterRows, int unfilteredRows)
{
    if (d->orientation == Qt::Vertical) {
        if (unfilteredRows - filterRows > 0)
            m_view->setResultsLabelText(tr("Result: %1 of %2").arg(filterRows).arg(unfilteredRows));
        else
            m_view->setResultsLabelText(tr("All intervals: %1").arg(filterRows));
    } else {
        m_view->setResultsLabelText("");
    }
}

RealTime AnnotationMultiTierEditorWidget::currentTime() const
{
    if (!d->model) return RealTime();
    int index = -1;
    if (d->model->orientation() == Qt::Vertical) {
        index = m_view->tableView()->currentIndex().row();
    } else {
        index = m_view->tableView()->currentIndex().column();
    }
    return d->model->timeAtTimelineIndex(index);
}

void AnnotationMultiTierEditorWidget::moveToTime(const RealTime &time)
{
    if (!d->model) return;
    int index = d->model->timelineIndexAtTime(time);
    if (d->model->orientation() == Qt::Vertical)
        moveToRow(d->model, index);
    else
        moveToColumn(d->model, index);
}

void AnnotationMultiTierEditorWidget::moveToTime(const RealTime &time, double &tMin_msec, double &tMax_msec)
{
    if (!d->model) return;
    int index = d->model->timelineIndexAtTime(time, tMin_msec, tMax_msec);
    if (d->model->orientation() == Qt::Vertical)
        moveToRow(d->model, index);
    else
        moveToColumn(d->model, index);
}


