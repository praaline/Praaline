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

#include "PlayParameters.h"

#include <iostream>

#include <QTextStream>

void
PlayParameters::copyFrom(const PlayParameters *pp)
{
    bool changed = false;

    if (m_playMuted != pp->isPlayMuted()) {
        m_playMuted = pp->isPlayMuted();
        emit playMutedChanged(m_playMuted);
        emit playAudibleChanged(!m_playMuted);
        changed = true;
    }

    if (m_playPan != pp->getPlayPan()) {
        m_playPan = pp->getPlayPan();
        emit playPanChanged(m_playPan);
        changed = true;
    }

    if (m_playGain != pp->getPlayGain()) {
        m_playGain = pp->getPlayGain();
        emit playGainChanged(m_playGain);
        changed = true;
    }

    if (m_playClipId != pp->getPlayClipId()) {
        m_playClipId = pp->getPlayClipId();
        emit playClipIdChanged(m_playClipId);
        changed = true;
    }

    if (changed) emit playParametersChanged();
}

void
PlayParameters::toXml(QTextStream &stream,
                      QString indent,
                      QString extraAttributes) const
{
    stream << indent;
    stream << QString("<playparameters mute=\"%1\" pan=\"%2\" gain=\"%3\" clipId=\"%4\" %6")
        .arg(m_playMuted ? "true" : "false")
        .arg(m_playPan)
        .arg(m_playGain)
        .arg(m_playClipId)
        .arg(extraAttributes);

    stream << ">\n";

    if (m_playClipId != "") {
        // for backward compatibility
        stream << indent << "  ";
        stream << QString("<plugin identifier=\"%1\" program=\"%2\"/>\n")
            .arg("sample_player")
            .arg(m_playClipId);
    }

    stream << indent << "</playparameters>\n";
}

void
PlayParameters::setPlayMuted(bool muted)
{
//    cerr << "PlayParameters: setPlayMuted(" << muted << ")" << endl;
    if (m_playMuted != muted) {
        m_playMuted = muted;
        emit playMutedChanged(muted);
        emit playAudibleChanged(!muted);
        emit playParametersChanged();
    }
}

void
PlayParameters::setPlayAudible(bool audible)
{
//    cerr << "PlayParameters(" << this << "): setPlayAudible(" << audible << ")" << endl;
    setPlayMuted(!audible);
}

void
PlayParameters::setPlayPan(float pan)
{
    if (m_playPan != pan) {
        m_playPan = pan;
        emit playPanChanged(pan);
        emit playParametersChanged();
    }
}

void
PlayParameters::setPlayGain(float gain)
{
    if (m_playGain != gain) {
        m_playGain = gain;
        emit playGainChanged(gain);
        emit playParametersChanged();
    }
}

void
PlayParameters::setPlayClipId(QString id)
{
    if (m_playClipId != id) {
        m_playClipId = id;
        emit playClipIdChanged(id);
        emit playParametersChanged();
    }
}
