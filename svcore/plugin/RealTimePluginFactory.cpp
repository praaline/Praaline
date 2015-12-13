// -*- c-basic-offset: 4 indent-tabs-mode: nil -*-

/*
    Rosegarden-4
    A sequencer and musical notation editor.

    This program is Copyright 2000-2006
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <bownie@bownie.com>

    The moral right of the authors to claim authorship of this work
    has been asserted.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "RealTimePluginFactory.h"
#include "PluginIdentifier.h"

#include "LADSPAPluginFactory.h"
#include "DSSIPluginFactory.h"

#include "system/System.h"
#include "base/Profiler.h"

#include <iostream>

sv_samplerate_t RealTimePluginFactory::m_sampleRate = 48000;

static LADSPAPluginFactory *_ladspaInstance = 0;
static LADSPAPluginFactory *_dssiInstance = 0;

RealTimePluginFactory::~RealTimePluginFactory()
{
}

RealTimePluginFactory *
RealTimePluginFactory::instance(QString pluginType)
{
    if (pluginType == "ladspa") {
	if (!_ladspaInstance) {
//	    cerr << "RealTimePluginFactory::instance(" << pluginType//		      << "): creating new LADSPAPluginFactory" << endl;
	    _ladspaInstance = new LADSPAPluginFactory();
	    _ladspaInstance->discoverPlugins();
	}
	return _ladspaInstance;
    } else if (pluginType == "dssi") {
	if (!_dssiInstance) {
//	    cerr << "RealTimePluginFactory::instance(" << pluginType//		      << "): creating new DSSIPluginFactory" << endl;
	    _dssiInstance = new DSSIPluginFactory();
	    _dssiInstance->discoverPlugins();
	}
	return _dssiInstance;
    }
	
    else return 0;
}

RealTimePluginFactory *
RealTimePluginFactory::instanceFor(QString identifier)
{
    QString type, soName, label;
    PluginIdentifier::parseIdentifier(identifier, type, soName, label);
    return instance(type);
}

std::vector<QString>
RealTimePluginFactory::getAllPluginIdentifiers()
{
    Profiler profiler("RealTimePluginFactory::getAllPluginIdentifiers");

    RealTimePluginFactory *factory;
    std::vector<QString> rv;
    
    // Query DSSI plugins before LADSPA ones.
    // This is to provide for the interesting possibility of plugins
    // providing either DSSI or LADSPA versions of themselves,
    // returning both versions if the LADSPA identifiers are queried
    // first but only the DSSI version if the DSSI identifiers are
    // queried first.

    factory = instance("dssi");
    if (factory) {
	const std::vector<QString> &tmp = factory->getPluginIdentifiers();
	for (size_t i = 0; i < tmp.size(); ++i) {
	    rv.push_back(tmp[i]);
	}
    }

    factory = instance("ladspa");
    if (factory) {
	const std::vector<QString> &tmp = factory->getPluginIdentifiers();
	for (size_t i = 0; i < tmp.size(); ++i) {
	    rv.push_back(tmp[i]);
	}
    }

    // Plugins can change the locale, revert it to default.
    RestoreStartupLocale();

    return rv;
}

void
RealTimePluginFactory::enumerateAllPlugins(std::vector<QString> &list)
{
    Profiler profiler("RealTimePluginFactory::enumerateAllPlugins");

    RealTimePluginFactory *factory;

    // Query DSSI plugins before LADSPA ones.
    // This is to provide for the interesting possibility of plugins
    // providing either DSSI or LADSPA versions of themselves,
    // returning both versions if the LADSPA identifiers are queried
    // first but only the DSSI version if the DSSI identifiers are
    // queried first.

    factory = instance("dssi");
    if (factory) factory->enumeratePlugins(list);

    factory = instance("ladspa");
    if (factory) factory->enumeratePlugins(list);
    
    // Plugins can change the locale, revert it to default.
    RestoreStartupLocale();
}

