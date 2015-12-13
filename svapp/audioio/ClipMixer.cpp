/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam, 2006-2014 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "ClipMixer.h"

#include <sndfile.h>
#include <cmath>

#include "base/Debug.h"

//#define DEBUG_CLIP_MIXER 1

ClipMixer::ClipMixer(int channels, sv_samplerate_t sampleRate, sv_frame_t blockSize) :
    m_channels(channels),
    m_sampleRate(sampleRate),
    m_blockSize(blockSize),
    m_clipData(0),
    m_clipLength(0),
    m_clipF0(0),
    m_clipRate(0)
{
}

ClipMixer::~ClipMixer()
{
    if (m_clipData) free(m_clipData);
}

void
ClipMixer::setChannelCount(int channels)
{
    m_channels = channels;
}

bool
ClipMixer::loadClipData(QString path, double f0, double level)
{
    if (m_clipData) {
        cerr << "ClipMixer::loadClipData: Already have clip loaded" << endl;
        return false;
    }

    SF_INFO info;
    SNDFILE *file;
    float *tmpFrames;
    sv_frame_t i;

    info.format = 0;
    file = sf_open(path.toLocal8Bit().data(), SFM_READ, &info);
    if (!file) {
	cerr << "ClipMixer::loadClipData: Failed to open file path \""
             << path << "\": " << sf_strerror(file) << endl;
	return false;
    }

    tmpFrames = (float *)malloc(info.frames * info.channels * sizeof(float));
    if (!tmpFrames) {
        cerr << "ClipMixer::loadClipData: malloc(" << info.frames * info.channels * sizeof(float) << ") failed" << endl;
        return false;
    }

    sf_readf_float(file, tmpFrames, info.frames);
    sf_close(file);

    m_clipData = (float *)malloc(info.frames * sizeof(float));
    if (!m_clipData) {
        cerr << "ClipMixer::loadClipData: malloc(" << info.frames * sizeof(float) << ") failed" << endl;
	free(tmpFrames);
	return false;
    }

    for (i = 0; i < info.frames; ++i) {
	int j;
	m_clipData[i] = 0.0f;
	for (j = 0; j < info.channels; ++j) {
	    m_clipData[i] += tmpFrames[i * info.channels + j] * float(level);
	}
    }

    free(tmpFrames);

    m_clipLength = info.frames;
    m_clipF0 = f0;
    m_clipRate = info.samplerate;

    return true;
}

void
ClipMixer::reset()
{
    m_playing.clear();
}

double
ClipMixer::getResampleRatioFor(double frequency)
{
    if (!m_clipData || !m_clipRate) return 1.0;
    double pitchRatio = m_clipF0 / frequency;
    double resampleRatio = m_sampleRate / m_clipRate;
    return pitchRatio * resampleRatio;
}

sv_frame_t
ClipMixer::getResampledClipDuration(double frequency)
{
    return sv_frame_t(ceil(double(m_clipLength) * getResampleRatioFor(frequency)));
}

void
ClipMixer::mix(float **toBuffers, 
               float gain,
               std::vector<NoteStart> newNotes, 
               std::vector<NoteEnd> endingNotes)
{
    foreach (NoteStart note, newNotes) {
        if (note.frequency > 20 && 
            note.frequency < 5000) {
            m_playing.push_back(note);
        }
    }

    std::vector<NoteStart> remaining;

    float *levels = new float[m_channels];

#ifdef DEBUG_CLIP_MIXER
    cerr << "ClipMixer::mix: have " << m_playing.size() << " playing note(s)"
         << " and " << endingNotes.size() << " note(s) ending here"
         << endl;
#endif

    foreach (NoteStart note, m_playing) {

        for (int c = 0; c < m_channels; ++c) {
            levels[c] = note.level * gain;
        }
        if (note.pan != 0.0 && m_channels == 2) {
            levels[0] *= 1.0f - note.pan;
            levels[1] *= note.pan + 1.0f;
        }

        sv_frame_t start = note.frameOffset;
        sv_frame_t durationHere = m_blockSize;
        if (start > 0) durationHere = m_blockSize - start;

        bool ending = false;

        foreach (NoteEnd end, endingNotes) {
            if (end.frequency == note.frequency && 
                end.frameOffset >= start &&
                end.frameOffset <= m_blockSize) {
                ending = true;
                durationHere = end.frameOffset;
                if (start > 0) durationHere = end.frameOffset - start;
                break;
            }
        }

        sv_frame_t clipDuration = getResampledClipDuration(note.frequency);
        if (start + clipDuration > 0) {
            if (start < 0 && start + clipDuration < durationHere) {
                durationHere = start + clipDuration;
            }
            if (durationHere > 0) {
                mixNote(toBuffers,
                        levels,
                        note.frequency,
                        start < 0 ? -start : 0,
                        start > 0 ?  start : 0,
                        durationHere,
                        ending);
            }
        }

        if (!ending) {
            NoteStart adjusted = note;
            adjusted.frameOffset -= m_blockSize;
            remaining.push_back(adjusted);
        }
    }

    delete[] levels;

    m_playing = remaining;
}

void
ClipMixer::mixNote(float **toBuffers,
                   float *levels,
                   float frequency,
                   sv_frame_t sourceOffset,
                   sv_frame_t targetOffset,
                   sv_frame_t sampleCount,
                   bool isEnd)
{
    if (!m_clipData) return;

    double ratio = getResampleRatioFor(frequency);
    
    double releaseTime = 0.01;
    sv_frame_t releaseSampleCount = sv_frame_t(round(releaseTime * m_sampleRate));
    if (releaseSampleCount > sampleCount) {
        releaseSampleCount = sampleCount;
    }
    double releaseFraction = 1.0/double(releaseSampleCount);

    for (sv_frame_t i = 0; i < sampleCount; ++i) {

        sv_frame_t s = sourceOffset + i;

        double os = double(s) / ratio;
        sv_frame_t osi = sv_frame_t(floor(os));

        //!!! just linear interpolation for now (same as SV's sample
        //!!! player). a small sinc kernel would be better and
        //!!! probably "good enough"
        double value = 0.0;
        if (osi < m_clipLength) {
            value += m_clipData[osi];
        }
        if (osi + 1 < m_clipLength) {
            value += (m_clipData[osi + 1] - m_clipData[osi]) * (os - double(osi));
        }
         
        if (isEnd && i + releaseSampleCount > sampleCount) {
            value *= releaseFraction * double(sampleCount - i); // linear ramp for release
        }

        for (int c = 0; c < m_channels; ++c) {
            toBuffers[c][targetOffset + i] += float(levels[c] * value);
        }
    }
}


