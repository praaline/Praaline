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

#include "FFTFuzzyAdapter.h"

#include <cassert>

FFTFuzzyAdapter::FFTFuzzyAdapter(const DenseTimeValueModel *model,
				 int channel,
				 WindowType windowType,
				 int windowSize,
				 int windowIncrement,
				 int fftSize,
				 bool polar,
				 int fillFromColumn) :
    m_server(0),
    m_xshift(0),
    m_yshift(0)
{
    m_server = FFTDataServer::getFuzzyInstance(model,
                                               channel,
                                               windowType,
                                               windowSize,
                                               windowIncrement,
                                               fftSize,
                                               polar,
                                               fillFromColumn);

    int xratio = windowIncrement / m_server->getWindowIncrement();
    int yratio = m_server->getFFTSize() / fftSize;

    while (xratio > 1) {
        if (xratio & 0x1) {
            cerr << "ERROR: FFTFuzzyAdapter: Window increment ratio "
                      << windowIncrement << " / "
                      << m_server->getWindowIncrement()
                      << " must be a power of two" << endl;
            assert(!(xratio & 0x1));
        }
        ++m_xshift;
        xratio >>= 1;
    }

    while (yratio > 1) {
        if (yratio & 0x1) {
            cerr << "ERROR: FFTFuzzyAdapter: FFT size ratio "
                      << m_server->getFFTSize() << " / " << fftSize
                      << " must be a power of two" << endl;
            assert(!(yratio & 0x1));
        }
        ++m_yshift;
        yratio >>= 1;
    }
}

FFTFuzzyAdapter::~FFTFuzzyAdapter()
{
    FFTDataServer::releaseInstance(m_server);
}

