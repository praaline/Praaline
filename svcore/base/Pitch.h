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

#ifndef _PITCH_H_
#define _PITCH_H_

#include <QString>

class Pitch
{
public:
    /**
     * Return the frequency at the given MIDI pitch plus centsOffset
     * cents (1/100ths of a semitone).  centsOffset does not have to
     * be in any particular range or sign.
     *
     * If concertA is non-zero, use that as the reference frequency
     * for the A at MIDI pitch 69; otherwise use the tuning frequency
     * specified in the application preferences (default 440Hz).
     */
    static double getFrequencyForPitch(int midiPitch,
				      double centsOffset = 0,
				      double concertA = 0.0);

    /**
     * Return the nearest MIDI pitch to the given frequency.
     *
     * If centsOffsetReturn is non-NULL, return in *centsOffsetReturn
     * the number of cents (1/100ths of a semitone) difference between
     * the given frequency and that of the returned MIDI pitch.  The
     * cents offset will be in the range [-50,50).
     * 
     * If concertA is non-zero, use that as the reference frequency
     * for the A at MIDI pitch 69; otherwise use the tuning frequency
     * specified in the application preferences (default 440Hz).
     */
    static int getPitchForFrequency(double frequency,
				    double *centsOffsetReturn = 0,
				    double concertA = 0.0);

    /**
     * Compatibility version of getPitchForFrequency accepting float
     * pointer argument.
     */
    static int getPitchForFrequency(double frequency,
				    float *centsOffsetReturn,
				    double concertA = 0.0) {
        double c;
        int p = getPitchForFrequency(frequency, &c, concertA);
        if (centsOffsetReturn) *centsOffsetReturn = float(c);
        return p;
    }

    /**
     * Return the nearest MIDI pitch range to the given frequency
     * range, that is, the difference in MIDI pitch values between the
     * higher and lower frequencies.
     *
     * If centsOffsetReturn is non-NULL, return in *centsOffsetReturn
     * the number of cents (1/100ths of a semitone) difference between
     * the given frequency difference and the returned MIDI pitch
     * range.  The cents offset will be in the range [-50,50).
     * 
     * If concertA is non-zero, use that as the reference frequency
     * for the A at MIDI pitch 69; otherwise use the tuning frequency
     * specified in the application preferences (default 440Hz).
     */
    static int getPitchForFrequencyDifference(double frequencyA,
                                              double frequencyB,
                                              double *centsOffsetReturn = 0,
                                              double concertA = 0.0);

    /**
     * Compatibility version of getPitchForFrequencyDifference
     * accepting float pointer argument.
     */
    static int getPitchForFrequencyDifference(double frequencyA,
                                              double frequencyB,
                                              float *centsOffsetReturn,
                                              double concertA = 0.0) {
        double c;
        int p = getPitchForFrequencyDifference(frequencyA, frequencyB,
                                               &c, concertA);
        if (centsOffsetReturn) *centsOffsetReturn = float(c);
        return p;
    }
    
    /**
     * Return the MIDI pitch for the given note number (0-12 where 0
     * is C) and octave number. The octave numbering system is based
     * on the application preferences (default is C4 = middle C,
     * though in previous SV releases that was C3).
     */
    static int getPitchForNoteAndOctave(int note, int octave);
    
    /**
     * Return the note number (0-12 where 0 is C) and octave number
     * for the given MIDI pitch. The octave numbering system is based
     * on the application preferences (default is C4 = middle C,
     * though in previous SV releases that was C3).
     */
    static void getNoteAndOctaveForPitch(int midiPitch, int &note, int &octave);

    /**
     * Return a string describing the given MIDI pitch, with optional
     * cents offset.  This consists of the note name, octave number,
     * and optional cents. The octave numbering system is based on the
     * application preferences (default is C4 = middle C, though in
     * previous SV releases that was C3).
     *
     * For example, "A#3" (A# in octave 3) or "C2-12c" (C in octave 2,
     * minus 12 cents).
     *
     * If useFlats is true, spell notes with flats instead of sharps,
     * e.g. Bb3 instead of A#3.
     */
    static QString getPitchLabel(int midiPitch,
				 double centsOffset = 0,
				 bool useFlats = false);
    
    /**
     * Return a string describing the nearest MIDI pitch to the given
     * frequency, with cents offset.
     *
     * If concertA is non-zero, use that as the reference frequency
     * for the A at MIDI pitch 69; otherwise use the tuning frequency
     * specified in the application preferences (default 440Hz).
     *
     * If useFlats is true, spell notes with flats instead of sharps,
     * e.g. Bb3 instead of A#3.
     */
    static QString getPitchLabelForFrequency(double frequency,
					     double concertA = 0.0,
					     bool useFlats = false);

    /**
     * Return a string describing the given pitch range in octaves,
     * semitones and cents.  This is in the form e.g. "1'2+4c".
     */
    static QString getLabelForPitchRange(int semis, double cents = 0);

    /**
     * Return true if the given frequency falls within the range of
     * MIDI note pitches, plus or minus half a semitone.  This is
     * equivalent to testing whether getPitchForFrequency returns a
     * pitch in the MIDI range (0 to 127 inclusive) with any cents
     * offset.
     *
     * If concertA is non-zero, use that as the reference frequency
     * for the A at MIDI pitch 69; otherwise use the tuning frequency
     * specified in the application preferences (default 440Hz).
     */
    static bool isFrequencyInMidiRange(double frequency,
                                       double concertA = 0.0);
};


#endif
