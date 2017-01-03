#include <QDebug>
#include <QPair>
#include <QMap>
#include <QMultiMap>
#include <QString>
#include <QList>

#include "pncore/annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

#include "AnnotationTierModel.h"

struct TimelineData {
    QString speakerID;
    int intervalNo;
    RealTime tMin;
    RealTime tCenter;
};

struct AnnotationTierModelData {
    AnnotationTierModelData() : orientation(Qt::Vertical) {}

    Qt::Orientation orientation;
    QMap<QString, QPointer<AnnotationTierGroup> > tiers;  // Speaker ID, corresponding tiers
    QString tiernameMinimal;
    QList<QPair<QString, QString> > attributes;           // Level ID, Attribute ID
    QList<TimelineData> timeline;
    QHash<QString, QColor> speakerBackgroundColors;
};

AnnotationTierModel::AnnotationTierModel(QMap<QString, QPointer<AnnotationTierGroup> > &tiers,
                                         const QString &tiernameMinimal, const QList<QPair<QString, QString> > &attributes,
                                         Qt::Orientation orientation, QObject *parent) :
    QAbstractTableModel(parent), d(new AnnotationTierModelData)
{
    d->orientation = orientation;
    d->tiers = tiers;
    d->tiernameMinimal = tiernameMinimal;
    d->attributes = attributes;

    QMultiMap<RealTime, TimelineData> timeline;
    // Calculate timeline for given tiers
    int i = 0;
    foreach (QString speakerID, d->tiers.keys()) {
        IntervalTier *tier = d->tiers.value(speakerID)->getIntervalTierByName(d->tiernameMinimal);
        if (!tier) continue;
        for (int intervalNo = 0; intervalNo < tier->count(); ++intervalNo) {
            TimelineData td;
            td.speakerID = speakerID;
            td.intervalNo = intervalNo;
            td.tMin = tier->interval(intervalNo)->tMin();
            td.tCenter = tier->interval(intervalNo)->tCenter();
            timeline.insert(td.tMin, td);
        }
        d->speakerBackgroundColors.insert(speakerID, QColor(QColor::colorNames().at(i)));
        ++i;
    }
    d->timeline = timeline.values();
}

AnnotationTierModel::~AnnotationTierModel()
{
}

QModelIndex AnnotationTierModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

int AnnotationTierModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (d->orientation == Qt::Vertical)
        return d->timeline.count();
    // else
    return d->attributes.count() + 3;
}

int AnnotationTierModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (d->orientation == Qt::Vertical)
        return d->attributes.count() + 3;
    // else
    return d->timeline.count();
}

RealTime AnnotationTierModel::getStartTime() const
{
    if (d->timeline.count() == 0) return RealTime();
    return d->timeline.first().tMin;
}

RealTime AnnotationTierModel::getEndTime() const
{
    if (d->timeline.count() == 0) return RealTime();
    QPointer<AnnotationTierGroup> spk_tiers = d->tiers.value(d->timeline.last().speakerID, 0);
    if (!spk_tiers) return RealTime();
    IntervalTier *tier_min = spk_tiers->getIntervalTierByName(d->tiernameMinimal);
    if (!tier_min) return RealTime();
    int i = d->timeline.last().intervalNo;
    if (i >= tier_min->count()) return RealTime();
    return tier_min->interval(i)->tMax();
}

QList<int> AnnotationTierModel::attributeIndicesExceptContext() const
{
    QList<int> ret;
    for (int attributeIndex = 0; attributeIndex < d->attributes.count(); ++attributeIndex) {
        QString attributeID = d->attributes.at(attributeIndex).second;
        if (attributeID != "_context") ret << attributeIndex;
    }
    return ret;
}

QVariant AnnotationTierModel::headerData(int section, Qt::Orientation orientation, int role) const
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

// This is a file-scoped helper function
QVariant dataCell(QPointer<AnnotationTierGroup> spk_tiers, TimelineData &td, QString levelID, QString attributeID)
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
    else if (attributeID.startsWith("_group:")) {
        QString levelIDgrouped = attributeID.section(":", 1, 1);
        IntervalTier *tier_groupped = spk_tiers->getIntervalTierByName(levelIDgrouped);
        if (!tier_groupped) return QVariant();
        QList<Interval *> intervals = tier_groupped->getIntervalsContainedIn(tier->interval(intvID));
        QString s;
        foreach (Interval *intv, intervals) s.append(intv->text()).append(" ");
        return QString("(%1)%2").arg(s.trimmed()).arg(tier->interval(intvID)->text());
    }
    else {
        return tier->interval(intvID)->attribute(attributeID);
    }
    return QVariant();
}

