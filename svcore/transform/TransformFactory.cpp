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

#include "TransformFactory.h"

#include "plugin/FeatureExtractionPluginFactory.h"
#include "plugin/RealTimePluginFactory.h"
#include "plugin/RealTimePluginInstance.h"
#include "plugin/PluginXml.h"

#include <vamp-hostsdk/Plugin.h>
#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginWrapper.h>

#include "rdf/PluginRDFIndexer.h"
#include "rdf/PluginRDFDescription.h"

#include "base/XmlExportable.h"

#include <iostream>
#include <set>

#include <QRegExp>
#include <QTextStream>

#include "base/Thread.h"

//#define DEBUG_TRANSFORM_FACTORY 1

TransformFactory *
TransformFactory::m_instance = new TransformFactory;

TransformFactory *
TransformFactory::getInstance()
{
    return m_instance;
}

void
TransformFactory::deleteInstance()
{
    cerr << "TransformFactory::deleteInstance called" << endl;
    delete m_instance;
    m_instance = 0;
}

TransformFactory::TransformFactory() :
    m_transformsPopulated(false),
    m_uninstalledTransformsPopulated(false),
    m_thread(0),
    m_exiting(false),
    m_populatingSlowly(false)
{
}

TransformFactory::~TransformFactory()
{
    m_exiting = true;
    if (m_thread) {
#ifdef DEBUG_TRANSFORM_FACTORY
        cerr << "TransformFactory::~TransformFactory: waiting on thread" << endl;
#endif
        m_thread->wait();
        delete m_thread;
#ifdef DEBUG_TRANSFORM_FACTORY
        cerr << "TransformFactory::~TransformFactory: waited and done" << endl;
#endif
    }
}

void
TransformFactory::startPopulationThread()
{
    m_uninstalledTransformsMutex.lock();

    if (m_thread) {
        m_uninstalledTransformsMutex.unlock();
        return;
    }
    m_thread = new UninstalledTransformsPopulateThread(this);

    m_uninstalledTransformsMutex.unlock();

    m_thread->start();
}

void
TransformFactory::UninstalledTransformsPopulateThread::run()
{
    m_factory->m_populatingSlowly = true;
    sleep(1);
    m_factory->populateUninstalledTransforms();
}

TransformList
TransformFactory::getAllTransformDescriptions()
{
    populateTransforms();

    std::set<TransformDescription> dset;
    for (TransformDescriptionMap::const_iterator i = m_transforms.begin();
	 i != m_transforms.end(); ++i) {
#ifdef DEBUG_TRANSFORM_FACTORY
        cerr << "inserting transform into set: id = " << i->second.identifier << endl;
#endif
	dset.insert(i->second);
    }

    TransformList list;
    for (std::set<TransformDescription>::const_iterator i = dset.begin();
	 i != dset.end(); ++i) {
#ifdef DEBUG_TRANSFORM_FACTORY
        cerr << "inserting transform into list: id = " << i->identifier << endl;
#endif
	list.push_back(*i);
    }

    return list;
}

TransformDescription
TransformFactory::getTransformDescription(TransformId id)
{
    populateTransforms();

    if (m_transforms.find(id) == m_transforms.end()) {
        return TransformDescription();
    }

    return m_transforms[id];
}

bool
TransformFactory::haveInstalledTransforms()
{
    populateTransforms();
    return !m_transforms.empty();
}

TransformList
TransformFactory::getUninstalledTransformDescriptions()
{
    m_populatingSlowly = false;
    populateUninstalledTransforms();
    
    std::set<TransformDescription> dset;
    for (TransformDescriptionMap::const_iterator i = m_uninstalledTransforms.begin();
	 i != m_uninstalledTransforms.end(); ++i) {
#ifdef DEBUG_TRANSFORM_FACTORY
        cerr << "inserting transform into set: id = " << i->second.identifier << endl;
#endif
	dset.insert(i->second);
    }

    TransformList list;
    for (std::set<TransformDescription>::const_iterator i = dset.begin();
	 i != dset.end(); ++i) {
#ifdef DEBUG_TRANSFORM_FACTORY
        cerr << "inserting transform into uninstalled list: id = " << i->identifier << endl;
#endif
	list.push_back(*i);
    }

    return list;
}

