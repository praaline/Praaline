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

#include "FileFeatureWriter.h"

#include "base/Exceptions.h"

#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDir>

using namespace std;
using namespace Vamp;

FileFeatureWriter::FileFeatureWriter(int support,
                                     QString extension) :
    m_prevstream(0),
    m_support(support),
    m_extension(extension),
    m_manyFiles(false),
    m_stdout(false),
    m_append(false),
    m_force(false)
{
    if (!(m_support & SupportOneFilePerTrack)) {
        if (m_support & SupportOneFilePerTrackTransform) {
            m_manyFiles = true;
        } else if (m_support & SupportOneFileTotal) {
            m_singleFileName = QString("output.%1").arg(m_extension);
        } else {
            cerr << "FileFeatureWriter::FileFeatureWriter: ERROR: Invalid support specification " << support << endl;
        }
    }
}

FileFeatureWriter::~FileFeatureWriter()
{
    while (!m_streams.empty()) {
        m_streams.begin()->second->flush();
        delete m_streams.begin()->second;
        m_streams.erase(m_streams.begin());
    }
    while (!m_files.empty()) {
        if (m_files.begin()->second) {
            cerr << "FileFeatureWriter::~FileFeatureWriter: NOTE: Closing feature file \""
                 << m_files.begin()->second->fileName() << "\"" << endl;
            delete m_files.begin()->second;
        }
        m_files.erase(m_files.begin());
    }
}

FileFeatureWriter::ParameterList
FileFeatureWriter::getSupportedParameters() const
{
    ParameterList pl;
    Parameter p;

    p.name = "basedir";
    p.description = "Base output directory path. (The default is the same directory as the input file.) The directory must exist already.";
    p.hasArg = true;
    pl.push_back(p);

    if (m_support & SupportOneFilePerTrackTransform &&
        m_support & SupportOneFilePerTrack) {
        p.name = "many-files";
        p.description = "Create a separate output file for every combination of input file and transform. The output file names will be based on the input file names. (The default is to create one output file per input audio file, and write all transform results for that input into it.)";
        p.hasArg = false;
        pl.push_back(p);
    }

    if (m_support & SupportOneFileTotal) {
        if (m_support & ~SupportOneFileTotal) { // not only option
            p.name = "one-file";
            if (m_support & SupportOneFilePerTrack) {
                p.description = "Write all transform results for all input files into the single named output file. (The default is to create one output file per input audio file, and write all transform results for that input into it.)";
            } else {
                p.description = "Write all transform results for all input files into the single named output file. (The default is to create a separate output file for each combination of input audio file and transform.)";
            }                
            p.hasArg = true;
            pl.push_back(p);
        }
    }
    if (m_support & SupportStdOut) {
        p.name = "stdout";
        p.description = "Write all transform results directly to standard output.";
        p.hasArg = false;
        pl.push_back(p);
    }

    p.name = "force";
    p.description = "If an output file already exists, overwrite it.";
    p.hasArg = false;
    pl.push_back(p);

    p.name = "append";
    p.description = "If an output file already exists, append data to it.";
    p.hasArg = false;
    pl.push_back(p);

    return pl;
}

void
FileFeatureWriter::setParameters(map<string, string> &params)
{
    for (map<string, string>::iterator i = params.begin();
         i != params.end(); ++i) {
        if (i->first == "basedir") {
            m_baseDir = i->second.c_str();
        } else if (i->first == "many-files") {
            if (m_support & SupportOneFilePerTrackTransform &&
                m_support & SupportOneFilePerTrack) {
                if (m_singleFileName != "") {
                    cerr << "FileFeatureWriter::setParameters: WARNING: Both one-file and many-files parameters provided, ignoring many-files" << endl;
                } else {
                    m_manyFiles = true;
                }
            }
        } else if (i->first == "one-file") {
            if (m_support & SupportOneFileTotal) {
                if (m_support & ~SupportOneFileTotal) { // not only option
                    // No, we cannot do this test because m_manyFiles
                    // may be on by default (for any FileFeatureWriter
                    // that supports OneFilePerTrackTransform but not
                    // OneFilePerTrack), so we need to be able to
                    // override it
//                    if (m_manyFiles) {
//                        cerr << "FileFeatureWriter::setParameters: WARNING: Both many-files and one-file parameters provided, ignoring one-file" << endl;
//                    } else {
                        m_singleFileName = i->second.c_str();
//                    }
                }
            }
        } else if (i->first == "stdout") {
            if (m_support & SupportStdOut) {
                if (m_singleFileName != "") {
                    cerr << "FileFeatureWriter::setParameters: WARNING: Both stdout and one-file provided, ignoring stdout" << endl;
                } else {
                    m_stdout = true;
                }
            }
        } else if (i->first == "append") {
            m_append = true;
        } else if (i->first == "force") {
            m_force = true;
        }
    }
}