QVariant AnnotationTierModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if (index.row() < 0 || index.row() >= rowCount()) return QVariant();
    if (index.column() < 0 || index.column() >= columnCount()) return QVariant();

    // Translate row index to timeline position
    TimelineData td;
    int dataCellIndex;
    if (d->orientation == Qt::Vertical) {
        td = d->timeline.at(index.row());
        dataCellIndex = index.column();
    }
    else {
        td = d->timeline.at(index.column());
        dataCellIndex = index.row();
    }
    int attributeIndex = dataCellIndex - 3;

    QPointer<AnnotationTierGroup> spk_tiers = d->tiers.value(td.speakerID, 0);
    if (!spk_tiers) return QVariant();
    IntervalTier *tier_min = spk_tiers->getIntervalTierByName(d->tiernameMinimal);
    if (!tier_min) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (dataCellIndex == 0) {
            return td.speakerID;
        }
        if (dataCellIndex == 1) {
            Interval *intv_min = tier_min->interval(td.intervalNo);
            if (intv_min) return intv_min->tMin().toDouble(); // QString::number(intv_min->tMin().toDouble(), 'f', 6);
            else return QVariant();
        }
        else if (dataCellIndex == 2) {
            Interval *intv_min = tier_min->interval(td.intervalNo);
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
        return d->speakerBackgroundColors.value(td.speakerID, QColor("White"));
    }
    return QVariant();
}

QList<AnnotationTierModel::AnnotationTierCell>
AnnotationTierModel::dataBlock(const RealTime &from, const RealTime &to, const QList<int> &attributeIndices)
{
    QList<AnnotationTierModel::AnnotationTierCell> ret;
    int timelineIndexFrom = timelineIndexAtTime(from);
    int timelineIndexTo = timelineIndexAtTime(to);
    for (int timelineIndex = timelineIndexFrom; timelineIndex <= timelineIndexTo; ++timelineIndex) {
        TimelineData td = d->timeline.at(timelineIndex);
        QPointer<AnnotationTierGroup> spk_tiers = d->tiers.value(td.speakerID, 0);
        if (!spk_tiers) continue;
        foreach (int attributeIndex, attributeIndices) {
            QString levelID = d->attributes.at(attributeIndex).first;
            QString attributeID = d->attributes.at(attributeIndex).second;
            QVariant value = dataCell(spk_tiers, td, levelID, attributeID);
            AnnotationTierModel::AnnotationTierCell cell;
            cell.tMin = td.tMin;
            cell.attributeIndex = attributeIndex;
            cell.speakerID = td.speakerID;
            cell.value = value;
            ret << cell;
        }
    }
    return ret;
}

Qt::ItemFlags AnnotationTierModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    if ( ((d->orientation == Qt::Vertical) && (index.column() >= 1)) ||
         ((d->orientation == Qt::Horizontal) && (index.row() >= 1)) )
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractTableModel::flags(index);
}

