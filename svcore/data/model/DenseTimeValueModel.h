/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _DENSE_TIME_VALUE_MODEL_H_
#define _DENSE_TIME_VALUE_MODEL_H_

#include <QObject>

#include "Model.h"

/**
 * Base class for models containing dense two-dimensional data (value
 * against time).  For example, audio waveform data.  Other time-value
 * plot data, especially if editable, will normally go into a
 * SparseTimeValueModel instead even if regularly sampled.
 */

class DenseTimeValueModel : public Model
{
    Q_OBJECT

public:
    DenseTimeValueModel();

    virtual ~DenseTimeValueModel();

    /**
     * Return the minimum possible value found in this model type.
     * (That is, the minimum that would be valid, not the minimum
     * actually found in a particular model).
     */
    virtual float getValueMinimum() const = 0;

    /**
     * Return the minimum possible value found in this model type.
     * (That is, the minimum that would be valid, not the minimum
     * actually found in a particular model).
     */
    virtual float getValueMaximum() const = 0;

    /**
     * Return the number of distinct channels for this model.
     */
    virtual int getChannelCount() const = 0;

    /**
     * Get the specified set of samples from the given channel of the
     * model in single-precision floating-point format.  Return the
     * number of samples actually retrieved.
     * If the channel is given as -1, mix all available channels and
     * return the result.
     */
    virtual sv_frame_t getData(int channel, sv_frame_t start, sv_frame_t count,
                               float *buffer) const = 0;

    /**
     * Get the specified set of samples from the given channel of the
     * model in double-precision floating-point format.  Return the
     * number of samples actually retrieved.
     * If the channel is given as -1, mix all available channels and
     * return the result.
     */
    virtual sv_frame_t getData(int channel, sv_frame_t start, sv_frame_t count,
                               double *buffer) const = 0;
    
    /**
     * Get the specified set of samples from given contiguous range
     * of channels of the model in single-precision floating-point
     * format.  Return the number of sample frames actually retrieved.
     */
    virtual sv_frame_t getData(int fromchannel, int tochannel,
                               sv_frame_t start, sv_frame_t count,
                               float **buffers) const = 0;

    virtual bool canPlay() const { return true; }
    virtual QString getDefaultPlayClipId() const { return ""; }

    virtual QString toDelimitedDataStringSubset(QString delimiter, sv_frame_t f0, sv_frame_t f1) const;

    QString getTypeName() const { return tr("Dense Time-Value"); }
    std::string getType() const { return "DenseTimeValueModel"; }
};

#endif
