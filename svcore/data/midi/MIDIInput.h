/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2009 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _MIDI_INPUT_H_
#define _MIDI_INPUT_H_

#include <QObject>
#include "MIDIEvent.h"

#include <vector>
#include "base/RingBuffer.h"
#include "base/FrameTimer.h"

class RtMidiIn;

class MIDIInput : public QObject
{
    Q_OBJECT

public:
    MIDIInput(QString name, FrameTimer *timer);
    virtual ~MIDIInput();

    bool isOK() const { return m_rtmidi != 0; }

    bool isEmpty() const { return getEventsAvailable() == 0; }
    int getEventsAvailable() const { return m_buffer.getReadSpace(); }
    MIDIEvent readEvent();

signals:
    void eventsAvailable();

protected:
    RtMidiIn *m_rtmidi;
    FrameTimer *m_frameTimer;

    static void staticCallback(double, std::vector<unsigned char> *, void *);
    void callback(double, std::vector<unsigned char> *);

    void postEvent(MIDIEvent);
    RingBuffer<MIDIEvent *> m_buffer;
};

#endif

