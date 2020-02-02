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


#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <algorithm>

#include "MIDIFileReader.h"

#include "data/midi/MIDIEvent.h"

#include "model/Model.h"
#include "base/Pitch.h"
#include "PraalineCore/Base/RealTime.h"
#include "model/NoteModel.h"

#include <QString>
#include <QFileInfo>

#include <sstream>

#include "base/Debug.h"

using std::string;
using std::ifstream;
using std::stringstream;
using std::ends;
using std::ios;
using std::vector;
using std::map;
using std::set;

using namespace MIDIConstants;

//#define MIDI_cerr 1


MIDIFileReader::MIDIFileReader(QString path,
                               MIDIFileImportPreferenceAcquirer *acquirer,
			       sv_samplerate_t mainModelSampleRate) :
    m_smpte(false),
    m_timingDivision(0),
    m_fps(0),
    m_subframes(0),
    m_format(MIDI_FILE_BAD_FORMAT),
    m_numberOfTracks(0),
    m_trackByteCount(0),
    m_decrementCount(false),
    m_path(path),
    m_midiFile(0),
    m_fileSize(0),
    m_mainModelSampleRate(mainModelSampleRate),
    m_acquirer(acquirer)
{
    if (parseFile()) {
	m_error = "";
    }
}

MIDIFileReader::~MIDIFileReader()
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
MIDIFileReader::isOK() const
{
    return (m_error == "");
}

QString
MIDIFileReader::getError() const
{
    return m_error;
}

long
MIDIFileReader::midiBytesToLong(const string& bytes)
{
    if (bytes.length() != 4) {
	throw MIDIException(tr("Wrong length for long data in MIDI stream (%1, should be %2)").arg(bytes.length()).arg(4));
    }

    long longRet = ((long)(((MIDIByte)bytes[0]) << 24)) |
                   ((long)(((MIDIByte)bytes[1]) << 16)) |
                   ((long)(((MIDIByte)bytes[2]) << 8)) |
                   ((long)((MIDIByte)(bytes[3])));

    return longRet;
}

int
MIDIFileReader::midiBytesToInt(const string& bytes)
{
    if (bytes.length() != 2) {
	throw MIDIException(tr("Wrong length for int data in MIDI stream (%1, should be %2)").arg(bytes.length()).arg(2));
    }

    int intRet = ((int)(((MIDIByte)bytes[0]) << 8)) |
                 ((int)(((MIDIByte)bytes[1])));
    return(intRet);
}


// Gets a single byte from the MIDI byte stream.  For each track
// section we can read only a specified number of bytes held in
// m_trackByteCount.
//
MIDIByte
MIDIFileReader::getMIDIByte()
{
    if (!m_midiFile) {
	throw MIDIException(tr("getMIDIByte called but no MIDI file open"));
    }

    if (m_midiFile->eof()) {
        throw MIDIException(tr("End of MIDI file encountered while reading"));
    }

    if (m_decrementCount && m_trackByteCount <= 0) {
        throw MIDIException(tr("Attempt to get more bytes than expected on Track"));
    }

    char byte;
    if (m_midiFile->read(&byte, 1)) {
	--m_trackByteCount;
	return (MIDIByte)byte;
    }

    throw MIDIException(tr("Attempt to read past MIDI file end"));
}


