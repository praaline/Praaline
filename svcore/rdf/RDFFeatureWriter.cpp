/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
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

#include <fstream>

#include "base/Exceptions.h"

#include "RDFFeatureWriter.h"
#include "RDFTransformFactory.h"
#include "PluginRDFIndexer.h"

#include <QTextStream>
#include <QTextCodec>
#include <QUrl>
#include <QFileInfo>
#include <QRegExp>

using namespace std;
using Vamp::Plugin;
using Vamp::PluginBase;

RDFFeatureWriter::RDFFeatureWriter() :
    FileFeatureWriter(SupportOneFilePerTrackTransform |
                      SupportOneFilePerTrack |
                      SupportOneFileTotal |
                      SupportStdOut,
                      "n3"),
    m_plain(false),
    m_network(false),
    m_networkRetrieved(false),
    m_count(0)
{
}

RDFFeatureWriter::~RDFFeatureWriter()
{
}

string
RDFFeatureWriter::getDescription() const
{
    return "Write output in Audio Features Ontology RDF/Turtle format.";
}

RDFFeatureWriter::ParameterList
RDFFeatureWriter::getSupportedParameters() const
{
    ParameterList pl = FileFeatureWriter::getSupportedParameters();
    Parameter p;

    p.name = "plain";
    p.description = "Use \"plain\" RDF even if transform metadata is available.";
    p.hasArg = false;
    pl.push_back(p);

    p.name = "audiofile-uri";
    p.description = "Link the output RDF to the given audio file URI instead of its actual location.";
    p.hasArg = true;
    pl.push_back(p);

    p.name = "track-uri";
    p.description = "Link the output RDF to the given track URI.";
    p.hasArg = true;
    pl.push_back(p);

    p.name = "maker-uri";
    p.description = "Link the track in the output RDF to the given foaf:maker URI.";
    p.hasArg = true;
    pl.push_back(p);

    p.name = "network";
    p.description = "Attempt to retrieve RDF descriptions of plugins from network, if not available locally";
    p.hasArg = false;
    pl.push_back(p);
    
    return pl;
}

void
RDFFeatureWriter::setParameters(map<string, string> &params)
{
    FileFeatureWriter::setParameters(params);

    for (map<string, string>::iterator i = params.begin();
         i != params.end(); ++i) {
        if (i->first == "plain") {
            m_plain = true;
        }
        if (i->first == "audiofile-uri") {
            m_userAudioFileUri = i->second.c_str();
        }
        if (i->first == "track-uri") {
            m_userTrackUri = i->second.c_str();
        }
        if (i->first == "maker-uri") {
            m_userMakerUri = i->second.c_str();
        }
        if (i->first == "network") {
            m_network = true;
        }
    }
}

void
RDFFeatureWriter::setTrackMetadata(QString trackId,
                                   TrackMetadata metadata)
{
//    cerr << "setTrackMetadata: title = " << metadata.title << ", maker = " << metadata.maker << endl;
    m_metadata[trackId] = metadata;
}

void
RDFFeatureWriter::setFixedEventTypeURI(QString uri)
{
    m_fixedEventTypeURI = uri;
}

