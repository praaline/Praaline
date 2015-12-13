/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2007 Chris Cannam and QMUL.
   
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _TRANSFORM_FACTORY_H_
#define _TRANSFORM_FACTORY_H_

#include "TransformDescription.h"

#include "base/TextMatcher.h"

#include <vamp-hostsdk/Plugin.h>

#include <QObject>
#include <QStringList>
#include <QThread>
#include <QMutex>

#include <map>
#include <set>

class TransformFactory : public QObject
{
    Q_OBJECT

public:
    TransformFactory();
    virtual ~TransformFactory();

    static TransformFactory *getInstance();
    static void deleteInstance(); // only when exiting

    /**
     * TransformFactory has a background thread that can populate
     * uninstalled transforms from network RDF resources.  It is not
     * started by default, but it's a good idea to start it when the
     * program starts up, if the uninstalled transforms may be of use
     * later; otherwise there will be a bottleneck the first time
     * they're requested.
     *
     * If this thread is not already running, start it now.
     */
    void startPopulationThread();

    TransformList getAllTransformDescriptions();
    TransformDescription getTransformDescription(TransformId id);
    bool haveInstalledTransforms();

    TransformList getUninstalledTransformDescriptions();
    TransformDescription getUninstalledTransformDescription(TransformId id);
    bool haveUninstalledTransforms(bool waitForCheckToComplete = false);
    
    typedef enum {
        TransformUnknown,
        TransformInstalled,
        TransformNotInstalled
    } TransformInstallStatus;

    TransformInstallStatus getTransformInstallStatus(TransformId id);

    std::vector<TransformDescription::Type> getAllTransformTypes();
    std::vector<QString> getTransformCategories(TransformDescription::Type);
    std::vector<QString> getTransformMakers(TransformDescription::Type);
    QString getTransformTypeName(TransformDescription::Type) const;

    typedef std::map<TransformId, TextMatcher::Match> SearchResults;
    SearchResults search(QString keyword);
    SearchResults search(QStringList keywords);
    
    /**
     * Return true if the given transform is known.
     */
    bool haveTransform(TransformId identifier);

    /**
     * A single transform ID can lead to many possible Transforms,
     * with different parameters and execution context settings.
     * Return the default one for the given transform.
     */
    Transform getDefaultTransformFor(TransformId identifier, sv_samplerate_t rate = 0);

    /**
     * Full name of a transform, suitable for putting on a menu.
     */
    QString getTransformName(TransformId identifier);

    /**
     * Brief but friendly name of a transform, suitable for use
     * as the name of the output layer.
     */
    QString getTransformFriendlyName(TransformId identifier);

    QString getTransformUnits(TransformId identifier);

    QString getTransformInfoUrl(TransformId identifier);

    Vamp::Plugin::InputDomain getTransformInputDomain(TransformId identifier);

    /**
     * Return true if the transform has any configurable parameters,
     * i.e. if getConfigurationForTransform can ever return a non-trivial
     * (not equivalent to empty) configuration string.
     */
    bool isTransformConfigurable(TransformId identifier);

    /**
     * If the transform has a prescribed number or range of channel
     * inputs, return true and set minChannels and maxChannels to the
     * minimum and maximum number of channel inputs the transform can
     * accept.  Return false if it doesn't care.
     */
    bool getTransformChannelRange(TransformId identifier,
                                  int &minChannels, int &maxChannels);

    /**
     * Load an appropriate plugin for the given transform and set the
     * parameters, program and configuration strings on that plugin
     * from the Transform object.
     *
     * Note that this requires that the transform has a meaningful
     * sample rate set, as that is used as the rate for the plugin.  A
     * Transform can legitimately have rate set at zero (= "use the
     * rate of the input source"), so the caller will need to test for
     * this case.
     *
     * Returns the plugin thus loaded.  This will be a
     * Vamp::PluginBase, but not necessarily a Vamp::Plugin (only if
     * the transform was a feature-extraction type -- call
     * downcastVampPlugin if you only want Vamp::Plugins).  Returns
     * NULL if no suitable plugin was available.
     *
     * The returned plugin is owned by the caller, and should be
     * deleted (using "delete") when no longer needed.
     */
    Vamp::PluginBase *instantiatePluginFor(const Transform &transform);

    /**
     * Convert a Vamp::PluginBase to a Vamp::Plugin, if it is one.
     * Return NULL otherwise.  This ill-fitting convenience function
     * is really just a dynamic_cast wrapper.
     */
    Vamp::Plugin *downcastVampPlugin(Vamp::PluginBase *);

    /**
     * Set the plugin parameters, program and configuration strings on
     * the given Transform object from the given plugin instance.
     * Note that no check is made whether the plugin is actually the
     * "correct" one for the transform.
     */
    void setParametersFromPlugin(Transform &transform, Vamp::PluginBase *plugin);

    /**
     * Set the parameters, program and configuration strings on the
     * given plugin from the given Transform object.
     */
    void setPluginParameters(const Transform &transform, Vamp::PluginBase *plugin);
    
    /**
     * If the given Transform object has no processing step and block
     * sizes set, set them to appropriate defaults for the given
     * plugin.
     */
    void makeContextConsistentWithPlugin(Transform &transform, Vamp::PluginBase *plugin); 

    /**
     * Retrieve a <plugin ... /> XML fragment that describes the
     * plugin parameters, program and configuration data for the given
     * transform.
     *
     * This function is provided for backward compatibility only.  Use
     * Transform::toXml where compatibility with PluginXml
     * descriptions of transforms is not required.
     */
    QString getPluginConfigurationXml(const Transform &transform);

    /**
     * Set the plugin parameters, program and configuration strings on
     * the given Transform object from the given <plugin ... /> XML
     * fragment.
     *
     * This function is provided for backward compatibility only.  Use
     * Transform(QString) where compatibility with PluginXml
     * descriptions of transforms is not required.
     */
    void setParametersFromPluginConfigurationXml(Transform &transform,
                                                 QString xml);

protected:
    typedef std::map<TransformId, TransformDescription> TransformDescriptionMap;

    TransformDescriptionMap m_transforms;
    bool m_transformsPopulated;

    TransformDescriptionMap m_uninstalledTransforms;
    bool m_uninstalledTransformsPopulated;

    void populateTransforms();
    void populateUninstalledTransforms();
    void populateFeatureExtractionPlugins(TransformDescriptionMap &);
    void populateRealTimePlugins(TransformDescriptionMap &);

    Vamp::PluginBase *instantiateDefaultPluginFor(TransformId id, sv_samplerate_t rate);
    QMutex m_transformsMutex;
    QMutex m_uninstalledTransformsMutex;

    class UninstalledTransformsPopulateThread : public QThread
    {
    public:
        UninstalledTransformsPopulateThread(TransformFactory *factory) :
            m_factory(factory) {
        }
        virtual void run();
        TransformFactory *m_factory;
    };

    UninstalledTransformsPopulateThread *m_thread;
    bool m_exiting;
    bool m_populatingSlowly;

    static TransformFactory *m_instance;
};


#endif
