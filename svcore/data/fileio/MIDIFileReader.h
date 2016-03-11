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
   This file copyright 2000-2006 Richard Bown and Chris Cannam.
*/

#ifndef _MIDI_FILE_READER_H_
#define _MIDI_FILE_READER_H_

#include "DataFileReader.h"
#include "base/BaseTypes.h"
#include "base/RealTime.h"

#include <map>
#include <set>
#include <vector>

#include <QObject>

class MIDIEvent;

typedef unsigned char MIDIByte;

class MIDIFileImportPreferenceAcquirer // welcome to our grand marble foyer
{
public:
    enum TrackPreference {
        ImportNothing,
        ImportSingleTrack,
        MergeAllTracks,
        MergeAllNonPercussionTracks
    };

    virtual ~MIDIFileImportPreferenceAcquirer() { }

    virtual TrackPreference getTrackImportPreference
    (QStringList trackNames, bool haveSomePercussion,
     QString &singleTrack) const = 0;

    virtual void showError(QString error) = 0;
};


class MIDIFileReader : public DataFileReader
{
    Q_OBJECT

public:
    MIDIFileReader(QString path,
                   MIDIFileImportPreferenceAcquirer *pref,
                   sv_samplerate_t mainModelSampleRate);
    virtual ~MIDIFileReader();

    virtual bool isOK() const;
    virtual QString getError() const;
    virtual Model *load() const;

protected:
    typedef std::vector<MIDIEvent *> MIDITrack;
    typedef std::map<unsigned int, MIDITrack> MIDIComposition;
    typedef std::pair<RealTime, double> TempoChange; // time, qpm
    typedef std::map<unsigned long, TempoChange> TempoMap; // key is MIDI time

    typedef enum {
	MIDI_SINGLE_TRACK_FILE          = 0x00,
	MIDI_SIMULTANEOUS_TRACK_FILE    = 0x01,
	MIDI_SEQUENTIAL_TRACK_FILE      = 0x02,
	MIDI_FILE_BAD_FORMAT            = 0xFF
    } MIDIFileFormatType;

    bool parseFile();
    bool parseHeader(const std::string &midiHeader);
    bool parseTrack(unsigned int &trackNum);

    Model *loadTrack(unsigned int trackNum,
		     Model *existingModel = 0,
		     int minProgress = 0,
		     int progressAmount = 100) const;

    bool consolidateNoteOffEvents(unsigned int track);
    void updateTempoMap(unsigned int track);
    void calculateTempoTimestamps();
    RealTime getTimeForMIDITime(unsigned long midiTime) const;

    // Internal convenience functions
    //
    int  midiBytesToInt(const std::string &bytes);
    long midiBytesToLong(const std::string &bytes);

    long getNumberFromMIDIBytes(int firstByte = -1);

    MIDIByte getMIDIByte();
    std::string getMIDIBytes(unsigned long bytes);

    bool skipToNextTrack();

    bool                   m_smpte;
    int                    m_timingDivision;   // pulses per quarter note
    int                    m_fps;              // if smpte
    int                    m_subframes;        // if smpte
    MIDIFileFormatType     m_format;
    unsigned int           m_numberOfTracks;

    long                   m_trackByteCount;
    bool                   m_decrementCount;

    std::map<int, QString> m_trackNames;
    std::set<unsigned int> m_loadableTracks;
    std::set<unsigned int> m_percussionTracks;
    MIDIComposition        m_midiComposition;
    TempoMap               m_tempoMap;

    QString                m_path;
    std::ifstream         *m_midiFile;
    size_t                 m_fileSize;
    QString                m_error;
    sv_samplerate_t        m_mainModelSampleRate;

    MIDIFileImportPreferenceAcquirer *m_acquirer;
};


#endif // _MIDI_FILE_READER_H_
