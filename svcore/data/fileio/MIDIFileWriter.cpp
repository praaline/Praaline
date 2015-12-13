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

#include "MIDIFileWriter.h"

#include "data/midi/MIDIEvent.h"
#include "model/NoteData.h"

#include "base/Pitch.h"

#include <algorithm>
#include <fstream>

using std::ofstream;
using std::string;
using std::ios;

using namespace MIDIConstants;

MIDIFileWriter::MIDIFileWriter(QString path, const NoteExportable *exportable,
                               sv_samplerate_t sampleRate, float tempo) :
    m_path(path),
    m_exportable(exportable),
    m_sampleRate(sampleRate),
    m_tempo(tempo),
    m_midiFile(0)
{
    if (!convert()) {
        m_error = "Conversion from model to internal MIDI format failed";
    }
}

MIDIFileWriter::~MIDIFileWriter()
{
    for (MIDIComposition::iterator i = m_midiComposition.begin();
	 i != m_midiComposition.end(); ++i) {
	
	for (MIDITrack::iterator j = i->second.begin();
	     j != i->second.end(); ++j) {
	    delete *j;
	}

	i->second.clear();
    }

    m_midiComposition.clear();
}

bool
MIDIFileWriter::isOK() const
{
    return m_error == "";
}

QString
MIDIFileWriter::getError() const
{
    return m_error;
}

void
MIDIFileWriter::write()
{
    writeComposition();
}

string
MIDIFileWriter::intToMIDIBytes(int number) const
{
    MIDIByte upper;
    MIDIByte lower;

    upper = MIDIByte((number & 0xFF00) >> 8);
    lower = MIDIByte( number & 0x00FF);

    string rv;
    rv += upper;
    rv += lower;
    return rv;
}

string
MIDIFileWriter::longToMIDIBytes(unsigned long number) const
{
    MIDIByte upper1;
    MIDIByte lower1;
    MIDIByte upper2;
    MIDIByte lower2;

    upper1 = MIDIByte((number & 0xff000000) >> 24);
    lower1 = MIDIByte((number & 0x00ff0000) >> 16);
    upper2 = MIDIByte((number & 0x0000ff00) >> 8);
    lower2 = MIDIByte((number & 0x000000ff));

    string rv;
    rv += upper1;
    rv += lower1;
    rv += upper2;
    rv += lower2;
    return rv;
}

// Turn a delta time into a MIDI time - overlapping into
// a maximum of four bytes using the MSB as the carry on
// flag.
//
string
MIDIFileWriter::longToVarBuffer(unsigned long number) const
{
    string rv;

    long inNumber = number;
    long outNumber;

    // get the lowest 7 bits of the number
    outNumber = number & 0x7f;

    // Shift and test and move the numbers
    // on if we need them - setting the MSB
    // as we go.
    //
    while ((inNumber >>= 7 ) > 0) {
        outNumber <<= 8;
        outNumber |= 0x80;
        outNumber += (inNumber & 0x7f);
    }

    // Now move the converted number out onto the buffer
    //
    while (true) {
        rv += (MIDIByte)(outNumber & 0xff);
        if (outNumber & 0x80)
            outNumber >>= 8;
        else
            break;
    }

    return rv;
}

bool
MIDIFileWriter::writeHeader()
{
    *m_midiFile << MIDI_FILE_HEADER;

    // Number of bytes in header
    *m_midiFile << (MIDIByte) 0x00;
    *m_midiFile << (MIDIByte) 0x00;
    *m_midiFile << (MIDIByte) 0x00;
    *m_midiFile << (MIDIByte) 0x06;

    // File format
    *m_midiFile << (MIDIByte) 0x00;
    *m_midiFile << (MIDIByte) m_format;

    *m_midiFile << intToMIDIBytes(m_numberOfTracks);

    *m_midiFile << intToMIDIBytes(m_timingDivision);

    return true;
}

bool
MIDIFileWriter::writeTrack(int trackNumber)
{
    bool retOK = true;
    MIDIByte eventCode = 0;
    MIDITrack::iterator midiEvent;

    // First we write into the trackBuffer, then write it out to the
    // file with its accompanying length.
    //
    string trackBuffer;

    for (midiEvent = m_midiComposition[trackNumber].begin();
         midiEvent != m_midiComposition[trackNumber].end();
         midiEvent++) {

        // Write the time to the buffer in MIDI format
        trackBuffer += longToVarBuffer((*midiEvent)->getTime());

        if ((*midiEvent)->isMeta()) {
            trackBuffer += MIDI_FILE_META_EVENT;
            trackBuffer += (*midiEvent)->getMetaEventCode();

            // Variable length number field
            trackBuffer += longToVarBuffer((*midiEvent)->
                                           getMetaMessage().length());

            trackBuffer += (*midiEvent)->getMetaMessage();
        } else {
            // Send the normal event code (with encoded channel information)
            if (((*midiEvent)->getEventCode() != eventCode) ||
                ((*midiEvent)->getEventCode() == MIDI_SYSTEM_EXCLUSIVE)) {
                trackBuffer += (*midiEvent)->getEventCode();
                eventCode = (*midiEvent)->getEventCode();
            }

            // Send the relevant data
            //
            switch ((*midiEvent)->getMessageType()) {
            case MIDI_NOTE_ON:
            case MIDI_NOTE_OFF:
            case MIDI_POLY_AFTERTOUCH:
                trackBuffer += (*midiEvent)->getData1();
                trackBuffer += (*midiEvent)->getData2();
                break;

            case MIDI_CTRL_CHANGE:
                trackBuffer += (*midiEvent)->getData1();
                trackBuffer += (*midiEvent)->getData2();
                break;

            case MIDI_PROG_CHANGE:
                trackBuffer += (*midiEvent)->getData1();
                break;

            case MIDI_CHNL_AFTERTOUCH:
                trackBuffer += (*midiEvent)->getData1();
                break;

            case MIDI_PITCH_BEND:
                trackBuffer += (*midiEvent)->getData1();
                trackBuffer += (*midiEvent)->getData2();
                break;

            case MIDI_SYSTEM_EXCLUSIVE:
                // write out message length
                trackBuffer +=
                    longToVarBuffer((*midiEvent)->getMetaMessage().length());

                // now the message
                trackBuffer += (*midiEvent)->getMetaMessage();
                break;

            default:
                break;
            }
        }
    }

    // Now we write the track - First the standard header..
    //
    *m_midiFile << MIDI_TRACK_HEADER;

    // ..now the length of the buffer..
    //
    *m_midiFile << longToMIDIBytes((long)trackBuffer.length());

    // ..then the buffer itself..
    //
    *m_midiFile << trackBuffer;

    return retOK;
}

