/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

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
   Rosegarden MIDI and audio sequencer and notation editor.
   This file copyright 2000-2006 Chris Cannam.
*/

#ifndef _AUDIO_LEVEL_H_
#define _AUDIO_LEVEL_H_

/**
 * AudioLevel converts audio sample levels between various scales:
 *
 *   - dB values (-inf -> 0dB)
 *   - floating-point values (-1.0 -> 1.0) such as used for a
 *     multiplier for gain or in floating-point WAV files
 *   - integer values intended to correspond to pixels on a fader
 *     or vu level scale.
 */

class AudioLevel
{
public:

    static const double DB_FLOOR;

    enum FaderType {
	     ShortFader = 0, // -40 -> +6  dB
              LongFader = 1, // -70 -> +10 dB
            IEC268Meter = 2, // -70 ->  0  dB
        IEC268LongMeter = 3, // -70 -> +10 dB (0dB aligns with LongFader)
	   PreviewLevel = 4
    };

    static double multiplier_to_dB(double multiplier);
    static double dB_to_multiplier(double dB);

    static double fader_to_dB(int level, int maxLevel, FaderType type);
    static int    dB_to_fader(double dB, int maxFaderLevel, FaderType type);

    static double fader_to_multiplier(int level, int maxLevel, FaderType type);
    static int    multiplier_to_fader(double multiplier, int maxFaderLevel,
				     FaderType type);

    // fast if "levels" doesn't change often -- for audio segment previews
    static int    multiplier_to_preview(double multiplier, int levels);
    static double preview_to_multiplier(int level, int levels);
};


#endif