void
RDFFeatureWriter::write(QString trackId,
                        const Transform &transform,
                        const Plugin::OutputDescriptor& output,
                        const Plugin::FeatureList& features,
                        std::string summaryType)
{
    QString pluginId = transform.getPluginIdentifier();

    if (m_rdfDescriptions.find(pluginId) == m_rdfDescriptions.end()) {

        if (m_network && !m_networkRetrieved) {
            PluginRDFIndexer::getInstance()->indexConfiguredURLs();
            m_networkRetrieved = true;
        }

        m_rdfDescriptions[pluginId] = PluginRDFDescription(pluginId);

        if (m_rdfDescriptions[pluginId].haveDescription()) {
            cerr << "NOTE: Have RDF description for plugin ID \""
                 << pluginId << "\"" << endl;
        } else {
            cerr << "NOTE: No RDF description for plugin ID \""
                 << pluginId << "\"" << endl;
            if (!m_network) {
                cerr << "      Consider using the --rdf-network option to retrieve plugin descriptions"  << endl;
                cerr << "      from the network where possible." << endl;
            }
        }
    }

    // Need to select appropriate output file for our track/transform
    // combination

    QTextStream *stream = getOutputStream(trackId, transform.getIdentifier(),
                                          QTextCodec::codecForName("UTF-8"));
    if (!stream) {
        throw FailedToOpenOutputStream(trackId, transform.getIdentifier());
    }

    if (m_startedStreamTransforms.find(stream) ==
        m_startedStreamTransforms.end()) {
//        cerr << "This stream is new, writing prefixes" << endl;
        writePrefixes(stream);
        if (m_singleFileName == "" && !m_stdout) {
            writeSignalDescription(stream, trackId);
        }
    }

    if (m_startedStreamTransforms[stream].find(transform) ==
        m_startedStreamTransforms[stream].end()) {
        m_startedStreamTransforms[stream].insert(transform);
        writeLocalFeatureTypes
            (stream, transform, output, m_rdfDescriptions[pluginId],
             summaryType);
    }

    if (m_singleFileName != "" || m_stdout) {
        if (m_startedTrackIds.find(trackId) == m_startedTrackIds.end()) {
            writeSignalDescription(stream, trackId);
            m_startedTrackIds.insert(trackId);
        }
    }

    QString timelineURI = m_trackTimelineURIs[trackId];
    
    if (timelineURI == "") {
        cerr << "RDFFeatureWriter: INTERNAL ERROR: writing features without having established a timeline URI!" << endl;
        exit(1);
    }

    if (summaryType != "") {

        writeSparseRDF(stream, transform, output, features,
                       m_rdfDescriptions[pluginId], timelineURI);

    } else if (m_rdfDescriptions[pluginId].haveDescription() &&
               m_rdfDescriptions[pluginId].getOutputDisposition
               (output.identifier.c_str()) == 
               PluginRDFDescription::OutputDense) {

        QString signalURI = m_trackSignalURIs[trackId];

        if (signalURI == "") {
            cerr << "RDFFeatureWriter: INTERNAL ERROR: writing dense features without having established a signal URI!" << endl;
            exit(1);
        }

        writeDenseRDF(stream, transform, output, features,
                      m_rdfDescriptions[pluginId], signalURI, timelineURI);

    } else if (!m_plain &&
               m_rdfDescriptions[pluginId].haveDescription() &&
               m_rdfDescriptions[pluginId].getOutputDisposition
               (output.identifier.c_str()) ==
               PluginRDFDescription::OutputTrackLevel &&
               m_rdfDescriptions[pluginId].getOutputFeatureAttributeURI
               (output.identifier.c_str()) != "") {

        QString signalURI = m_trackSignalURIs[trackId];

        if (signalURI == "") {
            cerr << "RDFFeatureWriter: INTERNAL ERROR: writing track-level features without having established a signal URI!" << endl;
            exit(1);
        }

        writeTrackLevelRDF(stream, transform, output, features,
                           m_rdfDescriptions[pluginId], signalURI);

    } else {

        writeSparseRDF(stream, transform, output, features,
                       m_rdfDescriptions[pluginId], timelineURI);
    }
}

void
RDFFeatureWriter::writePrefixes(QTextStream *sptr)
{
    QTextStream &stream = *sptr;

    stream << "@prefix dc: <http://purl.org/dc/elements/1.1/> .\n"
           << "@prefix mo: <http://purl.org/ontology/mo/> .\n"
           << "@prefix af: <http://purl.org/ontology/af/> .\n"
           << "@prefix foaf: <http://xmlns.com/foaf/0.1/> . \n"
           << "@prefix event: <http://purl.org/NET/c4dm/event.owl#> .\n"
           << "@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n"
           << "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .\n"
           << "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .\n"
           << "@prefix tl: <http://purl.org/NET/c4dm/timeline.owl#> .\n"
           << "@prefix vamp: <http://purl.org/ontology/vamp/> .\n"
           << "@prefix : <#> .\n\n";
}

void
RDFFeatureWriter::reviewFileForAppending(QString filename)
{
    // Appending to an RDF file is tricky, because we need to ensure
    // that our URIs differ from any already in the file.  This is a
    // dirty grubby low-rent way of doing that.  This function is
    // called by FileFeatureWriter::getOutputFile when in append mode.

//    cerr << "reviewFileForAppending(" << filename << ")" << endl;

    QFile file(filename);

    // just return, don't report failure -- function that called us will do that
    if (!file.open(QIODevice::ReadOnly)) return;

    QTextStream in(&file);

    QRegExp localObjectUriWithDigits(":[^ ]+_([0-9]+) a ");

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.length() > 120) { // probably data
            continue;
        }
        if (localObjectUriWithDigits.indexIn(line) > -1) {
            QString numeric = localObjectUriWithDigits.cap(1);
            int number = numeric.toInt();
            if (number >= m_count) m_count = number + 1;
        }
    }

    file.close();
}

