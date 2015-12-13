/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    FFT code from Don Cross's public domain FFT implementation.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "FFTapi.h"

#ifndef HAVE_FFTW3F

#include <cmath>
#include <iostream>

void
fft(unsigned int n, bool inverse, double *ri, double *ii, double *ro, double *io)
{
    if (!ri || !ro || !io) return;

    unsigned int bits;
    unsigned int i, j, k, m;
    unsigned int blockSize, blockEnd;

    double tr, ti;

    if (n < 2) return;
    if (n & (n-1)) return;

    double angle = 2.0 * M_PI;
    if (inverse) angle = -angle;

    for (i = 0; ; ++i) {
	if (n & (1 << i)) {
	    bits = i;
	    break;
	}
    }

    int *table = new int[n];

    for (i = 0; i < n; ++i) {
	
        m = i;
        
        for (j = k = 0; j < bits; ++j) {
            k = (k << 1) | (m & 1);
            m >>= 1;
        }
        
        table[i] = k;
    }

    if (ii) {
	for (i = 0; i < n; ++i) {
	    ro[table[i]] = ri[i];
	    io[table[i]] = ii[i];
	}
    } else {
	for (i = 0; i < n; ++i) {
	    ro[table[i]] = ri[i];
	    io[table[i]] = 0.0;
	}
    }

    blockEnd = 1;

    for (blockSize = 2; blockSize <= n; blockSize <<= 1) {

	double delta = angle / (double)blockSize;
	double sm2 = -sin(-2 * delta);
	double sm1 = -sin(-delta);
	double cm2 = cos(-2 * delta);
	double cm1 = cos(-delta);
	double w = 2 * cm1;
	double ar[3], ai[3];

	for (i = 0; i < n; i += blockSize) {

	    ar[2] = cm2;
	    ar[1] = cm1;

	    ai[2] = sm2;
	    ai[1] = sm1;

	    for (j = i, m = 0; m < blockEnd; j++, m++) {

		ar[0] = w * ar[1] - ar[2];
		ar[2] = ar[1];
		ar[1] = ar[0];

		ai[0] = w * ai[1] - ai[2];
		ai[2] = ai[1];
		ai[1] = ai[0];

		k = j + blockEnd;
		tr = ar[0] * ro[k] - ai[0] * io[k];
		ti = ar[0] * io[k] + ai[0] * ro[k];

		ro[k] = ro[j] - tr;
		io[k] = io[j] - ti;

		ro[j] += tr;
		io[j] += ti;
	    }
	}

	blockEnd = blockSize;
    }

/* fftw doesn't normalise, so nor will we

    if (inverse) {

	double denom = (double)n;

	for (i = 0; i < n; i++) {
	    ro[i] /= denom;
	    io[i] /= denom;
	}
    }
*/
    delete[] table;
}

struct fftf_plan_ {
    int size;
    int inverse;
    float *real;
    fftf_complex *cplx;
};

fftf_plan
fftf_plan_dft_r2c_1d(int n, float *in, fftf_complex *out, unsigned)
{
    if (n < 2) return 0;
    if (n & (n-1)) return 0;
    
    fftf_plan_ *plan = new fftf_plan_;
    plan->size = n;
    plan->inverse = 0;
    plan->real = in;
    plan->cplx = out;
    return plan;
}

fftf_plan
fftf_plan_dft_c2r_1d(int n, fftf_complex *in, float *out, unsigned)
{
    if (n < 2) return 0;
    if (n & (n-1)) return 0;
    
    fftf_plan_ *plan = new fftf_plan_;
    plan->size = n;
    plan->inverse = 1;
    plan->real = out;
    plan->cplx = in;
    return plan;
}

void
fftf_destroy_plan(fftf_plan p)
{
    delete p;
}

void
fftf_execute(const fftf_plan p)
{
    float *real = p->real;
    fftf_complex *cplx = p->cplx;
    int n = p->size;
    int forward = !p->inverse;

    double *ri = new double[n];
    double *ro = new double[n];
    double *io = new double[n];

    double *ii = 0;
    if (!forward) ii = new double[n];

    if (forward) {
        for (int i = 0; i < n; ++i) {
            ri[i] = real[i];
        }
    } else {
        for (int i = 0; i < n/2+1; ++i) {
            ri[i] = cplx[i][0];
            ii[i] = cplx[i][1];
            if (i > 0) {
                ri[n-i] = ri[i];
                ii[n-i] = -ii[i];
            }
        }
    }

    fft(n, !forward, ri, ii, ro, io);

    if (forward) {
        for (int i = 0; i < n/2+1; ++i) {
            cplx[i][0] = ro[i];
            cplx[i][1] = io[i];
        }
    } else {
        for (int i = 0; i < n; ++i) {
            real[i] = ro[i];
        }
    }

    delete[] ri;
    delete[] ro;
    delete[] io;
    if (ii) delete[] ii;
}

#endif