TransformDescription
TransformFactory::getUninstalledTransformDescription(TransformId id)
{
    m_populatingSlowly = false;
    populateUninstalledTransforms();

    if (m_uninstalledTransforms.find(id) == m_uninstalledTransforms.end()) {
        return TransformDescription();
    }

    return m_uninstalledTransforms[id];
}

bool
TransformFactory::haveUninstalledTransforms(bool waitForCheckToComplete)
{
    if (waitForCheckToComplete) {
        populateUninstalledTransforms();
    } else {
        if (!m_uninstalledTransformsMutex.tryLock()) {
            return false;
        }
        if (!m_uninstalledTransformsPopulated) {
            m_uninstalledTransformsMutex.unlock();
            return false;
        }
        m_uninstalledTransformsMutex.unlock();
    }

    return !m_uninstalledTransforms.empty();
}

TransformFactory::TransformInstallStatus
TransformFactory::getTransformInstallStatus(TransformId id)
{
    populateTransforms();

    if (m_transforms.find(id) != m_transforms.end()) {
        return TransformInstalled;
    }
    
    if (!m_uninstalledTransformsMutex.tryLock()) {
        // uninstalled transforms are being populated; this may take some time,
        // and they aren't critical
        return TransformUnknown;
    }

    if (!m_uninstalledTransformsPopulated) {
        m_uninstalledTransformsMutex.unlock();
        m_populatingSlowly = false;
        populateUninstalledTransforms();
        m_uninstalledTransformsMutex.lock();
    }

    if (m_uninstalledTransforms.find(id) != m_uninstalledTransforms.end()) {
        m_uninstalledTransformsMutex.unlock();
        return TransformNotInstalled;
    }

    m_uninstalledTransformsMutex.unlock();
    return TransformUnknown;
}
    

std::vector<TransformDescription::Type>
TransformFactory::getAllTransformTypes()
{
    populateTransforms();

    std::set<TransformDescription::Type> types;
    for (TransformDescriptionMap::const_iterator i = m_transforms.begin();
	 i != m_transforms.end(); ++i) {
        types.insert(i->second.type);
    }

    std::vector<TransformDescription::Type> rv;
    for (std::set<TransformDescription::Type>::iterator i = types.begin(); i != types.end(); ++i) {
        rv.push_back(*i);
    }

    return rv;
}

std::vector<QString>
TransformFactory::getTransformCategories(TransformDescription::Type transformType)
{
    populateTransforms();

    std::set<QString> categories;
    for (TransformDescriptionMap::const_iterator i = m_transforms.begin();
         i != m_transforms.end(); ++i) {
        if (i->second.type == transformType) {
            categories.insert(i->second.category);
        }
    }

    bool haveEmpty = false;
    
    std::vector<QString> rv;
    for (std::set<QString>::iterator i = categories.begin(); 
         i != categories.end(); ++i) {
        if (*i != "") rv.push_back(*i);
        else haveEmpty = true;
    }

    if (haveEmpty) rv.push_back(""); // make sure empty category sorts last

    return rv;
}

std::vector<QString>
TransformFactory::getTransformMakers(TransformDescription::Type transformType)
{
    populateTransforms();

    std::set<QString> makers;
    for (TransformDescriptionMap::const_iterator i = m_transforms.begin();
         i != m_transforms.end(); ++i) {
        if (i->second.type == transformType) {
            makers.insert(i->second.maker);
        }
    }

    bool haveEmpty = false;
    
    std::vector<QString> rv;
    for (std::set<QString>::iterator i = makers.begin(); 
         i != makers.end(); ++i) {
        if (*i != "") rv.push_back(*i);
        else haveEmpty = true;
    }

    if (haveEmpty) rv.push_back(""); // make sure empty category sorts last

    return rv;
}

QString
TransformFactory::getTransformTypeName(TransformDescription::Type type) const
{
    switch (type) {
    case TransformDescription::Analysis: return tr("Analysis");
    case TransformDescription::Effects: return tr("Effects");
    case TransformDescription::EffectsData: return tr("Effects Data");
    case TransformDescription::Generator: return tr("Generator");
    case TransformDescription::UnknownType: return tr("Other");
    }
    return tr("Other");
}

