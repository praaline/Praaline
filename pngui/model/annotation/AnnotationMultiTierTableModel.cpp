#include <QDebug>
#include <QPair>
#include <QMap>
#include <QMultiMap>
#include <QString>
#include <QList>

#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "AnnotationMultiTierTableModel.h"

struct AnnotationTierModelData {
    AnnotationTierModelData() : orientation(Qt::Vertical) {}

    Qt::Orientation orientation;
    SpeakerAnnotationTierGroupMap tiers;  // Speaker ID, corresponding tiers
    QString tiernameMinimal;
    QList<QPair<QString, QString> > attributes;           // Level ID, Attribute ID
};

AnnotationMultiTierTableModel::AnnotationMultiTierTableModel(SpeakerAnnotationTierGroupMap &tiers,
                                                             const QString &tiernameMinimal, const QList<QPair<QString, QString> > &attributes,
                                                             Qt::Orientation orientation, QObject *parent) :
    TimelineTableModelBase(parent), d(new AnnotationTierModelData)
{
    d->orientation = orientation;
    d->tiers = tiers;
    d->tiernameMinimal = tiernameMinimal;
    d->attributes = attributes;
    calculateTimeline(d->tiers, d->tiernameMinimal);
}

AnnotationMultiTierTableModel::~AnnotationMultiTierTableModel()
{
    delete d;
}

QModelIndex AnnotationMultiTierTableModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int AnnotationMultiTierTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (d->orientation == Qt::Vertical)
        return m_timeline.count();
    // else
    return d->attributes.count() + 3;
}

int AnnotationMultiTierTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (d->orientation == Qt::Vertical)
        return d->attributes.count() + 3;
    // else
    return m_timeline.count();
}

QList<int> AnnotationMultiTierTableModel::attributeIndicesExceptContext() const
{
    QList<int> ret;
    for (int attributeIndex = 0; attributeIndex < d->attributes.count(); ++attributeIndex) {
        QString attributeID = d->attributes.at(attributeIndex).second;
        if (attributeID != "_context") ret << attributeIndex;
    }
    return ret;
}

QVariant AnnotationMultiTierTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if ( ((d->orientation == Qt::Vertical) && (orientation == Qt::Horizontal)) ||
         ((d->orientation == Qt::Horizontal) && (orientation == Qt::Vertical)) ) {
        if (section == 0)
            return tr("Speaker ID");
        else if (section == 1)
            return tr("tMin");
        else if (section == 2)
            return tr("tMax");
        else if ((section - 3) >= 0 && (section - 3) < d->attributes.count()) {
            QString levelID = d->attributes.at(section - 3).first;
            QString attributeID = d->attributes.at(section - 3).second;
            if (attributeID.isEmpty())
                return levelID;
            else if (attributeID == "_context")
                return levelID + " context";
            else if (attributeID.startsWith("_group:"))
                return QString("(%1)%2").arg(attributeID.section(":", 1, 1)).arg(levelID);
            return attributeID;
        }
        else
            return QVariant();
    }
    else if ( ((d->orientation == Qt::Vertical) && (orientation == Qt::Vertical)) ||
              ((d->orientation == Qt::Horizontal) && (orientation == Qt::Horizontal)) ) {
        return section + 1;
    }
    return QVariant();
}

