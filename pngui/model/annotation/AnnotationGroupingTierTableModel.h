#ifndef SEQUENCETABLEMODEL_H
#define SEQUENCETABLEMODEL_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QAbstractTableModel>
#include "pncore/base/RealTime.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "TimelineTableModelBase.h"

struct AnnotationGroupingTierTableModelData;

class AnnotationGroupingTierTableModel : public TimelineTableModelBase
{
    Q_OBJECT

public:
    explicit AnnotationGroupingTierTableModel(Praaline::Core::SpeakerAnnotationTierGroupMap &tiers,
                                              const QString &tiernameGrouping, const QString &tiernameItems,
                                              const QStringList &attributesGrouping = QStringList(),
                                              const QStringList &attributesItems = QStringList(),
                                              QObject *parent = nullptr);
    ~AnnotationGroupingTierTableModel();

    QModelIndex parent(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void splitGroup(int groupIndex, int itemIndex);
    void mergeGroups(int groupIndexStart, int groupIndexEnd);

private:
    AnnotationGroupingTierTableModelData *d;
};

#endif // SEQUENCETABLEMODEL_H