void
TransformFactory::populateTransforms()
{
    MutexLocker locker(&m_transformsMutex,
                       "TransformFactory::populateTransforms");
    if (m_transformsPopulated) {
        return;
    }

    TransformDescriptionMap transforms;

    populateFeatureExtractionPlugins(transforms);
    if (m_exiting) return;
    populateRealTimePlugins(transforms);
    if (m_exiting) return;

    // disambiguate plugins with similar names

    std::map<QString, int> names;
    std::map<QString, QString> pluginSources;
    std::map<QString, QString> pluginMakers;

    for (TransformDescriptionMap::iterator i = transforms.begin();
         i != transforms.end(); ++i) {

        TransformDescription desc = i->second;

        QString td = desc.name;
        QString tn = td.section(": ", 0, 0);
        QString pn = desc.identifier.section(":", 1, 1);

        if (pluginSources.find(tn) != pluginSources.end()) {
            if (pluginSources[tn] != pn && pluginMakers[tn] != desc.maker) {
                ++names[tn];
            }
        } else {
            ++names[tn];
            pluginSources[tn] = pn;
            pluginMakers[tn] = desc.maker;
        }
    }

    std::map<QString, int> counts;
    m_transforms.clear();

    for (TransformDescriptionMap::iterator i = transforms.begin();
         i != transforms.end(); ++i) {

        TransformDescription desc = i->second;
	QString identifier = desc.identifier;
        QString maker = desc.maker;

        QString td = desc.name;
        QString tn = td.section(": ", 0, 0);
        QString to = td.section(": ", 1);

	if (names[tn] > 1) {
            maker.replace(QRegExp(tr(" [\\(<].*$")), "");
	    tn = QString("%1 [%2]").arg(tn).arg(maker);
	}

        if (to != "") {
            desc.name = QString("%1: %2").arg(tn).arg(to);
        } else {
            desc.name = tn;
        }

	m_transforms[identifier] = desc;
    }	    

    m_transformsPopulated = true;
}

void
TransformFactory::populateFeatureExtractionPlugins(TransformDescriptionMap &transforms)
{
    std::vector<QString> plugs =
	FeatureExtractionPluginFactory::getAllPluginIdentifiers();
    if (m_exiting) return;

    for (int i = 0; i < (int)plugs.size(); ++i) {

	QString pluginId = plugs[i];

	FeatureExtractionPluginFactory *factory =
	    FeatureExtractionPluginFactory::instanceFor(pluginId);

	if (!factory) {
	    cerr << "WARNING: TransformFactory::populateTransforms: No feature extraction plugin factory for instance " << pluginId << endl;
	    continue;
	}

	Vamp::Plugin *plugin = 
	    factory->instantiatePlugin(pluginId, 44100);

	if (!plugin) {
	    cerr << "WARNING: TransformFactory::populateTransforms: Failed to instantiate plugin " << pluginId << endl;
	    continue;
	}
		
	QString pluginName = plugin->getName().c_str();
        QString category = factory->getPluginCategory(pluginId);

	Vamp::Plugin::OutputList outputs =
	    plugin->getOutputDescriptors();

	for (int j = 0; j < (int)outputs.size(); ++j) {

	    QString transformId = QString("%1:%2")
		    .arg(pluginId).arg(outputs[j].identifier.c_str());

	    QString userName;
            QString friendlyName;
            QString units = outputs[j].unit.c_str();
            QString description = plugin->getDescription().c_str();
            QString maker = plugin->getMaker().c_str();
            if (maker == "") maker = tr("<unknown maker>");

            QString longDescription = description;

            if (longDescription == "") {
                if (outputs.size() == 1) {
                    longDescription = tr("Extract features using \"%1\" plugin (from %2)")
                        .arg(pluginName).arg(maker);
                } else {
                    longDescription = tr("Extract features using \"%1\" output of \"%2\" plugin (from %3)")
                        .arg(outputs[j].name.c_str()).arg(pluginName).arg(maker);
                }
            } else {
                if (outputs.size() == 1) {
                    longDescription = tr("%1 using \"%2\" plugin (from %3)")
                        .arg(longDescription).arg(pluginName).arg(maker);
                } else {
                    longDescription = tr("%1 using \"%2\" output of \"%3\" plugin (from %4)")
                        .arg(longDescription).arg(outputs[j].name.c_str()).arg(pluginName).arg(maker);
                }
            }                    

	    if (outputs.size() == 1) {
		userName = pluginName;
                friendlyName = pluginName;
	    } else {
		userName = QString("%1: %2")
		    .arg(pluginName)
		    .arg(outputs[j].name.c_str());
                friendlyName = outputs[j].name.c_str();
	    }

            bool configurable = (!plugin->getPrograms().empty() ||
                                 !plugin->getParameterDescriptors().empty());

#ifdef DEBUG_TRANSFORM_FACTORY
            cerr << "Feature extraction plugin transform: " << transformId << " friendly name: " << friendlyName << endl;
#endif

	    transforms[transformId] = 
                TransformDescription(TransformDescription::Analysis,
                                     category,
                                     transformId,
                                     userName,
                                     friendlyName,
                                     description,
                                     longDescription,
                                     maker,
                                     units,
                                     configurable);
	}

        delete plugin;
    }
}