// Private
QVariant AnnotationMultiTierTableModel::dataCell(AnnotationTierGroup *spk_tiers, TimelineData &td,
                                                 const QString &levelID, const QString &attributeID) const
{
    IntervalTier *tier = spk_tiers->getIntervalTierByName(levelID);
    if (!tier) return QVariant();
    int intvID = tier->intervalIndexAtTime(td.tCenter);
    if (intvID < 0) return QVariant();

    if (attributeID.isEmpty()) {
        return tier->interval(intvID)->text();
    }
    else if (attributeID == "_context") {
        QList<Interval *> left = tier->getContext(intvID, -7);
        QList<Interval *> right = tier->getContext(intvID, 7);
        QString s;
        foreach (Interval *intvL, left) s.append(intvL->text()).append(" ");
        s.append("<").append(tier->interval(intvID)->text()).append("> ");
        foreach (Interval *intvR, right) s.append(intvR->text()).append(" ");
        if (s.endsWith(" ")) s.chop(1);
        return s;
    }
    else if (attributeID.startsWith("_group:") || attributeID.startsWith("_group_contains:")) {
        QString levelIDgrouped = attributeID.section(":", 1, 1);
        IntervalTier *tier_groupped = spk_tiers->getIntervalTierByName(levelIDgrouped);
        if (!tier_groupped) return QVariant();
        QList<Interval *> intervals;
        if      (attributeID.startsWith("_group:"))          intervals = tier_groupped->getIntervalsOverlappingWith(tier->interval(intvID));
        else if (attributeID.startsWith("_group_contains:")) intervals = tier_groupped->getIntervalsContainedIn(tier->interval(intvID));
        QString s;
        foreach (Interval *intv, intervals) s.append(intv->text()).append(" ");
        return QString("(%1)%2").arg(s.trimmed()).arg(tier->interval(intvID)->text());
    }
    else if (attributeID.startsWith("_concat:") || attributeID.startsWith("_concat_contains:")) {
        QString levelIDgrouped = attributeID.section(":", 1, 1);
        IntervalTier *tier_groupped = spk_tiers->getIntervalTierByName(levelIDgrouped);
        if (!tier_groupped) return QVariant();
        QList<Interval *> intervals;
        if      (attributeID.startsWith("_concat:"))          intervals = tier_groupped->getIntervalsOverlappingWith(tier->interval(intvID));
        else if (attributeID.startsWith("_concat_contains:")) intervals = tier_groupped->getIntervalsContainedIn(tier->interval(intvID));
        QString s;
        foreach (Interval *intv, intervals) s.append(intv->text()).append(".");
        if (s.endsWith(".")) s.chop(1);
        return s.trimmed();
    }
    else {
        return tier->interval(intvID)->attribute(attributeID);
    }
    return QVariant();
}

QVariant AnnotationMultiTierTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= rowCount()) return QVariant();
    if (index.column() < 0 || index.column() >= columnCount()) return QVariant();

    // Translate row index to timeline position
    TimelineData td;
    int dataCellIndex;
    if (d->orientation == Qt::Vertical) {
        td = m_timeline.at(index.row());
        dataCellIndex = index.column();
    }
    else {
        td = m_timeline.at(index.column());
        dataCellIndex = index.row();
    }
    int attributeIndex = dataCellIndex - 3;

    AnnotationTierGroup *spk_tiers = d->tiers.value(td.speakerID, 0);
    if (!spk_tiers) return QVariant();
    IntervalTier *tier_min = spk_tiers->getIntervalTierByName(d->tiernameMinimal);
    if (!tier_min) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (dataCellIndex == 0) {
            return td.speakerID;
        }
        if (dataCellIndex == 1) {
            Interval *intv_min = tier_min->interval(td.index);
            if (intv_min) return intv_min->tMin().toDouble(); // QString::number(intv_min->tMin().toDouble(), 'f', 6);
            else return QVariant();
        }
        else if (dataCellIndex == 2) {
            Interval *intv_min = tier_min->interval(td.index);
            if (intv_min) return intv_min->tMax().toDouble(); // QString::number(intv_min->tMax().toDouble(), 'f', 6);
            else return QVariant();
        }
        else if (attributeIndex >= 0 && attributeIndex < d->attributes.count()) {
            QString levelID = d->attributes.at(attributeIndex).first;
            QString attributeID = d->attributes.at(attributeIndex).second;
            return dataCell(spk_tiers, td, levelID, attributeID);
        }
    }
    else if (role == Qt::BackgroundColorRole) {
        return m_speakerBackgroundColors.value(td.speakerID, QColor("White"));
    }
    return QVariant();
}

