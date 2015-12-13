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

#ifndef _PLUGIN_XML_H_
#define _PLUGIN_XML_H_

#include "base/XmlExportable.h"
#include <vamp-hostsdk/PluginBase.h>

class QXmlAttributes;

class PluginXml : public XmlExportable
{
public:
    PluginXml(Vamp::PluginBase *plugin);
    virtual ~PluginXml();

    /**
     * Export plugin settings to XML.
     */
    virtual void toXml(QTextStream &stream,
                       QString indent = "",
                       QString extraAttributes = "") const;

    /**
     * Set the parameters and program of a plugin from a set of XML
     * attributes.  This is a partial inverse of toXml.
     */
    virtual void setParameters(const QXmlAttributes &);

    /**
     * Set the parameters and program of a plugin from an XML plugin
     * element as returned by toXml.  This is a partial inverse of
     * toXml.
     */
    virtual void setParametersFromXml(QString xml);

    static QString encodeConfigurationChars(QString text);
    static QString decodeConfigurationChars(QString text);

protected:
    QString stripInvalidParameterNameCharacters(QString) const;

    Vamp::PluginBase *m_plugin;
};

#endif
