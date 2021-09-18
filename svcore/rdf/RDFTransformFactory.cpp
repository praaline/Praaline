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

#include "RDFTransformFactory.h"

#include <map>
#include <vector>

#include <QTextStream>
#include <QUrl>

#include <iostream>
#include <cmath>

#include "PluginRDFIndexer.h"
#include "PluginRDFDescription.h"
#include "base/ProgressReporter.h"
#include "plugin/PluginIdentifier.h"

#include "transform/TransformFactory.h"

#include <dataquay/BasicStore.h>
#include <dataquay/PropertyObject.h>

using Dataquay::Uri;
using Dataquay::Node;
using Dataquay::Nodes;
using Dataquay::Triple;
using Dataquay::Triples;
using Dataquay::BasicStore;
using Dataquay::PropertyObject;


class RDFTransformFactoryImpl
{
public:
    RDFTransformFactoryImpl(QString url);
    virtual ~RDFTransformFactoryImpl();
    
    bool isRDF();
    bool isOK();
    QString getErrorString() const;

    std::vector<Transform> getTransforms(ProgressReporter *);

    static QString writeTransformToRDF(const Transform &, QString);

protected:
    BasicStore *m_store;
    QString m_urlString;
    QString m_errorString;
    bool m_isRDF;
    bool setOutput(Transform &, QString);
    bool setParameters(Transform &, QString);
};


QString
RDFTransformFactory::getKnownExtensions()
{
    return "*.rdf *.n3 *.ttl";
}

RDFTransformFactory::RDFTransformFactory(QString url) :
    m_d(new RDFTransformFactoryImpl(url)) 
{
}

RDFTransformFactory::~RDFTransformFactory()
{
    delete m_d;
}

bool
RDFTransformFactory::isRDF()
{
    return m_d->isRDF();
}

bool
RDFTransformFactory::isOK()
{
    return m_d->isOK();
}

QString
RDFTransformFactory::getErrorString() const
{
    return m_d->getErrorString();
}

std::vector<Transform>
RDFTransformFactory::getTransforms(ProgressReporter *r)
{
    return m_d->getTransforms(r);
}

QString
RDFTransformFactory::writeTransformToRDF(const Transform &t, QString f)
{
    return RDFTransformFactoryImpl::writeTransformToRDF(t, f);
}

RDFTransformFactoryImpl::RDFTransformFactoryImpl(QString url) :
    m_store(new BasicStore),
    m_urlString(url),
    m_isRDF(false)
{
    //!!! retrieve data if remote... then
    m_store->addPrefix("vamp", Uri("http://purl.org/ontology/vamp/"));
    try {
        QUrl qurl;
        if (url.startsWith("file:")) {
            qurl = QUrl(url);
        } else {
            qurl = QUrl::fromLocalFile(url);
        }
        m_store->import(qurl, BasicStore::ImportIgnoreDuplicates);
        m_isRDF = true;
    } catch (...) { }
}

RDFTransformFactoryImpl::~RDFTransformFactoryImpl()
{
    delete m_store;
}

bool
RDFTransformFactoryImpl::isRDF()
{
    return m_isRDF;
}

bool
RDFTransformFactoryImpl::isOK()
{
    return (m_errorString == "");
}

QString
RDFTransformFactoryImpl::getErrorString() const
{
    return m_errorString;
}