void
TransformFactory::populateRealTimePlugins(TransformDescriptionMap &transforms)
{
    std::vector<QString> plugs =
	RealTimePluginFactory::getAllPluginIdentifiers();
    if (m_exiting) return;

    static QRegExp unitRE("[\\[\\(]([A-Za-z0-9/]+)[\\)\\]]$");

    for (int i = 0; i < (int)plugs.size(); ++i) {
        
	QString pluginId = plugs[i];

        RealTimePluginFactory *factory =
            RealTimePluginFactory::instanceFor(pluginId);

	if (!factory) {
	    cerr << "WARNING: TransformFactory::populateTransforms: No real time plugin factory for instance " << pluginId << endl;
	    continue;
	}

        const RealTimePluginDescriptor *descriptor =
            factory->getPluginDescriptor(pluginId);

        if (!descriptor) {
	    cerr << "WARNING: TransformFactory::populateTransforms: Failed to query plugin " << pluginId << endl;
	    continue;
	}
	
//!!!        if (descriptor->controlOutputPortCount == 0 ||
//            descriptor->audioInputPortCount == 0) continue;

//        cout << "TransformFactory::populateRealTimePlugins: plugin " << pluginId << " has " << descriptor->controlOutputPortCount << " control output ports, " << descriptor->audioOutputPortCount << " audio outputs, " << descriptor->audioInputPortCount << " audio inputs" << endl;
	
	QString pluginName = descriptor->name.c_str();
        QString category = factory->getPluginCategory(pluginId);
        bool configurable = (descriptor->parameterCount > 0);
        QString maker = descriptor->maker.c_str();
        if (maker == "") maker = tr("<unknown maker>");

        if (descriptor->audioInputPortCount > 0) {

            for (int j = 0; j < (int)descriptor->controlOutputPortCount; ++j) {

                QString transformId = QString("%1:%2").arg(pluginId).arg(j);
                QString userName;
                QString units;
                QString portName;

                if (j < (int)descriptor->controlOutputPortNames.size() &&
                    descriptor->controlOutputPortNames[j] != "") {

                    portName = descriptor->controlOutputPortNames[j].c_str();

                    userName = tr("%1: %2")
                        .arg(pluginName)
                        .arg(portName);

                    if (unitRE.indexIn(portName) >= 0) {
                        units = unitRE.cap(1);
                    }

                } else if (descriptor->controlOutputPortCount > 1) {

                    userName = tr("%1: Output %2")
                        .arg(pluginName)
                        .arg(j + 1);

                } else {

                    userName = pluginName;
                }

                QString description;

                if (portName != "") {
                    description = tr("Extract \"%1\" data output from \"%2\" effect plugin (from %3)")
                        .arg(portName)
                        .arg(pluginName)
                        .arg(maker);
                } else {
                    description = tr("Extract data output %1 from \"%2\" effect plugin (from %3)")
                        .arg(j + 1)
                        .arg(pluginName)
                        .arg(maker);
                }

                transforms[transformId] = 
                    TransformDescription(TransformDescription::EffectsData,
                                         category,
                                         transformId,
                                         userName,
                                         userName,
                                         "",
                                         description,
                                         maker,
                                         units,
                                         configurable);
            }
        }

        if (!descriptor->isSynth || descriptor->audioInputPortCount > 0) {

            if (descriptor->audioOutputPortCount > 0) {

                QString transformId = QString("%1:A").arg(pluginId);
                TransformDescription::Type type = TransformDescription::Effects;

                QString description = tr("Transform audio signal with \"%1\" effect plugin (from %2)")
                    .arg(pluginName)
                    .arg(maker);

                if (descriptor->audioInputPortCount == 0) {
                    type = TransformDescription::Generator;
                    QString description = tr("Generate audio signal using \"%1\" plugin (from %2)")
                        .arg(pluginName)
                        .arg(maker);
                }

                transforms[transformId] =
                    TransformDescription(type,
                                         category,
                                         transformId,
                                         pluginName,
                                         pluginName,
                                         "",
                                         description,
                                         maker,
                                         "",
                                         configurable);
            }
        }
    }
}

