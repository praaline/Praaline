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

#ifndef _DSSI_PLUGIN_FACTORY_H_
#define _DSSI_PLUGIN_FACTORY_H_

#define DSSI_API_LEVEL 2

#include "LADSPAPluginFactory.h"
#include "api/dssi.h"

#include <QMutex>

class DSSIPluginInstance;

class DSSIPluginFactory : public LADSPAPluginFactory
{
public:
    virtual ~DSSIPluginFactory();

    virtual void enumeratePlugins(std::vector<QString> &list);

    virtual RealTimePluginInstance *instantiatePlugin(QString identifier,
						      int clientId,
						      int position,
						      sv_samplerate_t sampleRate,
						      int blockSize,
						      int channels);

protected:
    DSSIPluginFactory();
    friend class RealTimePluginFactory;

    virtual std::vector<QString> getPluginPath();

    virtual std::vector<QString> getLRDFPath(QString &baseUri);

    virtual void discoverPluginsFrom(QString soName);

    virtual const LADSPA_Descriptor *getLADSPADescriptor(QString identifier);
    virtual const DSSI_Descriptor *getDSSIDescriptor(QString identifier);

    DSSI_Host_Descriptor m_hostDescriptor;
};

#endif