std::vector<Transform>
RDFTransformFactoryImpl::getTransforms(ProgressReporter *)
{
    std::vector<Transform> transforms;

    std::map<QString, Transform> uriTransformMap;

    Nodes tnodes = m_store->match
        (Triple(Node(), Uri("a"), m_store->expand("vamp:Transform"))).subjects();

    PluginRDFIndexer *indexer = PluginRDFIndexer::getInstance();

    foreach (Node tnode, tnodes) {

        Node pnode = m_store->complete
            (Triple(tnode, m_store->expand("vamp:plugin"), Node()));

        if (pnode == Node()) {
            cerr << "RDFTransformFactory: WARNING: No vamp:plugin for "
                 << "vamp:Transform node " << tnode
                 << ", skipping this transform" << endl;
            continue;
        }

        QString transformUri = tnode.value;
        QString pluginUri = pnode.value;

        QString pluginId = indexer->getIdForPluginURI(pluginUri);
        if (pluginId == "") {
            cerr << "RDFTransformFactory: WARNING: Unknown plugin <"
                 << pluginUri << "> for transform <"
                 << transformUri << ">, skipping this transform"
                 << endl;
            continue;
        }

        Transform transform;
        transform.setPluginIdentifier(pluginId);

        if (!setOutput(transform, transformUri)) {
            return transforms;
        }

        if (!setParameters(transform, transformUri)) {
            return transforms;
        }

        uriTransformMap[transformUri] = transform;

        static const char *optionals[] = {
            "program",
            "summary_type",
            "step_size",
            "block_size",
            "window_type",
            "sample_rate",
            "start", 
            "duration",
            "plugin_version"
        };
        
        for (int j = 0; j < int(sizeof(optionals)/sizeof(optionals[0])); ++j) {

            QString optional = optionals[j];

            Node onode = m_store->complete
                (Triple(Uri(transformUri),
                        m_store->expand(QString("vamp:") + optional), Node()));

            if (onode.type != Node::Literal) continue;

            if (optional == "program") {
                transform.setProgram(onode.value);
            } else if (optional == "summary_type") {
                transform.setSummaryType
                    (transform.stringToSummaryType(onode.value));
            } else if (optional == "step_size") {
                transform.setStepSize(onode.value.toUInt());
            } else if (optional == "block_size") {
                transform.setBlockSize(onode.value.toUInt());
            } else if (optional == "window_type") {
                transform.setWindowType
                    (Window<float>::getTypeForName
                     (onode.value.toLower().toStdString()));
            } else if (optional == "sample_rate") {
                transform.setSampleRate(onode.value.toFloat());
            } else if (optional == "start") {
                RealTime start = RealTime::fromXsdDuration(onode.value.toStdString());
                transform.setStartTime(start);
            } else if (optional == "duration") {
                RealTime duration = RealTime::fromXsdDuration(onode.value.toStdString());
                transform.setDuration(duration);
                if (duration == RealTime::zeroTime) {
                    cerr << "\nRDFTransformFactory: WARNING: Duration is specified as \"" << onode.value << "\" in RDF file,\n    but this evaluates to zero when parsed as an xsd:duration datatype.\n    The duration property will therefore be ignored.\n    To specify start time and duration use the xsd:duration format,\n    for example \"PT2.5S\"^^xsd:duration (for 2.5 seconds).\n\n";
                }
            } else if (optional == "plugin_version") {
                transform.setPluginVersion(onode.value);
            } else {
                cerr << "RDFTransformFactory: ERROR: Inconsistent optionals lists (unexpected optional \"" << optional << "\"" << endl;
            }
        }

        cerr << "RDFTransformFactory: NOTE: Transform is: " << endl;
        cerr << transform.toXmlString() << endl;

        transforms.push_back(transform);
    }
        
    return transforms;
}

bool
RDFTransformFactoryImpl::setOutput(Transform &transform,
                                   QString transformUri)
{
    Node outputNode = m_store->complete
        (Triple(Uri(transformUri), m_store->expand("vamp:output"), Node()));
    
    if (outputNode == Node()) return true;

    if (outputNode.type != Node::URI && outputNode.type != Node::Blank) {
        m_errorString = QString("vamp:output for output of transform <%1> is not a URI or blank node").arg(transformUri);
        return false;
    }

    // Now, outputNode might be the subject of a triple within m_store
    // that tells us the vamp:identifier, or it might be the subject
    // of a triple within the indexer that tells us it

    Node identNode = m_store->complete
        (Triple(outputNode, m_store->expand("vamp:identifier"), Node()));

    if (identNode == Node()) {
        PluginRDFIndexer *indexer = PluginRDFIndexer::getInstance();
        const BasicStore *index = indexer->getIndex();
        identNode = index->complete
            (Triple(outputNode, index->expand("vamp:identifier"), Node()));
    }

    if (identNode == Node() || identNode.type != Node::Literal) {
        m_errorString = QString("No vamp:identifier found for output of transform <%1>, or vamp:identifier is not a literal").arg(transformUri);
        return false;
    }

    transform.setOutput(identNode.value);

    return true;
}
        

bool
RDFTransformFactoryImpl::setParameters(Transform &transform,
                                       QString transformUri)
{
    Nodes bindings = m_store->match
        (Triple(Uri(transformUri), m_store->expand("vamp:parameter_binding"), Node())).objects();
    
    foreach (Node binding, bindings) {

        Node paramNode = m_store->complete
            (Triple(binding, m_store->expand("vamp:parameter"), Node()));

        if (paramNode == Node()) {
            cerr << "RDFTransformFactoryImpl::setParameters: No vamp:parameter for binding " << binding << endl;
            continue;
        }

        Node valueNode = m_store->complete
            (Triple(binding, m_store->expand("vamp:value"), Node()));

        if (paramNode == Node()) {
            cerr << "RDFTransformFactoryImpl::setParameters: No vamp:value for binding " << binding << endl;
            continue;
        }
        
        // As with output above, paramNode might be the subject of a
        // triple within m_store that tells us the vamp:identifier, or
        // it might be the subject of a triple within the indexer that
        // tells us it

        Node idNode = m_store->complete
            (Triple(paramNode, m_store->expand("vamp:identifier"), Node()));

        if (idNode == Node()) {
            PluginRDFIndexer *indexer = PluginRDFIndexer::getInstance();
            const BasicStore *index = indexer->getIndex();
            idNode = index->complete
                (Triple(paramNode, index->expand("vamp:identifier"), Node()));
        }

        if (idNode == Node() || idNode.type != Node::Literal) {
            cerr << "RDFTransformFactoryImpl::setParameters: No vamp:identifier for parameter " << paramNode << endl;
            continue;
        }
        
        transform.setParameter(idNode.value, valueNode.value.toFloat());
    }

    return true;
}

