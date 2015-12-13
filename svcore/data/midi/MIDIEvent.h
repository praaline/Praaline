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

#ifndef _MIDI_EVENT_H_
#define _MIDI_EVENT_H_

#include <QString>
#include <string>
#include <iostream>
#include <stdexcept>

#include "base/Debug.h"

typedef unsigned char MIDIByte;

namespace MIDIConstants
{
    static const char *const MIDI_FILE_HEADER         = "MThd";
    static const char *const MIDI_TRACK_HEADER        = "MTrk";

    static const MIDIByte MIDI_STATUS_BYTE_MASK       = 0x80;
    static const MIDIByte MIDI_MESSAGE_TYPE_MASK      = 0xF0;
    static const MIDIByte MIDI_CHANNEL_NUM_MASK       = 0x0F;

    static const MIDIByte MIDI_NOTE_OFF               = 0x80;
    static const MIDIByte MIDI_NOTE_ON                = 0x90;
    static const MIDIByte MIDI_POLY_AFTERTOUCH        = 0xA0;
    static const MIDIByte MIDI_CTRL_CHANGE            = 0xB0;
    static const MIDIByte MIDI_PROG_CHANGE            = 0xC0;
    static const MIDIByte MIDI_CHNL_AFTERTOUCH        = 0xD0;
    static const MIDIByte MIDI_PITCH_BEND             = 0xE0;
    static const MIDIByte MIDI_SELECT_CHNL_MODE       = 0xB0;
    static const MIDIByte MIDI_SYSTEM_EXCLUSIVE       = 0xF0;
    static const MIDIByte MIDI_TC_QUARTER_FRAME       = 0xF1;
    static const MIDIByte MIDI_SONG_POSITION_PTR      = 0xF2;
    static const MIDIByte MIDI_SONG_SELECT            = 0xF3;
    static const MIDIByte MIDI_TUNE_REQUEST           = 0xF6;
    static const MIDIByte MIDI_END_OF_EXCLUSIVE       = 0xF7;
    static const MIDIByte MIDI_TIMING_CLOCK           = 0xF8;
    static const MIDIByte MIDI_START                  = 0xFA;
    static const MIDIByte MIDI_CONTINUE               = 0xFB;
    static const MIDIByte MIDI_STOP                   = 0xFC;
    static const MIDIByte MIDI_ACTIVE_SENSING         = 0xFE;
    static const MIDIByte MIDI_SYSTEM_RESET           = 0xFF;
    static const MIDIByte MIDI_SYSEX_NONCOMMERCIAL    = 0x7D;
    static const MIDIByte MIDI_SYSEX_NON_RT           = 0x7E;
    static const MIDIByte MIDI_SYSEX_RT               = 0x7F;
    static const MIDIByte MIDI_SYSEX_RT_COMMAND       = 0x06;
    static const MIDIByte MIDI_SYSEX_RT_RESPONSE      = 0x07;
    static const MIDIByte MIDI_MMC_STOP               = 0x01;
    static const MIDIByte MIDI_MMC_PLAY               = 0x02;
    static const MIDIByte MIDI_MMC_DEFERRED_PLAY      = 0x03;
    static const MIDIByte MIDI_MMC_FAST_FORWARD       = 0x04;
    static const MIDIByte MIDI_MMC_REWIND             = 0x05;
    static const MIDIByte MIDI_MMC_RECORD_STROBE      = 0x06;
    static const MIDIByte MIDI_MMC_RECORD_EXIT        = 0x07;
    static const MIDIByte MIDI_MMC_RECORD_PAUSE       = 0x08;
    static const MIDIByte MIDI_MMC_PAUSE              = 0x08;
    static const MIDIByte MIDI_MMC_EJECT              = 0x0A;
    static const MIDIByte MIDI_MMC_LOCATE             = 0x44;
    static const MIDIByte MIDI_FILE_META_EVENT        = 0xFF;
    static const MIDIByte MIDI_SEQUENCE_NUMBER        = 0x00;
    static const MIDIByte MIDI_TEXT_EVENT             = 0x01;
    static const MIDIByte MIDI_COPYRIGHT_NOTICE       = 0x02;
    static const MIDIByte MIDI_TRACK_NAME             = 0x03;
    static const MIDIByte MIDI_INSTRUMENT_NAME        = 0x04;
    static const MIDIByte MIDI_LYRIC                  = 0x05;
    static const MIDIByte MIDI_TEXT_MARKER            = 0x06;
    static const MIDIByte MIDI_CUE_POINT              = 0x07;
    static const MIDIByte MIDI_CHANNEL_PREFIX         = 0x20;
    static const MIDIByte MIDI_CHANNEL_PREFIX_OR_PORT = 0x21;
    static const MIDIByte MIDI_END_OF_TRACK           = 0x2F;
    static const MIDIByte MIDI_SET_TEMPO              = 0x51;
    static const MIDIByte MIDI_SMPTE_OFFSET           = 0x54;
    static const MIDIByte MIDI_TIME_SIGNATURE         = 0x58;
    static const MIDIByte MIDI_KEY_SIGNATURE          = 0x59;
    static const MIDIByte MIDI_SEQUENCER_SPECIFIC     = 0x7F;
    static const MIDIByte MIDI_CONTROLLER_BANK_MSB      = 0x00;
    static const MIDIByte MIDI_CONTROLLER_VOLUME        = 0x07;
    static const MIDIByte MIDI_CONTROLLER_BANK_LSB      = 0x20;
    static const MIDIByte MIDI_CONTROLLER_MODULATION    = 0x01;
    static const MIDIByte MIDI_CONTROLLER_PAN           = 0x0A;
    static const MIDIByte MIDI_CONTROLLER_SUSTAIN       = 0x40;
    static const MIDIByte MIDI_CONTROLLER_RESONANCE     = 0x47;
    static const MIDIByte MIDI_CONTROLLER_RELEASE       = 0x48;
    static const MIDIByte MIDI_CONTROLLER_ATTACK        = 0x49;
    static const MIDIByte MIDI_CONTROLLER_FILTER        = 0x4A;
    static const MIDIByte MIDI_CONTROLLER_REVERB        = 0x5B;
    static const MIDIByte MIDI_CONTROLLER_CHORUS        = 0x5D;
    static const MIDIByte MIDI_CONTROLLER_NRPN_1        = 0x62;
    static const MIDIByte MIDI_CONTROLLER_NRPN_2        = 0x63;
    static const MIDIByte MIDI_CONTROLLER_RPN_1         = 0x64;
    static const MIDIByte MIDI_CONTROLLER_RPN_2         = 0x65;
    static const MIDIByte MIDI_CONTROLLER_SOUNDS_OFF    = 0x78;
    static const MIDIByte MIDI_CONTROLLER_RESET         = 0x79;
    static const MIDIByte MIDI_CONTROLLER_LOCAL         = 0x7A;
    static const MIDIByte MIDI_CONTROLLER_ALL_NOTES_OFF = 0x7B;
    static const MIDIByte MIDI_PERCUSSION_CHANNEL       = 9;
}

