/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008-2012 QMUL.
   
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "PluginRDFDescription.h"

#include "PluginRDFIndexer.h"

#include "base/Profiler.h"

#include "plugin/PluginIdentifier.h"

#include <dataquay/BasicStore.h>

#include <iostream>

using Dataquay::Uri;
using Dataquay::Node;
using Dataquay::Nodes;
using Dataquay::Triple;
using Dataquay::Triples;
using Dataquay::BasicStore;

PluginRDFDescription::PluginRDFDescription(QString pluginId) :
    m_pluginId(pluginId),
    m_haveDescription(false)
{
    PluginRDFIndexer *indexer = PluginRDFIndexer::getInstance();
    m_pluginUri = indexer->getURIForPluginId(pluginId);
    if (m_pluginUri == "") {
        cerr << "PluginRDFDescription: WARNING: No RDF description available for plugin ID \""
             << pluginId << "\"" << endl;
    } else {
        // All the data we need should be in our RDF model already:
        // if it's not there, we don't know where to find it anyway
        if (index()) {
            m_haveDescription = true;
        }
    }
}

PluginRDFDescription::~PluginRDFDescription()
{
}

bool
PluginRDFDescription::haveDescription() const
{
    return m_haveDescription;
}

QString
PluginRDFDescription::getPluginName() const
{
    return m_pluginName;
}

QString
PluginRDFDescription::getPluginDescription() const
{
    return m_pluginDescription;
}

QString
PluginRDFDescription::getPluginMaker() const
{
    return m_pluginMaker;
}

QString
PluginRDFDescription::getPluginInfoURL() const
{
    return m_pluginInfoURL;
}

QStringList
PluginRDFDescription::getOutputIds() const
{
    QStringList ids;
    for (OutputDispositionMap::const_iterator i = m_outputDispositions.begin();
         i != m_outputDispositions.end(); ++i) {
        ids.push_back(i->first);
    }
    return ids;
}

QString
PluginRDFDescription::getOutputName(QString outputId) const
{
    if (m_outputNames.find(outputId) == m_outputNames.end()) {
        return "";
    } 
    return m_outputNames.find(outputId)->second;
}

PluginRDFDescription::OutputDisposition
PluginRDFDescription::getOutputDisposition(QString outputId) const
{
    if (m_outputDispositions.find(outputId) == m_outputDispositions.end()) {
        return OutputDispositionUnknown;
    }
    return m_outputDispositions.find(outputId)->second;
}

QString
PluginRDFDescription::getOutputEventTypeURI(QString outputId) const
{
    if (m_outputEventTypeURIMap.find(outputId) ==
        m_outputEventTypeURIMap.end()) {
        return "";
    }
    return m_outputEventTypeURIMap.find(outputId)->second;
}

QString
PluginRDFDescription::getOutputFeatureAttributeURI(QString outputId) const
{
    if (m_outputFeatureAttributeURIMap.find(outputId) ==
        m_outputFeatureAttributeURIMap.end()) {
        return "";
    }
    return m_outputFeatureAttributeURIMap.find(outputId)->second;
}

QString
PluginRDFDescription::getOutputSignalTypeURI(QString outputId) const
{
    if (m_outputSignalTypeURIMap.find(outputId) ==
        m_outputSignalTypeURIMap.end()) {
        return "";
    }
    return m_outputSignalTypeURIMap.find(outputId)->second;
}

QString
PluginRDFDescription::getOutputUnit(QString outputId) const
{
    if (m_outputUnitMap.find(outputId) == m_outputUnitMap.end()) {
        return "";
    }
    return m_outputUnitMap.find(outputId)->second;
}

QString
PluginRDFDescription::getOutputUri(QString outputId) const
{
    if (m_outputUriMap.find(outputId) == m_outputUriMap.end()) {
        return "";
    }
    return m_outputUriMap.find(outputId)->second;
}

bool
PluginRDFDescription::index() 
{
    Profiler profiler("PluginRDFDescription::index");

    bool success = true;
    if (!indexMetadata()) success = false;
    if (!indexOutputs()) success = false;

    return success;
}

