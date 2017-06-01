/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2008 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _INTERVAL_MODEL_H_
#define _INTERVAL_MODEL_H_

#include "SparseValueModel.h"
#include "base/RealTime.h"

/**
 * Model containing sparse data (points with some properties) of which
 * the properties include a duration and an arbitrary float value.
 * The other properties depend on the point type.
 */

template <typename PointType>
class IntervalModel : public SparseValueModel<PointType>
{
public:
    IntervalModel(sv_samplerate_t sampleRate, int resolution,
                  bool notifyOnAdd = true) :
	SparseValueModel<PointType>(sampleRate, resolution, notifyOnAdd)
    { }

    IntervalModel(sv_samplerate_t sampleRate, int resolution,
                  float valueMinimum, float valueMaximum,
                  bool notifyOnAdd = true) :
	SparseValueModel<PointType>(sampleRate, resolution,
                                    valueMinimum, valueMaximum,
                                    notifyOnAdd)
    { }

    /**
     * PointTypes have a duration, so this returns all points that span any
     * of the given range (as well as the usual additional few before
     * and after).  Consequently this can be very slow (optimised data
     * structures still to be done!).
     */
    virtual typename SparseValueModel<PointType>::PointList getPoints(sv_frame_t start, sv_frame_t end) const;

    /**
     * PointTypes have a duration, so this returns all points that span the
     * given frame.  Consequently this can be very slow (optimised
     * data structures still to be done!).
     */
    virtual typename SparseValueModel<PointType>::PointList getPoints(sv_frame_t frame) const;

    virtual const typename SparseModel<PointType>::PointList &getPoints() const {
        return SparseModel<PointType>::getPoints(); 
    }

    /**
     * TabularModel methods.  
     */

    virtual QVariant getData(int row, int column, int role) const
    {
        if (column < 2) {
            return SparseValueModel<PointType>::getData
                (row, column, role);
        }

        typename SparseModel<PointType>::PointList::const_iterator i
            = SparseModel<PointType>::getPointListIteratorForRow(row);
        if (i == SparseModel<PointType>::m_points.end()) return QVariant();

        switch (column) {
        case 2:
            if (role == Qt::EditRole || role == TabularModel::SortRole) return i->value;
            else return QString("%1 %2").arg(i->value).arg
                     (IntervalModel<PointType>::getScaleUnits());
        case 3: return int(i->duration); //!!! could be better presented
        default: return QVariant();
        }
    }

    virtual UndoableCommand *getSetDataCommand(int row, int column, const QVariant &value, int role)
    {
        typedef IntervalModel<PointType> I;

        if (column < 2) {
            return SparseValueModel<PointType>::getSetDataCommand
                (row, column, value, role);
        }

        if (role != Qt::EditRole) return 0;
        typename I::PointList::const_iterator i
            = I::getPointListIteratorForRow(row);
        if (i == I::m_points.end()) return 0;
        typename I::EditCommand *command = new typename I::EditCommand
            (this, I::tr("Edit Data"));

        PointType point(*i);
        command->deletePoint(point);

        switch (column) {
        // column cannot be 0 or 1, those cases were handled above
        case 2: point.value = float(value.toDouble()); break;
        case 3: point.duration = value.toInt(); break;
        }

        command->addPoint(point);
        return command->finish();
    }

    virtual bool isColumnTimeValue(int column) const
    {
        // NB duration is not a "time value" -- that's for columns
        // whose sort ordering is exactly that of the frame time
        return (column < 2);
    }
};

template <typename PointType>
typename SparseValueModel<PointType>::PointList
IntervalModel<PointType>::getPoints(sv_frame_t start, sv_frame_t end) const
{
    typedef IntervalModel<PointType> I;

    if (start > end) return typename I::PointList();

    QMutex &mutex(I::m_mutex);
    QMutexLocker locker(&mutex);

    PointType endPoint(end);
    
    typename I::PointListConstIterator endItr = I::m_points.upper_bound(endPoint);

    if (endItr != I::m_points.end()) ++endItr;
    if (endItr != I::m_points.end()) ++endItr;

    typename I::PointList rv;

    for (typename I::PointListConstIterator i = endItr; i != I::m_points.begin(); ) {
        --i;
        if (i->frame < start) {
            if (i->frame + i->duration >= start) {
                rv.insert(*i);
            }
        } else if (i->frame <= end) {
            rv.insert(*i);
        }
    }

    return rv;
}

template <typename PointType>
typename SparseValueModel<PointType>::PointList
IntervalModel<PointType>::getPoints(sv_frame_t frame) const
{
    typedef IntervalModel<PointType> I;

    QMutex &mutex(I::m_mutex);
    QMutexLocker locker(&mutex);

    if (I::m_resolution == 0) return typename I::PointList();

    sv_frame_t start = (frame / I::m_resolution) * I::m_resolution;
    sv_frame_t end = start + I::m_resolution;

    PointType endPoint(end);
    
    typename I::PointListConstIterator endItr = I::m_points.upper_bound(endPoint);

    typename I::PointList rv;

    for (typename I::PointListConstIterator i = endItr; i != I::m_points.begin(); ) {
        --i;
        if (i->frame < start) {
            if (i->frame + i->duration >= start) {
                rv.insert(*i);
            }
        } else if (i->frame <= end) {
            rv.insert(*i);
        }
    }

    return rv;
}

#endif