// Gets a specified number of bytes from the MIDI byte stream.  For
// each track section we can read only a specified number of bytes
// held in m_trackByteCount.
//
string
MIDIFileReader::getMIDIBytes(unsigned long numberOfBytes)
{
    if (!m_midiFile) {
	throw MIDIException(tr("getMIDIBytes called but no MIDI file open"));
    }

    if (m_midiFile->eof()) {
        throw MIDIException(tr("End of MIDI file encountered while reading"));
    }

    if (m_decrementCount && (numberOfBytes > (unsigned long)m_trackByteCount)) {
        throw MIDIException(tr("Attempt to get more bytes than available on Track (%1, only have %2)").arg(numberOfBytes).arg(m_trackByteCount));
    }

    string stringRet;
    char fileMIDIByte;

    while (stringRet.length() < numberOfBytes &&
           m_midiFile->read(&fileMIDIByte, 1)) {
        stringRet += fileMIDIByte;
    }

    // if we've reached the end of file without fulfilling the
    // quota then panic as our parsing has performed incorrectly
    //
    if (stringRet.length() < numberOfBytes) {
        stringRet = "";
        throw MIDIException(tr("Attempt to read past MIDI file end"));
    }

    // decrement the byte count
    if (m_decrementCount)
        m_trackByteCount -= stringRet.length();

    return stringRet;
}


// Get a long number of variable length from the MIDI byte stream.
//
long
MIDIFileReader::getNumberFromMIDIBytes(int firstByte)
{
    if (!m_midiFile) {
	throw MIDIException(tr("getNumberFromMIDIBytes called but no MIDI file open"));
    }

    long longRet = 0;
    MIDIByte midiByte;

    if (firstByte >= 0) {
	midiByte = (MIDIByte)firstByte;
    } else if (m_midiFile->eof()) {
	return longRet;
    } else {
	midiByte = getMIDIByte();
    }

    longRet = midiByte;
    if (midiByte & 0x80) {
	longRet &= 0x7F;
	do {
	    midiByte = getMIDIByte();
	    longRet = (longRet << 7) + (midiByte & 0x7F);
	} while (!m_midiFile->eof() && (midiByte & 0x80));
    }

    return longRet;
}


// Seek to the next track in the midi file and set the number
// of bytes to be read in the counter m_trackByteCount.
//
bool
MIDIFileReader::skipToNextTrack()
{
    if (!m_midiFile) {
	throw MIDIException(tr("skipToNextTrack called but no MIDI file open"));
    }

    string buffer, buffer2;
    m_trackByteCount = -1;
    m_decrementCount = false;

    while (!m_midiFile->eof() && (m_decrementCount == false)) {
        buffer = getMIDIBytes(4); 
	if (buffer.compare(0, 4, MIDI_TRACK_HEADER) == 0) {
	    m_trackByteCount = midiBytesToLong(getMIDIBytes(4));
	    m_decrementCount = true;
	}
    }

    if (m_trackByteCount == -1) { // we haven't found a track
        return false;
    } else {
        return true;
    }
}