bool
PluginRDFDescription::indexMetadata()
{
    Profiler profiler("PluginRDFDescription::index");

    PluginRDFIndexer *indexer = PluginRDFIndexer::getInstance();
    const BasicStore *index = indexer->getIndex();
    Uri plugin(m_pluginUri);

    Node n = index->complete
        (Triple(plugin, index->expand("vamp:name"), Node()));

    if (n.type == Node::Literal && n.value != "") {
        m_pluginName = n.value;
    }

    n = index->complete
        (Triple(plugin, index->expand("dc:description"), Node()));

    if (n.type == Node::Literal && n.value != "") {
        m_pluginDescription = n.value;
    }

    n = index->complete
        (Triple(plugin, index->expand("foaf:maker"), Node()));

    if (n.type == Node::URI || n.type == Node::Blank) {
        n = index->complete(Triple(n, index->expand("foaf:name"), Node()));
        if (n.type == Node::Literal && n.value != "") {
            m_pluginMaker = n.value;
        }
    }

    // If we have a more-information URL for this plugin, then we take
    // that.  Otherwise, a more-information URL for the plugin library
    // would do nicely.

    n = index->complete
        (Triple(plugin, index->expand("foaf:page"), Node()));

    if (n.type == Node::URI && n.value != "") {
        m_pluginInfoURL = n.value;
    }

    n = index->complete
        (Triple(Node(), index->expand("vamp:available_plugin"), plugin));

    if (n.value != "") {
        n = index->complete(Triple(n, index->expand("foaf:page"), Node()));
        if (n.type == Node::URI && n.value != "") {
            m_pluginInfoURL = n.value;
        }
    }

    return true;
}

bool
PluginRDFDescription::indexOutputs()
{
    Profiler profiler("PluginRDFDescription::indexOutputs");
    
    PluginRDFIndexer *indexer = PluginRDFIndexer::getInstance();
    const BasicStore *index = indexer->getIndex();
    Uri plugin(m_pluginUri);

    Nodes outputs = index->match
        (Triple(plugin, index->expand("vamp:output"), Node())).objects();

    if (outputs.empty()) {
        cerr << "ERROR: PluginRDFDescription::indexURL: NOTE: No outputs defined for <"
             << m_pluginUri << ">" << endl;
        return false;
    }

    foreach (Node output, outputs) {

        if ((output.type != Node::URI && output.type != Node::Blank) ||
            output.value == "") {
            cerr << "ERROR: PluginRDFDescription::indexURL: No valid URI for output " << output << " of plugin <" << m_pluginUri << ">" << endl;
            return false;
        }
        
        Node n = index->complete(Triple(output, index->expand("vamp:identifier"), Node()));
        if (n.type != Node::Literal || n.value == "") {
            cerr << "ERROR: PluginRDFDescription::indexURL: No vamp:identifier for output <" << output << ">" << endl;
            return false;
        }
        QString outputId = n.value;

        m_outputUriMap[outputId] = output.value;

        n = index->complete(Triple(output, Uri("a"), Node()));
        QString outputType;
        if (n.type == Node::URI) outputType = n.value;

        n = index->complete(Triple(output, index->expand("vamp:unit"), Node()));
        QString outputUnit;
        if (n.type == Node::Literal) outputUnit = n.value;

        if (outputType.contains("DenseOutput")) {
            m_outputDispositions[outputId] = OutputDense;
        } else if (outputType.contains("SparseOutput")) {
            m_outputDispositions[outputId] = OutputSparse;
        } else if (outputType.contains("TrackLevelOutput")) {
            m_outputDispositions[outputId] = OutputTrackLevel;
        } else {
            m_outputDispositions[outputId] = OutputDispositionUnknown;
        }
//        cerr << "output " << output << " -> id " << outputId << ", type " << outputType << ", unit " 
//             << outputUnit << ", disposition " << m_outputDispositions[outputId] << endl;
            
        if (outputUnit != "") {
            m_outputUnitMap[outputId] = outputUnit;
        }

        n = index->complete(Triple(output, index->expand("dc:title"), Node()));
        if (n.type == Node::Literal && n.value != "") {
            m_outputNames[outputId] = n.value;
        }

        n = index->complete(Triple(output, index->expand("vamp:computes_event_type"), Node()));
//        cerr << output << " -> computes_event_type " << n << endl;
        if (n.type == Node::URI && n.value != "") {
            m_outputEventTypeURIMap[outputId] = n.value;
        }

        n = index->complete(Triple(output, index->expand("vamp:computes_feature"), Node()));
        if (n.type == Node::URI && n.value != "") {
            m_outputFeatureAttributeURIMap[outputId] = n.value;
        }

        n = index->complete(Triple(output, index->expand("vamp:computes_signal_type"), Node()));
        if (n.type == Node::URI && n.value != "") {
            m_outputSignalTypeURIMap[outputId] = n.value;
        }
    }

    return true;
}