void
RDFFeatureWriter::writeSignalDescription(QTextStream *sptr,
                                         QString trackId)
{
//    cerr << "RDFFeatureWriter::writeSignalDescription" << endl;

    QTextStream &stream = *sptr;

    /*
     * Describe signal we're analysing (AudioFile, Signal, TimeLine, etc.)
     */
    
    QUrl url(trackId, QUrl::StrictMode);
    QString scheme = url.scheme().toLower();
    bool local = (scheme == "" || scheme == "file" || scheme.length() == 1);

    if (local) {
        if (scheme == "") {
            url.setScheme("file");
            url.setPath(QFileInfo(url.path()).absoluteFilePath());
        } else if (scheme.length() == 1) { // DOS drive letter!
            url.setScheme("file");
            url.setPath(scheme + ":" + url.path());
        }
    }

    // Note reviewFileForAppending above (when opening in append mode)

    unsigned long signalCount = m_count++;

    if (m_trackSignalURIs.find(trackId) == m_trackSignalURIs.end()) {
        m_trackSignalURIs[trackId] = QString(":signal_%1").arg(signalCount);
    }
    QString signalURI = m_trackSignalURIs[trackId];
   
    if (m_trackTrackURIs.find(trackId) == m_trackTrackURIs.end()) {
        m_trackTrackURIs[trackId] = QString(":track_%1").arg(signalCount);
    }
    QString trackURI = m_trackTrackURIs[trackId];

    bool userSpecifiedTrack = false;
    if (m_userTrackUri != "") {
        trackURI = "<" + m_userTrackUri + ">";
        m_trackTrackURIs[trackId] = trackURI;
        userSpecifiedTrack = true;
    }
    
    if (m_trackTimelineURIs.find(trackId) == m_trackTimelineURIs.end()) {
        m_trackTimelineURIs[trackId] = QString(":signal_timeline_%1").arg(signalCount);
    }
    QString timelineURI = m_trackTimelineURIs[trackId];

    QString afURI = url.toEncoded().data();
    if (m_userAudioFileUri != "") afURI = m_userAudioFileUri;

    bool wantTrack = (userSpecifiedTrack ||
                      (m_userMakerUri != "") ||
                      (m_metadata.find(trackId) != m_metadata.end()));

//    cerr << "wantTrack = " << wantTrack << " (userSpecifiedTrack = "
//         << userSpecifiedTrack << ", m_userMakerUri = " << m_userMakerUri << ", have metadata = " << (m_metadata.find(trackId) != m_metadata.end()) << ")" << endl;

    if (wantTrack) {
        // We only write a Track at all if we have some title/artist
        // metadata to put in it, or if the user has requested a
        // specific track URI.  Otherwise we can't be sure that what
        // we have is a Track, in the publication sense -- it may just
        // be a fragment, a test file, whatever.  Since we'd have no
        // metadata to associate with our Track, the only effect of
        // including a Track would be to assert that this was one,
        // which is the one thing we wouldn't know...
        TrackMetadata tm;
        if (m_metadata.find(trackId) != m_metadata.end()) {
            tm = m_metadata[trackId];
        }
        stream << trackURI << " a mo:Track ";
        if (tm.title != "") {
            stream << ";\n    dc:title \"\"\"" << tm.title << "\"\"\" ";
        }
        if (m_userMakerUri != "") {
            stream << ";\n    foaf:maker <" << m_userMakerUri << "> ";
        } else if (tm.maker != "") {
            stream << ";\n    foaf:maker [ a mo:MusicArtist; foaf:name \"\"\"" << tm.maker << "\"\"\" ] ";
        }
        if (afURI != "") {
            stream << ";\n    mo:available_as <" << afURI << "> ";
        }
        stream << ".\n\n";
    }

    if (afURI != "") {
        stream << "<" << afURI << "> a mo:AudioFile ;\n";
        stream << "    mo:encodes " << signalURI << ".\n\n";
    }

    stream << signalURI << " a mo:Signal ;\n";

    stream << "    mo:time [\n"
           << "        a tl:Interval ;\n"
           << "        tl:onTimeLine "
           << timelineURI << "\n    ] .\n\n";

    stream << timelineURI << " a tl:Timeline .\n\n";
} 