// Read in a MIDI file.  The parsing process throws exceptions back up
// here if we run into trouble which we can then pass back out to
// whoever called us using a nice bool.
//
bool
MIDIFileReader::parseFile()
{
    m_error = "";

#ifdef MIDI_DEBUG
    cerr << "MIDIFileReader::open() : fileName = " << m_fileName.c_str() << endl;
#endif

    // Open the file
    m_midiFile = new ifstream(m_path.toLocal8Bit().data(),
			      ios::in | ios::binary);

    if (!*m_midiFile) {
	m_error = "File not found or not readable.";
	m_format = MIDI_FILE_BAD_FORMAT;
	delete m_midiFile;
        m_midiFile = 0;
	return false;
    }

    bool retval = false;

    try {

	// Set file size so we can count it off
	//
	m_midiFile->seekg(0, ios::end);
        std::streamoff off = m_midiFile->tellg();
	m_fileSize = 0;
        if (off > 0) m_fileSize = off;
	m_midiFile->seekg(0, ios::beg);

	// Parse the MIDI header first.  The first 14 bytes of the file.
	if (!parseHeader(getMIDIBytes(14))) {
	    m_format = MIDI_FILE_BAD_FORMAT;
	    m_error = "Not a MIDI file.";
	    goto done;
	}

	unsigned int i = 0;

	for (unsigned int j = 0; j < m_numberOfTracks; ++j) {

#ifdef MIDI_DEBUG
	    cerr << "Parsing Track " << j << endl;
#endif

	    if (!skipToNextTrack()) {
#ifdef MIDI_DEBUG
		cerr << "Couldn't find Track " << j << endl;
#endif
		m_error = "File corrupted or in non-standard format?";
		m_format = MIDI_FILE_BAD_FORMAT;
		goto done;
	    }

#ifdef MIDI_DEBUG
	    cerr << "Track has " << m_trackByteCount << " bytes" << endl;
#endif

	    // Run through the events taking them into our internal
	    // representation.
	    if (!parseTrack(i)) {
#ifdef MIDI_DEBUG
		cerr << "Track " << j << " parsing failed" << endl;
#endif
		m_error = "File corrupted or in non-standard format?";
		m_format = MIDI_FILE_BAD_FORMAT;
		goto done;
	    }

	    ++i; // j is the source track number, i the destination
	}
	
	m_numberOfTracks = i;
	retval = true;

    } catch (MIDIException e) {

        cerr << "MIDIFileReader::open() - caught exception - " << e.what() << endl;
	m_error = e.what();
    }
    
done:
    m_midiFile->close();
    delete m_midiFile;

    for (unsigned int track = 0; track < m_numberOfTracks; ++track) {

        // Convert the deltaTime to an absolute time since the track
        // start.  The addTime method returns the sum of the current
        // MIDI Event delta time plus the argument.

	unsigned long acc = 0;

        for (MIDITrack::iterator i = m_midiComposition[track].begin();
             i != m_midiComposition[track].end(); ++i) {
            acc = (*i)->addTime(acc);
        }

        if (consolidateNoteOffEvents(track)) { // returns true if some notes exist
	    m_loadableTracks.insert(track);
	}
    }

    for (unsigned int track = 0; track < m_numberOfTracks; ++track) {
        updateTempoMap(track);
    }

    calculateTempoTimestamps();

    return retval;
}

// Parse and ensure the MIDI Header is legitimate
//
bool
MIDIFileReader::parseHeader(const string &midiHeader)
{
    if (midiHeader.size() < 14) {
#ifdef MIDI_DEBUG
        cerr << "MIDIFileReader::parseHeader() - file header undersized" << endl;
#endif
        return false;
    }

    if (midiHeader.compare(0, 4, MIDI_FILE_HEADER) != 0) {
#ifdef MIDI_DEBUG
	cerr << "MIDIFileReader::parseHeader()"
	     << "- file header not found or malformed"
	     << endl;
#endif
	return false;
    }

    if (midiBytesToLong(midiHeader.substr(4,4)) != 6L) {
#ifdef MIDI_DEBUG
        cerr << "MIDIFileReader::parseHeader()"
	     << " - header length incorrect"
	     << endl;
#endif
        return false;
    }

    m_format = (MIDIFileFormatType) midiBytesToInt(midiHeader.substr(8,2));
    m_numberOfTracks = midiBytesToInt(midiHeader.substr(10,2));
    m_timingDivision = midiBytesToInt(midiHeader.substr(12,2));

    if (m_timingDivision >= 32768) {
        m_smpte = true;
        m_fps = 256 - (m_timingDivision >> 8);
        m_subframes = (m_timingDivision & 0xff);
    } else {
        m_smpte = false;
    }

    return true; 
}