class MIDIEvent
{
public:
    MIDIEvent(unsigned long deltaTime,
              int eventCode,
              int data1 = 0,
              int data2 = 0) :
	m_deltaTime(deltaTime),
	m_duration(0),
	m_metaEventCode(0)
    {
        if (eventCode < 0 || eventCode > 0xff ||
            data1 < 0 || data1 > 0xff ||
            data2 < 0 || data2 > 0xff) {
            throw std::domain_error("not all args within byte range");
        }
        m_eventCode = MIDIByte(eventCode);
        m_data1 = MIDIByte(data1);
        m_data2 = MIDIByte(data2);
    }

    MIDIEvent(unsigned long deltaTime,
              MIDIByte eventCode,
              MIDIByte metaEventCode,
              const std::string &metaMessage) :
	m_deltaTime(deltaTime),
	m_duration(0),
	m_eventCode(eventCode),
	m_data1(0),
	m_data2(0),
	m_metaEventCode(metaEventCode),
	m_metaMessage(metaMessage)
    { }

    MIDIEvent(unsigned long deltaTime,
              MIDIByte eventCode,
              const std::string &sysEx) :
	m_deltaTime(deltaTime),
	m_duration(0),
	m_eventCode(eventCode),
	m_data1(0),
	m_data2(0),
	m_metaEventCode(0),
	m_metaMessage(sysEx)
    { }

    ~MIDIEvent() { }

    void setTime(const unsigned long &time) { m_deltaTime = time; }
    void setDuration(const unsigned long& duration) { m_duration = duration;}
    unsigned long addTime(const unsigned long &time) {
	m_deltaTime += time;
	return m_deltaTime;
    }

    MIDIByte getMessageType() const
        { return (m_eventCode & MIDIConstants::MIDI_MESSAGE_TYPE_MASK); }

    MIDIByte getChannelNumber() const
        { return (m_eventCode & MIDIConstants::MIDI_CHANNEL_NUM_MASK); }

    unsigned long getTime() const { return m_deltaTime; }
    unsigned long getDuration() const { return m_duration; }

    MIDIByte getPitch() const { return m_data1; }
    MIDIByte getVelocity() const { return m_data2; }
    MIDIByte getData1() const { return m_data1; }
    MIDIByte getData2() const { return m_data2; }
    MIDIByte getEventCode() const { return m_eventCode; }

    bool isMeta() const { return (m_eventCode == MIDIConstants::MIDI_FILE_META_EVENT); }

    MIDIByte getMetaEventCode() const { return m_metaEventCode; }
    std::string getMetaMessage() const { return m_metaMessage; }
    void setMetaMessage(const std::string &meta) { m_metaMessage = meta; }

    friend bool operator<(const MIDIEvent &a, const MIDIEvent &b);

private:
    MIDIEvent& operator=(const MIDIEvent);

    unsigned long  m_deltaTime;
    unsigned long  m_duration;
    MIDIByte       m_eventCode;
    MIDIByte       m_data1;         // or Note
    MIDIByte       m_data2;         // or Velocity
    MIDIByte       m_metaEventCode;
    std::string    m_metaMessage;
};

// Comparator for sorting
//
struct MIDIEventCmp
{
    bool operator()(const MIDIEvent &mE1, const MIDIEvent &mE2) const
    { return mE1.getTime() < mE2.getTime(); }

    bool operator()(const MIDIEvent *mE1, const MIDIEvent *mE2) const
    { return mE1->getTime() < mE2->getTime(); }
};

class MIDIException : virtual public std::exception
{
public:
    MIDIException(QString message) throw() : m_message(message) {
        std::cerr << "WARNING: MIDI exception: "
		  << message.toLocal8Bit().data() << std::endl;
    }
    virtual ~MIDIException() throw() { }

    virtual const char *what() const throw() {
	return m_message.toLocal8Bit().data();
    }

protected:
    QString m_message;
};

#endif
