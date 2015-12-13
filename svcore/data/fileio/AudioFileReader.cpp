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

#include "AudioFileReader.h"

using std::vector;

vector<SampleBlock>
AudioFileReader::getDeInterleavedFrames(sv_frame_t start, sv_frame_t count) const
{
    SampleBlock interleaved = getInterleavedFrames(start, count);
    
    int channels = getChannelCount();
    sv_frame_t rc = interleaved.size() / channels;

    vector<SampleBlock> frames(channels, SampleBlock(rc, 0.f));
    
    for (int c = 0; c < channels; ++c) {
        for (sv_frame_t i = 0; i < rc; ++i) {
            frames[c][i] = interleaved[i * channels + c];
        }
    }

    return frames;
}

