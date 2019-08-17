#include <QObject>
#include <QPointer>
#include <QMap>
#include <QGridLayout>
#include <QFileDialog>
#include <QSortFilterProxyModel>
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

#include "pngui/model/annotation/AnnotationGroupingTierTableModel.h"
#include "pngui/delegate/IntervalSequenceDelegate.h"

#include "GroupingAnnotationEditorWidget.h"

using namespace Praaline::Core;


struct GroupingAnnotationEditorWidgetData {
    GroupingAnnotationEditorWidgetData() :
        model(0), intervalSequenceDelegate(0)
    {}

    AnnotationGroupingTierTableModel *model;
    QString tiernameGrouping;
    QString tiernameItems;
    QStringList attributesGrouping;
    QStringList attributesItems;

    IntervalSequenceDelegate *intervalSequenceDelegate;
};

GroupingAnnotationEditorWidget::GroupingAnnotationEditorWidget(QWidget *parent) :
    TimelineEditorWidgetBase(parent), d(new GroupingAnnotationEditorWidgetData)
{
    d->intervalSequenceDelegate = new IntervalSequenceDelegate(this);
    m_view->tableView()->setItemDelegate(d->intervalSequenceDelegate);
}

GroupingAnnotationEditorWidget::~GroupingAnnotationEditorWidget()
{
    if (d->model) delete d->model;
    delete d;
}

// TimelineEditorWidgetBase implementation

void GroupingAnnotationEditorWidget::clearModel()
{
    if (d->model) { delete d->model; d->model = 0; }
}

void GroupingAnnotationEditorWidget::initModel()
{
    if (d->tiernameGrouping.isEmpty()) return;
    if (d->tiernameItems.isEmpty()) return;
    if (m_tiers.isEmpty()) return;
    d->model = new AnnotationGroupingTierTableModel(m_tiers, d->tiernameGrouping, d->tiernameItems, d->attributesGrouping, d->attributesItems, this);
    SelectionListDataProviderProxy* p = new SelectionListDataProviderProxy(this);
    p->setSourceModel(d->model);
    m_view->tableView()->setModel(p);
    updateGeometry();
}

// Get and Set model properties

void GroupingAnnotationEditorWidget::setData(Praaline::Core::SpeakerAnnotationTierGroupMap &tierGroups,
                                             const QString &tiernameGrouping, const QString &tiernameItems,
                                             const QStringList &attributesGrouping, const QStringList &attributesItems)
{
    if (d->model) { delete d->model; d->model = 0; }
    m_tiers = tierGroups;
    d->tiernameGrouping = tiernameGrouping;
    d->tiernameItems = tiernameItems;
    d->attributesGrouping = attributesGrouping;
    d->attributesItems = attributesItems;
    initModel();
}

QString GroupingAnnotationEditorWidget::tiernameGrouping() const
{
    return d->tiernameGrouping;
}

void GroupingAnnotationEditorWidget::setTiernameGrouping(const QString &tiernameGrouping)
{
    if (d->model) { delete d->model; d->model = 0; }
    d->tiernameGrouping = tiernameGrouping;
    initModel();
}

QString GroupingAnnotationEditorWidget::tiernameItems() const
{
    return d->tiernameItems;
}

void GroupingAnnotationEditorWidget::setTiernameItems(const QString tiernameItems)
{
    if (d->model) { delete d->model; d->model = 0; }
    d->tiernameItems = tiernameItems;
    initModel();
}

QStringList GroupingAnnotationEditorWidget::attributesGrouping() const
{
    return d->attributesGrouping;
}

void GroupingAnnotationEditorWidget::setAttributesGrouping(const QStringList &attributesGrouping)
{
    if (d->model) { delete d->model; d->model = 0; }
    d->attributesGrouping = attributesGrouping;
    initModel();
}

QStringList GroupingAnnotationEditorWidget::attributesItems() const
{
    return d->attributesItems;
}

void GroupingAnnotationEditorWidget::setAttributesItems(const QStringList &attributesItems)
{
    if (d->model) { delete d->model; d->model = 0; }
    d->attributesItems = attributesItems;
    initModel();
}

AnnotationGroupingTierTableModel *GroupingAnnotationEditorWidget::model()
{
    return d->model;
}


void GroupingAnnotationEditorWidget::removeSorting()
{
    if (!d->model) return;
    m_view->tableView()->sortByColumn(1, Qt::AscendingOrder);
    m_view->tableView()->sortByColumn(-1, Qt::AscendingOrder);
}

RealTime GroupingAnnotationEditorWidget::currentTime() const
{
    if (!d->model) return RealTime();
    int index = m_view->tableView()->currentIndex().row();
    return d->model->timeAtTimelineIndex(index);
}

void GroupingAnnotationEditorWidget::moveToTime(const RealTime &time)
{
    if (!d->model) return;
    int index = d->model->timelineIndexAtTime(time);
    moveToRow(d->model, index);
}

void GroupingAnnotationEditorWidget::moveToTime(const RealTime &time, double &tMin_msec, double &tMax_msec)
{
    if (!d->model) return;
    int index = d->model->timelineIndexAtTime(time, tMin_msec, tMax_msec);
    moveToRow(d->model, index);
}


void GroupingAnnotationEditorWidget::groupingSplit(int itemIndex)
{
    if (!d->model) return;
    d->model->splitGroup(0, itemIndex);
}

void GroupingAnnotationEditorWidget::groupingMergeWithPrevious()
{
    if (!d->model) return;
    //d->model->mergeGroups(int groupIndexStart, int groupIndexEnd);
}

void GroupingAnnotationEditorWidget::groupingMergeWithNext()
{
    if (!d->model) return;
    //d->model->mergeGroups(int groupIndexStart, int groupIndexEnd);
}


