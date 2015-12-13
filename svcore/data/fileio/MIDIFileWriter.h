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
   This file copyright 2000-2007 Richard Bown and Chris Cannam
   and copyright 2007 QMUL.
*/

#ifndef _MIDI_FILE_WRITER_H_
#define _MIDI_FILE_WRITER_H_

#include "base/RealTime.h"
#include "base/BaseTypes.h"

#include <QString>

#include <vector>
#include <map>
#include <fstream>

class MIDIEvent;
class NoteExportable;

/**
 * Write a MIDI file.  This includes file write code for generic
 * simultaneous-track MIDI files, but the conversion stage only
 * supports a single-track MIDI file with fixed tempo, time signature
 * and timing division.
 */
class MIDIFileWriter 
{
public:
    MIDIFileWriter(QString path, 
                   const NoteExportable *exportable, 
                   sv_samplerate_t sampleRate, // used to convert exportable sample timings
                   float tempo = 120.f);
    virtual ~MIDIFileWriter();

    virtual bool isOK() const;
    virtual QString getError() const;

    virtual void write();

protected:
    typedef std::vector<MIDIEvent *> MIDITrack;
    typedef std::map<unsigned int, MIDITrack> MIDIComposition;

    typedef enum {
	MIDI_SINGLE_TRACK_FILE          = 0x00,
	MIDI_SIMULTANEOUS_TRACK_FILE    = 0x01,
	MIDI_SEQUENTIAL_TRACK_FILE      = 0x02,
	MIDI_FILE_BAD_FORMAT            = 0xFF
    } MIDIFileFormatType;

    std::string intToMIDIBytes(int number) const;
    std::string longToMIDIBytes(unsigned long number) const;
    std::string longToVarBuffer(unsigned long number) const;

    unsigned long getMIDITimeForTime(RealTime t) const;

    bool writeHeader();
    bool writeTrack(int track);
    bool writeComposition();
    
    bool convert();

    QString               m_path;
    const NoteExportable *m_exportable;
    sv_samplerate_t       m_sampleRate;
    float                 m_tempo;
    int                   m_timingDivision;   // pulses per quarter note
    MIDIFileFormatType    m_format;
    unsigned int          m_numberOfTracks;

    MIDIComposition       m_midiComposition;

    std::ofstream        *m_midiFile;
    QString               m_error;
};

#endif
