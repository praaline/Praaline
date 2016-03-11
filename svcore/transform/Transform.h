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

#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include "base/BaseTypes.h"
#include "base/XmlExportable.h"
#include "base/Window.h"
#include "base/RealTime.h"

#include <vamp-hostsdk/PluginBase.h>

#include <QString>

#include <map>
#include <vector>

typedef QString TransformId;

class QXmlAttributes;

class Transform : public XmlExportable
{
public:
    /**
     * Construct a new Transform with default data and no identifier.
     * The Transform object will be meaningless until some data and an
     * identifier have been set on it.
     *
     * To construct a Transform for use with a particular transform
     * identifier, use TransformFactory::getDefaultTransformFor.
     */
    Transform();

    /**
     * Construct a Transform by parsing the given XML data string.
     * This is the inverse of toXml.
     */
    Transform(QString xml);

    virtual ~Transform();

    /**
     * Compare two Transforms.  They only compare equal if every data
     * element matches.
     */
    bool operator==(const Transform &) const;
    
    /**
     * Order two Transforms, so that they can be used as keys in
     * containers.
     */
    bool operator<(const Transform &) const;

    void setIdentifier(TransformId id);
    TransformId getIdentifier() const;

    enum Type { FeatureExtraction, RealTimeEffect };

    Type getType() const;
    QString getPluginIdentifier() const;
    QString getOutput() const;
    
    void setPluginIdentifier(QString pluginIdentifier);
    void setOutput(QString output);

    // Turn a plugin ID and output name into a transform ID.  Note
    // that our pluginIdentifier is the same thing as the Vamp SDK's
    // PluginLoader::PluginKey.
    static TransformId getIdentifierForPluginOutput(QString pluginIdentifier,
                                                    QString output = "");

    typedef std::map<QString, float> ParameterMap;
    
    const ParameterMap &getParameters() const;
    void setParameters(const ParameterMap &pm);
    void setParameter(QString name, float value);

    typedef std::map<QString, QString> ConfigurationMap;

    const ConfigurationMap &getConfiguration() const;
    void setConfiguration(const ConfigurationMap &cm);
    void setConfigurationValue(QString name, QString value);

    enum SummaryType {

        // This is the same as Vamp::PluginSummarisingAdapter::SummaryType
        // except with NoSummary instead of UnknownSummaryType

        Minimum            = 0,
        Maximum            = 1,
        Mean               = 2,
        Median             = 3,
        Mode               = 4,
        Sum                = 5,
        Variance           = 6,
        StandardDeviation  = 7,
        Count              = 8,

        NoSummary          = 999
    };
    SummaryType getSummaryType() const;
    void setSummaryType(SummaryType type);

    QString getPluginVersion() const;
    void setPluginVersion(QString version);

    QString getProgram() const;
    void setProgram(QString program);
    
    int getStepSize() const;
    void setStepSize(int s);
    
    int getBlockSize() const;
    void setBlockSize(int s);
    
    WindowType getWindowType() const;
    void setWindowType(WindowType type);
    
    RealTime getStartTime() const;
    void setStartTime(RealTime t);
    
    RealTime getDuration() const; // 0 -> all
    void setDuration(RealTime d);
    
    sv_samplerate_t getSampleRate() const; // 0 -> as input
    void setSampleRate(sv_samplerate_t rate);

    void toXml(QTextStream &stream, QString indent = "",
               QString extraAttributes = "") const;

    /**
     * Set the main transform data from the given XML attributes.
     * This does not set the parameters or configuration, which are
     * exported to separate XML elements rather than attributes of the
     * transform element.
     * 
     * Note that this only sets those attributes which are actually
     * present in the argument.  Any attributes not defined in the
     * attribute will remain unchanged in the Transform.  If your aim
     * is to create a transform exactly matching the given attributes,
     * ensure you start from an empty transform rather than one that
     * has already been configured.
     */
    void setFromXmlAttributes(const QXmlAttributes &);

    static SummaryType stringToSummaryType(QString);
    static QString summaryTypeToString(SummaryType);

protected:
    TransformId m_id; // pluginid:output, that is type:soname:label:output
    
    static QString createIdentifier
    (QString type, QString soName, QString label, QString output);

    static void parseIdentifier
    (QString identifier,
     QString &type, QString &soName, QString &label, QString &output);

    template <typename A, typename B>
    bool mapLessThan(const std::map<A, B> &a, const std::map<A, B> &b) const {
        // Return true if a is "less than" b.  Ordering doesn't have
        // to be meaningful, just consistent.
        typename std::map<A, B>::const_iterator i;
        typename std::map<A, B>::const_iterator j;
        for (i = a.begin(), j = b.begin(); i != a.end(); ++i) {
            if (j == b.end()) return false; // a is longer than b
            if (i->first != j->first) return i->first < j->first;
            if (i->second != j->second) return i->second < j->second;
        }
        if (j != b.end()) return true; // a is shorter than b
        return false; // equal
    }

    ParameterMap m_parameters;
    ConfigurationMap m_configuration;
    SummaryType m_summaryType;
    QString m_pluginVersion;
    QString m_program;
    int m_stepSize;
    int m_blockSize;
    WindowType m_windowType;
    RealTime m_startTime;
    RealTime m_duration;
    sv_samplerate_t m_sampleRate;
};

typedef std::vector<Transform> Transforms;

#endif

