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

#include "AudioTargetFactory.h"

#include "AudioJACKTarget.h"
#include "AudioPortAudioTarget.h"
#include "AudioPulseAudioTarget.h"

#include "AudioCallbackPlayTarget.h"

#include <QCoreApplication>

#include <iostream>

AudioTargetFactory *
AudioTargetFactory::m_instance = 0;

AudioTargetFactory *
AudioTargetFactory::getInstance()
{
    if (!m_instance) m_instance = new AudioTargetFactory();
    return m_instance;
}

AudioTargetFactory::AudioTargetFactory()
{
}

std::vector<QString>
AudioTargetFactory::getCallbackTargetNames(bool includeAuto) const
{
    std::vector<QString> names;
    if (includeAuto) names.push_back("auto");

#ifdef HAVE_JACK
    names.push_back("jack");
#endif

#ifdef HAVE_LIBPULSE
    names.push_back("pulse");
#endif

#ifdef HAVE_PORTAUDIO_2_0
    names.push_back("port");
#endif

    return names;
}

QString
AudioTargetFactory::getCallbackTargetDescription(QString name) const
{
    if (name == "auto") {
        return QCoreApplication::translate("AudioTargetFactory",
                                           "(auto)");
    }
    if (name == "jack") {
        return QCoreApplication::translate("AudioTargetFactory",
                                           "JACK Audio Connection Kit");
    }
    if (name == "pulse") {
        return QCoreApplication::translate("AudioTargetFactory",
                                           "PulseAudio Server");
    }
    if (name == "port") {
        return QCoreApplication::translate("AudioTargetFactory",
                                           "Default Soundcard Device");
    }

    return "(unknown)";
}

QString
AudioTargetFactory::getDefaultCallbackTarget() const
{
    if (m_default == "") return "auto";
    return m_default;
}

bool
AudioTargetFactory::isAutoCallbackTarget(QString name) const
{
    return (name == "auto" || name == "");
}

void
AudioTargetFactory::setDefaultCallbackTarget(QString target)
{
    m_default = target;
}

AudioCallbackPlayTarget *
AudioTargetFactory::createCallbackTarget(AudioCallbackPlaySource *source)
{
    AudioCallbackPlayTarget *target = 0;

    if (m_default != "" && m_default != "auto") {

#ifdef HAVE_JACK
        if (m_default == "jack") target = new AudioJACKTarget(source);
#endif

#ifdef HAVE_LIBPULSE
        if (m_default == "pulse") target = new AudioPulseAudioTarget(source);
#endif

#ifdef HAVE_PORTAUDIO_2_0
        if (m_default == "port") target = new AudioPortAudioTarget(source);
#endif

        if (!target || !target->isOK()) {
            cerr << "WARNING: AudioTargetFactory::createCallbackTarget: Failed to open the requested target (\"" << m_default << "\")" << endl;
            delete target;
            return 0;
        } else {
            return target;
        }
    }

#ifdef HAVE_JACK
    target = new AudioJACKTarget(source);
    if (target->isOK()) return target;
    else {
	cerr << "WARNING: AudioTargetFactory::createCallbackTarget: Failed to open JACK target" << endl;
	delete target;
    }
#endif
    
#ifdef HAVE_LIBPULSE
    target = new AudioPulseAudioTarget(source);
    if (target->isOK()) return target;
    else {
	cerr << "WARNING: AudioTargetFactory::createCallbackTarget: Failed to open PulseAudio target" << endl;
	delete target;
    }
#endif
    
#ifdef HAVE_PORTAUDIO_2_0
    target = new AudioPortAudioTarget(source);
    if (target->isOK()) return target;
    else {
	cerr << "WARNING: AudioTargetFactory::createCallbackTarget: Failed to open PortAudio target" << endl;
	delete target;
    }
#endif

    cerr << "WARNING: AudioTargetFactory::createCallbackTarget: No suitable targets available" << endl;
    return 0;
}


