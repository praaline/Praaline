/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2013 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef AUDIO_TEST_DATA_H
#define AUDIO_TEST_DATA_H

#include <cmath>

#include "base/BaseTypes.h"

/**
 * Class that generates a single fixed test pattern to a given sample
 * rate and number of channels.
 *
 * The test pattern is two seconds long and consists of:
 *
 * -- in channel 0, a 600Hz sinusoid with peak amplitude 1.0
 *
 * -- in channel 1, four triangular forms with peaks at +1.0, -1.0,
 *    +1.0, -1.0 respectively, of 10ms width, starting at 0.0, 0.5,
 *    1.0 and 1.5 seconds; silence elsewhere
 *
 * -- in subsequent channels, a flat DC offset at +(channelNo / 20.0)
 */
class AudioTestData
{
public:
    AudioTestData(double rate, int channels) :
	m_channelCount(channels),
	m_duration(2.0),
	m_sampleRate(rate),
	m_sinFreq(600.0),
	m_pulseFreq(2)
    {
	m_frameCount = lrint(m_duration * m_sampleRate);
	m_data = new float[m_frameCount * m_channelCount];
	m_pulseWidth = 0.01 * m_sampleRate;
	generate();
    }

    ~AudioTestData() {
	delete[] m_data;
    }

    void generate() {

	double hpw = m_pulseWidth / 2.0;

	for (int i = 0; i < m_frameCount; ++i) {
	    for (int c = 0; c < m_channelCount; ++c) {

		double s = 0.0;

		if (c == 0) {

		    double phase = (i * m_sinFreq * 2.0 * M_PI) / m_sampleRate;
		    s = sin(phase);

		} else if (c == 1) {

		    int pulseNo = int((i * m_pulseFreq) / m_sampleRate);
		    int index = int(round((i * m_pulseFreq) -
                                          (m_sampleRate * pulseNo)));
		    if (index < m_pulseWidth) {
			s = 1.0 - fabs(hpw - index) / hpw;
			if (pulseNo % 2) s = -s;
		    }

		} else {

		    s = c / 20.0;
		}

		m_data[i * m_channelCount + c] = float(s);
	    }
	}
    }

    float *getInterleavedData() const {
	return m_data;
    }

    sv_frame_t getFrameCount() const { 
	return m_frameCount;
    }

    int getChannelCount() const {
	return m_channelCount;
    }

    sv_samplerate_t getSampleRate () const {
	return m_sampleRate;
    }

    double getDuration() const { // seconds
	return m_duration;
    }

private:
    float *m_data;
    sv_frame_t m_frameCount;
    int m_channelCount;
    double m_duration;
    sv_samplerate_t m_sampleRate;
    double m_sinFreq;
    double m_pulseFreq;
    double m_pulseWidth;
};

#endif

