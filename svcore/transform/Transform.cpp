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

#include "Transform.h"

#include "plugin/PluginIdentifier.h"

#include "plugin/FeatureExtractionPluginFactory.h"

#include <QXmlAttributes>

#include <QDomDocument>
#include <QDomElement>
#include <QDomNamedNodeMap>
#include <QDomAttr>

#include <QTextStream>

#include <iostream>

Transform::Transform() :
    m_summaryType(NoSummary),
    m_stepSize(0),
    m_blockSize(0),
    m_windowType(HanningWindow),
    m_sampleRate(0)
{
}

Transform::Transform(QString xml) :
    m_summaryType(NoSummary),
    m_stepSize(0),
    m_blockSize(0),
    m_windowType(HanningWindow),
    m_sampleRate(0)
{
    QDomDocument doc;
    
    QString error;
    int errorLine;
    int errorColumn;

    if (!doc.setContent(xml, false, &error, &errorLine, &errorColumn)) {
        cerr << "Transform::Transform: Error in parsing XML: "
                  << error << " at line " << errorLine
                  << ", column " << errorColumn << endl;
        cerr << "Input follows:" << endl;
        cerr << xml << endl;
        cerr << "Input ends." << endl;
        return;
    }
    
    QDomElement transformElt = doc.firstChildElement("transform");
    QDomNamedNodeMap attrNodes = transformElt.attributes();
    QXmlAttributes attrs;

    for (int i = 0; i < attrNodes.length(); ++i) {
        QDomAttr attr = attrNodes.item(i).toAttr();
        if (!attr.isNull()) attrs.append(attr.name(), "", "", attr.value());
    }

    setFromXmlAttributes(attrs);

    for (QDomElement paramElt = transformElt.firstChildElement("parameter");
         !paramElt.isNull();
         paramElt = paramElt.nextSiblingElement("parameter")) {

        QDomNamedNodeMap paramAttrs = paramElt.attributes();

        QDomAttr nameAttr = paramAttrs.namedItem("name").toAttr();
        if (nameAttr.isNull() || nameAttr.value() == "") continue;
        
        QDomAttr valueAttr = paramAttrs.namedItem("value").toAttr();
        if (valueAttr.isNull() || valueAttr.value() == "") continue;

        setParameter(nameAttr.value(), valueAttr.value().toFloat());
    }

    for (QDomElement configElt = transformElt.firstChildElement("configuration");
         !configElt.isNull();
         configElt = configElt.nextSiblingElement("configuration")) {

        QDomNamedNodeMap configAttrs = configElt.attributes();

        QDomAttr nameAttr = configAttrs.namedItem("name").toAttr();
        if (nameAttr.isNull() || nameAttr.value() == "") continue;
        
        QDomAttr valueAttr = configAttrs.namedItem("value").toAttr();
        if (valueAttr.isNull() || valueAttr.value() == "") continue;

        setConfigurationValue(nameAttr.value(), valueAttr.value());
    }
}

Transform::~Transform()
{
}

bool
Transform::operator==(const Transform &t) const
{
    bool identical =  
        m_id == t.m_id &&
        m_parameters == t.m_parameters &&
        m_configuration == t.m_configuration &&
        m_program == t.m_program &&
        m_summaryType == t.m_summaryType &&
        m_stepSize == t.m_stepSize &&
        m_blockSize == t.m_blockSize &&
        m_windowType == t.m_windowType &&
        m_startTime == t.m_startTime &&
        m_duration == t.m_duration &&
        m_sampleRate == t.m_sampleRate;
/*
    cerr << "Transform::operator==: identical = " << identical << endl;
    cerr << "A = " << endl;
    cerr << toXmlString() << endl;
    cerr << "B = " << endl;
    cerr << t.toXmlString() << endl;
*/
    return identical;
}

bool
Transform::operator<(const Transform &t) const
{
    if (m_id != t.m_id) {
        return m_id < t.m_id;
    }
    if (m_parameters != t.m_parameters) {
        return mapLessThan<QString, float>(m_parameters, t.m_parameters);
    }
    if (m_configuration != t.m_configuration) {
        return mapLessThan<QString, QString>(m_configuration, t.m_configuration);
    }
    if (m_program != t.m_program) {
        return m_program < t.m_program;
    }
    if (m_summaryType != t.m_summaryType) {
        return int(m_summaryType) < int(t.m_summaryType);
    }
    if (m_stepSize != t.m_stepSize) {
        return m_stepSize < t.m_stepSize;
    }
    if (m_blockSize != t.m_blockSize) {
        return m_blockSize < t.m_blockSize;
    }
    if (m_windowType != t.m_windowType) {
        return m_windowType < t.m_windowType;
    }
    if (m_startTime != t.m_startTime) {
        return m_startTime < t.m_startTime;
    }
    if (m_duration != t.m_duration) {
        return m_duration < t.m_duration;
    }
    if (m_sampleRate != t.m_sampleRate) {
        return m_sampleRate < t.m_sampleRate;
    }
    return false;
}

