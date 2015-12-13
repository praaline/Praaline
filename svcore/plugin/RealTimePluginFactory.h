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

/*
   This is a modified version of a source file from the 
   Rosegarden MIDI and audio sequencer and notation editor.
   This file copyright 2000-2006 Chris Cannam.
*/

#ifndef _REALTIME_PLUGIN_FACTORY_H_
#define _REALTIME_PLUGIN_FACTORY_H_

#include <QString>
#include <vector>

#include "base/Debug.h"
#include "base/BaseTypes.h"

class RealTimePluginInstance;

class RealTimePluginDescriptor
{
public:
    std::string name;
    std::string label;
    std::string maker;
    std::string copyright;
    std::string category;
    bool isSynth;
    unsigned int parameterCount;
    unsigned int audioInputPortCount;
    unsigned int audioOutputPortCount;
    unsigned int controlOutputPortCount;
    std::vector<std::string> controlOutputPortNames;
};

class RealTimePluginFactory
{
public:
    virtual ~RealTimePluginFactory();

    static RealTimePluginFactory *instance(QString pluginType);
    static RealTimePluginFactory *instanceFor(QString identifier);
    static std::vector<QString> getAllPluginIdentifiers();
    static void enumerateAllPlugins(std::vector<QString> &);

    static void setSampleRate(sv_samplerate_t sampleRate) { m_sampleRate = sampleRate; }

    /**
     * Look up the plugin path and find the plugins in it.  Called 
     * automatically after construction of a factory.
     */
    virtual void discoverPlugins() = 0;

    /**
     * Return a reference to a list of all plugin identifiers that can
     * be created by this factory.
     */
    virtual const std::vector<QString> &getPluginIdentifiers() const = 0;

    /**
     * Append to the given list descriptions of all the available
     * plugins and their ports.  This is in a standard format, see
     * the LADSPA implementation for details.
     */
    virtual void enumeratePlugins(std::vector<QString> &list) = 0;

    /**
     * Get some basic information about a plugin (rapidly).
     */
    virtual const RealTimePluginDescriptor *getPluginDescriptor(QString identifier) const = 0;

    /**
     * Instantiate a plugin.
     */
    virtual RealTimePluginInstance *instantiatePlugin(QString identifier,
						      int clientId,
						      int position,
						      sv_samplerate_t sampleRate,
						      int blockSize,
						      int channels) = 0;

    /**
     * Get category metadata about a plugin (without instantiating it).
     */
    virtual QString getPluginCategory(QString identifier) = 0;

protected:
    RealTimePluginFactory() { }

    // for call by RealTimePluginInstance dtor
    virtual void releasePlugin(RealTimePluginInstance *, QString identifier) = 0;
    friend class RealTimePluginInstance;

    static sv_samplerate_t m_sampleRate;
};

#endif
