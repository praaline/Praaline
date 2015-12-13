/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _FFT_API_H_
#define _FFT_API_H_

#ifdef HAVE_FFTW3F

#include <fftw3.h>

#define fftf_complex fftwf_complex
#define fftf_malloc fftwf_malloc
#define fftf_free fftwf_free
#define fftf_plan fftwf_plan
#define fftf_plan_dft_r2c_1d fftwf_plan_dft_r2c_1d
#define fftf_plan_dft_c2r_1d fftwf_plan_dft_c2r_1d
#define fftf_execute fftwf_execute
#define fftf_destroy_plan fftwf_destroy_plan

#else

// Provide a fallback FFT implementation if FFTW3f is not available.

typedef float fftf_complex[2];
#define fftf_malloc malloc
#define fftf_free free

struct fftf_plan_;
typedef fftf_plan_ *fftf_plan;

fftf_plan fftf_plan_dft_r2c_1d(int n, float *in, fftf_complex *out, unsigned);
fftf_plan fftf_plan_dft_c2r_1d(int n, fftf_complex *in, float *out, unsigned);
void fftf_execute(const fftf_plan p);
void fftf_destroy_plan(fftf_plan p);

#define FFTW_ESTIMATE 0
#define FFTW_MEASURE 0

#endif

#endif

