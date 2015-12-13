/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
   
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _PLUGIN_RDF_DESCRIPTION_H_
#define _PLUGIN_RDF_DESCRIPTION_H_

#include <QString>
#include <QStringList>
#include <map>

#include "base/Debug.h"

class PluginRDFDescription
{
public:
    PluginRDFDescription() : m_haveDescription(false) { }
    PluginRDFDescription(QString pluginId);
    ~PluginRDFDescription();

    enum OutputDisposition
    {
        OutputDispositionUnknown,
        OutputSparse,
        OutputDense,
        OutputTrackLevel
    };

    bool haveDescription() const;

    QString getPluginName() const;
    QString getPluginDescription() const;
    QString getPluginMaker() const;
    QString getPluginInfoURL() const;

    QStringList getOutputIds() const;
    QString getOutputName(QString outputId) const;
    OutputDisposition getOutputDisposition(QString outputId) const;
    QString getOutputEventTypeURI(QString outputId) const;
    QString getOutputFeatureAttributeURI(QString outputId) const;
    QString getOutputSignalTypeURI(QString outputId) const;
    QString getOutputUnit(QString outputId) const;
    QString getOutputUri(QString outputId) const;

protected:    
    typedef std::map<QString, OutputDisposition> OutputDispositionMap;
    typedef std::map<QString, QString> OutputStringMap;

    QString m_pluginId;
    QString m_pluginUri;
    bool m_haveDescription;
    QString m_pluginName;
    QString m_pluginDescription;
    QString m_pluginMaker;
    QString m_pluginInfoURL;
    OutputStringMap m_outputNames;
    OutputDispositionMap m_outputDispositions;
    OutputStringMap m_outputEventTypeURIMap;
    OutputStringMap m_outputFeatureAttributeURIMap;
    OutputStringMap m_outputSignalTypeURIMap;
    OutputStringMap m_outputUnitMap;
    OutputStringMap m_outputUriMap;
    bool index();
    bool indexMetadata();
    bool indexOutputs();
};

#endif

