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

#include "WaveFileModel.h"

#include "fileio/AudioFileReader.h"
#include "fileio/AudioFileReaderFactory.h"

#include "system/System.h"

#include "base/Preferences.h"

#include <QFileInfo>
#include <QTextStream>

#include <iostream>
#include <unistd.h>
#include <cmath>
#include <sndfile.h>

#include <cassert>

//#define DEBUG_WAVE_FILE_MODEL 1

PowerOfSqrtTwoZoomConstraint
WaveFileModel::m_zoomConstraint;

WaveFileModel::WaveFileModel(FileSource source, sv_samplerate_t targetRate) :
    m_source(source),
    m_path(source.getLocation()),
    m_reader(0),
    m_myReader(true),
    m_startFrame(0),
    m_fillThread(0),
    m_updateTimer(0),
    m_lastFillExtent(0),
    m_exiting(false),
    m_lastDirectReadStart(0),
    m_lastDirectReadCount(0)
{
    m_source.waitForData();
    if (m_source.isOK()) {
        bool normalise = Preferences::getInstance()->getNormaliseAudio();
        m_reader = AudioFileReaderFactory::createThreadingReader
            (m_source, targetRate, normalise);
        if (m_reader) {
            cerr << "WaveFileModel::WaveFileModel: reader rate: "
                      << m_reader->getSampleRate() << endl;
        }
    }
    if (m_reader) setObjectName(m_reader->getTitle());
    if (objectName() == "") setObjectName(QFileInfo(m_path).fileName());
    if (isOK()) fillCache();
}

WaveFileModel::WaveFileModel(FileSource source, AudioFileReader *reader) :
    m_source(source),
    m_path(source.getLocation()),
    m_reader(0),
    m_myReader(false),
    m_startFrame(0),
    m_fillThread(0),
    m_updateTimer(0),
    m_lastFillExtent(0),
    m_exiting(false)
{
    m_reader = reader;
    if (m_reader) setObjectName(m_reader->getTitle());
    if (objectName() == "") setObjectName(QFileInfo(m_path).fileName());
    fillCache();
}

WaveFileModel::~WaveFileModel()
{
    m_exiting = true;
    if (m_fillThread) m_fillThread->wait();
    if (m_myReader) delete m_reader;
    m_reader = 0;
}

bool
WaveFileModel::isOK() const
{
    return m_reader && m_reader->isOK();
}

bool
WaveFileModel::isReady(int *completion) const
{
    bool ready = (isOK() && (m_fillThread == 0));
    double c = double(m_lastFillExtent) / double(getEndFrame() - getStartFrame());
    static int prevCompletion = 0;
    if (completion) {
        *completion = int(c * 100.0 + 0.01);
        if (m_reader) {
            int decodeCompletion = m_reader->getDecodeCompletion();
            if (decodeCompletion < 90) *completion = decodeCompletion;
            else *completion = std::min(*completion, decodeCompletion);
        }
        if (*completion != 0 &&
            *completion != 100 &&
            prevCompletion != 0 &&
            prevCompletion > *completion) {
            // just to avoid completion going backwards
            *completion = prevCompletion;
        }
        prevCompletion = *completion;
    }
#ifdef DEBUG_WAVE_FILE_MODEL
    cerr << "WaveFileModel::isReady(): ready = " << ready << ", completion = " << (completion ? *completion : -1) << endl;
#endif
    return ready;
}

sv_frame_t
WaveFileModel::getFrameCount() const
{
    if (!m_reader) return 0;
    return m_reader->getFrameCount();
}

int
WaveFileModel::getChannelCount() const
{
    if (!m_reader) return 0;
    return m_reader->getChannelCount();
}

sv_samplerate_t
WaveFileModel::getSampleRate() const 
{
    if (!m_reader) return 0;
    return m_reader->getSampleRate();
}

sv_samplerate_t
WaveFileModel::getNativeRate() const 
{
    if (!m_reader) return 0;
    sv_samplerate_t rate = m_reader->getNativeRate();
    if (rate == 0) rate = getSampleRate();
    return rate;
}

