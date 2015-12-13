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

#ifndef _REALTIME_PLUGIN_INSTANCE_H_
#define _REALTIME_PLUGIN_INSTANCE_H_

#include <vamp-hostsdk/PluginBase.h>

#include "base/RealTime.h"
#include "base/AudioPlaySource.h"

#include <QString>
#include <QStringList>
#include <vector>
#include <string>
#include <map>

class RealTimePluginFactory;
	
/**
 * RealTimePluginInstance is an interface that an audio process can
 * use to refer to an instance of a plugin without needing to know
 * what type of plugin it is.
 *
 * The audio code calls run() on an instance that has been passed to
 * it, and assumes that the passing code has already initialised the
 * plugin, connected its inputs and outputs and so on, and that there
 * is an understanding in place about the sizes of the buffers in use
 * by the plugin.  All of this depends on the subclass implementation.
 *
 * The PluginInstance base class includes additional abstract methods
 * which the subclass of RealTimePluginInstance must implement.
 */

/*
 * N.B. RealTimePluginInstance, RealTimePluginFactory and their
 * subclasses are terrible code.  They've been reused, cut and pasted
 * and mangled too many times to fit too many different uses, and
 * could do with a good tidy.
 */

// These names are taken from LADSPA, but the values are not
// guaranteed to match

namespace PortType { // ORable
    static const int Input   = 1;
    static const int Output  = 2;
    static const int Control = 4;
    static const int Audio   = 8;
}

namespace PortHint { // ORable
    static const int NoHint  = 0;
    static const int Toggled = 1;
    static const int Integer = 2;
    static const int Logarithmic = 4;
    static const int SampleRate = 8;
}

class RealTimePluginInstance : public Vamp::PluginBase, public Auditionable
{
public:
    typedef float sample_t;

    virtual ~RealTimePluginInstance();

    virtual bool isOK() const = 0;

    virtual QString getPluginIdentifier() const = 0;

    /**
     * Run for one block, starting at the given time.  The start time
     * may be of interest to synths etc that may have queued events
     * waiting.  Other plugins can ignore it.  The count, if zero,
     * defaults to our fixed buffer size.
     */
    virtual void run(const RealTime &blockStartTime,
                     int count = 0) = 0;
    
    virtual int getBufferSize() const = 0;

    virtual int getAudioInputCount() const = 0;
    virtual int getAudioOutputCount() const = 0;

    virtual sample_t **getAudioInputBuffers() = 0;
    virtual sample_t **getAudioOutputBuffers() = 0;

    // Control inputs are known as parameters here
    virtual int getControlOutputCount() const = 0;
    virtual float getControlOutputValue(int n) const = 0;

//     virtual QStringList getPrograms() const { return QStringList(); }
//     virtual QString getCurrentProgram() const { return QString(); }
    virtual std::string getProgram(int /* bank */, int /* program */) const { return std::string(); }
//     virtual int getProgram(QString /* name */) const { return 0; } // bank << 16 + program
//     virtual void selectProgram(QString) { }

    virtual int getParameterCount() const = 0;
    virtual void setParameterValue(int parameter, float value) = 0;
    virtual float getParameterValue(int parameter) const = 0;
    virtual float getParameterDefault(int parameter) const = 0;
    virtual int getParameterDisplayHint(int parameter) const = 0;

    virtual std::string configure(std::string /* key */, std::string /* value */) { return std::string(); }

    virtual void sendEvent(const RealTime & /* eventTime */,
			   const void * /* event */) { }
    virtual void clearEvents() { }

    virtual bool isBypassed() const = 0;
    virtual void setBypassed(bool value) = 0;

    // This should be called after setup, but while not actually playing.
    virtual sv_frame_t getLatency() = 0;

    virtual void silence() = 0;
    virtual void discardEvents() { }
    virtual void setIdealChannelCount(int channels) = 0; // must also silence(); may also re-instantiate

    void setFactory(RealTimePluginFactory *f) { m_factory = f; } // ew

    virtual std::string getType() const { return "Real-Time Plugin"; }

    typedef std::map<std::string, std::string> ConfigurationPairMap;
    virtual ConfigurationPairMap getConfigurePairs() {
        return m_configurationData;
    }

protected:
    RealTimePluginInstance(RealTimePluginFactory *factory, QString identifier) :
	m_factory(factory), m_identifier(identifier) { }

    RealTimePluginFactory *m_factory;
    QString m_identifier;

    ConfigurationPairMap m_configurationData;

    friend class PluginFactory;
};


#endif