void
TransformFactory::populateUninstalledTransforms()
{
    if (m_exiting) return;

    populateTransforms();
    if (m_exiting) return;

    MutexLocker locker(&m_uninstalledTransformsMutex,
                       "TransformFactory::populateUninstalledTransforms");
    if (m_uninstalledTransformsPopulated) return;

    PluginRDFIndexer::getInstance()->indexConfiguredURLs();
    if (m_exiting) return;

    //!!! This will be amazingly slow

    QStringList ids = PluginRDFIndexer::getInstance()->getIndexedPluginIds();
    
    for (QStringList::const_iterator i = ids.begin(); i != ids.end(); ++i) {
        
        PluginRDFDescription desc(*i);

        QString name = desc.getPluginName();
#ifdef DEBUG_TRANSFORM_FACTORY
        if (name == "") {
            cerr << "TransformFactory::populateUninstalledTransforms: "
                 << "No name available for plugin " << *i
                 << ", skipping" << endl;
            continue;
        }
#endif

        QString description = desc.getPluginDescription();
        QString maker = desc.getPluginMaker();
        QString infoUrl = desc.getPluginInfoURL();

        QStringList oids = desc.getOutputIds();

        for (QStringList::const_iterator j = oids.begin(); j != oids.end(); ++j) {

            TransformId tid = Transform::getIdentifierForPluginOutput(*i, *j);
            
            if (m_transforms.find(tid) != m_transforms.end()) {
#ifdef DEBUG_TRANSFORM_FACTORY
                cerr << "TransformFactory::populateUninstalledTransforms: "
                          << tid << " is installed; adding info url if appropriate, skipping rest" << endl;
#endif
                if (infoUrl != "") {
                    if (m_transforms[tid].infoUrl == "") {
                        m_transforms[tid].infoUrl = infoUrl;
                    }
                }
                continue;
            }

#ifdef DEBUG_TRANSFORM_FACTORY
            cerr << "TransformFactory::populateUninstalledTransforms: "
                      << "adding " << tid << endl;
#endif

            QString oname = desc.getOutputName(*j);
            if (oname == "") oname = *j;
            
            TransformDescription td;
            td.type = TransformDescription::Analysis;
            td.category = "";
            td.identifier = tid;

            if (oids.size() == 1) {
                td.name = name;
            } else if (name != "") {
                td.name = tr("%1: %2").arg(name).arg(oname);
            }

            QString longDescription = description;
            //!!! basically duplicated from above
            if (longDescription == "") {
                if (oids.size() == 1) {
                    longDescription = tr("Extract features using \"%1\" plugin (from %2)")
                        .arg(name).arg(maker);
                } else {
                    longDescription = tr("Extract features using \"%1\" output of \"%2\" plugin (from %3)")
                        .arg(oname).arg(name).arg(maker);
                }
            } else {
                if (oids.size() == 1) {
                    longDescription = tr("%1 using \"%2\" plugin (from %3)")
                        .arg(longDescription).arg(name).arg(maker);
                } else {
                    longDescription = tr("%1 using \"%2\" output of \"%3\" plugin (from %4)")
                        .arg(longDescription).arg(oname).arg(name).arg(maker);
                }
            }                    

            td.friendlyName = name; //!!!???
            td.description = description;
            td.longDescription = longDescription;
            td.maker = maker;
            td.infoUrl = infoUrl;
            td.units = "";
            td.configurable = false;

            m_uninstalledTransforms[tid] = td;
        }

        if (m_exiting) return;
    }

    m_uninstalledTransformsPopulated = true;

#ifdef DEBUG_TRANSFORM_FACTORY
    cerr << "populateUninstalledTransforms exiting" << endl;
#endif
}

Transform
TransformFactory::getDefaultTransformFor(TransformId id, sv_samplerate_t rate)
{
    Transform t;
    t.setIdentifier(id);
    if (rate != 0) t.setSampleRate(rate);

    Vamp::PluginBase *plugin = instantiateDefaultPluginFor(id, rate);

    if (plugin) {
        t.setPluginVersion(QString("%1").arg(plugin->getPluginVersion()));
        setParametersFromPlugin(t, plugin);
        makeContextConsistentWithPlugin(t, plugin);
        delete plugin;
    }

    return t;
}