QString
WaveFileModel::getTitle() const
{
    QString title;
    if (m_reader) title = m_reader->getTitle();
    if (title == "") title = objectName();
    return title;
}

QString
WaveFileModel::getMaker() const
{
    if (m_reader) return m_reader->getMaker();
    return "";
}

QString
WaveFileModel::getLocation() const
{
    if (m_reader) return m_reader->getLocation();
    return "";
}

QString
WaveFileModel::getLocalFilename() const
{
    if (m_reader) return m_reader->getLocalFilename();
    return "";
}
    
sv_frame_t
WaveFileModel::getData(int channel, sv_frame_t start, sv_frame_t count,
                       float *buffer) const
{
    // Always read these directly from the file. 
    // This is used for e.g. audio playback.
    // Could be much more efficient (although compiler optimisation will help)

#ifdef DEBUG_WAVE_FILE_MODEL
    cout << "WaveFileModel::getData[" << this << "]: " << channel << ", " << start << ", " << count << ", " << buffer << endl;
#endif

    if (start >= m_startFrame) {
        start -= m_startFrame;
    } else {
        for (sv_frame_t i = 0; i < count; ++i) {
            buffer[i] = 0.f;
        }
        if (count <= m_startFrame - start) {
            return 0;
        } else {
            count -= (m_startFrame - start);
            start = 0;
        }
    }

    if (!m_reader || !m_reader->isOK() || count == 0) {
        for (sv_frame_t i = 0; i < count; ++i) buffer[i] = 0.f;
        return 0;
    }

#ifdef DEBUG_WAVE_FILE_MODEL
//    cerr << "WaveFileModel::getValues(" << channel << ", "
//              << start << ", " << end << "): calling reader" << endl;
#endif

    int channels = getChannelCount();

    SampleBlock frames = m_reader->getInterleavedFrames(start, count);

    sv_frame_t i = 0;

    int ch0 = channel, ch1 = channel;
    if (channel == -1) {
	ch0 = 0;
	ch1 = channels - 1;
    }
    
    while (i < count) {

	buffer[i] = 0.0;

	for (int ch = ch0; ch <= ch1; ++ch) {

	    sv_frame_t index = i * channels + ch;
	    if (index >= (sv_frame_t)frames.size()) break;
            
	    float sample = frames[index];
	    buffer[i] += sample;
	}

	++i;
    }

    return i;
}

sv_frame_t
WaveFileModel::getData(int channel, sv_frame_t start, sv_frame_t count,
                       double *buffer) const
{
#ifdef DEBUG_WAVE_FILE_MODEL
    cout << "WaveFileModel::getData(double)[" << this << "]: " << channel << ", " << start << ", " << count << ", " << buffer << endl;
#endif

    if (start > m_startFrame) {
        start -= m_startFrame;
    } else {
        for (sv_frame_t i = 0; i < count; ++i) buffer[i] = 0.0;
        if (count <= m_startFrame - start) {
            return 0;
        } else {
            count -= (m_startFrame - start);
            start = 0;
        }
    }

    if (!m_reader || !m_reader->isOK() || count == 0) {
        for (sv_frame_t i = 0; i < count; ++i) buffer[i] = 0.0;
        return 0;
    }

    int channels = getChannelCount();

    SampleBlock frames = m_reader->getInterleavedFrames(start, count);

    sv_frame_t i = 0;

    int ch0 = channel, ch1 = channel;
    if (channel == -1) {
	ch0 = 0;
	ch1 = channels - 1;
    }

    while (i < count) {

	buffer[i] = 0.0;

	for (int ch = ch0; ch <= ch1; ++ch) {

	    sv_frame_t index = i * channels + ch;
	    if (index >= (sv_frame_t)frames.size()) break;
            
	    float sample = frames[index];
	    buffer[i] += sample;
	}

	++i;
    }

    return i;
}

