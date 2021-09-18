/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "WritableWaveFileModel.h"

#include "base/TempDirectory.h"
#include "base/Exceptions.h"

#include "fileio/WavFileWriter.h"
#include "fileio/WavFileReader.h"

#include <QDir>
#include <QTextStream>

#include <cassert>
#include <iostream>
#include <stdint.h>

//#define DEBUG_WRITABLE_WAVE_FILE_MODEL 1

WritableWaveFileModel::WritableWaveFileModel(sv_samplerate_t sampleRate,
					     int channels,
					     QString path) :
    m_model(0),
    m_writer(0),
    m_reader(0),
    m_sampleRate(sampleRate),
    m_channels(channels),
    m_frameCount(0),
    m_startFrame(0),
    m_completion(0)
{
    if (path.isEmpty()) {
        try {
            QDir dir(TempDirectory::getInstance()->getPath());
            path = dir.filePath(QString("written_%1.wav")
                                .arg((intptr_t)this));
        } catch (DirectoryCreationFailed f) {
            cerr << "WritableWaveFileModel: Failed to create temporary directory" << endl;
            return;
        }
    }

    // Write directly to the target file, so that we can do
    // incremental writes and concurrent reads
    m_writer = new WavFileWriter(path, sampleRate, channels,
                                 WavFileWriter::WriteToTarget);
    if (!m_writer->isOK()) {
        cerr << "WritableWaveFileModel: Error in creating WAV file writer: " << m_writer->getError() << endl;
        delete m_writer; 
        m_writer = 0;
        return;
    }

    FileSource source(m_writer->getPath());

    m_reader = new WavFileReader(source, true);
    if (!m_reader->getError().isEmpty()) {
        cerr << "WritableWaveFileModel: Error in creating wave file reader" << endl;
        delete m_reader;
        m_reader = 0;
        return;
    }
    
    m_model = new WaveFileModel(source, m_reader);
    if (!m_model->isOK()) {
        cerr << "WritableWaveFileModel: Error in creating wave file model" << endl;
        delete m_model;
        m_model = 0;
        delete m_reader;
        m_reader = 0;
        return;
    }
    m_model->setStartFrame(m_startFrame);

    connect(m_model, &Model::modelChanged, this, &Model::modelChanged);
    connect(m_model, &Model::modelChangedWithin,
            this, &Model::modelChangedWithin);
}

WritableWaveFileModel::~WritableWaveFileModel()
{
    delete m_model;
    delete m_writer;
    delete m_reader;
}

void
WritableWaveFileModel::setStartFrame(sv_frame_t startFrame)
{
    m_startFrame = startFrame;
    if (m_model) m_model->setStartFrame(startFrame);
}

bool
WritableWaveFileModel::addSamples(float **samples, sv_frame_t count)
{
    if (!m_writer) return false;

#ifdef DEBUG_WRITABLE_WAVE_FILE_MODEL
//    cerr << "WritableWaveFileModel::addSamples(" << count << ")" << endl;
#endif

    if (!m_writer->writeSamples(samples, count)) {
        cerr << "ERROR: WritableWaveFileModel::addSamples: writer failed: " << m_writer->getError() << endl;
        return false;
    }

    m_frameCount += count;

    static int updateCounter = 0;

    if (m_reader && m_reader->getChannelCount() == 0) {
#ifdef DEBUG_WRITABLE_WAVE_FILE_MODEL
        cerr << "WritableWaveFileModel::addSamples(" << count << "): calling updateFrameCount (initial)" << endl;
#endif
        m_reader->updateFrameCount();
    } else if (++updateCounter == 100) {
#ifdef DEBUG_WRITABLE_WAVE_FILE_MODEL
        cerr << "WritableWaveFileModel::addSamples(" << count << "): calling updateFrameCount (periodic)" << endl;
#endif
        if (m_reader) m_reader->updateFrameCount();
        updateCounter = 0;
    }

    return true;
}

bool
WritableWaveFileModel::isOK() const
{
    bool ok = (m_writer && m_writer->isOK());
//    cerr << "WritableWaveFileModel::isOK(): ok = " << ok << endl;
    return ok;
}

bool
WritableWaveFileModel::isReady(int *completion) const
{
    if (completion) *completion = m_completion;
    return (m_completion == 100);
}

void
WritableWaveFileModel::setCompletion(int completion)
{
    m_completion = completion;
    if (completion == 100) {
        if (m_reader) m_reader->updateDone();
    }
}

sv_frame_t
WritableWaveFileModel::getFrameCount() const
{
//    cerr << "WritableWaveFileModel::getFrameCount: count = " << m_frameCount << endl;
    return m_frameCount;
}

sv_frame_t
WritableWaveFileModel::getData(int channel, sv_frame_t start, sv_frame_t count,
                               float *buffer) const
{
    if (!m_model || m_model->getChannelCount() == 0) return 0;
    return m_model->getData(channel, start, count, buffer);
}

sv_frame_t
WritableWaveFileModel::getData(int channel, sv_frame_t start, sv_frame_t count,
                               double *buffer) const
{
    if (!m_model || m_model->getChannelCount() == 0) return 0;
    return m_model->getData(channel, start, count, buffer);
}

sv_frame_t
WritableWaveFileModel::getData(int fromchannel, int tochannel,
                               sv_frame_t start, sv_frame_t count,
                               float **buffers) const
{
    if (!m_model || m_model->getChannelCount() == 0) return 0;
    return m_model->getData(fromchannel, tochannel, start, count, buffers);
}    

int
WritableWaveFileModel::getSummaryBlockSize(int desired) const
{
    if (!m_model) return desired;
    return m_model->getSummaryBlockSize(desired);
}

void
WritableWaveFileModel::getSummaries(int channel, sv_frame_t start, sv_frame_t count,
                                    RangeBlock &ranges,
                                    int &blockSize) const
{
    ranges.clear();
    if (!m_model || m_model->getChannelCount() == 0) return;
    m_model->getSummaries(channel, start, count, ranges, blockSize);
}

WritableWaveFileModel::Range
WritableWaveFileModel::getSummary(int channel, sv_frame_t start, sv_frame_t count) const
{
    if (!m_model || m_model->getChannelCount() == 0) return Range();
    return m_model->getSummary(channel, start, count);
}

void
WritableWaveFileModel::toXml(QTextStream &out,
                             QString indent,
                             QString extraAttributes) const
{
    // We don't actually write the data to XML.  We just write a brief
    // description of the model.  Any code that uses this class is
    // going to need to be aware that it will have to make separate
    // arrangements for the audio file itself.

    Model::toXml
        (out, indent,
         QString("type=\"writablewavefile\" file=\"%1\" channels=\"%2\" %3")
         .arg(encodeEntities(m_writer->getPath()))
         .arg(m_model->getChannelCount()).arg(extraAttributes));
}