Vamp::PluginBase *
TransformFactory::instantiatePluginFor(const Transform &transform)
{
    Vamp::PluginBase *plugin = instantiateDefaultPluginFor
        (transform.getIdentifier(), transform.getSampleRate());

    if (plugin) {
        setPluginParameters(transform, plugin);
    }

    return plugin;
}

Vamp::PluginBase *
TransformFactory::instantiateDefaultPluginFor(TransformId identifier,
                                              sv_samplerate_t rate)
{
    Transform t;
    t.setIdentifier(identifier);
    if (rate == 0) rate = 44100.0;
    QString pluginId = t.getPluginIdentifier();

    Vamp::PluginBase *plugin = 0;

    if (t.getType() == Transform::FeatureExtraction) {

        FeatureExtractionPluginFactory *factory = 
            FeatureExtractionPluginFactory::instanceFor(pluginId);

        if (factory) {
            plugin = factory->instantiatePlugin(pluginId, rate);
        }

    } else {

        RealTimePluginFactory *factory = 
            RealTimePluginFactory::instanceFor(pluginId);

        if (factory) {
            plugin = factory->instantiatePlugin(pluginId, 0, 0, rate, 1024, 1);
        }
    }

    return plugin;
}

Vamp::Plugin *
TransformFactory::downcastVampPlugin(Vamp::PluginBase *plugin)
{
    Vamp::Plugin *vp = dynamic_cast<Vamp::Plugin *>(plugin);
    if (!vp) {
//        cerr << "makeConsistentWithPlugin: not a Vamp::Plugin" << endl;
        vp = dynamic_cast<Vamp::PluginHostAdapter *>(plugin); //!!! why?
}
    if (!vp) {
//        cerr << "makeConsistentWithPlugin: not a Vamp::PluginHostAdapter" << endl;
        vp = dynamic_cast<Vamp::HostExt::PluginWrapper *>(plugin); //!!! no, I mean really why?
    }
    if (!vp) {
//        cerr << "makeConsistentWithPlugin: not a Vamp::HostExt::PluginWrapper" << endl;
    }
    return vp;
}

bool
TransformFactory::haveTransform(TransformId identifier)
{
    populateTransforms();
    return (m_transforms.find(identifier) != m_transforms.end());
}

QString
TransformFactory::getTransformName(TransformId identifier)
{
    if (m_transforms.find(identifier) != m_transforms.end()) {
	return m_transforms[identifier].name;
    } else return "";
}

QString
TransformFactory::getTransformFriendlyName(TransformId identifier)
{
    if (m_transforms.find(identifier) != m_transforms.end()) {
	return m_transforms[identifier].friendlyName;
    } else return "";
}

QString
TransformFactory::getTransformUnits(TransformId identifier)
{
    if (m_transforms.find(identifier) != m_transforms.end()) {
	return m_transforms[identifier].units;
    } else return "";
}

QString
TransformFactory::getTransformInfoUrl(TransformId identifier)
{
    if (m_transforms.find(identifier) != m_transforms.end()) {
	return m_transforms[identifier].infoUrl;
    } else return "";
}

Vamp::Plugin::InputDomain
TransformFactory::getTransformInputDomain(TransformId identifier)
{
    Transform transform;
    transform.setIdentifier(identifier);

    if (transform.getType() != Transform::FeatureExtraction) {
        return Vamp::Plugin::TimeDomain;
    }

    Vamp::Plugin *plugin =
        downcastVampPlugin(instantiateDefaultPluginFor(identifier, 0));

    if (plugin) {
        Vamp::Plugin::InputDomain d = plugin->getInputDomain();
        delete plugin;
        return d;
    }

    return Vamp::Plugin::TimeDomain;
}

bool
TransformFactory::isTransformConfigurable(TransformId identifier)
{
    if (m_transforms.find(identifier) != m_transforms.end()) {
	return m_transforms[identifier].configurable;
    } else return false;
}