QList<AnnotationMultiTierTableModel::AnnotationTierCell>
AnnotationMultiTierTableModel::dataBlock(const RealTime &from, const RealTime &to, const QList<int> &attributeIndices)
{
    QList<AnnotationMultiTierTableModel::AnnotationTierCell> ret;
    int timelineIndexFrom = timelineIndexAtTime(from);
    int timelineIndexTo = timelineIndexAtTime(to);
    for (int timelineIndex = timelineIndexFrom; timelineIndex <= timelineIndexTo; ++timelineIndex) {
        TimelineData td = m_timeline.at(timelineIndex);
        AnnotationTierGroup *spk_tiers = d->tiers.value(td.speakerID, 0);
        if (!spk_tiers) continue;
        foreach (int attributeIndex, attributeIndices) {
            QString levelID = d->attributes.at(attributeIndex).first;
            QString attributeID = d->attributes.at(attributeIndex).second;
            QVariant value = dataCell(spk_tiers, td, levelID, attributeID);
            AnnotationMultiTierTableModel::AnnotationTierCell cell;
            cell.tMin = td.tMin;
            cell.attributeIndex = attributeIndex;
            cell.speakerID = td.speakerID;
            cell.value = value;
            ret << cell;
        }
    }
    return ret;
}

Qt::ItemFlags AnnotationMultiTierTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    if ( ((d->orientation == Qt::Vertical) && (index.column() >= 1)) ||
         ((d->orientation == Qt::Horizontal) && (index.row() >= 1)) )
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractTableModel::flags(index);
}

bool AnnotationMultiTierTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;
    if (role != Qt::EditRole) return false;
    if (index.row() < 0 || index.row() >= rowCount()) return false;
    if (index.column() < 0 || index.column() >= columnCount()) return false;

    TimelineData td;
    int timelineIndex, dataCellIndex;
    if (d->orientation == Qt::Vertical) {
        timelineIndex = index.row();
        dataCellIndex = index.column();
        td = m_timeline.at(timelineIndex);
    }
    else {
        timelineIndex = index.column();
        dataCellIndex = index.row();
        td = m_timeline.at(timelineIndex);
    }
    int attributeIndex = dataCellIndex - 3;

    AnnotationTierGroup *spk_tiers = d->tiers.value(td.speakerID, 0);
    if (!spk_tiers) return false;

    if (dataCellIndex == 1) {
        IntervalTier *tier_minimal = spk_tiers->getIntervalTierByName(d->tiernameMinimal);
        if (!tier_minimal) return false;
        int intervalIndex = tier_minimal->intervalIndexAtTime(td.tCenter);
        bool result = tier_minimal->moveBoundary(intervalIndex, RealTime::fromSeconds(value.toDouble()));
        if (!result) return false;
        // update timeline (previous interval's center and max, our min and center)
        if ((timelineIndex > 0) && (intervalIndex > 0)) {
            m_timeline[timelineIndex - 1].tCenter = tier_minimal->interval(intervalIndex - 1)->tCenter();
            m_timeline[timelineIndex - 1].tMax = tier_minimal->interval(intervalIndex - 1)->tMax();
        }
        m_timeline[timelineIndex].tMin = tier_minimal->interval(intervalIndex)->tMin();
        m_timeline[timelineIndex].tCenter = tier_minimal->interval(intervalIndex)->tCenter();
        // refresh grid
        if (d->orientation == Qt::Vertical) {
            int refreshFrom = index.row() - 1; if (refreshFrom < 0) refreshFrom = 0;
            emit dataChanged(AnnotationMultiTierTableModel::index(refreshFrom, 0), AnnotationMultiTierTableModel::index(index.row(), columnCount() - 1));
        } else {
            int refreshFrom = index.column() - 1; if (refreshFrom < 0) refreshFrom = 0;
            emit dataChanged(AnnotationMultiTierTableModel::index(0, refreshFrom), AnnotationMultiTierTableModel::index(rowCount() - 1, index.column()));
        }
        return true;
    }
    else if (dataCellIndex == 2) {
        IntervalTier *tier_minimal = spk_tiers->getIntervalTierByName(d->tiernameMinimal);
        if (!tier_minimal) return false;
        int intervalIndex = tier_minimal->intervalIndexAtTime(td.tCenter);
        bool result = tier_minimal->moveBoundary(intervalIndex + 1, RealTime::fromSeconds(value.toDouble()));
        if (!result) return false;
        // update timeline (our center and max, next interval's min and center)
        m_timeline[timelineIndex].tCenter = tier_minimal->interval(intervalIndex)->tCenter();
        m_timeline[timelineIndex].tMax = tier_minimal->interval(intervalIndex)->tMax();
        if ((timelineIndex < m_timeline.count() - 1) && (intervalIndex < tier_minimal->count() -1)) {
            m_timeline[timelineIndex + 1].tMin = tier_minimal->interval(intervalIndex + 1)->tMin();
            m_timeline[timelineIndex + 1].tCenter = tier_minimal->interval(intervalIndex + 1)->tCenter();
        }
        // refresh grid
        if (d->orientation == Qt::Vertical) {
            int refreshTo = index.row() + 1; if (refreshTo == rowCount()) refreshTo = rowCount() - 1;
            emit dataChanged(AnnotationMultiTierTableModel::index(index.row(), 0), AnnotationMultiTierTableModel::index(refreshTo, columnCount() - 1));
        } else {
            int refreshTo = index.column() + 1; if (refreshTo == columnCount()) refreshTo = columnCount() - 1;
            emit dataChanged(AnnotationMultiTierTableModel::index(0, index.column()), AnnotationMultiTierTableModel::index(rowCount() - 1, refreshTo));
        }
        return true;
    }
    else if (attributeIndex >= 0 && attributeIndex < d->attributes.count()) {
        IntervalTier *tier = spk_tiers->getIntervalTierByName(d->attributes.at(attributeIndex).first);
        if (!tier) return false;
        Interval *intv = tier->intervalAtTime(td.tCenter);
        if (!intv) return false;
        if (d->attributes.at(attributeIndex).second.isEmpty()) {
            intv->setText(value.toString());
            emit(dataChanged(index, index));
            return true;
        }
        else {
            intv->setAttribute(d->attributes.at(attributeIndex).second, value);
            emit(dataChanged(index, index));
            return true;
        }
    }
    return false;
}