void
RDFFeatureWriter::writeLocalFeatureTypes(QTextStream *sptr,
                                         const Transform &transform,
                                         const Plugin::OutputDescriptor &od,
                                         PluginRDFDescription &desc, 
                                         std::string summaryType)
{
    QString outputId = od.identifier.c_str();
    QTextStream &stream = *sptr;

    // There is no "needFeatureType" for track-level outputs, because
    // we can't meaningfully write a feature at all if we don't know
    // what property to use for it.  If the output is track level but
    // there is no feature type given, we have to revert to events.

    bool needEventType = false;
    bool needSignalType = false;

    //!!! bin names, extents and so on can be written out using e.g. vamp:bin_names ( "a" "b" "c" ) 

    if (summaryType == "" &&
        desc.getOutputDisposition(outputId) == 
        PluginRDFDescription::OutputDense) {

        // no feature events, so may need signal type but won't need
        // event type

        if (m_plain) {

            needSignalType = true;

        } else if (desc.getOutputSignalTypeURI(outputId) == "") {
            
            needSignalType = true;
        }

    } else if (desc.getOutputDisposition(outputId) ==
               PluginRDFDescription::OutputTrackLevel) {

        // see note above -- need to generate an event type if no
        // feature type given, or if in plain mode

        if (m_plain) {
        
            needEventType = true;

        } else if (desc.getOutputFeatureAttributeURI(outputId) == "") {
    
            if (desc.getOutputEventTypeURI(outputId) == "") {

                needEventType = true;
            }
        }

    } else {

        // may need event type but won't need signal type

        if (m_plain) {
        
            needEventType = true;
    
        } else if (desc.getOutputEventTypeURI(outputId) == "") {

            needEventType = true;
        }
    }

    QString transformUri;
    if (m_transformURIs.find(transform) != m_transformURIs.end()) {
        transformUri = m_transformURIs[transform];
    } else {
        transformUri = QString(":transform_%1_%2").arg(m_count++).arg(outputId);
        m_transformURIs[transform] = transformUri;
    }

    if (transform.getIdentifier() != "") {
        stream << endl
               << RDFTransformFactory::writeTransformToRDF(transform, transformUri)
               << endl;
    }

    if (needEventType && m_fixedEventTypeURI == "") {

        QString uri;
        if (m_syntheticEventTypeURIs.find(transform) !=
            m_syntheticEventTypeURIs.end()) {
            uri = m_syntheticEventTypeURIs[transform];
        } else {
            uri = QString(":event_type_%1").arg(m_count++);
            m_syntheticEventTypeURIs[transform] = uri;
        }

        stream << uri
               << " rdfs:subClassOf event:Event ;" << endl
               << "    dc:title \"" << od.name.c_str() << "\" ;" << endl
               << "    dc:format \"" << od.unit.c_str() << "\" ;" << endl
               << "    dc:description \"" << od.description.c_str() << "\" ."
               << endl << endl;
    }

    if (needSignalType) {

        QString uri;
        if (m_syntheticSignalTypeURIs.find(transform) !=
            m_syntheticSignalTypeURIs.end()) {
            uri = m_syntheticSignalTypeURIs[transform];
        } else {
            uri = QString(":signal_type_%1").arg(m_count++);
            m_syntheticSignalTypeURIs[transform] = uri;
        }

        stream << uri
               << " rdfs:subClassOf af:Signal ;" << endl
               << "    dc:title \"" << od.name.c_str() << "\" ;" << endl
               << "    dc:format \"" << od.unit.c_str() << "\" ;" << endl
               << "    dc:description \"" << od.description.c_str() << "\" ."
               << endl << endl;
    }
}

