/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "ResamplingWavFileReader.h"

#include "WavFileReader.h"
#include "base/Profiler.h"
#include "base/ProgressReporter.h"

#include <QFileInfo>

ResamplingWavFileReader::ResamplingWavFileReader(FileSource source,
						 ResampleMode resampleMode,
						 CacheMode mode,
						 size_t targetRate,
                                                 ProgressReporter *reporter) :
    CodedAudioFileReader(mode, targetRate),
    m_source(source),
    m_path(source.getLocalFilename()),
    m_cancelled(false),
    m_processed(0),
    m_completion(0),
    m_original(0),
    m_reporter(reporter),
    m_decodeThread(0)
{
    m_channelCount = 0;
    m_fileRate = 0;

    SVDEBUG << "ResamplingWavFileReader::ResamplingWavFileReader(\""
              << m_path << "\"): rate " << targetRate << endl;

    Profiler profiler("ResamplingWavFileReader::ResamplingWavFileReader", true);

    m_original = new WavFileReader(m_path);
    if (!m_original->isOK()) {
        m_error = m_original->getError();
        return;
    }

    m_channelCount = m_original->getChannelCount();
    m_fileRate = m_original->getSampleRate();

    initialiseDecodeCache();

    if (resampleMode == ResampleAtOnce) {

        if (m_reporter) {
            connect(m_reporter, SIGNAL(cancelled()), this, SLOT(cancelled()));
            m_reporter->setMessage
                (tr("Resampling %1...").arg(QFileInfo(m_path).fileName()));
        }

        size_t blockSize = 16384;
        size_t total = m_original->getFrameCount();

        SampleBlock block;

        for (size_t i = 0; i < total; i += blockSize) {

            size_t count = blockSize;
            if (i + count > total) count = total - i;

            m_original->getInterleavedFrames(i, count, block);
            addBlock(block);

            if (m_cancelled) break;
        }

        if (isDecodeCacheInitialised()) finishDecodeCache();
        endSerialised();

        if (m_reporter) m_reporter->setProgress(100);

        delete m_original;
        m_original = 0;

    } else {

        if (m_reporter) m_reporter->setProgress(100);

        m_decodeThread = new DecodeThread(this);
        m_decodeThread->start();
    }
}

ResamplingWavFileReader::~ResamplingWavFileReader()
{
    if (m_decodeThread) {
        m_cancelled = true;
        m_decodeThread->wait();
        delete m_decodeThread;
    }
    
    delete m_original;
}

void
ResamplingWavFileReader::cancelled()
{
    m_cancelled = true;
}

void
ResamplingWavFileReader::DecodeThread::run()
{
    if (m_reader->m_cacheMode == CacheInTemporaryFile) {
        m_reader->startSerialised("ResamplingWavFileReader::Decode");
    }

    size_t blockSize = 16384;
    size_t total = m_reader->m_original->getFrameCount();
    
    SampleBlock block;
    
    for (size_t i = 0; i < total; i += blockSize) {
        
        size_t count = blockSize;
        if (i + count > total) count = total - i;
        
        m_reader->m_original->getInterleavedFrames(i, count, block);
        m_reader->addBlock(block);

        if (m_reader->m_cancelled) break;
    }
    
    if (m_reader->isDecodeCacheInitialised()) m_reader->finishDecodeCache();
    m_reader->m_completion = 100;

    m_reader->endSerialised();

    delete m_reader->m_original;
    m_reader->m_original = 0;
} 

void
ResamplingWavFileReader::addBlock(const SampleBlock &frames)
{
    addSamplesToDecodeCache(frames);

    m_processed += frames.size();

    float ratio = float(m_sampleRate) / float(m_fileRate);

    int progress = lrint((float(m_processed) * ratio * 100) /
                         float(m_original->getFrameCount()));

    if (progress > 99) progress = 99;
    m_completion = progress;
    
    if (m_reporter) {
        m_reporter->setProgress(progress);
    }
}

void
ResamplingWavFileReader::getSupportedExtensions(std::set<QString> &extensions)
{
    WavFileReader::getSupportedExtensions(extensions);
}

bool
ResamplingWavFileReader::supportsExtension(QString extension)
{
    return WavFileReader::supportsExtension(extension);
}

bool
ResamplingWavFileReader::supportsContentType(QString type)
{
    return WavFileReader::supportsContentType(type);
}

bool
ResamplingWavFileReader::supports(FileSource &source)
{
    return WavFileReader::supports(source);
}