QString
FileFeatureWriter::createOutputFilename(QString trackId,
                                        TransformId transformId)
{
    if (m_singleFileName != "") {
        if (QFileInfo(m_singleFileName).exists() && !(m_force || m_append)) {
            cerr << endl << "FileFeatureWriter: ERROR: Specified output file \"" << m_singleFileName << "\" exists and neither --" << getWriterTag() << "-force nor --" << getWriterTag() << "-append flag is specified -- not overwriting" << endl;
            cerr << "NOTE: To find out how to fix this problem, read the help for the --" << getWriterTag() << "-force" << endl << "and --" << getWriterTag() << "-append options" << endl;
            return "";
        }
        return m_singleFileName;
    }

    if (m_stdout) {
        return "";
    }
    
    QUrl url(trackId, QUrl::StrictMode);
    QString scheme = url.scheme().toLower();
    bool local = (scheme == "" || scheme == "file" || scheme.length() == 1);

    QString dirname, basename;
    QString infilename = url.toLocalFile();
    if (infilename == "") {
        infilename = url.path();
    }
    basename = QFileInfo(infilename).completeBaseName();
    if (scheme.length() == 1) {
        infilename = scheme + ":" + infilename; // DOS drive!
    }

//    cerr << "trackId = " << trackId << ", url = " << url.toString() << ", infilename = "
//         << infilename << ", basename = " << basename << ", m_baseDir = " << m_baseDir << endl;

    if (m_baseDir != "") dirname = QFileInfo(m_baseDir).absoluteFilePath();
    else if (local) dirname = QFileInfo(infilename).absolutePath();
    else dirname = QDir::currentPath();

//    cerr << "dirname = " << dirname << endl;

    QString filename;

    if (m_manyFiles && transformId != "") {
        filename = QString("%1_%2.%3").arg(basename).arg(transformId).arg(m_extension);
    } else {
        filename = QString("%1.%2").arg(basename).arg(m_extension);
    }

    filename.replace(':', '_'); // ':' not permitted in Windows

    filename = QDir(dirname).filePath(filename);

    if (QFileInfo(filename).exists() && !(m_force || m_append)) {
        cerr << endl << "FileFeatureWriter: ERROR: Output file \"" << filename << "\" exists (for input file or URL \"" << trackId << "\" and transform \"" << transformId << "\") and neither --" << getWriterTag() << "-force nor --" << getWriterTag() << "-append is specified -- not overwriting" << endl;
        cerr << "NOTE: To find out how to fix this problem, read the help for the --" << getWriterTag() << "-force" << endl << "and --" << getWriterTag() << "-append options" << endl;
        return "";
    }
    
    return filename;
}

void
FileFeatureWriter::testOutputFile(QString trackId,
                                  TransformId transformId)
{
    // Obviously, if we're writing to stdout we can't test for an
    // openable output file. But when writing a single file we don't
    // want to either, because this test would fail on the second and
    // subsequent input files (because the file would already exist).
    // getOutputFile does the right thing in this case, so we just
    // leave it to it
    if (m_stdout || m_singleFileName != "") return;

    QString filename = createOutputFilename(trackId, transformId);
    if (filename == "") {
        throw FailedToOpenOutputStream(trackId, transformId);
    }
}

FileFeatureWriter::TrackTransformPair
FileFeatureWriter::getFilenameKey(QString trackId,
                                  TransformId transformId)
{
    TrackTransformPair key;

    if (m_singleFileName != "") {
        key = TrackTransformPair("", "");
    } else if (m_manyFiles) {
        key = TrackTransformPair(trackId, transformId);
    } else {
        key = TrackTransformPair(trackId, "");
    }

    return key;
}    

QString
FileFeatureWriter::getOutputFilename(QString trackId,
                                     TransformId transformId)
{
    TrackTransformPair key = getFilenameKey(trackId, transformId);
    if (m_filenames.find(key) == m_filenames.end()) {
        m_filenames[key] = createOutputFilename(trackId, transformId);
    }
    return m_filenames[key];
}

QFile *
FileFeatureWriter::getOutputFile(QString trackId,
                                 TransformId transformId)
{
    TrackTransformPair key = getFilenameKey(trackId, transformId);

    if (m_files.find(key) == m_files.end()) {

        QString filename = createOutputFilename(trackId, transformId);

        if (filename == "") { // stdout or failure
            return 0;
        }

        cerr << "FileFeatureWriter: NOTE: Using output filename \""
             << filename << "\"" << endl;

        if (m_append) {
            cerr << "FileFeatureWriter: NOTE: Calling reviewFileForAppending" << endl;
            reviewFileForAppending(filename);
        }
        
        QFile *file = new QFile(filename);
        QIODevice::OpenMode mode = (QIODevice::WriteOnly);
        if (m_append) mode |= QIODevice::Append;
                       
        if (!file->open(mode)) {
            cerr << "FileFeatureWriter: ERROR: Failed to open output file \"" << filename
                 << "\" for writing" << endl;
            delete file;
            m_files[key] = 0;
            throw FailedToOpenFile(filename);
        }
        
        m_files[key] = file;
    }

    return m_files[key];
}


QTextStream *FileFeatureWriter::getOutputStream(QString trackId,
                                                TransformId transformId,
                                                QTextCodec *codec)
{
    QFile *file = getOutputFile(trackId, transformId);
    if (!file && !m_stdout) {
        return 0;
    }
    
    if (m_streams.find(file) == m_streams.end()) {
        if (m_stdout) {
            m_streams[file] = new QTextStream(stdout);
        } else {
            m_streams[file] = new QTextStream(file);
        }
        m_streams[file]->setCodec(codec);
    }

    QTextStream *stream = m_streams[file];

    if (m_prevstream && stream != m_prevstream) {
        m_prevstream->flush();
    }
    m_prevstream = stream;

    return stream;
}
            

void
FileFeatureWriter::flush()
{
    if (m_prevstream) {
        m_prevstream->flush();
    }
}


void
FileFeatureWriter::finish()
{
//    cerr << "FileFeatureWriter::finish()" << endl;

    if (m_singleFileName != "" || m_stdout) return;

    while (!m_streams.empty()) {
        m_streams.begin()->second->flush();
        delete m_streams.begin()->second;
        m_streams.erase(m_streams.begin());
    }
    while (!m_files.empty()) {
        if (m_files.begin()->second) {
            cerr << "FileFeatureWriter::finish: NOTE: Closing feature file \""
                 << m_files.begin()->second->fileName() << "\"" << endl;
            delete m_files.begin()->second;
        }
        m_files.erase(m_files.begin());
    }
    m_prevstream = 0;
}