bool
TransformFactory::getTransformChannelRange(TransformId identifier,
                                           int &min, int &max)
{
    QString id = identifier.section(':', 0, 2);

    if (FeatureExtractionPluginFactory::instanceFor(id)) {

        Vamp::Plugin *plugin = 
            FeatureExtractionPluginFactory::instanceFor(id)->
            instantiatePlugin(id, 44100);
        if (!plugin) return false;

        min = (int)plugin->getMinChannelCount();
        max = (int)plugin->getMaxChannelCount();
        delete plugin;

        return true;

    } else if (RealTimePluginFactory::instanceFor(id)) {

        // don't need to instantiate

        const RealTimePluginDescriptor *descriptor = 
            RealTimePluginFactory::instanceFor(id)->
            getPluginDescriptor(id);
        if (!descriptor) return false;

        min = descriptor->audioInputPortCount;
        max = descriptor->audioInputPortCount;

        return true;
    }

    return false;
}

void
TransformFactory::setParametersFromPlugin(Transform &transform,
                                          Vamp::PluginBase *plugin)
{
    Transform::ParameterMap pmap;

    //!!! record plugin & API version

    //!!! check that this is the right plugin!

    Vamp::PluginBase::ParameterList parameters =
        plugin->getParameterDescriptors();

    for (Vamp::PluginBase::ParameterList::const_iterator i = parameters.begin();
         i != parameters.end(); ++i) {
        pmap[i->identifier.c_str()] = plugin->getParameter(i->identifier);
//        cerr << "TransformFactory::setParametersFromPlugin: parameter "
//                  << i->identifier << " -> value " <<
//            pmap[i->identifier.c_str()] << endl;
    }

    transform.setParameters(pmap);

    if (plugin->getPrograms().empty()) {
        transform.setProgram("");
    } else {
        transform.setProgram(plugin->getCurrentProgram().c_str());
    }

    RealTimePluginInstance *rtpi =
        dynamic_cast<RealTimePluginInstance *>(plugin);

    Transform::ConfigurationMap cmap;

    if (rtpi) {

        RealTimePluginInstance::ConfigurationPairMap configurePairs =
            rtpi->getConfigurePairs();

        for (RealTimePluginInstance::ConfigurationPairMap::const_iterator i
                 = configurePairs.begin(); i != configurePairs.end(); ++i) {
            cmap[i->first.c_str()] = i->second.c_str();
        }
    }

    transform.setConfiguration(cmap);
}

void
TransformFactory::setPluginParameters(const Transform &transform,
                                      Vamp::PluginBase *plugin)
{
    //!!! check plugin & API version (see e.g. PluginXml::setParameters)

    //!!! check that this is the right plugin!

    RealTimePluginInstance *rtpi =
        dynamic_cast<RealTimePluginInstance *>(plugin);

    if (rtpi) {
        const Transform::ConfigurationMap &cmap = transform.getConfiguration();
        for (Transform::ConfigurationMap::const_iterator i = cmap.begin();
             i != cmap.end(); ++i) {
            rtpi->configure(i->first.toStdString(), i->second.toStdString());
        }
    }

    if (transform.getProgram() != "") {
        plugin->selectProgram(transform.getProgram().toStdString());
    }

    const Transform::ParameterMap &pmap = transform.getParameters();

    Vamp::PluginBase::ParameterList parameters =
        plugin->getParameterDescriptors();

    for (Vamp::PluginBase::ParameterList::const_iterator i = parameters.begin();
         i != parameters.end(); ++i) {
        QString key = i->identifier.c_str();
        Transform::ParameterMap::const_iterator pmi = pmap.find(key);
        if (pmi != pmap.end()) {
            plugin->setParameter(i->identifier, pmi->second);
        }
    }
}

void
TransformFactory::makeContextConsistentWithPlugin(Transform &transform,
                                                  Vamp::PluginBase *plugin)
{
    const Vamp::Plugin *vp = downcastVampPlugin(plugin);

    if (!vp) {
        // time domain input for real-time effects plugin
        if (!transform.getBlockSize()) {
            if (!transform.getStepSize()) transform.setStepSize(1024);
            transform.setBlockSize(transform.getStepSize());
        } else {
            transform.setStepSize(transform.getBlockSize());
        }
    } else {
        Vamp::Plugin::InputDomain domain = vp->getInputDomain();
        if (!transform.getStepSize()) {
            transform.setStepSize((int)vp->getPreferredStepSize());
        }
        if (!transform.getBlockSize()) {
            transform.setBlockSize((int)vp->getPreferredBlockSize());
        }
        if (!transform.getBlockSize()) {
            transform.setBlockSize(1024);
        }
        if (!transform.getStepSize()) {
            if (domain == Vamp::Plugin::FrequencyDomain) {
//                cerr << "frequency domain, step = " << blockSize/2 << endl;
                transform.setStepSize(transform.getBlockSize()/2);
            } else {
//                cerr << "time domain, step = " << blockSize/2 << endl;
                transform.setStepSize(transform.getBlockSize());
            }
        }
    }
}

