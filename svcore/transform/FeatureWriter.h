/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.

    Sonic Annotator
    A utility for batch feature extraction from audio files.

    Mark Levy, Chris Sutton and Chris Cannam, Queen Mary, University of London.
    Copyright 2007-2008 QMUL.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _FEATURE_WRITER_H_
#define _FEATURE_WRITER_H_

#include <string>
#include <map>
#include <vector>

#include <QString>

#include "Transform.h"

#include <vamp-hostsdk/Plugin.h>

using std::string;
using std::map;
using std::vector;

class FeatureWriter
{
public:
    virtual ~FeatureWriter() { }

    virtual string getDescription() const = 0;

    struct Parameter { // parameter of the writer, not the plugin
        string name;
        string description;
        bool hasArg;
    };
    typedef vector<Parameter> ParameterList;
    virtual ParameterList getSupportedParameters() const {
        return ParameterList();
    }

    virtual void setParameters(map<string, string> &) {
        return;
    }

    struct TrackMetadata {
        QString title;
        QString maker;
    };
    virtual void setTrackMetadata(QString /* trackid */, TrackMetadata) { }

    class FailedToOpenOutputStream : virtual public std::exception
    {
    public:
        FailedToOpenOutputStream(QString trackId, QString transformId) throw() :
            m_trackId(trackId),
            m_transformId(transformId)
        { }
        virtual ~FailedToOpenOutputStream() throw() { }
        virtual const char *what() const throw() {
            return QString("Failed to open output stream for track id \"%1\", transform id \"%2\"")
                .arg(m_trackId).arg(m_transformId).toLocal8Bit().data();
        }            
        
    protected:
        QString m_trackId;
        QString m_transformId;
    };

    /**
     * Notify the writer that we are about to start extraction for
     * input file N of M (where N is 1..M). May be useful when writing
     * multiple outputs into a single file where some syntactic
     * element is needed to connect them.
     */
    virtual void setNofM(int /* N */, int /* M */) { }

    // may throw FailedToOpenFile or other exceptions
    virtual void write(QString trackid,
                       const Transform &transform,
                       const Vamp::Plugin::OutputDescriptor &output,
                       const Vamp::Plugin::FeatureList &features,
                       std::string summaryType = "") = 0;

    /**
     * Throw FailedToOpenOutputStream if we can already tell that we
     * will be unable to write to the output file.  This is called to
     * test the output stream before processing begins.  The writer
     * may legitimately succeed here but still fail later -- this is
     * really an optimisation to ensure that easy-to-recognise failure
     * cases fail early.
     */
    virtual void testOutputFile(QString /* trackId */, TransformId) { }

    virtual void flush() { } // whatever the last stream was

    virtual void finish() = 0;

    virtual QString getWriterTag() const = 0;
};

#endif