Qt::Orientation AnnotationMultiTierTableModel::orientation() const
{
    return d->orientation;
}

void AnnotationMultiTierTableModel::setOrientation(Qt::Orientation orientation)
{
    if (d->orientation == orientation) return;
    beginResetModel();
    d->orientation = orientation;
    endResetModel();
}

// A timeline index corresponds to a row in a vertically-oriented model nd to a column in a horizontally-oriented model.
// Override default implementation.
QModelIndex AnnotationMultiTierTableModel::modelIndexAtTime(const RealTime &time) const
{
    int timelineIndex = timelineIndexAtTime(time);
    if (d->orientation == Qt::Vertical)
        return TimelineTableModelBase::index(timelineIndex, 0);
    // else
    return TimelineTableModelBase::index(0, timelineIndex);
}

bool AnnotationMultiTierTableModel::splitAnnotations(const QModelIndex &index, RealTime splitMinimalAt)
{
    if (!index.isValid()) return false;
    if (d->orientation == Qt::Vertical)
        return splitAnnotations(index.row(), index.column(), splitMinimalAt);
    // else
    return splitAnnotations(index.column(), index.row(), splitMinimalAt);
}

bool AnnotationMultiTierTableModel::splitAnnotations(int timelineIndex, int dataIndex, RealTime splitMinimalAt)
{
    QString levelID;
    if (dataIndex < 3)
        levelID = d->tiernameMinimal;
    else if ((dataIndex - 3) >= 0 && (dataIndex - 3) < d->attributes.count())
        levelID = d->attributes.at(dataIndex - 3).first;

    if (timelineIndex < 0 || timelineIndex >= m_timeline.size()) return false;
    TimelineData td = m_timeline.at(timelineIndex);
    QString speakerID = td.speakerID;
    AnnotationTierGroup *spk_tiers = d->tiers.value(td.speakerID, 0);
    if (!spk_tiers) return false;
    IntervalTier *tier = spk_tiers->getIntervalTierByName(levelID);
    if (!tier) return false;

    if (levelID == d->tiernameMinimal) {
        if (splitMinimalAt == RealTime(0, 0)) {
            // no valid split point given
            splitMinimalAt = td.tCenter;
        }
        if ((splitMinimalAt <= td.tMin) || (splitMinimalAt >= (td.tCenter * 2.0 - td.tMin))) {
            // no valid split point given
            splitMinimalAt = td.tCenter;
        }
        // Create new interval and timeline record
        Interval *newInterval = tier->split(splitMinimalAt);
        if (!newInterval) return false;
        TimelineData tdNew;
        tdNew.speakerID = speakerID;
        tdNew.tMin = newInterval->tMin();
        tdNew.tCenter = newInterval->tCenter();
        tdNew.tMax = newInterval->tMax();
        tdNew.index = td.index + 1;
        // Refresh model
        if (d->orientation == Qt::Vertical)
            beginInsertRows(QModelIndex(), timelineIndex + 1, timelineIndex + 1);
        else
            beginInsertColumns(QModelIndex(), timelineIndex + 1, timelineIndex + 1);
        m_timeline[timelineIndex].tCenter = tier->interval(td.index)->tCenter();
        m_timeline[timelineIndex].tMax = tier->interval(td.index)->tMax();
        m_timeline.insert(timelineIndex + 1, tdNew);
        for (int iter = timelineIndex + 2; iter < m_timeline.count(); ++iter) {
            if (m_timeline.at(iter).speakerID != speakerID) continue;
            m_timeline[iter].index = m_timeline.at(iter).index + 1;
        }
        int refreshFrom = timelineIndex - 1; if (refreshFrom < 0) refreshFrom = 0;
        if (d->orientation == Qt::Vertical) {
            emit dataChanged(index(refreshFrom, 0), index(rowCount() - 1, columnCount() - 1));
            endInsertRows();
        } else {
            emit dataChanged(index(0, refreshFrom), index(rowCount() - 1, columnCount() - 1));
            endInsertColumns();
        }
    }
    else {
        IntervalTier *tier_min = spk_tiers->getIntervalTierByName(d->tiernameMinimal);
        if (!tier_min) return false;
        RealTime tSplit = tier_min->interval(td.index)->tMax();
        tier->split(tSplit);
        int refreshFrom = timelineIndex - 1; if (refreshFrom < 0) refreshFrom = 0;
        if (d->orientation == Qt::Vertical)
            emit dataChanged(index(refreshFrom, 0), index(timelineIndex + 1, columnCount() - 1));
        else
            emit dataChanged(index(0, refreshFrom), index(rowCount() - 1, timelineIndex + 1));
    }
    return true;
}