QString
TransformFactory::getPluginConfigurationXml(const Transform &t)
{
    QString xml;

    Vamp::PluginBase *plugin = instantiateDefaultPluginFor
        (t.getIdentifier(), 0);
    if (!plugin) {
        cerr << "TransformFactory::getPluginConfigurationXml: "
                  << "Unable to instantiate plugin for transform \""
                  << t.getIdentifier() << "\"" << endl;
        return xml;
    }

    setPluginParameters(t, plugin);

    QTextStream out(&xml);
    PluginXml(plugin).toXml(out);
    delete plugin;

    return xml;
}

void
TransformFactory::setParametersFromPluginConfigurationXml(Transform &t,
                                                          QString xml)
{
    Vamp::PluginBase *plugin = instantiateDefaultPluginFor
        (t.getIdentifier(), 0);
    if (!plugin) {
        cerr << "TransformFactory::setParametersFromPluginConfigurationXml: "
                  << "Unable to instantiate plugin for transform \""
                  << t.getIdentifier() << "\"" << endl;
        return;
    }

    PluginXml(plugin).setParametersFromXml(xml);
    setParametersFromPlugin(t, plugin);
    delete plugin;
}

TransformFactory::SearchResults
TransformFactory::search(QString keyword)
{
    QStringList keywords;
    keywords << keyword;
    return search(keywords);
}

TransformFactory::SearchResults
TransformFactory::search(QStringList keywords)
{
    populateTransforms();

    if (keywords.size() > 1) {
        // Additional score for all keywords in a row
        keywords.push_back(keywords.join(" "));
    }

    SearchResults results;
    TextMatcher matcher;

    for (TransformDescriptionMap::const_iterator i = m_transforms.begin();
         i != m_transforms.end(); ++i) {

        TextMatcher::Match match;

        match.key = i->first;
        
        matcher.test(match, keywords,
                     getTransformTypeName(i->second.type),
                     tr("Plugin type"), 5);

        matcher.test(match, keywords, i->second.category, tr("Category"), 20);
        matcher.test(match, keywords, i->second.identifier, tr("System Identifier"), 6);
        matcher.test(match, keywords, i->second.name, tr("Name"), 30);
        matcher.test(match, keywords, i->second.description, tr("Description"), 20);
        matcher.test(match, keywords, i->second.maker, tr("Maker"), 10);
        matcher.test(match, keywords, i->second.units, tr("Units"), 10);

        if (match.score > 0) results[i->first] = match;
    }

    if (!m_uninstalledTransformsMutex.tryLock()) {
        // uninstalled transforms are being populated; this may take some time,
        // and they aren't critical, but we will speed them up if necessary
        cerr << "TransformFactory::search: Uninstalled transforms mutex is held, skipping" << endl;
        m_populatingSlowly = false;
        return results;
    }

    if (!m_uninstalledTransformsPopulated) {
        cerr << "WARNING: TransformFactory::search: Uninstalled transforms are not populated yet" << endl
                  << "and are not being populated either -- was the thread not started correctly?" << endl;
        m_uninstalledTransformsMutex.unlock();
        return results;
    }

    m_uninstalledTransformsMutex.unlock();

    for (TransformDescriptionMap::const_iterator i = m_uninstalledTransforms.begin();
         i != m_uninstalledTransforms.end(); ++i) {

        TextMatcher::Match match;

        match.key = i->first;
        
        matcher.test(match, keywords,
                     getTransformTypeName(i->second.type),
                     tr("Plugin type"), 2);

        matcher.test(match, keywords, i->second.category, tr("Category"), 10);
        matcher.test(match, keywords, i->second.identifier, tr("System Identifier"), 3);
        matcher.test(match, keywords, i->second.name, tr("Name"), 15);
        matcher.test(match, keywords, i->second.description, tr("Description"), 10);
        matcher.test(match, keywords, i->second.maker, tr("Maker"), 5);
        matcher.test(match, keywords, i->second.units, tr("Units"), 5);

        if (match.score > 0) results[i->first] = match;
    }

    return results;
}

