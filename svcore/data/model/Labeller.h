/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _LABELLER_H_
#define _LABELLER_H_

#include "SparseModel.h"
#include "SparseValueModel.h"

#include "base/Selection.h"

#include <QObject>

#include <map>
#include <iostream>

class Labeller : public QObject
{
    Q_OBJECT

public:
    enum ValueType {
        ValueNone,
        ValueFromSimpleCounter,
        ValueFromCyclicalCounter,
        ValueFromTwoLevelCounter,
        ValueFromFrameNumber,
        ValueFromRealTime,
        ValueFromDurationFromPrevious,
        ValueFromDurationToNext,
        ValueFromTempoFromPrevious,
        ValueFromTempoToNext,
        ValueFromExistingNeighbour,
        ValueFromLabel
    };

    // uses:
    //
    // 1. when adding points to a time-value model, generate values
    // for those points based on their times or labels or a counter
    //
    // 2. when adding a single point to a time-instant model, generate
    // a label for it based on its time and that of the previous point
    // or a counter
    //
    // 3. when adding a single point to a time-instant model, generate
    // a label for the previous point based on its time and that of
    // the point just added (as tempo is based on time to the next
    // point, not the previous one)
    //
    // 4. re-label a set of points that have already been added to a
    // model

    Labeller(ValueType type = ValueNone) :
        m_type(type),
        m_counter(1),
        m_counter2(1),
        m_cycle(4),
        m_dp(10),
        m_rate(0) { }

    Labeller(const Labeller &l) :
        QObject(),
        m_type(l.m_type),
        m_counter(l.m_counter),
        m_counter2(l.m_counter2),
        m_cycle(l.m_cycle),
        m_dp(l.m_dp),
        m_rate(l.m_rate) { }

    virtual ~Labeller() { }

    typedef std::map<ValueType, QString> TypeNameMap;
    TypeNameMap getTypeNames() const {
        TypeNameMap m;
        m[ValueNone]
            = tr("No numbering");
        m[ValueFromSimpleCounter]
            = tr("Simple counter");
        m[ValueFromCyclicalCounter]
            = tr("Cyclical counter");
        m[ValueFromTwoLevelCounter]
            = tr("Cyclical two-level counter (bar/beat)");
        m[ValueFromFrameNumber]
            = tr("Audio sample frame number");
        m[ValueFromRealTime]
            = tr("Time in seconds");
        m[ValueFromDurationToNext]
            = tr("Duration to the following item");
        m[ValueFromTempoToNext]
            = tr("Tempo (bpm) based on duration to following item");
        m[ValueFromDurationFromPrevious]
            = tr("Duration since the previous item");
        m[ValueFromTempoFromPrevious]
            = tr("Tempo (bpm) based on duration since previous item");
        m[ValueFromExistingNeighbour]
            = tr("Same as the nearest previous item");
        m[ValueFromLabel]
            = tr("Value extracted from the item's label (where possible)");
        return m;
    }

    ValueType getType() const { return m_type; }
    void setType(ValueType type) { m_type = type; }

    int getCounterValue() const { return m_counter; }
    void setCounterValue(int v) { m_counter = v; }

    int getSecondLevelCounterValue() const { return m_counter2; }
    void setSecondLevelCounterValue(int v) { m_counter2 = v; }

    int getCounterCycleSize() const { return m_cycle; }
    void setCounterCycleSize(int s) {
        m_cycle = s;
        m_dp = 1;
        while (s > 0) {
            s /= 10;
            m_dp *= 10;
        }
        if (m_counter > m_cycle) m_counter = 1;
    }

    void setSampleRate(sv_samplerate_t rate) { m_rate = rate; }

    void resetCounters() {
        m_counter = 1;
        m_counter2 = 1;
        m_cycle = 4;
    }

    void incrementCounter() {
        m_counter++;
        if (m_type == ValueFromCyclicalCounter ||
            m_type == ValueFromTwoLevelCounter) {
            if (m_counter > m_cycle) {
                m_counter = 1;
                m_counter2++;
            }
        }
    }

    template <typename PointType>
    void label(PointType &newPoint, PointType *prevPoint = 0) {
        if (m_type == ValueNone) {
            newPoint.label = "";
        } else if (m_type == ValueFromTwoLevelCounter) {
            newPoint.label = tr("%1.%2").arg(m_counter2).arg(m_counter);
            incrementCounter();
        } else if (m_type == ValueFromFrameNumber) {
            // avoid going through floating-point value
            newPoint.label = tr("%1").arg(newPoint.frame);
        } else {
            float value = getValueFor<PointType>(newPoint, prevPoint);
            if (actingOnPrevPoint() && prevPoint) {
                prevPoint->label = QString("%1").arg(value);
            } else {
                newPoint.label = QString("%1").arg(value);
            }
        }
    }
        
