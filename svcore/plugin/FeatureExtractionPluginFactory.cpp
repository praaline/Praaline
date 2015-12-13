/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "FeatureExtractionPluginFactory.h"
#include "PluginIdentifier.h"

#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginWrapper.h>

#include "system/System.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <iostream>

#include "base/Profiler.h"

//#define DEBUG_PLUGIN_SCAN_AND_INSTANTIATE 1

class PluginDeletionNotifyAdapter : public Vamp::HostExt::PluginWrapper {
public:
    PluginDeletionNotifyAdapter(Vamp::Plugin *plugin,
                                FeatureExtractionPluginFactory *factory) :
        PluginWrapper(plugin), m_factory(factory) { }
    virtual ~PluginDeletionNotifyAdapter();
protected:
    FeatureExtractionPluginFactory *m_factory;
};

PluginDeletionNotifyAdapter::~PluginDeletionNotifyAdapter()
{
    // see notes in vamp-sdk/hostext/PluginLoader.cpp from which this is drawn
    Vamp::Plugin *p = m_plugin;
    delete m_plugin;
    m_plugin = 0;
    // acceptable use after free here, as pluginDeleted uses p only as
    // pointer key and does not deref it
    if (m_factory) m_factory->pluginDeleted(p);
}

static FeatureExtractionPluginFactory *_nativeInstance = 0;

FeatureExtractionPluginFactory *
FeatureExtractionPluginFactory::instance(QString pluginType)
{
    if (pluginType == "vamp") {
	if (!_nativeInstance) {
//	    cerr << "FeatureExtractionPluginFactory::instance(" << pluginType//		      << "): creating new FeatureExtractionPluginFactory" << endl;
	    _nativeInstance = new FeatureExtractionPluginFactory();
	}
	return _nativeInstance;
    }

    else return 0;
}

FeatureExtractionPluginFactory *
FeatureExtractionPluginFactory::instanceFor(QString identifier)
{
    QString type, soName, label;
    PluginIdentifier::parseIdentifier(identifier, type, soName, label);
    return instance(type);
}

std::vector<QString>
FeatureExtractionPluginFactory::getPluginPath()
{
    if (!m_pluginPath.empty()) return m_pluginPath;

    std::vector<std::string> p = Vamp::PluginHostAdapter::getPluginPath();
    for (size_t i = 0; i < p.size(); ++i) m_pluginPath.push_back(p[i].c_str());
    return m_pluginPath;
}

std::vector<QString>
FeatureExtractionPluginFactory::getAllPluginIdentifiers()
{
    FeatureExtractionPluginFactory *factory;
    std::vector<QString> rv;
    
    factory = instance("vamp");
    if (factory) {
	std::vector<QString> tmp = factory->getPluginIdentifiers();
	for (size_t i = 0; i < tmp.size(); ++i) {
//            cerr << "identifier: " << tmp[i] << endl;
	    rv.push_back(tmp[i]);
	}
    }

    // Plugins can change the locale, revert it to default.
    RestoreStartupLocale();

    return rv;
}