void
Transform::setIdentifier(TransformId id)
{
    m_id = id;
}

TransformId
Transform::getIdentifier() const
{
    return m_id;
}

QString
Transform::createIdentifier(QString type, QString soName, QString label,
                            QString output)
{
    QString pluginId = PluginIdentifier::createIdentifier(type, soName, label);
    return pluginId + ":" + output;
}

void
Transform::parseIdentifier(QString identifier,
                           QString &type, QString &soName,
                           QString &label, QString &output)
{
    output = identifier.section(':', 3);
    PluginIdentifier::parseIdentifier(identifier.section(':', 0, 2),
                                      type, soName, label);
}

Transform::Type
Transform::getType() const
{
    if (FeatureExtractionPluginFactory::instanceFor(getPluginIdentifier())) {
        return FeatureExtraction;
    } else {
        // We don't have an unknown/invalid return value, so always
        // return this
        return RealTimeEffect;
    }
}

QString
Transform::getPluginIdentifier() const
{
    return m_id.section(':', 0, 2);
}

QString
Transform::getOutput() const
{
    return m_id.section(':', 3);
}

void
Transform::setPluginIdentifier(QString pluginIdentifier)
{
    m_id = pluginIdentifier + ':' + getOutput();
}

void
Transform::setOutput(QString output)
{
    m_id = getPluginIdentifier() + ':' + output;
}

TransformId
Transform::getIdentifierForPluginOutput(QString pluginIdentifier,
                                        QString output)
{
    return pluginIdentifier + ':' + output;
}

const Transform::ParameterMap &
Transform::getParameters() const
{
    return m_parameters;
}

void
Transform::setParameters(const ParameterMap &pm)
{
    m_parameters = pm;
}

void
Transform::setParameter(QString name, float value)
{
//    cerr << "Transform::setParameter(" << name//              << ") -> " << value << endl;
    m_parameters[name] = value;
}

const Transform::ConfigurationMap &
Transform::getConfiguration() const
{
    return m_configuration;
}

void
Transform::setConfiguration(const ConfigurationMap &cm)
{
    m_configuration = cm;
}

void
Transform::setConfigurationValue(QString name, QString value)
{
    cerr << "Transform::setConfigurationValue(" << name              << ") -> " << value << endl;
    m_configuration[name] = value;
}

QString
Transform::getPluginVersion() const
{
    return m_pluginVersion;
}

void
Transform::setPluginVersion(QString version)
{
    m_pluginVersion = version;
}

QString
Transform::getProgram() const
{
    return m_program;
}

void
Transform::setProgram(QString program)
{
    m_program = program;
}

Transform::SummaryType
Transform::getSummaryType() const
{
    return m_summaryType;
}

void
Transform::setSummaryType(SummaryType type)
{
    m_summaryType = type;
}
    
int
Transform::getStepSize() const
{
    return m_stepSize;
}

void
Transform::setStepSize(int s)
{
    m_stepSize = s;
}
    
int
Transform::getBlockSize() const
{
    return m_blockSize;
}

void
Transform::setBlockSize(int s)
{
    m_blockSize = s;
}

WindowType
Transform::getWindowType() const
{
    return m_windowType;
}

void
Transform::setWindowType(WindowType type)
{
    m_windowType = type;
}

RealTime
Transform::getStartTime() const
{
    return m_startTime;
}

void
Transform::setStartTime(RealTime t)
{
    m_startTime = t;
}

RealTime
Transform::getDuration() const
{
    return m_duration;
}

void
Transform::setDuration(RealTime d)
{
    m_duration = d;
}
    
sv_samplerate_t
Transform::getSampleRate() const
{
    return m_sampleRate;
}

void
Transform::setSampleRate(sv_samplerate_t rate)
{
    m_sampleRate = rate;
}