// Extract the contents from a MIDI file track and places it into
// our local map of MIDI events.
//
bool
MIDIFileReader::parseTrack(unsigned int &lastTrackNum)
{
    MIDIByte midiByte, metaEventCode, data1, data2;
    MIDIByte eventCode = 0x80;
    string metaMessage;
    long messageLength;
    long deltaTime;
    long accumulatedTime = 0;

    // The trackNum passed in to this method is the default track for
    // all events provided they're all on the same channel.  If we find
    // events on more than one channel, we increment trackNum and record
    // the mapping from channel to trackNum in this channelTrackMap.
    // We then return the new trackNum by reference so the calling
    // method knows we've got more tracks than expected.

    // This would be a vector<unsigned int> but we need -1 to indicate
    // "not yet used"
    vector<int> channelTrackMap(16, -1);

    // This is used to store the last absolute time found on each track,
    // allowing us to modify delta-times correctly when separating events
    // out from one to multiple tracks
    //
    map<int, unsigned long> trackTimeMap;

    // Meta-events don't have a channel, so we place them in a fixed
    // track number instead
    unsigned int metaTrack = lastTrackNum;

    // Remember the last non-meta status byte (-1 if we haven't seen one)
    int runningStatus = -1;

    bool firstTrack = true;

    while (!m_midiFile->eof() && (m_trackByteCount > 0)) {

	if (eventCode < 0x80) {
#ifdef MIDI_DEBUG
	    cerr << "WARNING: Invalid event code " << eventCode
		 << " in MIDI file" << endl;
#endif
	    throw MIDIException(tr("Invalid event code %1 found").arg(int(eventCode)));
	}

        deltaTime = getNumberFromMIDIBytes();

#ifdef MIDI_DEBUG
	cerr << "read delta time " << deltaTime << endl;
#endif

        // Get a single byte
        midiByte = getMIDIByte();

        if (!(midiByte & MIDI_STATUS_BYTE_MASK)) {

	    if (runningStatus < 0) {
		throw MIDIException(tr("Running status used for first event in track"));
	    }

	    eventCode = (MIDIByte)runningStatus;
	    data1 = midiByte;

#ifdef MIDI_DEBUG
	    cerr << "using running status (byte " << int(midiByte) << " found)" << endl;
#endif
        } else {
#ifdef MIDI_DEBUG
	    cerr << "have new event code " << int(midiByte) << endl;
#endif
            eventCode = midiByte;
	    data1 = getMIDIByte();
	}

        if (eventCode == MIDI_FILE_META_EVENT) {

	    metaEventCode = data1;
            messageLength = getNumberFromMIDIBytes();

//#ifdef MIDI_DEBUG
		cerr << "Meta event of type " << int(metaEventCode) << " and " << messageLength << " bytes found, putting on track " << metaTrack << endl;
//#endif
            metaMessage = getMIDIBytes(messageLength);

	    long gap = accumulatedTime - trackTimeMap[metaTrack];
	    accumulatedTime += deltaTime;
	    deltaTime += gap;
	    trackTimeMap[metaTrack] = accumulatedTime;

            MIDIEvent *e = new MIDIEvent(deltaTime,
                                         MIDI_FILE_META_EVENT,
                                         metaEventCode,
                                         metaMessage);

	    m_midiComposition[metaTrack].push_back(e);

	    if (metaEventCode == MIDI_TRACK_NAME) {
		m_trackNames[metaTrack] = metaMessage.c_str();
	    }

        } else { // non-meta events

	    runningStatus = eventCode;

            MIDIEvent *midiEvent;

	    int channel = (eventCode & MIDI_CHANNEL_NUM_MASK);
	    if (channelTrackMap[channel] == -1) {
		if (!firstTrack) ++lastTrackNum;
		else firstTrack = false;
		channelTrackMap[channel] = lastTrackNum;
	    }

	    unsigned int trackNum = channelTrackMap[channel];
	    
	    // accumulatedTime is abs time of last event on any track;
	    // trackTimeMap[trackNum] is that of last event on this track
	    
	    long gap = accumulatedTime - trackTimeMap[trackNum];
	    accumulatedTime += deltaTime;
	    deltaTime += gap;
	    trackTimeMap[trackNum] = accumulatedTime;

            switch (eventCode & MIDI_MESSAGE_TYPE_MASK) {

            case MIDI_NOTE_ON:
            case MIDI_NOTE_OFF:
            case MIDI_POLY_AFTERTOUCH:
            case MIDI_CTRL_CHANGE:
                data2 = getMIDIByte();

                // create and store our event
                midiEvent = new MIDIEvent(deltaTime, eventCode, data1, data2);

                /*
		cerr << "MIDI event for channel " << channel << " (track "
			  << trackNum << ")" << endl;
		midiEvent->print();
                          */


                m_midiComposition[trackNum].push_back(midiEvent);

		if (midiEvent->getChannelNumber() == MIDI_PERCUSSION_CHANNEL) {
		    m_percussionTracks.insert(trackNum);
		}

                break;

            case MIDI_PITCH_BEND:
                data2 = getMIDIByte();

                // create and store our event
                midiEvent = new MIDIEvent(deltaTime, eventCode, data1, data2);
                m_midiComposition[trackNum].push_back(midiEvent);
                break;

            case MIDI_PROG_CHANGE:
            case MIDI_CHNL_AFTERTOUCH:
                // create and store our event
                midiEvent = new MIDIEvent(deltaTime, eventCode, data1);
                m_midiComposition[trackNum].push_back(midiEvent);
                break;

            case MIDI_SYSTEM_EXCLUSIVE:
                messageLength = getNumberFromMIDIBytes(data1);

#ifdef MIDI_DEBUG
		cerr << "SysEx of " << messageLength << " bytes found" << endl;
#endif

                metaMessage= getMIDIBytes(messageLength);

                if (MIDIByte(metaMessage[metaMessage.length() - 1]) !=
                        MIDI_END_OF_EXCLUSIVE)
                {
#ifdef MIDI_DEBUG
                    cerr << "MIDIFileReader::parseTrack() - "
                              << "malformed or unsupported SysEx type"
                              << endl;
#endif
                    continue;
                }

                // chop off the EOX 
                // length fixed by Pedro Lopez-Cabanillas (20030523)
                //
                metaMessage = metaMessage.substr(0, metaMessage.length()-1);

                midiEvent = new MIDIEvent(deltaTime,
                                          MIDI_SYSTEM_EXCLUSIVE,
                                          metaMessage);
                m_midiComposition[trackNum].push_back(midiEvent);
                break;

            default:
#ifdef MIDI_DEBUG
                cerr << "MIDIFileReader::parseTrack()" 
                          << " - Unsupported MIDI Event Code:  "
                          << (int)eventCode << endl;
#endif
                break;
            } 
        }
    }

    if (lastTrackNum > metaTrack) {
	for (unsigned int track = metaTrack + 1; track <= lastTrackNum; ++track) {
	    m_trackNames[track] = QString("%1 <%2>")
		.arg(m_trackNames[metaTrack]).arg(track - metaTrack + 1);
	}
    }

    return true;
}

