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
#include "PraalineCore/Base/RealTime.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "TimelineTableModelBase.h"

struct AnnotationTierModelData;

class AnnotationMultiTierTableModel : public TimelineTableModelBase
{
    Q_OBJECT
public:
    struct AnnotationTierCell {
        RealTime tMin;
        int attributeIndex;
        QString speakerID;
        QVariant value;
    };

    explicit AnnotationMultiTierTableModel(Praaline::Core::SpeakerAnnotationTierGroupMap &tiers,
                                           const QString &tiernameMinimal,
                                           const QList<QPair<QString, QString> > &attributes,
                                           Qt::Orientation orientation,
                                           QObject *parent = nullptr);
    ~AnnotationMultiTierTableModel() override;

    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation);
    QModelIndex modelIndexAtTime(const RealTime &time) const override;

    bool splitAnnotations(const QModelIndex &index, RealTime splitMinimalAt = RealTime());
    bool splitAnnotations(int timelineIndex, int dataIndex, RealTime splitMinimalAt = RealTime());
    bool mergeAnnotations(const QModelIndexList &indices);
    bool mergeAnnotations(int dataIndex, const QList<int> &timelineIndices);

    QList<int> attributeIndicesExceptContext() const;
    QList<AnnotationTierCell> dataBlock(const RealTime &from, const RealTime &to, const QList<int> &attributeIndices);

signals:

public slots:

private:
    AnnotationTierModelData *d;

    QVariant dataCell(Praaline::Core::AnnotationTierGroup *spk_tiers, TimelineData &td,
                      const QString &levelID, const QString &attributeID) const;
};

#endif // ANNOTATIONTIERMODEL_H