void
RDFFeatureWriter::writeSparseRDF(QTextStream *sptr,
                                 const Transform &transform,
                                 const Plugin::OutputDescriptor& od,
                                 const Plugin::FeatureList& featureList,
                                 PluginRDFDescription &desc,
                                 QString timelineURI)
{
//    cerr << "RDFFeatureWriter::writeSparseRDF: have " << featureList.size() << " features" << endl;

    if (featureList.empty()) return;
    QTextStream &stream = *sptr;
        
    bool plain = (m_plain || !desc.haveDescription());

    QString outputId = od.identifier.c_str();

    // iterate through FeatureLists
        
    for (int i = 0; i < (int)featureList.size(); ++i) {

        const Plugin::Feature &feature = featureList[i];
        unsigned long featureNumber = m_count++;

        stream << ":event_" << featureNumber << " a ";

        if (m_fixedEventTypeURI != "") {
            stream << m_fixedEventTypeURI << " ;\n";
        } else {
            QString eventTypeURI = desc.getOutputEventTypeURI(outputId);
            if (plain || eventTypeURI == "") {
                if (m_syntheticEventTypeURIs.find(transform) != 
                    m_syntheticEventTypeURIs.end()) {
                    stream << m_syntheticEventTypeURIs[transform] << " ;\n";
                } else {
                    stream << ":event_type_" << outputId << " ;\n";
                }
            } else {
                stream << "<" << eventTypeURI << "> ;\n";
            }
        }

        QString timestamp = feature.timestamp.toString().c_str();
        timestamp.replace(QRegExp("^ +"), "");

        if (feature.hasDuration && feature.duration > Vamp::RealTime::zeroTime) {

            QString duration = feature.duration.toString().c_str();
            duration.replace(QRegExp("^ +"), "");

            stream << "    event:time [ \n"
                   << "        a tl:Interval ;\n"
                   << "        tl:onTimeLine " << timelineURI << " ;\n"
                   << "        tl:beginsAt \"PT" << timestamp
                   << "S\"^^xsd:duration ;\n"
                   << "        tl:duration \"PT" << duration
                   << "S\"^^xsd:duration ;\n"
                   << "    ] ";

        } else {

            stream << "    event:time [ \n"
                   << "        a tl:Instant ;\n" //location of the event in time
                   << "        tl:onTimeLine " << timelineURI << " ;\n"
                   << "        tl:at \"PT" << timestamp
                   << "S\"^^xsd:duration ;\n    ] ";
        }

        if (transform.getIdentifier() != "") {
            stream << ";\n";
            stream << "    vamp:computed_by " << m_transformURIs[transform] << " ";
        }

        if (feature.label.length() > 0) {
            stream << ";\n";
            stream << "    rdfs:label \"\"\"" << feature.label.c_str() << "\"\"\" ";
        }

        if (!feature.values.empty()) {
            stream << ";\n";
            //!!! named bins?
            stream << "    af:feature \"" << feature.values[0];
            for (int j = 1; j < (int)feature.values.size(); ++j) {
                stream << " " << feature.values[j];
            }
            stream << "\" ";
        }

        stream << ".\n";
    }
}

void
RDFFeatureWriter::writeTrackLevelRDF(QTextStream *sptr,
                                     const Transform &,
                                     const Plugin::OutputDescriptor& od,
                                     const Plugin::FeatureList& featureList,
                                     PluginRDFDescription &desc,
                                     QString signalURI)
{
    if (featureList.empty()) return;
    QTextStream &stream = *sptr;
        
//    bool plain = (m_plain || !desc.haveDescription());

    QString outputId = od.identifier.c_str();
    QString featureUri = desc.getOutputFeatureAttributeURI(outputId);

    if (featureUri == "") {
        cerr << "RDFFeatureWriter::writeTrackLevelRDF: ERROR: No feature URI available -- this function should not have been called!" << endl;
        return;
    }

    for (int i = 0; i < (int)featureList.size(); ++i) {

        const Plugin::Feature &feature = featureList[i];

        if (feature.values.empty()) {

            if (feature.label == "") continue;

            stream << signalURI << " " << featureUri << " \"\"\""
                   << feature.label.c_str() << "\"\"\" .\n";

        } else {

            stream << signalURI << " " << featureUri << " \""
                   << feature.values[0] << "\"^^xsd:float .\n";
        }
    }
}