void
Transform::toXml(QTextStream &out, QString indent, QString extraAttributes) const
{
    out << indent;

    bool haveContent = true;
    if (m_parameters.empty() && m_configuration.empty()) haveContent = false;

    out << QString("<transform\n    id=\"%1\"\n    pluginVersion=\"%2\"\n    program=\"%3\"\n    stepSize=\"%4\"\n    blockSize=\"%5\"\n    windowType=\"%6\"\n    startTime=\"%7\"\n    duration=\"%8\"\n    sampleRate=\"%9\"")
        .arg(encodeEntities(m_id))
        .arg(encodeEntities(m_pluginVersion))
        .arg(encodeEntities(m_program))
        .arg(m_stepSize)
        .arg(m_blockSize)
        .arg(encodeEntities(Window<float>::getNameForType(m_windowType).c_str()))
        .arg(encodeEntities(m_startTime.toString().c_str()))
        .arg(encodeEntities(m_duration.toString().c_str()))
        .arg(m_sampleRate);

    if (m_summaryType != NoSummary) {
        out << QString("\n    summaryType=\"%1\"").arg(summaryTypeToString(m_summaryType));
    }

    if (extraAttributes != "") {
        out << " " << extraAttributes;
    }

    if (haveContent) {

        out << ">\n";

        for (ParameterMap::const_iterator i = m_parameters.begin();
             i != m_parameters.end(); ++i) {
            out << indent << "  "
                << QString("<parameter name=\"%1\" value=\"%2\"/>\n")
                .arg(encodeEntities(i->first))
                .arg(i->second);
        }
        
        for (ConfigurationMap::const_iterator i = m_configuration.begin();
             i != m_configuration.end(); ++i) {
            out << indent << "  "
                << QString("<configuration name=\"%1\" value=\"%2\"/>\n")
                .arg(encodeEntities(i->first))
                .arg(encodeEntities(i->second));
        }

        out << indent << "</transform>\n";

    } else {

        out << "/>\n";
    }
}

Transform::SummaryType
Transform::stringToSummaryType(QString str)
{
    str = str.toLower();
    if (str == "minimum" || str == "min") return Minimum;
    if (str == "maximum" || str == "max") return Maximum;
    if (str == "mean") return Mean;
    if (str == "median") return Median;
    if (str == "mode") return Mode;
    if (str == "sum") return Sum;
    if (str == "variance") return Variance;
    if (str == "standard-deviation" || str == "standardDeviation" ||
        str == "standard deviation" || str == "sd") return StandardDeviation;
    if (str == "count") return Count;
    if (str == "") return NoSummary;
    cerr << "Transform::stringToSummaryType: unknown summary type \""
              << str << "\"" << endl;
    return NoSummary;
}

QString
Transform::summaryTypeToString(SummaryType type)
{
    switch (type) {
    case Minimum: return "min";
    case Maximum: return "max";
    case Mean: return "mean";
    case Median: return "median";
    case Mode: return "mode";
    case Sum: return "sum";
    case Variance: return "variance";
    case StandardDeviation: return "sd";
    case Count: return "count";
    case NoSummary: return "";
    default:
        cerr << "Transform::summaryTypeToString: unexpected summary type "
                  << int(type) << endl;
        return "";
    }
}

void
Transform::setFromXmlAttributes(const QXmlAttributes &attrs)
{
    if (attrs.value("id") != "") {
        setIdentifier(attrs.value("id"));
    }

    if (attrs.value("pluginVersion") != "") {
        setPluginVersion(attrs.value("pluginVersion"));
    }

    if (attrs.value("program") != "") {
        setProgram(attrs.value("program"));
    }

    if (attrs.value("stepSize") != "") {
        setStepSize(attrs.value("stepSize").toInt());
    }

    if (attrs.value("blockSize") != "") {
        setBlockSize(attrs.value("blockSize").toInt());
    }

    if (attrs.value("windowType") != "") {
        setWindowType(Window<float>::getTypeForName
                      (attrs.value("windowType").toStdString()));
    }

    if (attrs.value("startTime") != "") {
        setStartTime(RealTime::fromString(attrs.value("startTime").toStdString()));
    }

    if (attrs.value("duration") != "") {
        setDuration(RealTime::fromString(attrs.value("duration").toStdString()));
    }
    
    if (attrs.value("sampleRate") != "") {
        setSampleRate(attrs.value("sampleRate").toFloat());
    }

    if (attrs.value("summaryType") != "") {
        setSummaryType(stringToSummaryType(attrs.value("summaryType")));
    }
}

