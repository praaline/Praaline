#ifndef TIMELINEEDITORWIDGETBASE_H
#define TIMELINEEDITORWIDGETBASE_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>
#include <QModelIndex>
#include <QItemSelection>
#include <QWidget>
#include "pncore/base/RealTime.h"
#include "pngui/model/annotation/TimelineTableModelBase.h"
#include "GridViewWidget.h"

namespace Praaline {
namespace Core {
class AnnotationTierGroup;
}
}

// Base class for timeline-based editor widgets, showing their data in a GridViewWidget

class TimelineEditorWidgetBase : public QWidget
{
    Q_OBJECT

public:
    explicit TimelineEditorWidgetBase(QWidget *parent = nullptr);
    ~TimelineEditorWidgetBase();

    virtual void addTierGroup(const QString &speakerID, QPointer<Praaline::Core::AnnotationTierGroup> tierGroup);
    virtual void removeTierGroup(const QString &speakerID);
    virtual const Praaline::Core::SpeakerAnnotationTierGroupMap &tierGroups() const;

    virtual void moveToTime(const RealTime &time) = 0;
    virtual void moveToTime(const RealTime &time, double &tMin_msec, double &tMax_msec) = 0;
    virtual RealTime currentTime() const = 0;

    virtual void resizeColumnsToContents();

signals:
    void selectedRowsChanged(QList<int> selectedRowsChanged);
    void currentIndexChanged(const QModelIndex &current, const QModelIndex &previous);

protected slots:
    virtual void resultChanged(int filterRows, int unfilteredRows);
    virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    virtual void contextMenuRequested(const QPoint &point);

protected:
    virtual void clearModel() {}
    virtual void initModel() {}

    virtual void moveToRow(TimelineTableModelBase *model, int row);
    virtual void moveToColumn(TimelineTableModelBase *model, int column);

    Praaline::Core::SpeakerAnnotationTierGroupMap m_tiers;
    GridViewWidget *m_view;
    QList<int> m_selectedRows;
};

#endif // TIMELINEEDITORWIDGETBASE_H
