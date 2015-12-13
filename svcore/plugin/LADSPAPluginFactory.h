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
   This file copyright 2000-2006 Chris Cannam and Richard Bown.
*/

#ifndef _LADSPA_PLUGIN_FACTORY_H_
#define _LADSPA_PLUGIN_FACTORY_H_

#include "RealTimePluginFactory.h"
#include "api/ladspa.h"

#include <vector>
#include <map>
#include <set>
#include <QString>

class LADSPAPluginInstance;

class LADSPAPluginFactory : public RealTimePluginFactory
{
public:
    virtual ~LADSPAPluginFactory();

    virtual void discoverPlugins();

    virtual const std::vector<QString> &getPluginIdentifiers() const;

    virtual void enumeratePlugins(std::vector<QString> &list);

    virtual const RealTimePluginDescriptor *getPluginDescriptor(QString identifier) const;

    virtual RealTimePluginInstance *instantiatePlugin(QString identifier,
						      int clientId,
						      int position,
						      sv_samplerate_t sampleRate,
						      int blockSize,
						      int channels);

    virtual QString getPluginCategory(QString identifier);

    float getPortMinimum(const LADSPA_Descriptor *, int port);
    float getPortMaximum(const LADSPA_Descriptor *, int port);
    float getPortDefault(const LADSPA_Descriptor *, int port);
    float getPortQuantization(const LADSPA_Descriptor *, int port);
    int getPortDisplayHint(const LADSPA_Descriptor *, int port);

protected:
    LADSPAPluginFactory();
    friend class RealTimePluginFactory;

    virtual std::vector<QString> getPluginPath();

    virtual std::vector<QString> getLRDFPath(QString &baseUri);

    virtual void discoverPluginsFrom(QString soName);
    virtual void generateTaxonomy(QString uri, QString base);
    virtual void generateFallbackCategories();

    virtual void releasePlugin(RealTimePluginInstance *, QString);

    virtual const LADSPA_Descriptor *getLADSPADescriptor(QString identifier);

    void loadLibrary(QString soName);
    void unloadLibrary(QString soName);
    void unloadUnusedLibraries();

    std::vector<QString> m_identifiers;
    std::map<QString, RealTimePluginDescriptor *> m_rtDescriptors;

    std::map<QString, QString> m_taxonomy;
    std::map<unsigned long, QString> m_lrdfTaxonomy;
    std::map<unsigned long, std::map<int, float> > m_portDefaults;

    std::set<RealTimePluginInstance *> m_instances;

    typedef std::map<QString, void *> LibraryHandleMap;
    LibraryHandleMap m_libraryHandles;
};

#endif

