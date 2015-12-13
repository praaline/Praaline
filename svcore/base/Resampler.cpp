/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */
/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

/*
   This is a modified version of a source file from the 
   Rubber Band audio timestretcher library.
   This file copyright 2007 Chris Cannam.
*/

#include "Resampler.h"

#include <cstdlib>
#include <cmath>

#include <iostream>

#include <samplerate.h>

#include "Debug.h"

class Resampler::D
{
public:
    D(Quality quality, int channels, sv_frame_t chunkSize);
    ~D();

    sv_frame_t resample(float **in, float **out,
                 sv_frame_t incount, double ratio,
                 bool final);

    sv_frame_t resampleInterleaved(float *in, float *out,
                            sv_frame_t incount, double ratio,
                            bool final);

    void reset();

protected:
    SRC_STATE *m_src;
    float *m_iin;
    float *m_iout;
    int m_channels;
    sv_frame_t m_iinsize;
    sv_frame_t m_ioutsize;
};

Resampler::D::D(Quality quality, int channels, sv_frame_t chunkSize) :
    m_src(0),
    m_iin(0),
    m_iout(0),
    m_channels(channels),
    m_iinsize(0),
    m_ioutsize(0)
{
    int err = 0;
    m_src = src_new(quality == Best ? SRC_SINC_BEST_QUALITY :
                    quality == Fastest ? SRC_LINEAR :
                    SRC_SINC_FASTEST,
                    channels, &err);

    //!!! check err, throw

    if (chunkSize > 0 && m_channels > 1) {
        //!!! alignment?
        m_iinsize = chunkSize * m_channels;
        m_ioutsize = chunkSize * m_channels * 2;
        m_iin = (float *)malloc(m_iinsize * sizeof(float));
        m_iout = (float *)malloc(m_ioutsize * sizeof(float));
    }
}

Resampler::D::~D()
{
    src_delete(m_src);
    if (m_iinsize > 0) {
        free(m_iin);
    }
    if (m_ioutsize > 0) {
        free(m_iout);
    }
}

sv_frame_t
Resampler::D::resample(float **in, float **out,
                       sv_frame_t incount, double ratio,
                       bool final)
{
    if (m_channels == 1) {
        return resampleInterleaved(*in, *out, incount, ratio, final);
    }

    sv_frame_t outcount = lrint(ceil(double(incount) * ratio));

    if (incount * m_channels > m_iinsize) {
        m_iinsize = incount * m_channels;
        m_iin = (float *)realloc(m_iin, m_iinsize * sizeof(float));
    }
    if (outcount * m_channels > m_ioutsize) {
        m_ioutsize = outcount * m_channels;
        m_iout = (float *)realloc(m_iout, m_ioutsize * sizeof(float));
    }
    for (sv_frame_t i = 0; i < incount; ++i) {
        for (int c = 0; c < m_channels; ++c) {
            m_iin[i * m_channels + c] = in[c][i];
        }
    }
    
    sv_frame_t gen = resampleInterleaved(m_iin, m_iout, incount, ratio, final);

    for (sv_frame_t i = 0; i < gen; ++i) {
        for (int c = 0; c < m_channels; ++c) {
            out[c][i] = m_iout[i * m_channels + c];
        }
    }

    return gen;
}

sv_frame_t
Resampler::D::resampleInterleaved(float *in, float *out,
                                  sv_frame_t incount, double ratio,
                                  bool final)
{
    SRC_DATA data;

    sv_frame_t outcount = lrint(ceil(double(incount) * ratio));

    data.data_in = in;
    data.data_out = out;
    data.input_frames = incount;
    data.output_frames = outcount;
    data.src_ratio = ratio;
    data.end_of_input = (final ? 1 : 0);

    int err = src_process(m_src, &data);

    if (err) {
        cerr << "Resampler: ERROR: src_process returned error: " <<
            src_strerror(err) << endl;
        return 0;
    }

    if (data.input_frames_used != incount) {
        cerr << "Resampler: NOTE: input_frames_used == " << data.input_frames_used << " (while incount = " << incount << ")" << endl;
    }

    return data.output_frames_gen;
}

void
Resampler::D::reset()
{
    src_reset(m_src);
}

Resampler::Resampler(Quality quality, int channels, sv_frame_t chunkSize)
{
    m_d = new D(quality, channels, chunkSize);
}

Resampler::~Resampler()
{
    delete m_d;
}

sv_frame_t 
Resampler::resample(float **in, float **out,
                    sv_frame_t incount, double ratio,
                    bool final)
{
    return m_d->resample(in, out, incount, ratio, final);
}

sv_frame_t 
Resampler::resampleInterleaved(float *in, float *out,
                               sv_frame_t incount, double ratio,
                               bool final)
{
    return m_d->resampleInterleaved(in, out, incount, ratio, final);
}

void
Resampler::reset()
{
    m_d->reset();
}