bool
MIDIFileWriter::writeComposition()
{
    bool retOK = true;

    m_midiFile =
        new ofstream(m_path.toLocal8Bit().data(), ios::out | ios::binary);

    if (!(*m_midiFile)) {
        m_error = "Can't open file for writing.";
        delete m_midiFile;
        m_midiFile = 0;
        return false;
    }

    if (!writeHeader()) {
        retOK = false;
    }

    for (unsigned int i = 0; i < m_numberOfTracks; i++) {
        if (!writeTrack(i)) {
            retOK = false;
        }
    }

    m_midiFile->close();
    delete m_midiFile;
    m_midiFile = 0;

    if (!retOK) {
        m_error = "MIDI file write failed";
    }

    return retOK;
}

bool
MIDIFileWriter::convert()
{
    m_timingDivision = 480;
    m_format = MIDI_SINGLE_TRACK_FILE;
    m_numberOfTracks = 1;

    int track = 0;

    MIDIEvent *event;

    event = new MIDIEvent(0, MIDI_FILE_META_EVENT, MIDI_CUE_POINT,
                          "Exported from Sonic Visualiser");
    m_midiComposition[track].push_back(event);

    event = new MIDIEvent(0, MIDI_FILE_META_EVENT, MIDI_CUE_POINT,
                          "http://www.sonicvisualiser.org/");
    m_midiComposition[track].push_back(event);

    long tempoValue = long(60000000.0 / m_tempo + 0.01);
    string tempoString;
    tempoString += (MIDIByte)(tempoValue >> 16 & 0xFF);
    tempoString += (MIDIByte)(tempoValue >> 8 & 0xFF);
    tempoString += (MIDIByte)(tempoValue & 0xFF);

    event = new MIDIEvent(0, MIDI_FILE_META_EVENT, MIDI_SET_TEMPO,
                          tempoString);
    m_midiComposition[track].push_back(event);

    // Omit time signature

    NoteList notes = m_exportable->getNotes();

    for (NoteList::const_iterator i = notes.begin(); i != notes.end(); ++i) {

        sv_frame_t frame = i->start;
        sv_frame_t duration = i->duration;
        int pitch = i->midiPitch;
        int velocity = i->velocity;
        int channel = i->channel;

        if (pitch < 0) pitch = 0;
        if (pitch > 127) pitch = 127;

        if (channel < 0) channel = 0;
        if (channel > 15) channel = 0;

        // Convert frame to MIDI time

        double seconds = double(frame) / m_sampleRate;
        double quarters = (seconds * m_tempo) / 60.0;
        unsigned long midiTime = int(quarters * m_timingDivision + 0.5);

        // Get the sounding time for the matching NOTE_OFF
        seconds = double(frame + duration) / m_sampleRate;
        quarters = (seconds * m_tempo) / 60.0;
        unsigned long endTime = int(quarters * m_timingDivision + 0.5);

        // At this point all the notes we insert have absolute times
        // in the delta time fields.  We resolve these into delta
        // times further down (can't do it until all the note offs are
        // in place).

        event = new MIDIEvent(midiTime,
                              MIDI_NOTE_ON | channel,
                              pitch,
                              velocity);
        m_midiComposition[track].push_back(event);

        event = new MIDIEvent(endTime,
                              MIDI_NOTE_OFF | channel,
                              pitch,
                              127); // loudest silence you can muster

        m_midiComposition[track].push_back(event);
    }
    
    // Now gnash through the MIDI events and turn the absolute times
    // into delta times.
    //
    for (unsigned int i = 0; i < m_numberOfTracks; i++) {

        unsigned long lastMidiTime = 0;

        // First sort the track with the MIDIEvent comparator.  Use
        // stable_sort so that events with equal times are maintained
        // in their current order.
        //
        std::stable_sort(m_midiComposition[i].begin(),
                         m_midiComposition[i].end(),
                         MIDIEventCmp());

        for (MIDITrack::iterator it = m_midiComposition[i].begin();
             it != m_midiComposition[i].end(); it++) {
            unsigned long deltaTime = (*it)->getTime() - lastMidiTime;
            lastMidiTime = (*it)->getTime();
            (*it)->setTime(deltaTime);
        }

        // Insert end of track event (delta time = 0)
        //
        event = new MIDIEvent(0, MIDI_FILE_META_EVENT,
                              MIDI_END_OF_TRACK, "");

        m_midiComposition[i].push_back(event);
    }

    return true;
}