QString
RDFTransformFactoryImpl::writeTransformToRDF(const Transform &transform,
                                             QString uri)
{
    QString str;
    QTextStream s(&str);

    // assumes the usual prefixes are available; requires that uri be
    // a local fragment (e.g. ":transform") rather than a uri enclosed
    // in <>, so that we can suffix it if need be

    QString pluginId = transform.getPluginIdentifier();
    QString pluginUri = PluginRDFIndexer::getInstance()->getURIForPluginId(pluginId);

    if (pluginUri != "") {
        s << uri << " a vamp:Transform ;" << Qt::endl;
        s << "    vamp:plugin <" << QUrl(pluginUri).toEncoded().data() << "> ;" << Qt::endl;
    } else {
        cerr << "WARNING: RDFTransformFactory::writeTransformToRDF: No plugin URI available for plugin id \"" << pluginId << "\", writing synthetic plugin and library resources" << endl;
        QString type, soname, label;
        PluginIdentifier::parseIdentifier(pluginId, type, soname, label);
        s << uri << "_plugin a vamp:Plugin ;" << Qt::endl;
        s << "    vamp:identifier \"" << label << "\" .\n" << Qt::endl;
        s << uri << "_library a vamp:PluginLibrary ;" << Qt::endl;
        s << "    vamp:identifier \"" << soname << "\" ;" << Qt::endl;
        s << "    vamp:available_plugin " << uri << "_plugin .\n" << Qt::endl;
        s << uri << " a vamp:Transform ;" << Qt::endl;
        s << "    vamp:plugin " << uri << "_plugin ;" << Qt::endl;
    }

    PluginRDFDescription description(pluginId);
    QString outputId = transform.getOutput();
    QString outputUri = description.getOutputUri(outputId);

    if (transform.getOutput() != "" && outputUri == "") {
        cerr << "WARNING: RDFTransformFactory::writeTransformToRDF: No output URI available for transform output id \"" << transform.getOutput() << "\", writing a synthetic output resource" << endl;
    }

    if (transform.getStepSize() != 0) {
        s << "    vamp:step_size \"" << transform.getStepSize() << "\"^^xsd:int ; " << Qt::endl;
    }
    if (transform.getBlockSize() != 0) {
        s << "    vamp:block_size \"" << transform.getBlockSize() << "\"^^xsd:int ; " << Qt::endl;
    }
    if (transform.getWindowType() != HanningWindow) {
        s << "    vamp:window_type \"" <<
            Window<float>::getNameForType(transform.getWindowType()).c_str()
          << "\" ; " << endl;
    }
    if (transform.getStartTime() != RealTime::zeroTime) {
        s << "    vamp:start \"" << transform.getStartTime().toXsdDuration().c_str() << "\"^^xsd:duration ; " << Qt::endl;
    }
    if (transform.getDuration() != RealTime::zeroTime) {
        s << "    vamp:duration \"" << transform.getDuration().toXsdDuration().c_str() << "\"^^xsd:duration ; " << Qt::endl;
    }
    if (transform.getSampleRate() != 0) {
        s << "    vamp:sample_rate \"" << transform.getSampleRate() << "\"^^xsd:float ; " << Qt::endl;
    }
    if (transform.getPluginVersion() != "") {
        s << "    vamp:plugin_version \"\"\"" << transform.getPluginVersion() << "\"\"\" ; " << Qt::endl;
    }
    
    QString program = transform.getProgram();
    if (program != "") {
        s << "    vamp:program \"\"\"" << program << "\"\"\" ;" << Qt::endl;
    }

    QString summary = transform.summaryTypeToString(transform.getSummaryType());
    if (summary != "") {
        s << "    vamp:summary_type \"" << summary << "\" ;" << Qt::endl;
    }

    Transform::ParameterMap parameters = transform.getParameters();
    for (Transform::ParameterMap::const_iterator i = parameters.begin();
         i != parameters.end(); ++i) {
        QString name = i->first;
        float value = i->second;
        s << "    vamp:parameter_binding [" << Qt::endl;
        s << "        vamp:parameter [ vamp:identifier \"" << name << "\" ] ;" << Qt::endl;
        s << "        vamp:value \"" << value << "\"^^xsd:float ;" << Qt::endl;
        s << "    ] ;" << Qt::endl;
    }

    if (outputUri != "") {
        s << "    vamp:output <" << QUrl(outputUri).toEncoded().data() << "> ." << Qt::endl;
    } else if (outputId != "") {
        s << "    vamp:output [ vamp:identifier \"" << outputId << "\" ] ." << Qt::endl;
    } else {
        s << "    ." << Qt::endl;
    }

    return str;
}