// Delete dead NOTE OFF and NOTE ON/Zero Velocity Events after
// reading them and modifying their relevant NOTE ONs.  Return true
// if there are some notes in this track.
//
bool
MIDIFileReader::consolidateNoteOffEvents(unsigned int track)
{
    bool notesOnTrack = false;
    bool noteOffFound;

    for (MIDITrack::iterator i = m_midiComposition[track].begin();
	 i != m_midiComposition[track].end(); i++) {

        if ((*i)->getMessageType() == MIDI_NOTE_ON && (*i)->getVelocity() > 0) {

	    notesOnTrack = true;
            noteOffFound = false;

            for (MIDITrack::iterator j = i;
		 j != m_midiComposition[track].end(); j++) {

                if (((*j)->getChannelNumber() == (*i)->getChannelNumber()) &&
		    ((*j)->getPitch() == (*i)->getPitch()) &&
                    ((*j)->getMessageType() == MIDI_NOTE_OFF ||
                    ((*j)->getMessageType() == MIDI_NOTE_ON &&
                     (*j)->getVelocity() == 0x00))) {

                    (*i)->setDuration((*j)->getTime() - (*i)->getTime());

                    delete *j;
                    m_midiComposition[track].erase(j);

                    noteOffFound = true;
                    break;
                }
            }

            // If no matching NOTE OFF has been found then set
            // Event duration to length of track
            //
            if (!noteOffFound) {
		MIDITrack::iterator j = m_midiComposition[track].end();
		--j;
                (*i)->setDuration((*j)->getTime() - (*i)->getTime());
	    }
        }
    }

    return notesOnTrack;
}

