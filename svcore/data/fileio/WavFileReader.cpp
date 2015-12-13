/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "WavFileReader.h"

#include <iostream>

#include <QMutexLocker>
#include <QFileInfo>

WavFileReader::WavFileReader(FileSource source, bool fileUpdating) :
    m_file(0),
    m_source(source),
    m_path(source.getLocalFilename()),
    m_seekable(false),
    m_lastStart(0),
    m_lastCount(0),
    m_updating(fileUpdating)
{
    m_frameCount = 0;
    m_channelCount = 0;
    m_sampleRate = 0;

    m_fileInfo.format = 0;
    m_fileInfo.frames = 0;
    m_file = sf_open(m_path.toLocal8Bit(), SFM_READ, &m_fileInfo);

    if (!m_file || (!fileUpdating && m_fileInfo.channels <= 0)) {
	cerr << "WavFileReader::initialize: Failed to open file at \""
                  << m_path << "\" ("
		  << sf_strerror(m_file) << ")" << endl;

	if (m_file) {
	    m_error = QString("Couldn't load audio file '%1':\n%2")
		.arg(m_path).arg(sf_strerror(m_file));
	} else {
	    m_error = QString("Failed to open audio file '%1'")
		.arg(m_path);
	}
	return;
    }

    if (m_fileInfo.channels > 0) {

        m_frameCount = m_fileInfo.frames;
        m_channelCount = m_fileInfo.channels;
        m_sampleRate = m_fileInfo.samplerate;

        m_seekable = (m_fileInfo.seekable != 0);

        // Our m_seekable reports whether a file is rapidly seekable,
        // so things like Ogg don't qualify. We cautiously report
        // every file type of "at least" the historical period of Ogg
        // or FLAC as non-seekable.
        int type = m_fileInfo.format & SF_FORMAT_TYPEMASK;
//        cerr << "WavFileReader: format type is " << type << " (flac, ogg are " << SF_FORMAT_FLAC << ", " << SF_FORMAT_OGG << ")" << endl;
        if (type >= SF_FORMAT_FLAC || type >= SF_FORMAT_OGG) {
//            cerr << "WavFileReader: Recording as non-seekable" << endl;
            m_seekable = false;
        }
    }

//    cerr << "WavFileReader: Frame count " << m_frameCount << ", channel count " << m_channelCount << ", sample rate " << m_sampleRate << ", seekable " << m_seekable << endl;

}

WavFileReader::~WavFileReader()
{
    if (m_file) sf_close(m_file);
}

void
WavFileReader::updateFrameCount()
{
    QMutexLocker locker(&m_mutex);

    sv_frame_t prevCount = m_fileInfo.frames;

    if (m_file) {
        sf_close(m_file);
        m_file = sf_open(m_path.toLocal8Bit(), SFM_READ, &m_fileInfo);
        if (!m_file || m_fileInfo.channels <= 0) {
            cerr << "WavFileReader::updateFrameCount: Failed to open file at \"" << m_path << "\" ("
                      << sf_strerror(m_file) << ")" << endl;
        }
    }

//    cerr << "WavFileReader::updateFrameCount: now " << m_fileInfo.frames << endl;

    m_frameCount = m_fileInfo.frames;

    if (m_channelCount == 0) {
        m_channelCount = m_fileInfo.channels;
        m_sampleRate = m_fileInfo.samplerate;
    }

    if (m_frameCount != prevCount) {
//        cerr << "frameCountChanged" << endl;
        emit frameCountChanged();
    }
}

void
WavFileReader::updateDone()
{
    updateFrameCount();
    m_updating = false;
}

SampleBlock
WavFileReader::getInterleavedFrames(sv_frame_t start, sv_frame_t count) const
{
    if (count == 0) return SampleBlock();

    QMutexLocker locker(&m_mutex);

    if (!m_file || !m_channelCount) {
        return SampleBlock();
    }

    if (start >= m_fileInfo.frames) {
//        cerr << "WavFileReader::getInterleavedFrames: " << start
//                  << " > " << m_fileInfo.frames << endl;
	return SampleBlock();
    }

    if (start + count > m_fileInfo.frames) {
	count = m_fileInfo.frames - start;
    }

    if (start != m_lastStart || count != m_lastCount) {

	if (sf_seek(m_file, start, SEEK_SET) < 0) {
	    return SampleBlock();
	}

        sv_frame_t n = count * m_fileInfo.channels;
        m_buffer.resize(n);
	
        sf_count_t readCount = 0;

	if ((readCount = sf_readf_float(m_file, m_buffer.data(), count)) < 0) {
	    return SampleBlock();
	}

        m_buffer.resize(readCount * m_fileInfo.channels);
        
	m_lastStart = start;
	m_lastCount = readCount;
    }

    return m_buffer;
}

void
WavFileReader::getSupportedExtensions(std::set<QString> &extensions)
{
    int count;

    if (sf_command(0, SFC_GET_FORMAT_MAJOR_COUNT, &count, sizeof(count))) {
        extensions.insert("wav");
        extensions.insert("aiff");
        extensions.insert("aifc");
        extensions.insert("aif");
        return;
    }

    SF_FORMAT_INFO info;
    for (int i = 0; i < count; ++i) {
        info.format = i;
        if (!sf_command(0, SFC_GET_FORMAT_MAJOR, &info, sizeof(info))) {
            QString ext = QString(info.extension).toLower();
            extensions.insert(ext);
            if (ext == "oga") {
                // libsndfile is awfully proper, it says it only
                // supports .oga but lots of Ogg audio files in the
                // wild are .ogg and it will accept that
                extensions.insert("ogg");
            }
        }
    }
}

bool
WavFileReader::supportsExtension(QString extension)
{
    std::set<QString> extensions;
    getSupportedExtensions(extensions);
    return (extensions.find(extension.toLower()) != extensions.end());
}

bool
WavFileReader::supportsContentType(QString type)
{
    return (type == "audio/x-wav" ||
            type == "audio/x-aiff" ||
            type == "audio/basic");
}

bool
WavFileReader::supports(FileSource &source)
{
    return (supportsExtension(source.getExtension()) ||
            supportsContentType(source.getContentType()));
}