std::vector<QString>
FeatureExtractionPluginFactory::getPluginIdentifiers()
{
    Profiler profiler("FeatureExtractionPluginFactory::getPluginIdentifiers");

    std::vector<QString> rv;
    std::vector<QString> path = getPluginPath();
    
    for (std::vector<QString>::iterator i = path.begin(); i != path.end(); ++i) {

#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
        cerr << "FeatureExtractionPluginFactory::getPluginIdentifiers: scanning directory " << i-<< endl;
#endif

	QDir pluginDir(*i, PLUGIN_GLOB,
                       QDir::Name | QDir::IgnoreCase,
                       QDir::Files | QDir::Readable);

	for (unsigned int j = 0; j < pluginDir.count(); ++j) {

            QString soname = pluginDir.filePath(pluginDir[j]);

#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
            cerr << "FeatureExtractionPluginFactory::getPluginIdentifiers: trying potential library " << soname << endl;
#endif

            void *libraryHandle = DLOPEN(soname, RTLD_LAZY | RTLD_LOCAL);
            
            if (!libraryHandle) {
                cerr << "WARNING: FeatureExtractionPluginFactory::getPluginIdentifiers: Failed to load library " << soname << ": " << DLERROR() << endl;
                continue;
            }

#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
            cerr << "FeatureExtractionPluginFactory::getPluginIdentifiers: It's a library all right, checking for descriptor" << endl;
#endif

            VampGetPluginDescriptorFunction fn = (VampGetPluginDescriptorFunction)
                DLSYM(libraryHandle, "vampGetPluginDescriptor");

            if (!fn) {
                cerr << "WARNING: FeatureExtractionPluginFactory::getPluginIdentifiers: No descriptor function in " << soname << endl;
                if (DLCLOSE(libraryHandle) != 0) {
                    cerr << "WARNING: FeatureExtractionPluginFactory::getPluginIdentifiers: Failed to unload library " << soname << endl;
                }
                continue;
            }

#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
            cerr << "FeatureExtractionPluginFactory::getPluginIdentifiers: Vamp descriptor found" << endl;
#endif

            const VampPluginDescriptor *descriptor = 0;
            int index = 0;

            std::map<std::string, int> known;
            bool ok = true;

            while ((descriptor = fn(VAMP_API_VERSION, index))) {

                if (known.find(descriptor->identifier) != known.end()) {
                    cerr << "WARNING: FeatureExtractionPluginFactory::getPluginIdentifiers: Plugin library "
                              << soname
                              << " returns the same plugin identifier \""
                              << descriptor->identifier << "\" at indices "
                              << known[descriptor->identifier] << " and "
                              << index << endl;
                    cerr << "FeatureExtractionPluginFactory::getPluginIdentifiers: Avoiding this library (obsolete API?)" << endl;
                    ok = false;
                    break;
                } else {
                    known[descriptor->identifier] = index;
                }

                ++index;
            }

            if (ok) {

                index = 0;

                while ((descriptor = fn(VAMP_API_VERSION, index))) {

                    QString id = PluginIdentifier::createIdentifier
                        ("vamp", soname, descriptor->identifier);
                    rv.push_back(id);
#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
                    cerr << "FeatureExtractionPluginFactory::getPluginIdentifiers: Found plugin id " << id << " at index " << index << endl;
#endif
                    ++index;
                }
            }
            
            if (DLCLOSE(libraryHandle) != 0) {
                cerr << "WARNING: FeatureExtractionPluginFactory::getPluginIdentifiers: Failed to unload library " << soname << endl;
            }
	}
    }

    generateTaxonomy();

    return rv;
}

QString
FeatureExtractionPluginFactory::findPluginFile(QString soname, QString inDir)
{
    QString file = "";

#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
    cerr << "FeatureExtractionPluginFactory::findPluginFile(\""
              << soname << "\", \"" << inDir << "\")"
              << endl;
#endif

    if (inDir != "") {

        QDir dir(inDir, PLUGIN_GLOB,
                 QDir::Name | QDir::IgnoreCase,
                 QDir::Files | QDir::Readable);
        if (!dir.exists()) return "";

        file = dir.filePath(QFileInfo(soname).fileName());

        if (QFileInfo(file).exists() && QFileInfo(file).isFile()) {

#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
            cerr << "FeatureExtractionPluginFactory::findPluginFile: "
                      << "found trivially at " << file << endl;
#endif

            return file;
        }

	for (unsigned int j = 0; j < dir.count(); ++j) {
            file = dir.filePath(dir[j]);
            if (QFileInfo(file).baseName() == QFileInfo(soname).baseName()) {

#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
                cerr << "FeatureExtractionPluginFactory::findPluginFile: "
                          << "found \"" << soname << "\" at " << file << endl;
#endif

                return file;
            }
        }

#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
        cerr << "FeatureExtractionPluginFactory::findPluginFile (with dir): "
                  << "not found" << endl;
#endif

        return "";

    } else {

        QFileInfo fi(soname);

        if (fi.isAbsolute() && fi.exists() && fi.isFile()) {
#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
            cerr << "FeatureExtractionPluginFactory::findPluginFile: "
                      << "found trivially at " << soname << endl;
#endif
            return soname;
        }

        if (fi.isAbsolute() && fi.absolutePath() != "") {
            file = findPluginFile(soname, fi.absolutePath());
            if (file != "") return file;
        }

        std::vector<QString> path = getPluginPath();
        for (std::vector<QString>::iterator i = path.begin();
             i != path.end(); ++i) {
            if (*i != "") {
                file = findPluginFile(soname, *i);
                if (file != "") return file;
            }
        }

#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
        cerr << "FeatureExtractionPluginFactory::findPluginFile: "
                  << "not found" << endl;
#endif

        return "";
    }
}