sv_frame_t
WaveFileModel::getData(int fromchannel, int tochannel,
                       sv_frame_t start, sv_frame_t count,
                       float **buffer) const
{
#ifdef DEBUG_WAVE_FILE_MODEL
    cout << "WaveFileModel::getData[" << this << "]: " << fromchannel << "," << tochannel << ", " << start << ", " << count << ", " << buffer << endl;
#endif

    int channels = getChannelCount();

    if (fromchannel > tochannel) {
        cerr << "ERROR: WaveFileModel::getData: fromchannel ("
                  << fromchannel << ") > tochannel (" << tochannel << ")"
                  << endl;
        return 0;
    }

    if (tochannel >= channels) {
        cerr << "ERROR: WaveFileModel::getData: tochannel ("
                  << tochannel << ") >= channel count (" << channels << ")"
                  << endl;
        return 0;
    }

    if (fromchannel == tochannel) {
        return getData(fromchannel, start, count, buffer[0]);
    }

    int reqchannels = (tochannel - fromchannel) + 1;

    // Always read these directly from the file. 
    // This is used for e.g. audio playback.
    // Could be much more efficient (although compiler optimisation will help)

    if (start >= m_startFrame) {
        start -= m_startFrame;
    } else {
        for (int c = 0; c < reqchannels; ++c) {
            for (sv_frame_t i = 0; i < count; ++i) buffer[c][i] = 0.f;
        }
        if (count <= m_startFrame - start) {
            return 0;
        } else {
            count -= (m_startFrame - start);
            start = 0;
        }
    }

    if (!m_reader || !m_reader->isOK() || count == 0) {
        for (int c = 0; c < reqchannels; ++c) {
            for (sv_frame_t i = 0; i < count; ++i) buffer[c][i] = 0.f;
        }
        return 0;
    }

    SampleBlock frames = m_reader->getInterleavedFrames(start, count);

    sv_frame_t i = 0;

    sv_frame_t index = 0, available = frames.size();

    while (i < count) {

        if (index >= available) break;

        int destc = 0;

        for (int c = 0; c < channels; ++c) {
            
            if (c >= fromchannel && c <= tochannel) {
                buffer[destc][i] = frames[index];
                ++destc;
            }

            ++index;
        }

        ++i;
    }

    return i;
}

int
WaveFileModel::getSummaryBlockSize(int desired) const
{
    int cacheType = 0;
    int power = m_zoomConstraint.getMinCachePower();
    int roundedBlockSize = m_zoomConstraint.getNearestBlockSize
        (desired, cacheType, power, ZoomConstraint::RoundDown);
    if (cacheType != 0 && cacheType != 1) {
        // We will be reading directly from file, so can satisfy any
        // blocksize requirement
        return desired;
    } else {
        return roundedBlockSize;
    }
}    

