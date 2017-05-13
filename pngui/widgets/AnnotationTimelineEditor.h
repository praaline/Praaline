#ifndef ANNOTATIONTIMELINEEDITOR_H
#define ANNOTATIONTIMELINEEDITOR_H

#include <QObject>
#include <QPointer>
#include <QMap>
#include <QWidget>
#include <QItemSelection>
#include <QIdentityProxyModel>
#include <QModelIndex>
#include "base/RealTime.h"
#include "model/annotation/AnnotationTierModel.h"
#include "SelectionListDataProviderProxy.h"

struct AnnotationTimelineEditorData;

class AnnotationTimelineEditor : public QWidget
{
    Q_OBJECT

public:
    AnnotationTimelineEditor(QWidget *parent = 0);
    ~AnnotationTimelineEditor();

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation);

    void setData(QMap<QString, QPointer<Praaline::Core::AnnotationTierGroup> > &tierGroups,
                 const QList<QPair<QString, QString> > &columns);
    void addTierGroup(QString speakerID, QPointer<Praaline::Core::AnnotationTierGroup> tierGroup);
    void removeTierGroup(QString speakerID);
    const QMap<QString, QPointer<Praaline::Core::AnnotationTierGroup> > &tierGroups() const;

    void setColumns(const QList<QPair<QString, QString> > &columns);
    const QList<QPair<QString, QString> > &columns() const;

    AnnotationTierModel *model();

    void moveToTime(const RealTime &time);
    void moveToTime(const RealTime &time, double &tMin_msec, double &tMax_msec);
    RealTime currentTime() const;

    void annotationsSplit(const RealTime &time);

    bool exportToTabSeparated(const QString &filename);

    void resizeColumnsToContents();

public slots:
    void annotationsSplit();
    void annotationsMerge();
    void toggleOrientation();
    void removeSorting();
    void exportToTabSeparated();

private slots:
    void resultChanged(int filterRows, int unfilteredRows);
    void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void contextMenuRequested(const QPoint & point);

signals:
    void selectedRowsChanged(QList<int> selectedRowsChanged);
    void currentIndexChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void initModel();
    void initAdvancedTableView();
    void moveToRow(int row);
    void moveToColumn(int row);

    AnnotationTimelineEditorData *d;
};

#endif // ANNOTATIONTIMELINEEDITOR_H