bool AnnotationTierModel::setData(const QModelIndex &index, const QVariant &value, int role)
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
        td = d->timeline.at(timelineIndex);
    }
    else {
        timelineIndex = index.column();
        dataCellIndex = index.row();
        td = d->timeline.at(timelineIndex);
    }
    int attributeIndex = dataCellIndex - 3;

    QPointer<AnnotationTierGroup> spk_tiers = d->tiers.value(td.speakerID, 0);
    if (!spk_tiers) return false;

    if (dataCellIndex == 1) {
        IntervalTier *tier_minimal = spk_tiers->getIntervalTierByName(d->tiernameMinimal);
        if (!tier_minimal) return false;
        int intervalIndex = tier_minimal->intervalIndexAtTime(td.tCenter);
        bool result = tier_minimal->moveBoundary(intervalIndex, RealTime::fromSeconds(value.toDouble()));
        if (!result) return false;
        // update timeline (previous interval's center, our min and center)
        if ((timelineIndex > 0) && (intervalIndex > 0))
            d->timeline[timelineIndex - 1].tCenter = tier_minimal->interval(intervalIndex - 1)->tCenter();
        d->timeline[timelineIndex].tMin = tier_minimal->interval(intervalIndex)->tMin();
        d->timeline[timelineIndex].tCenter = tier_minimal->interval(intervalIndex)->tCenter();
        // refresh grid
        if (d->orientation == Qt::Vertical) {
            int refreshFrom = index.row() - 1; if (refreshFrom < 0) refreshFrom = 0;
            emit dataChanged(AnnotationTierModel::index(refreshFrom, 0), AnnotationTierModel::index(index.row(), columnCount() - 1));
        } else {
            int refreshFrom = index.column() - 1; if (refreshFrom < 0) refreshFrom = 0;
            emit dataChanged(AnnotationTierModel::index(0, refreshFrom), AnnotationTierModel::index(rowCount() - 1, index.column()));
        }
        return true;
    }
    else if (dataCellIndex == 2) {
        IntervalTier *tier_minimal = spk_tiers->getIntervalTierByName(d->tiernameMinimal);
        if (!tier_minimal) return false;
        int intervalIndex = tier_minimal->intervalIndexAtTime(td.tCenter);
        bool result = tier_minimal->moveBoundary(intervalIndex + 1, RealTime::fromSeconds(value.toDouble()));
        if (!result) return false;
        // update timeline (our center, next interval's min and center)
        d->timeline[timelineIndex].tCenter = tier_minimal->interval(intervalIndex)->tCenter();
        if ((timelineIndex < d->timeline.count() - 1) && (intervalIndex < tier_minimal->count() -1)) {
            d->timeline[timelineIndex + 1].tMin = tier_minimal->interval(intervalIndex + 1)->tMin();
            d->timeline[timelineIndex + 1].tCenter = tier_minimal->interval(intervalIndex + 1)->tCenter();
        }
        // refresh grid
        if (d->orientation == Qt::Vertical) {
            int refreshTo = index.row() + 1; if (refreshTo == rowCount()) refreshTo = rowCount() - 1;
            emit dataChanged(AnnotationTierModel::index(index.row(), 0), AnnotationTierModel::index(refreshTo, columnCount() - 1));
        } else {
            int refreshTo = index.column() + 1; if (refreshTo == columnCount()) refreshTo = columnCount() - 1;
            emit dataChanged(AnnotationTierModel::index(0, index.column()), AnnotationTierModel::index(rowCount() - 1, refreshTo));
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

Qt::Orientation AnnotationTierModel::orientation() const
{
    return d->orientation;
}

void AnnotationTierModel::setOrientation(Qt::Orientation orientation)
{
    if (d->orientation == orientation) return;
    beginResetModel();
    d->orientation = orientation;
    endResetModel();
}

// A timeline index corresponds to a row in a vertically oriented model
// and to a column in a horizontally oriented model.
int AnnotationTierModel::timelineIndexAtTime(const RealTime &time) const
{
    int timelineIndex = 0;
    for (timelineIndex = 0; timelineIndex < d->timeline.count(); ++timelineIndex) {
        if (d->timeline.at(timelineIndex).tMin >= time) break;
    }
    return timelineIndex;
}

int AnnotationTierModel::timelineIndexAtTime(const RealTime &time, double &tMin_msec, double &tMax_msec) const
{
    int timelineIndex = 0;
    for (timelineIndex = 0; timelineIndex < d->timeline.count(); ++timelineIndex) {
        if (d->timeline.at(timelineIndex).tMin >= time) break;
    }
    tMin_msec = d->timeline.at(timelineIndex).tMin.toDouble() * 1000.0;
    double tCenter_msec = d->timeline.at(timelineIndex).tCenter.toDouble() * 1000.0;
    tMax_msec = 2.0 * tCenter_msec - tMin_msec;
    return timelineIndex;
}

RealTime AnnotationTierModel::timeAtTimelineIndex(int timelineIndex) const
{
    if ((timelineIndex < 0) || (timelineIndex > d->timeline.count() - 1)) return RealTime(-1, 0);
    return d->timeline.at(timelineIndex).tMin;
}

QModelIndex AnnotationTierModel::modelIndexAtTime(const RealTime &time) const
{
    int timelineIndex = timelineIndexAtTime(time);
    if (d->orientation == Qt::Vertical)
        return AnnotationTierModel::index(timelineIndex, 0);
    // else
    return AnnotationTierModel::index(0, timelineIndex);
}


bool AnnotationTierModel::splitAnnotations(const QModelIndex &index, RealTime splitMinimalAt)
{
    if (!index.isValid()) return false;
    if (d->orientation == Qt::Vertical)
        return splitAnnotations(index.row(), index.column(), splitMinimalAt);
    // else
    return splitAnnotations(index.column(), index.row(), splitMinimalAt);
}

bool AnnotationTierModel::splitAnnotations(int timelineIndex, int dataIndex, RealTime splitMinimalAt)
{
    QString levelID;
    if (dataIndex < 3)
        levelID = d->tiernameMinimal;
    else if ((dataIndex - 3) >= 0 && (dataIndex - 3) < d->attributes.count())
        levelID = d->attributes.at(dataIndex - 3).first;

    if (timelineIndex < 0 || timelineIndex >= d->timeline.size()) return false;
    TimelineData td = d->timeline.at(timelineIndex);
    QString speakerID = td.speakerID;
    QPointer<AnnotationTierGroup> spk_tiers = d->tiers.value(td.speakerID, 0);
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
        tdNew.intervalNo = td.intervalNo + 1;
        // Refresh model
        if (d->orientation == Qt::Vertical)
            beginInsertRows(QModelIndex(), timelineIndex + 1, timelineIndex + 1);
        else
            beginInsertColumns(QModelIndex(), timelineIndex + 1, timelineIndex + 1);
        d->timeline[timelineIndex].tCenter = tier->interval(td.intervalNo)->tCenter();
        d->timeline.insert(timelineIndex + 1, tdNew);
        for (int iter = timelineIndex + 2; iter < d->timeline.count(); ++iter) {
            if (d->timeline.at(iter).speakerID != speakerID) continue;
            d->timeline[iter].intervalNo = d->timeline.at(iter).intervalNo + 1;
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
        RealTime tSplit = tier_min->interval(td.intervalNo)->tMax();
        tier->split(tSplit);
        int refreshFrom = timelineIndex - 1; if (refreshFrom < 0) refreshFrom = 0;
        if (d->orientation == Qt::Vertical)
            emit dataChanged(index(refreshFrom, 0), index(timelineIndex + 1, columnCount() - 1));
        else
            emit dataChanged(index(0, refreshFrom), index(rowCount() - 1, timelineIndex + 1));
    }
    return true;
}

bool AnnotationTierModel::mergeAnnotations(const QModelIndexList &indices)
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


bool AnnotationTierModel::mergeAnnotations(int dataIndex, const QList<int> &timelineIndices)
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
    TimelineData tdFirst = d->timeline.at(timelineIndices.first());
    QString speakerID = tdFirst.speakerID;
    foreach (int timelineIndex, timelineIndices) {
        if (d->timeline.at(timelineIndex).speakerID != speakerID)
            return false; // cannot merge intervals from various speakers
    }
    TimelineData tdLast = d->timeline.at(timelineIndices.last());
    // Get the set of tiers for this speaker
    QPointer<AnnotationTierGroup> spk_tiers = d->tiers.value(speakerID, 0);
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
        d->timeline[timelineIndices.first()].tCenter = merged->tCenter();
        for (int timelineIndex = timelineIndices.last(); timelineIndex >= timelineIndices.first() + 1; --timelineIndex) {
            // qDebug() << d->timeline.at(timelineIndices.first() + 1).speakerID << d->timeline.at(timelineIndices.first() + 1).intervalNo << d->timeline.at(timelineIndices.first() + 1).tMin.toDouble();
            d->timeline.removeAt(timelineIndices.first() + 1);
        }
        for (int timelineIndex = timelineIndices.first() + 1; timelineIndex < d->timeline.count(); ++ timelineIndex) {
            if (d->timeline.at(timelineIndex).speakerID != speakerID) continue;
            d->timeline[timelineIndex].intervalNo = d->timeline.at(timelineIndex).intervalNo - numberOfIntervalsMerged;
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
        RealTime tMinMerged = tier_min->interval(tdFirst.intervalNo)->tMin();
        RealTime tMaxMerged = tier_min->interval(tdLast.intervalNo)->tMax();
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