void
WaveFileModel::getSummaries(int channel, sv_frame_t start, sv_frame_t count,
                            RangeBlock &ranges, int &blockSize) const
{
    ranges.clear();
    if (!isOK()) return;
    ranges.reserve((count / blockSize) + 1);

    if (start > m_startFrame) start -= m_startFrame;
    else if (count <= m_startFrame - start) return;
    else {
        count -= (m_startFrame - start);
        start = 0;
    }

    int cacheType = 0;
    int power = m_zoomConstraint.getMinCachePower();
    int roundedBlockSize = m_zoomConstraint.getNearestBlockSize
        (blockSize, cacheType, power, ZoomConstraint::RoundDown);

    int channels = getChannelCount();

    if (cacheType != 0 && cacheType != 1) {

	// We need to read directly from the file.  We haven't got
	// this cached.  Hope the requested area is small.  This is
	// not optimal -- we'll end up reading the same frames twice
	// for stereo files, in two separate calls to this method.
	// We could fairly trivially handle this for most cases that
	// matter by putting a single cache in getInterleavedFrames
	// for short queries.

        m_directReadMutex.lock();

        if (m_lastDirectReadStart != start ||
            m_lastDirectReadCount != count ||
            m_directRead.empty()) {

            m_directRead = m_reader->getInterleavedFrames(start, count);
            m_lastDirectReadStart = start;
            m_lastDirectReadCount = count;
        }

	float max = 0.0, min = 0.0, total = 0.0;
	sv_frame_t i = 0, got = 0;

	while (i < count) {

	    sv_frame_t index = i * channels + channel;
	    if (index >= (sv_frame_t)m_directRead.size()) break;
            
	    float sample = m_directRead[index];
            if (sample > max || got == 0) max = sample;
	    if (sample < min || got == 0) min = sample;
            total += fabsf(sample);

	    ++i;
            ++got;
            
            if (got == blockSize) {
                ranges.push_back(Range(min, max, total / float(got)));
                min = max = total = 0.0f;
                got = 0;
	    }
	}

        m_directReadMutex.unlock();

	if (got > 0) {
            ranges.push_back(Range(min, max, total / float(got)));
	}

	return;

    } else {

	QMutexLocker locker(&m_mutex);
    
	const RangeBlock &cache = m_cache[cacheType];

        blockSize = roundedBlockSize;

	sv_frame_t cacheBlock, div;
        
	if (cacheType == 0) {
	    cacheBlock = (1 << m_zoomConstraint.getMinCachePower());
            div = (1 << power) / cacheBlock;
	} else {
	    cacheBlock = sv_frame_t((1 << m_zoomConstraint.getMinCachePower()) * sqrt(2.) + 0.01);
            div = sv_frame_t(((1 << power) * sqrt(2.) + 0.01) / double(cacheBlock));
	}

	sv_frame_t startIndex = start / cacheBlock;
	sv_frame_t endIndex = (start + count) / cacheBlock;

	float max = 0.0, min = 0.0, total = 0.0;
	sv_frame_t i = 0, got = 0;

#ifdef DEBUG_WAVE_FILE_MODEL
	cerr << "blockSize is " << blockSize << ", cacheBlock " << cacheBlock << ", start " << start << ", count " << count << " (frame count " << getFrameCount() << "), power is " << power << ", div is " << div << ", startIndex " << startIndex << ", endIndex " << endIndex << endl;
#endif

	for (i = 0; i <= endIndex - startIndex; ) {
        
	    sv_frame_t index = (i + startIndex) * channels + channel;
	    if (index >= (sv_frame_t)cache.size()) break;
            
            const Range &range = cache[index];
            if (range.max() > max || got == 0) max = range.max();
            if (range.min() < min || got == 0) min = range.min();
            total += range.absmean();
            
	    ++i;
            ++got;
            
	    if (got == div) {
		ranges.push_back(Range(min, max, total / float(got)));
                min = max = total = 0.0f;
                got = 0;
	    }
	}
		
	if (got > 0) {
            ranges.push_back(Range(min, max, total / float(got)));
	}
    }

#ifdef DEBUG_WAVE_FILE_MODEL
    cerr << "returning " << ranges.size() << " ranges" << endl;
#endif
    return;
}

