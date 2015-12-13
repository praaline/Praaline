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

#ifndef _RANGE_SUMMARISABLE_TIME_VALUE_MODEL_H_
#define _RANGE_SUMMARISABLE_TIME_VALUE_MODEL_H_

#include <QObject>

#include "DenseTimeValueModel.h"
#include "base/ZoomConstraint.h"

#include <stdint.h>

/**
 * Base class for models containing dense two-dimensional data (value
 * against time) that may be meaningfully represented in a zoomed view
 * using min/max range summaries.  Audio waveform data is an obvious
 * example: think "peaks and minima" for "ranges".
 */

class RangeSummarisableTimeValueModel : public DenseTimeValueModel
{
    Q_OBJECT

public:
    RangeSummarisableTimeValueModel() { }

    class Range
    {
    public:
        Range() : 
            m_new(true), m_min(0.f), m_max(0.f), m_absmean(0.f) { }
        Range(const Range &r) : 
            m_new(true), m_min(r.m_min), m_max(r.m_max), m_absmean(r.m_absmean) { }
        Range(float min, float max, float absmean) :
            m_new(true), m_min(min), m_max(max), m_absmean(absmean) { }

        float min() const { return m_min; }
        float max() const { return m_max; }
        float absmean() const { return m_absmean; }

        void setMin(float min) { m_min = min; m_new = false; }
        void setMax(float max) { m_max = max; m_new = false; }
        void setAbsmean(float absmean) { m_absmean = absmean; }

        void sample(float s) {
            if (m_new) {
                m_min = s;
                m_max = s;
                m_new = false;
            } else {
                if (s < m_min) m_min = s;
                if (s > m_max) m_max = s;
            }
        }
        
    private:
        bool m_new;
        float m_min;
        float m_max;
        float m_absmean;
    };

    typedef std::vector<Range> RangeBlock;

    /**
     * Return ranges from the given start frame, corresponding to the
     * given number of underlying sample frames, summarised at the
     * given block size.  duration / blockSize ranges should ideally
     * be returned.
     *
     * If the given block size is not supported by this model
     * (according to its zoom constraint), also modify the blockSize
     * parameter so as to return the block size that was actually
     * obtained.
     */
    virtual void getSummaries(int channel, sv_frame_t start, sv_frame_t count,
                              RangeBlock &ranges,
                              int &blockSize) const = 0;

    /**
     * Return the range from the given start frame, corresponding to
     * the given number of underlying sample frames, summarised at a
     * block size equal to the distance between start and end frames.
     */
    virtual Range getSummary(int channel, sv_frame_t start, sv_frame_t count) const = 0;

    virtual int getSummaryBlockSize(int desired) const = 0;

    QString getTypeName() const { return tr("Range-Summarisable Time-Value"); }
    std::string getType() const { return "RangeSummarisableTimeValueModel"; }
};

#endif

