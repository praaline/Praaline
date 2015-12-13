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

#ifndef CLIP_MIXER_H
#define CLIP_MIXER_H

#include <QString>
#include <vector>

#include "base/BaseTypes.h"

/**
 * Mix in synthetic notes produced by resampling a prerecorded
 * clip. (i.e. this is an implementation of a digital sampler in the
 * musician's sense.) This can mix any number of notes of arbitrary
 * frequency, so long as they all use the same sample clip.
 */

class ClipMixer
{
public:
    ClipMixer(int channels, sv_samplerate_t sampleRate, sv_frame_t blockSize);
    ~ClipMixer();

    void setChannelCount(int channels);

    /**
     * Load a sample clip from a wav file. This can only happen once:
     * construct a new ClipMixer if you want a different clip. The
     * clip was recorded at a pitch with fundamental frequency clipF0,
     * and should be scaled by level (in the range 0-1) when playing
     * back.
     */
    bool loadClipData(QString clipFilePath, double clipF0, double level);

    void reset(); // discarding any playing notes

    struct NoteStart {
	sv_frame_t frameOffset; // within current processing block
	float frequency; // Hz
	float level; // volume in range (0,1]
	float pan; // range [-1,1]
    };

    struct NoteEnd {
	sv_frame_t frameOffset; // in current processing block
        float frequency; // matching note start
    };

    void mix(float **toBuffers, 
             float gain,
	     std::vector<NoteStart> newNotes, 
	     std::vector<NoteEnd> endingNotes);

private:
    int m_channels;
    sv_samplerate_t m_sampleRate;
    sv_frame_t m_blockSize;

    QString m_clipPath;

    float *m_clipData;
    sv_frame_t m_clipLength;
    double m_clipF0;
    sv_samplerate_t m_clipRate;

    std::vector<NoteStart> m_playing;

    double getResampleRatioFor(double frequency);
    sv_frame_t getResampledClipDuration(double frequency);

    void mixNote(float **toBuffers, 
                 float *levels,
                 float frequency,
                 sv_frame_t sourceOffset, // within resampled note
                 sv_frame_t targetOffset, // within target buffer
                 sv_frame_t sampleCount,
                 bool isEnd);
};


#endif