WaveFileModel::Range
WaveFileModel::getSummary(int channel, sv_frame_t start, sv_frame_t count) const
{
    Range range;
    if (!isOK()) return range;

    if (start > m_startFrame) start -= m_startFrame;
    else if (count <= m_startFrame - start) return range;
    else {
        count -= (m_startFrame - start);
        start = 0;
    }

    int blockSize;
    for (blockSize = 1; blockSize <= count; blockSize *= 2);
    if (blockSize > 1) blockSize /= 2;

    bool first = false;

    sv_frame_t blockStart = (start / blockSize) * blockSize;
    sv_frame_t blockEnd = ((start + count) / blockSize) * blockSize;

    if (blockStart < start) blockStart += blockSize;
        
    if (blockEnd > blockStart) {
        RangeBlock ranges;
        getSummaries(channel, blockStart, blockEnd - blockStart, ranges, blockSize);
        for (int i = 0; i < (int)ranges.size(); ++i) {
            if (first || ranges[i].min() < range.min()) range.setMin(ranges[i].min());
            if (first || ranges[i].max() > range.max()) range.setMax(ranges[i].max());
            if (first || ranges[i].absmean() < range.absmean()) range.setAbsmean(ranges[i].absmean());
            first = false;
        }
    }

    if (blockStart > start) {
        Range startRange = getSummary(channel, start, blockStart - start);
        range.setMin(std::min(range.min(), startRange.min()));
        range.setMax(std::max(range.max(), startRange.max()));
        range.setAbsmean(std::min(range.absmean(), startRange.absmean()));
    }

    if (blockEnd < start + count) {
        Range endRange = getSummary(channel, blockEnd, start + count - blockEnd);
        range.setMin(std::min(range.min(), endRange.min()));
        range.setMax(std::max(range.max(), endRange.max()));
        range.setAbsmean(std::min(range.absmean(), endRange.absmean()));
    }

    return range;
}

void
WaveFileModel::fillCache()
{
    m_mutex.lock();

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(fillTimerTimedOut()));
    m_updateTimer->start(100);

    m_fillThread = new RangeCacheFillThread(*this);
    connect(m_fillThread, SIGNAL(finished()), this, SLOT(cacheFilled()));

    m_mutex.unlock();
    m_fillThread->start();

#ifdef DEBUG_WAVE_FILE_MODEL
    cerr << "WaveFileModel::fillCache: started fill thread" << endl;
#endif
}   

void
WaveFileModel::fillTimerTimedOut()
{
    if (m_fillThread) {
	sv_frame_t fillExtent = m_fillThread->getFillExtent();
#ifdef DEBUG_WAVE_FILE_MODEL
        cerr << "WaveFileModel::fillTimerTimedOut: extent = " << fillExtent << endl;
#endif
	if (fillExtent > m_lastFillExtent) {
	    emit modelChangedWithin(m_lastFillExtent, fillExtent);
	    m_lastFillExtent = fillExtent;
	}
    } else {
#ifdef DEBUG_WAVE_FILE_MODEL
        cerr << "WaveFileModel::fillTimerTimedOut: no thread" << endl;
#endif
	emit modelChanged();
    }
}

void
WaveFileModel::cacheFilled()
{
    m_mutex.lock();
    delete m_fillThread;
    m_fillThread = 0;
    delete m_updateTimer;
    m_updateTimer = 0;
    m_mutex.unlock();
    if (getEndFrame() > m_lastFillExtent) {
        emit modelChangedWithin(m_lastFillExtent, getEndFrame());
    }
    emit modelChanged();
    emit ready();
#ifdef DEBUG_WAVE_FILE_MODEL
    cerr << "WaveFileModel::cacheFilled" << endl;
#endif
}

