#ifndef ANNOTATIONTIERMODEL_H
#define ANNOTATIONTIERMODEL_H

#include <QObject>
#include <QPointer>
#include <QAbstractTableModel>
#include <QPair>
#include <QList>
#include <QMap>
#include <QHash>
#include <QColor>
#include <QMultiMap>
#include "pncore/base/RealTime.h"
#include "pncore/annotation/AnnotationTierGroup.h"

struct AnnotationTierModelData;

class AnnotationTierModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    struct AnnotationTierCell {
        RealTime tMin;
        int attributeIndex;
        QString speakerID;
        QVariant value;
    };

    explicit AnnotationTierModel(QMap<QString, QPointer<Praaline::Core::AnnotationTierGroup> > &tiers,
                                 const QString &tiernameMinimal,
                                 const QList<QPair<QString, QString> > &attributes,
                                 Qt::Orientation orientation,
                                 QObject *parent = 0);
    ~AnnotationTierModel();

    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation);

    int timelineIndexAtTime(const RealTime &time) const;
    int timelineIndexAtTime(const RealTime &time, double &tMin_msec, double &tMax_msec) const;
    QModelIndex modelIndexAtTime(const RealTime &time) const;
    RealTime timeAtTimelineIndex(int timelineIndex) const;

    bool splitAnnotations(const QModelIndex &index, RealTime splitMinimalAt = RealTime());
    bool splitAnnotations(int timelineIndex, int dataIndex, RealTime splitMinimalAt = RealTime());
    bool mergeAnnotations(const QModelIndexList &indices);
    bool mergeAnnotations(int dataIndex, const QList<int> &timelineIndices);

    RealTime getStartTime() const;
    RealTime getEndTime() const;
    QList<int> attributeIndicesExceptContext() const;
    QList<AnnotationTierCell> dataBlock(const RealTime &from, const RealTime &to, const QList<int> &attributeIndices);

signals:

public slots:

private:
    AnnotationTierModelData *d;
};

#endif // ANNOTATIONTIERMODEL_H