bool AnnotationMultiTierTableModel::mergeAnnotations(const QModelIndexList &indices)
{
    if (indices.isEmpty()) return false;
    int dataIndex;
    QList<int> timelineIndices;
    if (d->orientation == Qt::Vertical) {
        dataIndex = indices.first().column();
        foreach (QModelIndex index, indices)
            timelineIndices << index.row();
        return mergeAnnotations(dataIndex, timelineIndices);
    } else {
        dataIndex = indices.first().row();
        foreach (QModelIndex index, indices)
            timelineIndices << index.row();
        return mergeAnnotations(dataIndex, timelineIndices);
    }
    return false;
}


bool AnnotationMultiTierTableModel::mergeAnnotations(int dataIndex, const QList<int> &timelineIndices)
{
    QString levelID;
    if (dataIndex < 3)
        levelID = d->tiernameMinimal;
    else if ((dataIndex - 3) >= 0 && (dataIndex - 3) < d->attributes.count())
        levelID = d->attributes.at(dataIndex - 3).first;

    if (timelineIndices.isEmpty()) return false;
    if (timelineIndices.count() == 1) return false;
    // Check that rows are contiguous and increasing
    for (int i = 0; i < timelineIndices.count() - 1; ++i) {
        if (timelineIndices.at(i) > timelineIndices.at(i+1)) return false;
    }
    // Check that the intervals we are asked to merge all belong to the same speaker
    TimelineData tdFirst = m_timeline.at(timelineIndices.first());
    QString speakerID = tdFirst.speakerID;
    foreach (int timelineIndex, timelineIndices) {
        if (m_timeline.at(timelineIndex).speakerID != speakerID)
            return false; // cannot merge intervals from various speakers
    }
    TimelineData tdLast = m_timeline.at(timelineIndices.last());
    // Get the set of tiers for this speaker
    AnnotationTierGroup *spk_tiers = d->tiers.value(speakerID, 0);
    if (!spk_tiers) return false;
    // Get the interval tier on which we are merging
    IntervalTier *tier = spk_tiers->getIntervalTierByName(levelID);
    if (!tier) return false;

    if (levelID == d->tiernameMinimal) {
        if (d->orientation == Qt::Vertical)
            beginRemoveRows(QModelIndex(), timelineIndices.first() + 1, timelineIndices.last());
        else
            beginRemoveColumns(QModelIndex(), timelineIndices.first() + 1, timelineIndices.last());
        int indexFirst = tier->intervalIndexAtTime(tdFirst.tCenter);
        int indexLast = tier->intervalIndexAtTime(tdLast.tCenter);
        if (indexLast == -1) indexLast = tier->count() - 1;
        Interval *merged = tier->merge(indexFirst, indexLast, " ");
        if (!merged) {
            if (d->orientation == Qt::Vertical)
                endRemoveRows();
            else
                endRemoveColumns();
            return false;
        }
        int numberOfIntervalsMerged = indexLast - indexFirst;
        m_timeline[timelineIndices.first()].tCenter = merged->tCenter();
        for (int timelineIndex = timelineIndices.last(); timelineIndex >= timelineIndices.first() + 1; --timelineIndex) {
            // qDebug() << d->timeline.at(timelineIndices.first() + 1).speakerID << d->timeline.at(timelineIndices.first() + 1).intervalNo << d->timeline.at(timelineIndices.first() + 1).tMin.toDouble();
            m_timeline.removeAt(timelineIndices.first() + 1);
        }
        for (int timelineIndex = timelineIndices.first() + 1; timelineIndex < m_timeline.count(); ++ timelineIndex) {
            if (m_timeline.at(timelineIndex).speakerID != speakerID) continue;
            m_timeline[timelineIndex].index = m_timeline.at(timelineIndex).index - numberOfIntervalsMerged;
        }
        int refreshFrom = timelineIndices.first() - 1; if (refreshFrom < 0) refreshFrom = 0;
        if (d->orientation == Qt::Vertical) {
            emit dataChanged(index(refreshFrom, 0), index(rowCount() - 1, columnCount() - 1));
            endRemoveRows();
        } else {
            emit dataChanged(index(0, refreshFrom), index(rowCount() - 1, columnCount() - 1));
            endRemoveColumns();
        }
    }
    else {
        IntervalTier *tier_min = spk_tiers->getIntervalTierByName(d->tiernameMinimal);
        if (!tier_min) return false;
        RealTime tMinMerged = tier_min->interval(tdFirst.index)->tMin();
        RealTime tMaxMerged = tier_min->interval(tdLast.index)->tMax();
        int indexFirst = tier->intervalIndexAtTime(tdFirst.tCenter);
        int indexLast = tier->intervalIndexAtTime(tdLast.tCenter);
        if (tier->interval(indexFirst)->tMin() != tMinMerged)
            tier->split(tMinMerged);
        if (tier->interval(indexLast)->tMax() != tMaxMerged)
            tier->split(tMaxMerged);
        indexFirst = tier->intervalIndexAtTime(tdFirst.tCenter);
        indexLast = tier->intervalIndexAtTime(tdLast.tCenter);
        tier->merge(indexFirst, indexLast, " ");
        int refreshFrom = timelineIndices.first() - 1; if (refreshFrom < 0) refreshFrom = 0;
        if (d->orientation == Qt::Vertical)
            emit dataChanged(index(refreshFrom, 0), index(timelineIndices.last() + 1, columnCount() - 1));
        else
            emit dataChanged(index(0, refreshFrom), index(rowCount() - 1, timelineIndices.last() + 1));
    }
    return true;
}