Vamp::Plugin *
FeatureExtractionPluginFactory::instantiatePlugin(QString identifier,
						  sv_samplerate_t inputSampleRate)
{
    Profiler profiler("FeatureExtractionPluginFactory::instantiatePlugin");

    Vamp::Plugin *rv = 0;
    Vamp::PluginHostAdapter *plugin = 0;

    const VampPluginDescriptor *descriptor = 0;
    int index = 0;

    QString type, soname, label;
    PluginIdentifier::parseIdentifier(identifier, type, soname, label);
    if (type != "vamp") {
	cerr << "FeatureExtractionPluginFactory::instantiatePlugin: Wrong factory for plugin type " << type << endl;
	return 0;
    }

    QString found = findPluginFile(soname);

    if (found == "") {
        cerr << "FeatureExtractionPluginFactory::instantiatePlugin: Failed to find library file " << soname << endl;
        return 0;
    } else if (found != soname) {

#ifdef DEBUG_PLUGIN_SCAN_AND_INSTANTIATE
        cerr << "FeatureExtractionPluginFactory::instantiatePlugin: Given library name was " << soname << ", found at " << found << endl;
        cerr << soname << " -> " << found << endl;
#endif

    }        

    soname = found;

    void *libraryHandle = DLOPEN(soname, RTLD_LAZY | RTLD_LOCAL);
            
    if (!libraryHandle) {
        cerr << "FeatureExtractionPluginFactory::instantiatePlugin: Failed to load library " << soname << ": " << DLERROR() << endl;
        return 0;
    }

    VampGetPluginDescriptorFunction fn = (VampGetPluginDescriptorFunction)
        DLSYM(libraryHandle, "vampGetPluginDescriptor");
    
    if (!fn) {
        cerr << "FeatureExtractionPluginFactory::instantiatePlugin: No descriptor function in " << soname << endl;
        goto done;
    }

    while ((descriptor = fn(VAMP_API_VERSION, index))) {
        if (label == descriptor->identifier) break;
        ++index;
    }

    if (!descriptor) {
        cerr << "FeatureExtractionPluginFactory::instantiatePlugin: Failed to find plugin \"" << label << "\" in library " << soname << endl;
        goto done;
    }

    plugin = new Vamp::PluginHostAdapter(descriptor, float(inputSampleRate));

    if (plugin) {
        m_handleMap[plugin] = libraryHandle;
        rv = new PluginDeletionNotifyAdapter(plugin, this);
    }

//    cerr << "FeatureExtractionPluginFactory::instantiatePlugin: Constructed Vamp plugin, rv is " << rv << endl;

    //!!! need to dlclose() when plugins from a given library are unloaded

done:
    if (!rv) {
        if (DLCLOSE(libraryHandle) != 0) {
            cerr << "WARNING: FeatureExtractionPluginFactory::instantiatePlugin: Failed to unload library " << soname << endl;
        }
    }

//    cerr << "FeatureExtractionPluginFactory::instantiatePlugin: Instantiated plugin " << label << " from library " << soname << ": descriptor " << descriptor << ", rv "<< rv << ", label " << rv->getName() << ", outputs " << rv->getOutputDescriptors().size() << endl;
    
    return rv;
}

void
FeatureExtractionPluginFactory::pluginDeleted(Vamp::Plugin *plugin)
{
    void *handle = m_handleMap[plugin];
    if (handle) {
//        cerr << "unloading library " << handle << " for plugin " << plugin << endl;
        DLCLOSE(handle);
    }
    m_handleMap.erase(plugin);
}

QString
FeatureExtractionPluginFactory::getPluginCategory(QString identifier)
{
    return m_taxonomy[identifier];
}

void
FeatureExtractionPluginFactory::generateTaxonomy()
{
    std::vector<QString> pluginPath = getPluginPath();
    std::vector<QString> path;

    for (size_t i = 0; i < pluginPath.size(); ++i) {
	if (pluginPath[i].contains("/lib/")) {
	    QString p(pluginPath[i]);
            path.push_back(p);
	    p.replace("/lib/", "/share/");
	    path.push_back(p);
	}
	path.push_back(pluginPath[i]);
    }

    for (size_t i = 0; i < path.size(); ++i) {

	QDir dir(path[i], "*.cat");

//	cerr << "LADSPAPluginFactory::generateFallbackCategories: directory " << path[i] << " has " << dir.count() << " .cat files" << endl;
	for (unsigned int j = 0; j < dir.count(); ++j) {

	    QFile file(path[i] + "/" + dir[j]);

//	    cerr << "LADSPAPluginFactory::generateFallbackCategories: about to open " << (path[i]+ "/" + dir[j]) << endl;

	    if (file.open(QIODevice::ReadOnly)) {
//		    cerr << "...opened" << endl;
		QTextStream stream(&file);
		QString line;

		while (!stream.atEnd()) {
		    line = stream.readLine();
//		    cerr << "line is: \"" << line << "\"" << endl;
		    QString id = PluginIdentifier::canonicalise
                        (line.section("::", 0, 0));
		    QString cat = line.section("::", 1, 1);
		    m_taxonomy[id] = cat;
//		    cerr << "FeatureExtractionPluginFactory: set id \"" << id << "\" to cat \"" << cat << "\"" << endl;
		}
	    }
	}
    }
}    