// Add any tempo events found in the given track to the global tempo map.
//
void
MIDIFileReader::updateTempoMap(unsigned int track)
{
    cerr << "updateTempoMap for track " << track << " (" << m_midiComposition[track].size() << " events)" << endl;

    for (MIDITrack::iterator i = m_midiComposition[track].begin();
	 i != m_midiComposition[track].end(); ++i) {

        if ((*i)->isMeta() &&
	    (*i)->getMetaEventCode() == MIDI_SET_TEMPO) {

	    MIDIByte m0 = (*i)->getMetaMessage()[0];
	    MIDIByte m1 = (*i)->getMetaMessage()[1];
	    MIDIByte m2 = (*i)->getMetaMessage()[2];
	    
	    long tempo = (((m0 << 8) + m1) << 8) + m2;

	    cerr << "updateTempoMap: have tempo, it's " << tempo << " at " << (*i)->getTime() << endl;

	    if (tempo != 0) {
		double qpm = 60000000.0 / double(tempo);
		m_tempoMap[(*i)->getTime()] =
		    TempoChange(RealTime::zeroTime, qpm);
	    }
        }
    }
}

void
MIDIFileReader::calculateTempoTimestamps()
{
    unsigned long lastMIDITime = 0;
    RealTime lastRealTime = RealTime::zeroTime;
    double tempo = 120.0;
    int td = m_timingDivision;
    if (td == 0) td = 96;

    for (TempoMap::iterator i = m_tempoMap.begin(); i != m_tempoMap.end(); ++i) {
	
	unsigned long mtime = i->first;
	unsigned long melapsed = mtime - lastMIDITime;
	double quarters = double(melapsed) / double(td);
	double seconds = (60.0 * quarters) / tempo;

	RealTime t = lastRealTime + RealTime::fromSeconds(seconds);

	i->second.first = t;

	lastRealTime = t;
	lastMIDITime = mtime;
	tempo = i->second.second;
    }
}

RealTime
MIDIFileReader::getTimeForMIDITime(unsigned long midiTime) const
{
    unsigned long tempoMIDITime = 0;
    RealTime tempoRealTime = RealTime::zeroTime;
    double tempo = 120.0;

    TempoMap::const_iterator i = m_tempoMap.lower_bound(midiTime);
    if (i != m_tempoMap.begin()) {
	--i;
	tempoMIDITime = i->first;
	tempoRealTime = i->second.first;
	tempo = i->second.second;
    }

    int td = m_timingDivision;
    if (td == 0) td = 96;

    unsigned long melapsed = midiTime - tempoMIDITime;
    double quarters = double(melapsed) / double(td);
    double seconds = (60.0 * quarters) / tempo;

/*
    cerr << "MIDIFileReader::getTimeForMIDITime(" << midiTime << ")"
	      << endl;
    cerr << "timing division = " << td << endl;
    cerr << "nearest tempo event (of " << m_tempoMap.size() << ") is at " << tempoMIDITime << " ("
	      << tempoRealTime << ")" << endl;
    cerr << "quarters since then = " << quarters << endl;
    cerr << "tempo = " << tempo << " quarters per minute" << endl;
    cerr << "seconds since then = " << seconds << endl;
    cerr << "resulting time = " << (tempoRealTime + RealTime::fromSeconds(seconds)) << endl;
*/

    return tempoRealTime + RealTime::fromSeconds(seconds);
}

