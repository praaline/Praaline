#include <QDebug>
#include <QPair>
#include <QMap>
#include <QMultiMap>
#include <QString>
#include <QList>

#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/SequenceTier.h"
using namespace Praaline::Core;

#include "AnnotationGroupingTierTableModel.h"

struct AnnotationGroupingTierTableModelData {
    SpeakerAnnotationTierGroupMap tiers;  // Speaker ID, corresponding tiers
    QString tiernameGrouping;
    QString tiernameItems;
    QStringList attributesGrouping;
    QStringList attributesItems;
};

AnnotationGroupingTierTableModel::AnnotationGroupingTierTableModel(Praaline::Core::SpeakerAnnotationTierGroupMap &tiers,
                                                                   const QString &tiernameGrouping, const QString &tiernameItems,
                                                                   const QStringList &attributesGrouping, const QStringList &attributesItems,
                                                                   QObject *parent) :
    TimelineTableModelBase(parent), d(new AnnotationGroupingTierTableModelData)
{
    d->tiers = tiers;
    d->tiernameGrouping = tiernameGrouping;
    d->tiernameItems = tiernameItems;
    d->attributesGrouping = attributesGrouping;
    d->attributesItems = attributesItems;
    foreach (QString speakerID, tiers.keys()) {
        IntervalTier *tier_grouping = tiers.value(speakerID)->getIntervalTierByName(tiernameGrouping);
        IntervalTier *tier_items = tiers.value(speakerID)->getIntervalTierByName(tiernameItems);
        if ((!tier_grouping) && tier_items) {
            tiers[speakerID]->addTier(new IntervalTier(tiernameGrouping, tier_items->tMin(), tier_items->tMax()));
        }
    }
    calculateTimeline(d->tiers, d->tiernameGrouping);
}

AnnotationGroupingTierTableModel::~AnnotationGroupingTierTableModel()
{
    delete d;
}

QModelIndex AnnotationGroupingTierTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int AnnotationGroupingTierTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_timeline.count();
}

int AnnotationGroupingTierTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->attributesGrouping.count() + 5;
}

QVariant AnnotationGroupingTierTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        if (section == 0)
            return tr("Speaker ID");
        else if (section == 1)
            return tr("tMin");
        else if (section == 2)
            return tr("tMax");
        else if (section == 3)
            return tr("Sequence of %1").arg(d->tiernameItems);
        else if (section == 4)
            return d->tiernameGrouping;
        else if ((section - 5) >= 0 && (section - 5) < d->attributesGrouping.count()) {
            QString attributeID = d->attributesGrouping.at(section - 5);
            if (attributeID.isEmpty())
                return d->tiernameGrouping;
            return attributeID;
        }
        else
            return QVariant();
    }
    else if (orientation == Qt::Vertical) {
        return section + 1;
    }
    return QVariant();
}

QVariant AnnotationGroupingTierTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= rowCount()) return QVariant();
    if (index.column() < 0 || index.column() >= columnCount()) return QVariant();

    // Translate row index to timeline position
    TimelineData td = m_timeline.at(index.row());
    int dataCellIndex = index.column();
    int attributeIndex = index.column() - 5;

    // Get grouping interval
    AnnotationTierGroup *spk_tiers = d->tiers.value(td.speakerID, 0);
    if (!spk_tiers) return QVariant();
    IntervalTier *tier_grouping = spk_tiers->getIntervalTierByName(d->tiernameGrouping);
    if (!tier_grouping) return QVariant();
    Interval *intv_group = tier_grouping->interval(td.index);
    if (!intv_group) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (dataCellIndex == 0) {
            return td.speakerID;
        }
        if (dataCellIndex == 1) {
            return intv_group->tMin().toDouble();
        }
        else if (dataCellIndex == 2) {
            return intv_group->tMax().toDouble();
        }
        else if (dataCellIndex == 3) {
            IntervalTier *tier_items = spk_tiers->getIntervalTierByName(d->tiernameItems);
            if (!tier_items) return QVariant();
            QList<Interval *> intervals = tier_items->getIntervalsContainedIn(intv_group);
            return QVariant::fromValue(intervals);
        }
        else if (dataCellIndex == 4) {
            return intv_group->text();
        }
        else if (attributeIndex >= 0 && attributeIndex < d->attributesGrouping.count()) {
            return intv_group->attribute(d->attributesGrouping.at(attributeIndex));
        }
    }
    else if (role == Qt::BackgroundColorRole) {
        return m_speakerBackgroundColors.value(td.speakerID, QColor("White"));
    }
    return QVariant();
}

bool AnnotationGroupingTierTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    if (index.row() < 0 || index.row() >= rowCount()) return false;
    if (index.column() < 0 || index.column() >= columnCount()) return false;

    // Translate row index to timeline position
    TimelineData td = m_timeline.at(index.row());
    int dataCellIndex = index.column();
    int attributeIndex = index.column() - 5;

    // Get grouping interval
    AnnotationTierGroup *spk_tiers = d->tiers.value(td.speakerID, 0);
    if (!spk_tiers) return false;
    IntervalTier *tier_grouping = spk_tiers->getIntervalTierByName(d->tiernameGrouping);
    if (!tier_grouping) return false;
    Interval *intv_group = tier_grouping->interval(td.index);
    if (!intv_group) return false;

    if (dataCellIndex == 3) {

    }
    else if (dataCellIndex == 4) {
        intv_group->setText(value.toString());
        emit dataChanged(index, index);
        return true;
    }
    else if (attributeIndex >= 0 && attributeIndex < d->attributesGrouping.count()) {
        intv_group->setAttribute(d->attributesGrouping.at(attributeIndex), value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags AnnotationGroupingTierTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    if (index.column() >= 3)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractTableModel::flags(index);
}

void AnnotationGroupingTierTableModel::splitGroup(int groupIndex, int itemIndex)
{
    Q_UNUSED(groupIndex)
    Q_UNUSED(itemIndex)
}

void AnnotationGroupingTierTableModel::mergeGroups(int groupIndexStart, int groupIndexEnd)
{
    Q_UNUSED(groupIndexStart)
    Q_UNUSED(groupIndexEnd)
}



