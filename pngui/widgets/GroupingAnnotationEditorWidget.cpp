#include <QObject>
#include <QPointer>
#include <QMap>
#include <QGridLayout>
#include <QFileDialog>
#include <QSortFilterProxyModel>
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
#include "grid/qvaluefilter.h"
#include "GridViewWidget.h"

#include "model/annotation/AnnotationGroupingTierTableModel.h"
#include "GroupingAnnotationEditorWidget.h"

using namespace Praaline::Core;


struct GroupingAnnotationEditorWidgetData {
    GroupingAnnotationEditorWidgetData() :
        model(0)
    {}

    AnnotationGroupingTierTableModel *model;
    QString tiernameGrouping;
    QString tiernameItems;
    QStringList attributesGrouping;
    QStringList attributesItems;
};

GroupingAnnotationEditorWidget::GroupingAnnotationEditorWidget(QWidget *parent) :
    TimelineEditorWidgetBase(parent), d(new GroupingAnnotationEditorWidgetData)
{
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

void GroupingAnnotationEditorWidget::setData(QMap<QString, QPointer<Praaline::Core::AnnotationTierGroup> > &tierGroups,
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