Model *
MIDIFileReader::load() const
{
    if (!isOK()) return 0;

    if (m_loadableTracks.empty()) {
        if (m_acquirer) {
            m_acquirer->showError
                (tr("MIDI file \"%1\" has no notes in any track").arg(m_path));
        }
	return 0;
    }

    std::set<unsigned int> tracksToLoad;

    if (m_loadableTracks.size() == 1) {

	tracksToLoad.insert(*m_loadableTracks.begin());

    } else {

        QStringList displayNames;

	for (set<unsigned int>::iterator i = m_loadableTracks.begin();
	     i != m_loadableTracks.end(); ++i) {

	    unsigned int trackNo = *i;
	    QString label;

	    QString perc;
	    if (m_percussionTracks.find(trackNo) != m_percussionTracks.end()) {
		perc = tr(" - uses GM percussion channel");
	    }

	    if (m_trackNames.find(trackNo) != m_trackNames.end()) {
		label = tr("Track %1 (%2)%3")
		    .arg(trackNo).arg(m_trackNames.find(trackNo)->second)
		    .arg(perc);
	    } else {
		label = tr("Track %1 (untitled)%3").arg(trackNo).arg(perc);
	    }

            displayNames << label;
	}

        QString singleTrack;

        bool haveSomePercussion = 
            (!m_percussionTracks.empty() &&
             (m_percussionTracks.size() < m_loadableTracks.size()));

        MIDIFileImportPreferenceAcquirer::TrackPreference pref;

        if (m_acquirer) {
            pref = m_acquirer->getTrackImportPreference(displayNames,
                                                        haveSomePercussion,
                                                        singleTrack);
        } else {
            pref = MIDIFileImportPreferenceAcquirer::MergeAllTracks;
        }

        if (pref == MIDIFileImportPreferenceAcquirer::ImportNothing) return 0;

        if (pref == MIDIFileImportPreferenceAcquirer::MergeAllTracks ||
            pref == MIDIFileImportPreferenceAcquirer::MergeAllNonPercussionTracks) {
            
            for (set<unsigned int>::iterator i = m_loadableTracks.begin();
                 i != m_loadableTracks.end(); ++i) {
                
		if (pref == MIDIFileImportPreferenceAcquirer::MergeAllTracks ||
		    m_percussionTracks.find(*i) == m_percussionTracks.end()) {
                    
		    tracksToLoad.insert(*i);
		}
	    }

	} else {
	    
	    int j = 0;

	    for (set<unsigned int>::iterator i = m_loadableTracks.begin();
		 i != m_loadableTracks.end(); ++i) {
		
		if (singleTrack == displayNames[j]) {
		    tracksToLoad.insert(*i);
		    break;
		}
		
		++j;
	    }
	}
    }

    if (tracksToLoad.empty()) return 0;

    int n = int(tracksToLoad.size()), count = 0;
    Model *model = 0;

    for (std::set<unsigned int>::iterator i = tracksToLoad.begin();
	 i != tracksToLoad.end(); ++i) {

	int minProgress = (100 * count) / n;
	int progressAmount = 100 / n;

	model = loadTrack(*i, model, minProgress, progressAmount);

	++count;
    }

    if (dynamic_cast<NoteModel *>(model)) {
	dynamic_cast<NoteModel *>(model)->setCompletion(100);
    }

    return model;
}

