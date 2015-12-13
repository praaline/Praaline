/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RANGE_MAPPER_H_
#define _RANGE_MAPPER_H_

#include <QString>

#include "Debug.h"
#include <map>

class RangeMapper 
{
public:
    virtual ~RangeMapper() { }

    /**
     * Return the position that maps to the given value, rounding to
     * the nearest position and clamping to the minimum and maximum
     * extents of the mapper's positional range.
     */
    virtual int getPositionForValue(double value) const = 0;

    /**
     * Return the position that maps to the given value, rounding to
     * the nearest position, without clamping. That is, whatever
     * mapping function is in use will be projected even outside the
     * minimum and maximum extents of the mapper's positional
     * range. (The mapping outside that range is not guaranteed to be
     * exact, except if the mapper is a linear one.)
     */
    virtual int getPositionForValueUnclamped(double value) const = 0;

    /**
     * Return the value mapped from the given position, clamping to
     * the minimum and maximum extents of the mapper's value range.
     */
    virtual double getValueForPosition(int position) const = 0;

    /**
     * Return the value mapped from the given positionq, without
     * clamping. That is, whatever mapping function is in use will be
     * projected even outside the minimum and maximum extents of the
     * mapper's value range. (The mapping outside that range is not
     * guaranteed to be exact, except if the mapper is a linear one.)
     */
    virtual double getValueForPositionUnclamped(int position) const = 0;

    /**
     * Get the unit of the mapper's value range.
     */
    virtual QString getUnit() const { return ""; }
};


class LinearRangeMapper : public RangeMapper
{
public:
    /**
     * Map values in range minval->maxval linearly into integer range
     * minpos->maxpos. minval and minpos must be less than maxval and
     * maxpos respectively. If inverted is true, the range will be
     * mapped "backwards" (minval to maxpos and maxval to minpos).
     */
    LinearRangeMapper(int minpos, int maxpos,
                      double minval, double maxval,
                      QString unit = "", bool inverted = false);
    
    virtual int getPositionForValue(double value) const;
    virtual int getPositionForValueUnclamped(double value) const;

    virtual double getValueForPosition(int position) const;
    virtual double getValueForPositionUnclamped(int position) const;

    virtual QString getUnit() const { return m_unit; }

protected:
    int m_minpos;
    int m_maxpos;
    double m_minval;
    double m_maxval;
    QString m_unit;
    bool m_inverted;
};

class LogRangeMapper : public RangeMapper
{
public:
    /**
     * Map values in range minval->maxval into integer range
     * minpos->maxpos such that logs of the values are mapped
     * linearly. minval must be greater than zero, and minval and
     * minpos must be less than maxval and maxpos respectively. If
     * inverted is true, the range will be mapped "backwards" (minval
     * to maxpos and maxval to minpos).
     */
    LogRangeMapper(int minpos, int maxpos,
                   double minval, double maxval,
                   QString m_unit = "", bool inverted = false);

    static void convertRatioMinLog(double ratio, double minlog,
                                   int minpos, int maxpos,
                                   double &minval, double &maxval);

    static void convertMinMax(int minpos, int maxpos,
                              double minval, double maxval,
                              double &ratio, double &minlog);

    virtual int getPositionForValue(double value) const;
    virtual int getPositionForValueUnclamped(double value) const;

    virtual double getValueForPosition(int position) const;
    virtual double getValueForPositionUnclamped(int position) const;

    virtual QString getUnit() const { return m_unit; }

protected:
    int m_minpos;
    int m_maxpos;
    double m_ratio;
    double m_minlog;
    double m_maxlog;
    QString m_unit;
    bool m_inverted;
};

class InterpolatingRangeMapper : public RangeMapper
{
public:
    typedef std::map<double, int> CoordMap;

    /**
     * Given a series of (value, position) coordinate mappings,
     * construct a range mapper that maps arbitrary values, in the
     * range between minimum and maximum of the provided values, onto
     * coordinates using linear interpolation between the supplied
     * points.
     *
     *!!! todo: Cubic -- more generally useful than linear interpolation
     *!!! todo: inverted flag
     *
     * The set of provided mappings must contain at least two
     * coordinates.
     *
     * It is expected that the values and positions in the coordinate
     * mappings will both be monotonically increasing (i.e. no
     * inflections in the mapping curve). Behaviour is undefined if
     * this is not the case.
     */
    InterpolatingRangeMapper(CoordMap pointMappings,
                             QString unit);

    virtual int getPositionForValue(double value) const;
    virtual int getPositionForValueUnclamped(double value) const;

    virtual double getValueForPosition(int position) const;
    virtual double getValueForPositionUnclamped(int position) const;

    virtual QString getUnit() const { return m_unit; }

protected:
    CoordMap m_mappings;
    std::map<int, double> m_reverse;
    QString m_unit;

    template <typename T>
    double interpolate(T *mapping, double v) const;
};

class AutoRangeMapper : public RangeMapper
{
public:
    enum MappingType {
        Interpolating,
        StraightLine,
        Logarithmic,
    };

    typedef std::map<double, int> CoordMap;

    /**
     * Given a series of (value, position) coordinate mappings,
     * construct a range mapper that maps arbitrary values, in the
     * range between minimum and maximum of the provided values, onto
     * coordinates. 
     *
     * The mapping used may be
     * 
     *    Interpolating -- an InterpolatingRangeMapper will be used
     * 
     *    StraightLine -- a LinearRangeMapper from the minimum to
     *    maximum value coordinates will be used, ignoring all other
     *    supplied coordinate mappings
     * 
     *    Logarithmic -- a LogRangeMapper from the minimum to
     *    maximum value coordinates will be used, ignoring all other
     *    supplied coordinate mappings
     *
     * The mapping will be chosen automatically by looking at the
     * supplied coordinates. If the supplied coordinates fall on a
     * straight line, a StraightLine mapping will be used; if they
     * fall on a log curve, a Logarithmic mapping will be used;
     * otherwise an Interpolating mapping will be used.
     *
     *!!! todo: inverted flag
     *
     * The set of provided mappings must contain at least two
     * coordinates, or at least three if the points are not supposed
     * to be in a straight line.
     *
     * It is expected that the values and positions in the coordinate
     * mappings will both be monotonically increasing (i.e. no
     * inflections in the mapping curve). Behaviour is undefined if
     * this is not the case.
     */
    AutoRangeMapper(CoordMap pointMappings,
                    QString unit);

    ~AutoRangeMapper();

    /**
     * Return the mapping type in use.
     */
    MappingType getType() const { return m_type; }

    virtual int getPositionForValue(double value) const;
    virtual int getPositionForValueUnclamped(double value) const;

    virtual double getValueForPosition(int position) const;
    virtual double getValueForPositionUnclamped(int position) const;

    virtual QString getUnit() const { return m_unit; }

protected:
    MappingType m_type;
    CoordMap m_mappings;
    QString m_unit;
    RangeMapper *m_mapper;

    MappingType chooseMappingTypeFor(const CoordMap &);
};

#endif