void
WaveFileModel::RangeCacheFillThread::run()
{
    int cacheBlockSize[2];
    cacheBlockSize[0] = (1 << m_model.m_zoomConstraint.getMinCachePower());
    cacheBlockSize[1] = (int((1 << m_model.m_zoomConstraint.getMinCachePower()) *
                                        sqrt(2.) + 0.01));
    
    sv_frame_t frame = 0;
    const sv_frame_t readBlockSize = 16384;
    SampleBlock block;

    if (!m_model.isOK()) return;
    
    int channels = m_model.getChannelCount();
    bool updating = m_model.m_reader->isUpdating();

    if (updating) {
        while (channels == 0 && !m_model.m_exiting) {
//            cerr << "WaveFileModel::fill: Waiting for channels..." << endl;
            sleep(1);
            channels = m_model.getChannelCount();
        }
    }

    Range *range = new Range[2 * channels];
    float *means = new float[2 * channels];
    int count[2];
    count[0] = count[1] = 0;
    for (int i = 0; i < 2 * channels; ++i) {
        means[i] = 0.f;
    }

    bool first = true;

    while (first || updating) {

        updating = m_model.m_reader->isUpdating();
        m_frameCount = m_model.getFrameCount();

//        cerr << "WaveFileModel::fill: frame = " << frame << ", count = " << m_frameCount << endl;

        while (frame < m_frameCount) {

//            cerr << "WaveFileModel::fill inner loop: frame = " << frame << ", count = " << m_frameCount << ", blocksize " << readBlockSize << endl;

            if (updating && (frame + readBlockSize > m_frameCount)) break;

            block = m_model.m_reader->getInterleavedFrames(frame, readBlockSize);

//            cerr << "block is " << block.size() << endl;

            for (sv_frame_t i = 0; i < readBlockSize; ++i) {
		
                if (channels * i + channels > (int)block.size()) break;

                for (int ch = 0; ch < channels; ++ch) {

                    sv_frame_t index = channels * i + ch;
                    float sample = block[index];
                    
                    for (int cacheType = 0; cacheType < 2; ++cacheType) { // cache type
                        
                        sv_frame_t rangeIndex = ch * 2 + cacheType;
                        range[rangeIndex].sample(sample);
                        means[rangeIndex] += fabsf(sample);
                    }
                }

                //!!! this looks like a ludicrous way to do synchronisation
                QMutexLocker locker(&m_model.m_mutex);

                for (int cacheType = 0; cacheType < 2; ++cacheType) {

                    if (++count[cacheType] == cacheBlockSize[cacheType]) {
                        
                        for (int ch = 0; ch < int(channels); ++ch) {
                            int rangeIndex = ch * 2 + cacheType;
                            means[rangeIndex] = means[rangeIndex] / float(count[cacheType]);
                            range[rangeIndex].setAbsmean(means[rangeIndex]);
                            m_model.m_cache[cacheType].push_back(range[rangeIndex]);
                            range[rangeIndex] = Range();
                            means[rangeIndex] = 0.f;
                        }

                        count[cacheType] = 0;
                    }
                }
                
                ++frame;
            }
            
            if (m_model.m_exiting) break;
            
            m_fillExtent = frame;
        }

//        cerr << "WaveFileModel: inner loop ended" << endl;

        first = false;
        if (m_model.m_exiting) break;
        if (updating) {
//            cerr << "sleeping..." << endl;
            sleep(1);
        }
    }

    if (!m_model.m_exiting) {

        QMutexLocker locker(&m_model.m_mutex);

        for (int cacheType = 0; cacheType < 2; ++cacheType) {

            if (count[cacheType] > 0) {

                for (int ch = 0; ch < int(channels); ++ch) {
                    int rangeIndex = ch * 2 + cacheType;
                    means[rangeIndex] = means[rangeIndex] / float(count[cacheType]);
                    range[rangeIndex].setAbsmean(means[rangeIndex]);
                    m_model.m_cache[cacheType].push_back(range[rangeIndex]);
                    range[rangeIndex] = Range();
                    means[rangeIndex] = 0.f;
                }

                count[cacheType] = 0;
            }
            
            const Range &rr = *m_model.m_cache[cacheType].begin();
            MUNLOCK(&rr, m_model.m_cache[cacheType].capacity() * sizeof(Range));
        }
    }
    
    delete[] means;
    delete[] range;

    m_fillExtent = m_frameCount;

#ifdef DEBUG_WAVE_FILE_MODEL        
    for (int cacheType = 0; cacheType < 2; ++cacheType) {
        cerr << "Cache type " << cacheType << " now contains " << m_model.m_cache[cacheType].size() << " ranges" << endl;
    }
#endif
}

void
WaveFileModel::toXml(QTextStream &out,
                     QString indent,
                     QString extraAttributes) const
{
    Model::toXml(out, indent,
                 QString("type=\"wavefile\" file=\"%1\" %2")
                 .arg(encodeEntities(m_path)).arg(extraAttributes));
}

    