Model *
MIDIFileReader::loadTrack(unsigned int trackToLoad,
			  Model *existingModel,
			  int minProgress,
			  int progressAmount) const
{
    if (m_midiComposition.find(trackToLoad) == m_midiComposition.end()) {
	return 0;
    }

    NoteModel *model = 0;

    if (existingModel) {
	model = dynamic_cast<NoteModel *>(existingModel);
	if (!model) {
	    cerr << "WARNING: MIDIFileReader::loadTrack: Existing model given, but it isn't a NoteModel -- ignoring it" << endl;
	}
    }

    if (!model) {
	model = new NoteModel(m_mainModelSampleRate, 1, 0.0, 0.0, false);
	model->setValueQuantization(1.0);
        model->setObjectName(QFileInfo(m_path).fileName());
    }

    const MIDITrack &track = m_midiComposition.find(trackToLoad)->second;

    int totalEvents = int(track.size());
    int count = 0;

    bool sharpKey = true;

    for (MIDITrack::const_iterator i = track.begin(); i != track.end(); ++i) {

        RealTime rt;
        unsigned long midiTime = (*i)->getTime();

        if (m_smpte) {
            rt = RealTime::frame2RealTime(midiTime, m_fps * m_subframes);
        } else {
            rt = getTimeForMIDITime(midiTime);
        }

	// We ignore most of these event types for now, though in
	// theory some of the text ones could usefully be incorporated

	if ((*i)->isMeta()) {

	    switch((*i)->getMetaEventCode()) {

	    case MIDI_KEY_SIGNATURE:
		// minorKey = (int((*i)->getMetaMessage()[1]) != 0);
		sharpKey = (int((*i)->getMetaMessage()[0]) >= 0);
		break;

	    case MIDI_TEXT_EVENT:
	    case MIDI_LYRIC:
	    case MIDI_TEXT_MARKER:
	    case MIDI_COPYRIGHT_NOTICE:
	    case MIDI_TRACK_NAME:
		// The text events that we could potentially use
		break;

	    case MIDI_SET_TEMPO:
		// Already dealt with in a separate pass previously
		break;

	    case MIDI_TIME_SIGNATURE:
		// Not yet!
		break;

	    case MIDI_SEQUENCE_NUMBER:
	    case MIDI_CHANNEL_PREFIX_OR_PORT:
	    case MIDI_INSTRUMENT_NAME:
	    case MIDI_CUE_POINT:
	    case MIDI_CHANNEL_PREFIX:
	    case MIDI_SEQUENCER_SPECIFIC:
	    case MIDI_SMPTE_OFFSET:
	    default:
		break;
	    }

	} else {

	    switch ((*i)->getMessageType()) {

	    case MIDI_NOTE_ON:

                if ((*i)->getVelocity() == 0) break; // effective note-off
		else {
		    RealTime endRT;
                    unsigned long endMidiTime = (*i)->getTime() + (*i)->getDuration();
                    if (m_smpte) {
                        endRT = RealTime::frame2RealTime(endMidiTime, m_fps * m_subframes);
                    } else {
                        endRT = getTimeForMIDITime(endMidiTime);
                    }

		    long startFrame = RealTime::realTime2Frame
			(rt, model->getSampleRate());

		    long endFrame = RealTime::realTime2Frame
			(endRT, model->getSampleRate());

		    QString pitchLabel = Pitch::getPitchLabel((*i)->getPitch(),
							      0, 
							      !sharpKey);

		    QString noteLabel = tr("%1 - vel %2")
			.arg(pitchLabel).arg(int((*i)->getVelocity()));

                    float level = float((*i)->getVelocity()) / 128.f;

		    Note note(startFrame, (*i)->getPitch(),
			      endFrame - startFrame, level, noteLabel);

//		    cerr << "Adding note " << startFrame << "," << (endFrame-startFrame) << " : " << int((*i)->getPitch()) << endl;

		    model->addPoint(note);
		    break;
		}

            case MIDI_PITCH_BEND:
		// I guess we could make some use of this...
                break;

            case MIDI_NOTE_OFF:
            case MIDI_PROG_CHANGE:
            case MIDI_CTRL_CHANGE:
            case MIDI_SYSTEM_EXCLUSIVE:
            case MIDI_POLY_AFTERTOUCH:
            case MIDI_CHNL_AFTERTOUCH:
                break;

            default:
                break;
            }
	}

	model->setCompletion(minProgress +
			     (count * progressAmount) / totalEvents);
	++count;
    }

    return model;
}