void
RDFFeatureWriter::writeDenseRDF(QTextStream *sptr,
                                const Transform &transform,
                                const Plugin::OutputDescriptor& od,
                                const Plugin::FeatureList& featureList,
                                PluginRDFDescription &desc,
                                QString signalURI, 
                                QString timelineURI)
{
    if (featureList.empty()) return;

    StringTransformPair sp(signalURI, transform);

    if (m_openDenseFeatures.find(sp) == m_openDenseFeatures.end()) {

        StreamBuffer b(sptr, "");
        m_openDenseFeatures[sp] = b;
        
        QString &str(m_openDenseFeatures[sp].second);
        QTextStream stream(&str);

        bool plain = (m_plain || !desc.haveDescription());
        QString outputId = od.identifier.c_str();

        unsigned long featureNumber = m_count++;

        // need to write out feature timeline map -- for this we need
        // the sample rate, window length and hop size from the
        // transform

        stream << "\n:feature_timeline_" << featureNumber << " a tl:DiscreteTimeLine .\n\n";

        sv_samplerate_t sampleRate;
        int stepSize, blockSize;

        // If the output is FixedSampleRate, we need to draw the
        // sample rate and step size from the output descriptor;
        // otherwise they come from the transform

        if (od.sampleType == Plugin::OutputDescriptor::FixedSampleRate) {

            sampleRate = od.sampleRate;
            stepSize = 1;
            blockSize = 1;

        } else {

            sampleRate = transform.getSampleRate();
            if (sampleRate == 0.f) {
                cerr << "RDFFeatureWriter: INTERNAL ERROR: writing dense features without having set the sample rate properly!" << endl;
                return;
            }

            stepSize = transform.getStepSize();
            if (stepSize == 0) {
                cerr << "RDFFeatureWriter: INTERNAL ERROR: writing dense features without having set the step size properly!" << endl;
                return;
            }

            blockSize = transform.getBlockSize();
            if (blockSize == 0) {
                cerr << "RDFFeatureWriter: INTERNAL ERROR: writing dense features without having set the block size properly!" << endl;
                return;
            }
        }

        stream << ":feature_timeline_map_" << featureNumber
               << " a tl:UniformSamplingWindowingMap ;\n"
               << "    tl:rangeTimeLine :feature_timeline_" << featureNumber << " ;\n"
               << "    tl:domainTimeLine " << timelineURI << " ;\n"
               << "    tl:sampleRate \"" << sampleRate << "\"^^xsd:float ;\n"
               << "    tl:windowLength \"" << blockSize << "\"^^xsd:int ;\n"
               << "    tl:hopSize \"" << stepSize << "\"^^xsd:int .\n\n";

        stream << signalURI << " af:signal_feature :feature_"
               << featureNumber << " ." << endl << endl;

        stream << ":feature_" << featureNumber << " a ";

        QString signalTypeURI = desc.getOutputSignalTypeURI(outputId);
        if (plain || signalTypeURI == "") {
            if (m_syntheticSignalTypeURIs.find(transform) !=
                m_syntheticSignalTypeURIs.end()) {
                stream << m_syntheticSignalTypeURIs[transform] << " ;\n";
            } else {
                stream << ":signal_type_" << outputId << " ;\n";
            }
        } else {
            stream << "<" << signalTypeURI << "> ;\n";
        }

        stream << "    mo:time ["
               << "\n        a tl:Interval ;"
               << "\n        tl:onTimeLine :feature_timeline_" << featureNumber << " ;";

        RealTime startrt = transform.getStartTime();
        RealTime durationrt = transform.getDuration();

        sv_frame_t start = RealTime::realTime2Frame
            (startrt, sampleRate) / stepSize;
        sv_frame_t duration = RealTime::realTime2Frame
            (durationrt, sampleRate) / stepSize;

        if (start != 0) {
            stream << "\n        tl:start \"" << start << "\"^^xsd:int ;";
        }
        if (duration != 0) {
            stream << "\n        tl:duration \"" << duration << "\"^^xsd:int ;";
        }

        stream << "\n    ] ;\n";

        if (transform.getIdentifier() != "") {
            stream << "    vamp:computed_by " << m_transformURIs[transform] << " ;\n";
        }

        if (od.hasFixedBinCount) {
            // We only know the height, so write the width as zero
            stream << "    af:dimensions \"" << od.binCount << " 0\" ;\n";
        }

        stream << "    af:value \"";
    }

    QString &str = m_openDenseFeatures[sp].second;
    QTextStream stream(&str);

    for (int i = 0; i < (int)featureList.size(); ++i) {

        const Plugin::Feature &feature = featureList[i];

        for (int j = 0; j < (int)feature.values.size(); ++j) {
            stream << feature.values[j] << " ";
        }
    }
}

void RDFFeatureWriter::finish()
{
//    cerr << "RDFFeatureWriter::finish()" << endl;

    // close any open dense feature literals

    for (map<StringTransformPair, StreamBuffer>::iterator i =
             m_openDenseFeatures.begin();
         i != m_openDenseFeatures.end(); ++i) {
//        cerr << "closing a stream" << endl;
        StreamBuffer &b = i->second;
        *(b.first) << b.second << "\" ." << endl;
    }

    m_openDenseFeatures.clear();
    m_startedStreamTransforms.clear();

    FileFeatureWriter::finish();
}


