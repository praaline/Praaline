/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2009 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _FRAME_TIMER_H_
#define _FRAME_TIMER_H_

#include "base/BaseTypes.h"

/**
 * A trivial interface for things that permit retrieving "the current
 * frame".  Implementations of this interface are used, for example,
 * for timestamping incoming MIDI events when tapping to MIDI.
 */

class FrameTimer
{
public:
    virtual sv_frame_t getFrame() const = 0;
};

#endif
