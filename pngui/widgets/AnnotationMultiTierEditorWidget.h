#ifndef ANNOTATIONMULTITIEREDITORWIDGET_H
#define ANNOTATIONMULTITIEREDITORWIDGET_H

#include <QObject>
#include <QPointer>
#include <QMap>
#include <QWidget>
#include <QItemSelection>
#include <QIdentityProxyModel>
#include <QModelIndex>
#include "PraalineCore/Base/RealTime.h"
#include "model/annotation/AnnotationMultiTierTableModel.h"
#include "TimelineEditorWidgetBase.h"
#include "SelectionListDataProviderProxy.h"

struct AnnotationMultiTierEditorWidgetData;

class AnnotationMultiTierEditorWidget : public TimelineEditorWidgetBase
{
    Q_OBJECT

public:
    explicit AnnotationMultiTierEditorWidget(QWidget *parent = nullptr);
    ~AnnotationMultiTierEditorWidget();

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation);

    void setData(Praaline::Core::SpeakerAnnotationTierGroupMap &tierGroups,
                 const QList<QPair<QString, QString> > &columns);

    void setColumns(const QList<QPair<QString, QString> > &columns);
    const QList<QPair<QString, QString> > &columns() const;

    AnnotationMultiTierTableModel *model();

    void annotationsSplit(const RealTime &time);

    void moveToTime(const RealTime &time) override;
    void moveToTime(const RealTime &time, double &tMin_msec, double &tMax_msec) override;
    RealTime currentTime() const override;

public slots:
    void annotationsSplit();
    void annotationsMerge();
    void toggleOrientation();
    void removeSorting();

protected slots:
    void resultChanged(int filterRows, int unfilteredRows) override;

protected:
    void clearModel() override;
    void initModel() override;

private:
    AnnotationMultiTierEditorWidgetData *d;
};

#endif // ANNOTATIONMULTITIEREDITORWIDGET_H