    template <typename PointType>
    void labelAll(SparseModel<PointType> &model, MultiSelection *ms) {

        typename SparseModel<PointType>::PointList::iterator i;
        typename SparseModel<PointType>::PointList pl(model.getPoints());

        typename SparseModel<PointType>::EditCommand *command =
            new typename SparseModel<PointType>::EditCommand
            (&model, tr("Label Points"));

        PointType prevPoint(0);

        for (i = pl.begin(); i != pl.end(); ++i) {

            bool inRange = true;
            if (ms) {
                Selection s(ms->getContainingSelection(i->frame, false));
                if (s.isEmpty() || !s.contains(i->frame)) {
                    inRange = false;
                }
            }

            PointType p(*i);

            if (!inRange) {
                prevPoint = p;
                continue;
            }

            if (actingOnPrevPoint()) {
                if (i != pl.begin()) {
                    command->deletePoint(prevPoint);
                    label<PointType>(p, &prevPoint);
                    command->addPoint(prevPoint);
                }
            } else {
                command->deletePoint(p);
                label<PointType>(p, &prevPoint);
                command->addPoint(p);
            }

            prevPoint = p;
        }

        command->finish();
    }

    template <typename PointType>
    void setValue(PointType &newPoint, PointType *prevPoint = 0) {
        if (m_type == ValueFromExistingNeighbour) {
            if (!prevPoint) {
                std::cerr << "ERROR: Labeller::setValue: Previous point required but not provided" << std::endl;
            } else {
                newPoint.value = prevPoint->value;
            }
        } else {
            float value = getValueFor<PointType>(newPoint, prevPoint);
            if (actingOnPrevPoint() && prevPoint) {
                prevPoint->value = value;
            } else {
                newPoint.value = value;
            }
        }
    }

    bool requiresPrevPoint() const {
        return (m_type == ValueFromDurationFromPrevious ||
                m_type == ValueFromDurationToNext ||
                m_type == ValueFromTempoFromPrevious ||
                m_type == ValueFromDurationToNext);
    }

    bool actingOnPrevPoint() const {
        return (m_type == ValueFromDurationToNext ||
                m_type == ValueFromTempoToNext);
    }

protected:
    template <typename PointType>
    float getValueFor(PointType &newPoint, PointType *prevPoint)
    {
        float value = 0.f;

        switch (m_type) {

        case ValueNone:
            value = 0;
            break;

        case ValueFromSimpleCounter:
        case ValueFromCyclicalCounter:
            value = float(m_counter);
            incrementCounter();
            break;

        case ValueFromTwoLevelCounter:
            value = float(m_counter2 + double(m_counter) / double(m_dp));
            incrementCounter();
            break;

        case ValueFromFrameNumber:
            value = float(newPoint.frame);
            break;
            
        case ValueFromRealTime: 
            if (m_rate == 0.0) {
                std::cerr << "ERROR: Labeller::getValueFor: Real-time conversion required, but no sample rate set" << std::endl;
            } else {
                value = float(double(newPoint.frame) / m_rate);
            }
            break;

        case ValueFromDurationToNext:
        case ValueFromTempoToNext:
        case ValueFromDurationFromPrevious:
        case ValueFromTempoFromPrevious:
            if (m_rate == 0.0) {
                std::cerr << "ERROR: Labeller::getValueFor: Real-time conversion required, but no sample rate set" << std::endl;
            } else if (!prevPoint) {
                std::cerr << "ERROR: Labeller::getValueFor: Time difference required, but only one point provided" << std::endl;
            } else {
                sv_frame_t f0 = prevPoint->frame, f1 = newPoint.frame;
                if (m_type == ValueFromDurationToNext ||
                    m_type == ValueFromDurationFromPrevious) {
                    value = float(double(f1 - f0) / m_rate);
                } else {
                    if (f1 > f0) {
                        value = float((60.0 * m_rate) / double(f1 - f0));
                    }
                }
            }
            break;

        case ValueFromExistingNeighbour:
            // need to deal with this in the calling function, as this
            // function must handle points that don't have values to
            // read from
            break;

        case ValueFromLabel:
            if (newPoint.label != "") {
                // more forgiving than QString::toFloat()
                value = float(atof(newPoint.label.toLocal8Bit()));
            } else {
                value = 0.f;
            }
            break;
        }

        return value;
    }

    ValueType m_type;
    int m_counter;
    int m_counter2;
    int m_cycle;
    int m_dp;
    sv_samplerate_t m_rate;
};

#endif
