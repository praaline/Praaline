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

#include "AudioCallbackPlaySource.h"

#include "AudioGenerator.h"

#include "data/model/Model.h"
#include "base/ViewManagerBase.h"
#include "base/PlayParameterRepository.h"
#include "base/Preferences.h"
#include "data/model/DenseTimeValueModel.h"
#include "data/model/WaveFileModel.h"
#include "data/model/SparseOneDimensionalModel.h"
#include "plugin/RealTimePluginInstance.h"

#include "AudioCallbackPlayTarget.h"

#include <rubberband/RubberBandStretcher.h>
using namespace RubberBand;

#include <iostream>
#include <cassert>

//#define DEBUG_AUDIO_PLAY_SOURCE 1
//#define DEBUG_AUDIO_PLAY_SOURCE_PLAYING 1

static const int DEFAULT_RING_BUFFER_SIZE = 131071;

AudioCallbackPlaySource::AudioCallbackPlaySource(ViewManagerBase *manager,
                                                 QString clientName) :
    m_viewManager(manager),
    m_audioGenerator(new AudioGenerator()),
    m_clientName(clientName),
    m_readBuffers(0),
    m_writeBuffers(0),
    m_readBufferFill(0),
    m_writeBufferFill(0),
    m_bufferScavenger(1),
    m_sourceChannelCount(0),
    m_blockSize(1024),
    m_sourceSampleRate(0),
    m_targetSampleRate(0),
    m_playLatency(0),
    m_target(0),
    m_lastRetrievalTimestamp(0.0),
    m_lastRetrievedBlockSize(0),
    m_trustworthyTimestamps(true),
    m_lastCurrentFrame(0),
    m_playing(false),
    m_exiting(false),
    m_lastModelEndFrame(0),
    m_ringBufferSize(DEFAULT_RING_BUFFER_SIZE),
    m_outputLeft(0.0),
    m_outputRight(0.0),
    m_auditioningPlugin(0),
    m_auditioningPluginBypassed(false),
    m_playStartFrame(0),
    m_playStartFramePassed(false),
    m_timeStretcher(0),
    m_monoStretcher(0),
    m_stretchRatio(1.0),
    m_stretchMono(false),
    m_stretcherInputCount(0),
    m_stretcherInputs(0),
    m_stretcherInputSizes(0),
    m_fillThread(0),
    m_converter(0),
    m_crapConverter(0),
    m_resampleQuality(Preferences::getInstance()->getResampleQuality())
{
    m_viewManager->setAudioPlaySource(this);

    connect(m_viewManager, &ViewManagerBase::selectionChanged,
	    this, &AudioCallbackPlaySource::selectionChanged);
    connect(m_viewManager, SIGNAL(playLoopModeChanged()),
	    this, SLOT(playLoopModeChanged()));
    connect(m_viewManager, SIGNAL(playSelectionModeChanged()),
	    this, SLOT(playSelectionModeChanged()));

    connect(this, SIGNAL(playStatusChanged(bool)),
            m_viewManager, SLOT(playStatusChanged(bool)));

    connect(PlayParameterRepository::getInstance(),
	    SIGNAL(playParametersChanged(PlayParameters *)),
	    this, SLOT(playParametersChanged(PlayParameters *)));

    connect(Preferences::getInstance(),
            &PropertyContainer::propertyChanged,
            this, &AudioCallbackPlaySource::preferenceChanged);
}

AudioCallbackPlaySource::~AudioCallbackPlaySource()
{
#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "AudioCallbackPlaySource::~AudioCallbackPlaySource entering" << endl;
#endif
    m_exiting = true;

    if (m_fillThread) {
#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySource dtor: awakening thread" << endl;
#endif
        m_condition.wakeAll();
	m_fillThread->wait();
	delete m_fillThread;
    }

    clearModels();
    
    if (m_readBuffers != m_writeBuffers) {
	delete m_readBuffers;
    }

    delete m_writeBuffers;

    delete m_audioGenerator;

    for (int i = 0; i < m_stretcherInputCount; ++i) {
        delete[] m_stretcherInputs[i];
    }
    delete[] m_stretcherInputSizes;
    delete[] m_stretcherInputs;

    delete m_timeStretcher;
    delete m_monoStretcher;

    m_bufferScavenger.scavenge(true);
    m_pluginScavenger.scavenge(true);
#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "AudioCallbackPlaySource::~AudioCallbackPlaySource finishing" << endl;
#endif
}

void
AudioCallbackPlaySource::addModel(Model *model)
{
    if (m_models.find(model) != m_models.end()) return;

    bool willPlay = m_audioGenerator->addModel(model);

    m_mutex.lock();

    m_models.insert(model);
    if (model->getEndFrame() > m_lastModelEndFrame) {
	m_lastModelEndFrame = model->getEndFrame();
    }

    bool buffersChanged = false, srChanged = false;

    int modelChannels = 1;
    DenseTimeValueModel *dtvm = dynamic_cast<DenseTimeValueModel *>(model);
    if (dtvm) modelChannels = dtvm->getChannelCount();
    if (modelChannels > m_sourceChannelCount) {
	m_sourceChannelCount = modelChannels;
    }

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySource: Adding model with " << modelChannels << " channels at rate " << model->getSampleRate() << endl;
#endif

    if (m_sourceSampleRate == 0) {

	m_sourceSampleRate = model->getSampleRate();
	srChanged = true;

    } else if (model->getSampleRate() != m_sourceSampleRate) {

        // If this is a dense time-value model and we have no other, we
        // can just switch to this model's sample rate

        if (dtvm) {

            bool conflicting = false;

            for (std::set<Model *>::const_iterator i = m_models.begin();
                 i != m_models.end(); ++i) {
                // Only wave file models can be considered conflicting --
                // writable wave file models are derived and we shouldn't
                // take their rates into account.  Also, don't give any
                // particular weight to a file that's already playing at
                // the wrong rate anyway
                WaveFileModel *wfm = dynamic_cast<WaveFileModel *>(*i);
                if (wfm && wfm != dtvm &&
                    wfm->getSampleRate() != model->getSampleRate() &&
                    wfm->getSampleRate() == m_sourceSampleRate) {
                    cerr << "AudioCallbackPlaySource::addModel: Conflicting wave file model " << *i << " found" << endl;
                    conflicting = true;
                    break;
                }
            }

            if (conflicting) {

                cerr << "AudioCallbackPlaySource::addModel: ERROR: "
                          << "New model sample rate does not match" << endl
                          << "existing model(s) (new " << model->getSampleRate()
                          << " vs " << m_sourceSampleRate
                          << "), playback will be wrong"
                          << endl;
                
                emit sampleRateMismatch(model->getSampleRate(),
                                        m_sourceSampleRate,
                                        false);
            } else {
                m_sourceSampleRate = model->getSampleRate();
                srChanged = true;
            }
        }
    }

    if (!m_writeBuffers || (int)m_writeBuffers->size() < getTargetChannelCount()) {
	clearRingBuffers(true, getTargetChannelCount());
	buffersChanged = true;
    } else {
	if (willPlay) clearRingBuffers(true);
    }

    if (buffersChanged || srChanged) {
	if (m_converter) {
	    src_delete(m_converter);
            src_delete(m_crapConverter);
	    m_converter = 0;
            m_crapConverter = 0;
	}
    }

    rebuildRangeLists();

    m_mutex.unlock();

    m_audioGenerator->setTargetChannelCount(getTargetChannelCount());

    if (!m_fillThread) {
	m_fillThread = new FillThread(*this);
	m_fillThread->start();
    }

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySource::addModel: now have " << m_models.size() << " model(s) -- emitting modelReplaced" << endl;
#endif

    if (buffersChanged || srChanged) {
	emit modelReplaced();
    }

    connect(model, &Model::modelChangedWithin,
            this, &AudioCallbackPlaySource::modelChangedWithin);

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySource::addModel: awakening thread" << endl;
#endif

    m_condition.wakeAll();
}

void
AudioCallbackPlaySource::modelChangedWithin(sv_frame_t 
#ifdef DEBUG_AUDIO_PLAY_SOURCE
                                            startFrame
#endif
                                            , sv_frame_t endFrame)
{
#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "AudioCallbackPlaySource::modelChangedWithin(" << startFrame << "," << endFrame << ")" << endl;
#endif
    if (endFrame > m_lastModelEndFrame) {
        m_lastModelEndFrame = endFrame;
        rebuildRangeLists();
    }
}

void
AudioCallbackPlaySource::removeModel(Model *model)
{
    m_mutex.lock();

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySource::removeModel(" << model << ")" << endl;
#endif

    disconnect(model, &Model::modelChangedWithin,
               this, &AudioCallbackPlaySource::modelChangedWithin);

    m_models.erase(model);

    if (m_models.empty()) {
	if (m_converter) {
	    src_delete(m_converter);
            src_delete(m_crapConverter);
	    m_converter = 0;
            m_crapConverter = 0;
	}
	m_sourceSampleRate = 0;
    }

    sv_frame_t lastEnd = 0;
    for (std::set<Model *>::const_iterator i = m_models.begin();
	 i != m_models.end(); ++i) {
#ifdef DEBUG_AUDIO_PLAY_SOURCE
	cout << "AudioCallbackPlaySource::removeModel(" << model << "): checking end frame on model " << *i << endl;
#endif
	if ((*i)->getEndFrame() > lastEnd) {
            lastEnd = (*i)->getEndFrame();
        }
#ifdef DEBUG_AUDIO_PLAY_SOURCE
	cout << "(done, lastEnd now " << lastEnd << ")" << endl;
#endif
    }
    m_lastModelEndFrame = lastEnd;

    m_audioGenerator->removeModel(model);

    m_mutex.unlock();

    clearRingBuffers();
}

void
AudioCallbackPlaySource::clearModels()
{
    m_mutex.lock();

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySource::clearModels()" << endl;
#endif

    m_models.clear();

    if (m_converter) {
	src_delete(m_converter);
        src_delete(m_crapConverter);
	m_converter = 0;
        m_crapConverter = 0;
    }

    m_lastModelEndFrame = 0;

    m_sourceSampleRate = 0;

    m_mutex.unlock();

    m_audioGenerator->clearModels();

    clearRingBuffers();
}    

void
AudioCallbackPlaySource::clearRingBuffers(bool haveLock, int count)
{
    if (!haveLock) m_mutex.lock();

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "clearRingBuffers" << endl;
#endif

    rebuildRangeLists();

    if (count == 0) {
	if (m_writeBuffers) count = int(m_writeBuffers->size());
    }

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "current playing frame = " << getCurrentPlayingFrame() << endl;

    cerr << "write buffer fill (before) = " << m_writeBufferFill << endl;
#endif
    
    m_writeBufferFill = getCurrentBufferedFrame();

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "current buffered frame = " << m_writeBufferFill << endl;
#endif

    if (m_readBuffers != m_writeBuffers) {
	delete m_writeBuffers;
    }

    m_writeBuffers = new RingBufferVector;

    for (int i = 0; i < count; ++i) {
	m_writeBuffers->push_back(new RingBuffer<float>(m_ringBufferSize));
    }

    m_audioGenerator->reset();
    
//    cout << "AudioCallbackPlaySource::clearRingBuffers: Created "
//	      << count << " write buffers" << endl;

    if (!haveLock) {
	m_mutex.unlock();
    }
}

void
AudioCallbackPlaySource::play(sv_frame_t startFrame)
{
    if (!m_sourceSampleRate) {
        cerr << "AudioCallbackPlaySource::play: No source sample rate available, not playing" << endl;
        return;
    }
    
    if (m_viewManager->getPlaySelectionMode() &&
	!m_viewManager->getSelections().empty()) {

        cerr << "AudioCallbackPlaySource::play: constraining frame " << startFrame << " to selection = ";

        startFrame = m_viewManager->constrainFrameToSelection(startFrame);

        cerr << startFrame << endl;

    } else {
	if (startFrame >= m_lastModelEndFrame) {
	    startFrame = 0;
	}
    }

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "play(" << startFrame << ") -> playback model ";
#endif

    startFrame = m_viewManager->alignReferenceToPlaybackFrame(startFrame);

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << startFrame << endl;
#endif

    // The fill thread will automatically empty its buffers before
    // starting again if we have not so far been playing, but not if
    // we're just re-seeking.
    // NO -- we can end up playing some first -- always reset here

    m_mutex.lock();

    if (m_timeStretcher) {
        m_timeStretcher->reset();
    }
    if (m_monoStretcher) {
        m_monoStretcher->reset();
    }

    m_readBufferFill = m_writeBufferFill = startFrame;
    if (m_readBuffers) {
        for (int c = 0; c < getTargetChannelCount(); ++c) {
            RingBuffer<float> *rb = getReadRingBuffer(c);
#ifdef DEBUG_AUDIO_PLAY_SOURCE
            cerr << "reset ring buffer for channel " << c << endl;
#endif
            if (rb) rb->reset();
        }
    }
    if (m_converter) src_reset(m_converter);
    if (m_crapConverter) src_reset(m_crapConverter);

    m_mutex.unlock();

    m_audioGenerator->reset();

    m_playStartFrame = startFrame;
    m_playStartFramePassed = false;
    m_playStartedAt = RealTime::zeroTime;
    if (m_target) {
        m_playStartedAt = RealTime::fromSeconds(m_target->getCurrentTime());
    }

    bool changed = !m_playing;
    m_lastRetrievalTimestamp = 0;
    m_lastCurrentFrame = 0;
    m_playing = true;

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySource::play: awakening thread" << endl;
#endif

    m_condition.wakeAll();
    if (changed) {
        emit playStatusChanged(m_playing);
        emit activity(tr("Play from %1").arg
                      (RealTime::frame2RealTime
                       (m_playStartFrame, m_sourceSampleRate).toText().c_str()));
    }
}

void
AudioCallbackPlaySource::stop()
{
#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "AudioCallbackPlaySource::stop()" << endl;
#endif
    bool changed = m_playing;
    m_playing = false;

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySource::stop: awakening thread" << endl;
#endif

    m_condition.wakeAll();
    m_lastRetrievalTimestamp = 0;
    if (changed) {
        emit playStatusChanged(m_playing);
        emit activity(tr("Stop at %1").arg
                      (RealTime::frame2RealTime
                       (m_lastCurrentFrame, m_sourceSampleRate).toText().c_str()));
    }
    m_lastCurrentFrame = 0;
}

void
AudioCallbackPlaySource::selectionChanged()
{
    if (m_viewManager->getPlaySelectionMode()) {
	clearRingBuffers();
    }
}

void
AudioCallbackPlaySource::playLoopModeChanged()
{
    clearRingBuffers();
}

void
AudioCallbackPlaySource::playSelectionModeChanged()
{
    if (!m_viewManager->getSelections().empty()) {
	clearRingBuffers();
    }
}

void
AudioCallbackPlaySource::playParametersChanged(PlayParameters *)
{
    clearRingBuffers();
}

void
AudioCallbackPlaySource::preferenceChanged(PropertyContainer::PropertyName n)
{
    if (n == "Resample Quality") {
        setResampleQuality(Preferences::getInstance()->getResampleQuality());
    }
}

void
AudioCallbackPlaySource::audioProcessingOverload()
{
    cerr << "Audio processing overload!" << endl;

    if (!m_playing) return;

    RealTimePluginInstance *ap = m_auditioningPlugin;
    if (ap && !m_auditioningPluginBypassed) {
        m_auditioningPluginBypassed = true;
        emit audioOverloadPluginDisabled();
        return;
    }

    if (m_timeStretcher &&
        m_timeStretcher->getTimeRatio() < 1.0 &&
        m_stretcherInputCount > 1 &&
        m_monoStretcher && !m_stretchMono) {
        m_stretchMono = true;
        emit audioTimeStretchMultiChannelDisabled();
        return;
    }
}

void
AudioCallbackPlaySource::setTarget(AudioCallbackPlayTarget *target, int size)
{
    m_target = target;
    cout << "AudioCallbackPlaySource::setTarget: Block size -> " << size << endl;
    if (size != 0) {
        m_blockSize = size;
    }
    if (size * 4 > m_ringBufferSize) {
        cerr << "AudioCallbackPlaySource::setTarget: Buffer size "
                  << size << " > a quarter of ring buffer size "
                  << m_ringBufferSize << ", calling for more ring buffer"
                  << endl;
        m_ringBufferSize = size * 4;
        if (m_writeBuffers && !m_writeBuffers->empty()) {
            clearRingBuffers();
        }
    }
}

int
AudioCallbackPlaySource::getTargetBlockSize() const
{
//    cout << "AudioCallbackPlaySource::getTargetBlockSize() -> " << m_blockSize << endl;
    return int(m_blockSize);
}

void
AudioCallbackPlaySource::setTargetPlayLatency(sv_frame_t latency)
{
    m_playLatency = latency;
}

sv_frame_t
AudioCallbackPlaySource::getTargetPlayLatency() const
{
    return m_playLatency;
}

sv_frame_t
AudioCallbackPlaySource::getCurrentPlayingFrame()
{
    // This method attempts to estimate which audio sample frame is
    // "currently coming through the speakers".

    sv_samplerate_t targetRate = getTargetSampleRate();
    sv_frame_t latency = m_playLatency; // at target rate
    RealTime latency_t = RealTime::zeroTime;

    if (targetRate != 0) {
        latency_t = RealTime::frame2RealTime(latency, targetRate);
    }

    return getCurrentFrame(latency_t);
}

sv_frame_t
AudioCallbackPlaySource::getCurrentBufferedFrame()
{
    return getCurrentFrame(RealTime::zeroTime);
}

sv_frame_t
AudioCallbackPlaySource::getCurrentFrame(RealTime latency_t)
{
    // We resample when filling the ring buffer, and time-stretch when
    // draining it.  The buffer contains data at the "target rate" and
    // the latency provided by the target is also at the target rate.
    // Because of the multiple rates involved, we do the actual
    // calculation using RealTime instead.

    sv_samplerate_t sourceRate = getSourceSampleRate();
    sv_samplerate_t targetRate = getTargetSampleRate();

    if (sourceRate == 0 || targetRate == 0) return 0;

    int inbuffer = 0; // at target rate

    for (int c = 0; c < getTargetChannelCount(); ++c) {
	RingBuffer<float> *rb = getReadRingBuffer(c);
	if (rb) {
	    int here = rb->getReadSpace();
	    if (c == 0 || here < inbuffer) inbuffer = here;
	}
    }

    sv_frame_t readBufferFill = m_readBufferFill;
    sv_frame_t lastRetrievedBlockSize = m_lastRetrievedBlockSize;
    double lastRetrievalTimestamp = m_lastRetrievalTimestamp;
    double currentTime = 0.0;
    if (m_target) currentTime = m_target->getCurrentTime();

    bool looping = m_viewManager->getPlayLoopMode();

    RealTime inbuffer_t = RealTime::frame2RealTime(inbuffer, targetRate);

    sv_frame_t stretchlat = 0;
    double timeRatio = 1.0;

    if (m_timeStretcher) {
        stretchlat = m_timeStretcher->getLatency();
        timeRatio = m_timeStretcher->getTimeRatio();
    }

    RealTime stretchlat_t = RealTime::frame2RealTime(stretchlat, targetRate);

    // When the target has just requested a block from us, the last
    // sample it obtained was our buffer fill frame count minus the
    // amount of read space (converted back to source sample rate)
    // remaining now.  That sample is not expected to be played until
    // the target's play latency has elapsed.  By the time the
    // following block is requested, that sample will be at the
    // target's play latency minus the last requested block size away
    // from being played.

    RealTime sincerequest_t = RealTime::zeroTime;
    RealTime lastretrieved_t = RealTime::zeroTime;

    if (m_target &&
        m_trustworthyTimestamps &&
        lastRetrievalTimestamp != 0.0) {

        lastretrieved_t = RealTime::frame2RealTime
            (lastRetrievedBlockSize, targetRate);

        // calculate number of frames at target rate that have elapsed
        // since the end of the last call to getSourceSamples

        if (m_trustworthyTimestamps && !looping) {

            // this adjustment seems to cause more problems when looping
            double elapsed = currentTime - lastRetrievalTimestamp;

            if (elapsed > 0.0) {
                sincerequest_t = RealTime::fromSeconds(elapsed);
            }
        }

    } else {

        lastretrieved_t = RealTime::frame2RealTime
            (getTargetBlockSize(), targetRate);
    }

    RealTime bufferedto_t = RealTime::frame2RealTime(readBufferFill, sourceRate);

    if (timeRatio != 1.0) {
        lastretrieved_t = lastretrieved_t / timeRatio;
        sincerequest_t = sincerequest_t / timeRatio;
        latency_t = latency_t / timeRatio;
    }

#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
    cerr << "\nbuffered to: " << bufferedto_t << ", in buffer: " << inbuffer_t << ", time ratio " << timeRatio << "\n  stretcher latency: " << stretchlat_t << ", device latency: " << latency_t << "\n  since request: " << sincerequest_t << ", last retrieved quantity: " << lastretrieved_t << endl;
#endif

    // Normally the range lists should contain at least one item each
    // -- if playback is unconstrained, that item should report the
    // entire source audio duration.

    if (m_rangeStarts.empty()) {
        rebuildRangeLists();
    }

    if (m_rangeStarts.empty()) {
        // this code is only used in case of error in rebuildRangeLists
        RealTime playing_t = bufferedto_t
            - latency_t - stretchlat_t - lastretrieved_t - inbuffer_t
            + sincerequest_t;
        if (playing_t < RealTime::zeroTime) playing_t = RealTime::zeroTime;
        sv_frame_t frame = RealTime::realTime2Frame(playing_t, sourceRate);
        return m_viewManager->alignPlaybackFrameToReference(frame);
    }

    int inRange = 0;
    int index = 0;

    for (int i = 0; i < (int)m_rangeStarts.size(); ++i) {
        if (bufferedto_t >= m_rangeStarts[i]) {
            inRange = index;
        } else {
            break;
        }
        ++index;
    }

    if (inRange >= int(m_rangeStarts.size())) {
        inRange = int(m_rangeStarts.size())-1;
    }

    RealTime playing_t = bufferedto_t;

    playing_t = playing_t
        - latency_t - stretchlat_t - lastretrieved_t - inbuffer_t
        + sincerequest_t;

    // This rather gross little hack is used to ensure that latency
    // compensation doesn't result in the playback pointer appearing
    // to start earlier than the actual playback does.  It doesn't
    // work properly (hence the bail-out in the middle) because if we
    // are playing a relatively short looped region, the playing time
    // estimated from the buffer fill frame may have wrapped around
    // the region boundary and end up being much smaller than the
    // theoretical play start frame, perhaps even for the entire
    // duration of playback!

    if (!m_playStartFramePassed) {
        RealTime playstart_t = RealTime::frame2RealTime(m_playStartFrame,
                                                        sourceRate);
        if (playing_t < playstart_t) {
//            cerr << "playing_t " << playing_t << " < playstart_t " 
//                      << playstart_t << endl;
            if (/*!!! sincerequest_t > RealTime::zeroTime && */
                m_playStartedAt + latency_t + stretchlat_t <
                RealTime::fromSeconds(currentTime)) {
//                cerr << "but we've been playing for long enough that I think we should disregard it (it probably results from loop wrapping)" << endl;
                m_playStartFramePassed = true;
            } else {
                playing_t = playstart_t;
            }
        } else {
            m_playStartFramePassed = true;
        }
    }
 
#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
    cerr << "playing_t " << playing_t;
#endif

    playing_t = playing_t - m_rangeStarts[inRange];
 
#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
    cerr << " as offset into range " << inRange << " (start =" << m_rangeStarts[inRange] << " duration =" << m_rangeDurations[inRange] << ") = " << playing_t << endl;
#endif

    while (playing_t < RealTime::zeroTime) {

        if (inRange == 0) {
            if (looping) {
                inRange = int(m_rangeStarts.size()) - 1;
            } else {
                break;
            }
        } else {
            --inRange;
        }

        playing_t = playing_t + m_rangeDurations[inRange];
    }

    playing_t = playing_t + m_rangeStarts[inRange];

#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
    cerr << "  playing time: " << playing_t << endl;
#endif

    if (!looping) {
        if (inRange == (int)m_rangeStarts.size()-1 &&
            playing_t >= m_rangeStarts[inRange] + m_rangeDurations[inRange]) {
cerr << "Not looping, inRange " << inRange << " == rangeStarts.size()-1, playing_t " << playing_t << " >= m_rangeStarts[inRange] " << m_rangeStarts[inRange] << " + m_rangeDurations[inRange] " << m_rangeDurations[inRange] << " -- stopping" << endl;
            stop();
        }
    }

    if (playing_t < RealTime::zeroTime) playing_t = RealTime::zeroTime;

    sv_frame_t frame = RealTime::realTime2Frame(playing_t, sourceRate);

    if (m_lastCurrentFrame > 0 && !looping) {
        if (frame < m_lastCurrentFrame) {
            frame = m_lastCurrentFrame;
        }
    }

    m_lastCurrentFrame = frame;

    return m_viewManager->alignPlaybackFrameToReference(frame);
}

void
AudioCallbackPlaySource::rebuildRangeLists()
{
    bool constrained = (m_viewManager->getPlaySelectionMode());

    m_rangeStarts.clear();
    m_rangeDurations.clear();

    sv_samplerate_t sourceRate = getSourceSampleRate();
    if (sourceRate == 0) return;

    RealTime end = RealTime::frame2RealTime(m_lastModelEndFrame, sourceRate);
    if (end == RealTime::zeroTime) return;

    if (!constrained) {
        m_rangeStarts.push_back(RealTime::zeroTime);
        m_rangeDurations.push_back(end);
        return;
    }

    MultiSelection::SelectionList selections = m_viewManager->getSelections();
    MultiSelection::SelectionList::const_iterator i;

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "AudioCallbackPlaySource::rebuildRangeLists" << endl;
#endif

    if (!selections.empty()) {

        for (i = selections.begin(); i != selections.end(); ++i) {
            
            RealTime start =
                (RealTime::frame2RealTime
                 (m_viewManager->alignReferenceToPlaybackFrame(i->getStartFrame()),
                  sourceRate));
            RealTime duration = 
                (RealTime::frame2RealTime
                 (m_viewManager->alignReferenceToPlaybackFrame(i->getEndFrame()) -
                  m_viewManager->alignReferenceToPlaybackFrame(i->getStartFrame()),
                  sourceRate));
            
            m_rangeStarts.push_back(start);
            m_rangeDurations.push_back(duration);
        }
    } else {
        m_rangeStarts.push_back(RealTime::zeroTime);
        m_rangeDurations.push_back(end);
    }

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "Now have " << m_rangeStarts.size() << " play ranges" << endl;
#endif
}

void
AudioCallbackPlaySource::setOutputLevels(float left, float right)
{
    m_outputLeft = left;
    m_outputRight = right;
}

bool
AudioCallbackPlaySource::getOutputLevels(float &left, float &right)
{
    left = m_outputLeft;
    right = m_outputRight;
    return true;
}

void
AudioCallbackPlaySource::setTargetSampleRate(sv_samplerate_t sr)
{
    bool first = (m_targetSampleRate == 0);

    m_targetSampleRate = sr;
    initialiseConverter();

    if (first && (m_stretchRatio != 1.f)) {
        // couldn't create a stretcher before because we had no sample
        // rate: make one now
        setTimeStretch(m_stretchRatio);
    }
}

void
AudioCallbackPlaySource::initialiseConverter()
{
    m_mutex.lock();

    if (m_converter) {
        src_delete(m_converter);
        src_delete(m_crapConverter);
        m_converter = 0;
        m_crapConverter = 0;
    }

    if (getSourceSampleRate() != getTargetSampleRate()) {

	int err = 0;

	m_converter = src_new(m_resampleQuality == 2 ? SRC_SINC_BEST_QUALITY :
                              m_resampleQuality == 1 ? SRC_SINC_MEDIUM_QUALITY :
                              m_resampleQuality == 0 ? SRC_SINC_FASTEST :
                                                       SRC_SINC_MEDIUM_QUALITY,
			      getTargetChannelCount(), &err);

        if (m_converter) {
            m_crapConverter = src_new(SRC_LINEAR,
                                      getTargetChannelCount(),
                                      &err);
        }

	if (!m_converter || !m_crapConverter) {
	    cerr
		<< "AudioCallbackPlaySource::setModel: ERROR in creating samplerate converter: "
		<< src_strerror(err) << endl;

            if (m_converter) {
                src_delete(m_converter);
                m_converter = 0;
            } 

            if (m_crapConverter) {
                src_delete(m_crapConverter);
                m_crapConverter = 0;
            }

            m_mutex.unlock();

            emit sampleRateMismatch(getSourceSampleRate(),
                                    getTargetSampleRate(),
                                    false);
	} else {

            m_mutex.unlock();

            emit sampleRateMismatch(getSourceSampleRate(),
                                    getTargetSampleRate(),
                                    true);
        }
    } else {
        m_mutex.unlock();
    }
}

void
AudioCallbackPlaySource::setResampleQuality(int q)
{
    if (q == m_resampleQuality) return;
    m_resampleQuality = q;

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cerr << "AudioCallbackPlaySource::setResampleQuality: setting to "
              << m_resampleQuality << endl;
#endif

    initialiseConverter();
}

void
AudioCallbackPlaySource::setAuditioningEffect(Auditionable *a)
{
    RealTimePluginInstance *plugin = dynamic_cast<RealTimePluginInstance *>(a);
    if (a && !plugin) {
        cerr << "WARNING: AudioCallbackPlaySource::setAuditioningEffect: auditionable object " << a << " is not a real-time plugin instance" << endl;
    }

    m_mutex.lock();
    m_auditioningPlugin = plugin;
    m_auditioningPluginBypassed = false;
    m_mutex.unlock();
}

void
AudioCallbackPlaySource::setSoloModelSet(std::set<Model *> s)
{
    m_audioGenerator->setSoloModelSet(s);
    clearRingBuffers();
}

void
AudioCallbackPlaySource::clearSoloModelSet()
{
    m_audioGenerator->clearSoloModelSet();
    clearRingBuffers();
}

sv_samplerate_t
AudioCallbackPlaySource::getTargetSampleRate() const
{
    if (m_targetSampleRate) return m_targetSampleRate;
    else return getSourceSampleRate();
}

int
AudioCallbackPlaySource::getSourceChannelCount() const
{
    return m_sourceChannelCount;
}

int
AudioCallbackPlaySource::getTargetChannelCount() const
{
    if (m_sourceChannelCount < 2) return 2;
    return m_sourceChannelCount;
}

sv_samplerate_t
AudioCallbackPlaySource::getSourceSampleRate() const
{
    return m_sourceSampleRate;
}

void
AudioCallbackPlaySource::setTimeStretch(double factor)
{
    m_stretchRatio = factor;

    if (!getTargetSampleRate()) return; // have to make our stretcher later

    if (m_timeStretcher || (factor == 1.0)) {
        // stretch ratio will be set in next process call if appropriate
    } else {
        m_stretcherInputCount = getTargetChannelCount();
        RubberBandStretcher *stretcher = new RubberBandStretcher
            (int(getTargetSampleRate()),
             m_stretcherInputCount,
             RubberBandStretcher::OptionProcessRealTime,
             factor);
        RubberBandStretcher *monoStretcher = new RubberBandStretcher
            (int(getTargetSampleRate()),
             1,
             RubberBandStretcher::OptionProcessRealTime,
             factor);
        m_stretcherInputs = new float *[m_stretcherInputCount];
        m_stretcherInputSizes = new sv_frame_t[m_stretcherInputCount];
        for (int c = 0; c < m_stretcherInputCount; ++c) {
            m_stretcherInputSizes[c] = 16384;
            m_stretcherInputs[c] = new float[m_stretcherInputSizes[c]];
        }
        m_monoStretcher = monoStretcher;
        m_timeStretcher = stretcher;
    }

    emit activity(tr("Change time-stretch factor to %1").arg(factor));
}

sv_frame_t
AudioCallbackPlaySource::getSourceSamples(sv_frame_t count, float **buffer)
{
    if (!m_playing) {
#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
        cerr << "AudioCallbackPlaySource::getSourceSamples: Not playing" << endl;
#endif
	for (int ch = 0; ch < getTargetChannelCount(); ++ch) {
	    for (int i = 0; i < count; ++i) {
		buffer[ch][i] = 0.0;
	    }
	}
	return 0;
    }

#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
    cerr << "AudioCallbackPlaySource::getSourceSamples: Playing" << endl;
#endif

    // Ensure that all buffers have at least the amount of data we
    // need -- else reduce the size of our requests correspondingly

    for (int ch = 0; ch < getTargetChannelCount(); ++ch) {

        RingBuffer<float> *rb = getReadRingBuffer(ch);
        
        if (!rb) {
            cerr << "WARNING: AudioCallbackPlaySource::getSourceSamples: "
                      << "No ring buffer available for channel " << ch
                      << ", returning no data here" << endl;
            count = 0;
            break;
        }

        int rs = rb->getReadSpace();
        if (rs < count) {
#ifdef DEBUG_AUDIO_PLAY_SOURCE
            cerr << "WARNING: AudioCallbackPlaySource::getSourceSamples: "
                      << "Ring buffer for channel " << ch << " has only "
                      << rs << " (of " << count << ") samples available ("
                      << "ring buffer size is " << rb->getSize() << ", write "
                      << "space " << rb->getWriteSpace() << "), "
                      << "reducing request size" << endl;
#endif
            count = rs;
        }
    }

    if (count == 0) return 0;

    RubberBandStretcher *ts = m_timeStretcher;
    RubberBandStretcher *ms = m_monoStretcher;

    double ratio = ts ? ts->getTimeRatio() : 1.0;

    if (ratio != m_stretchRatio) {
        if (!ts) {
            cerr << "WARNING: AudioCallbackPlaySource::getSourceSamples: Time ratio change to " << m_stretchRatio << " is pending, but no stretcher is set" << endl;
            m_stretchRatio = 1.0;
        } else {
            ts->setTimeRatio(m_stretchRatio);
            if (ms) ms->setTimeRatio(m_stretchRatio);
            if (m_stretchRatio >= 1.0) m_stretchMono = false;
        }
    }

    int stretchChannels = m_stretcherInputCount;
    if (m_stretchMono) {
        if (ms) {
            ts = ms;
            stretchChannels = 1;
        } else {
            m_stretchMono = false;
        }
    }

    if (m_target) {
        m_lastRetrievedBlockSize = count;
        m_lastRetrievalTimestamp = m_target->getCurrentTime();
    }

    if (!ts || ratio == 1.f) {

	int got = 0;

	for (int ch = 0; ch < getTargetChannelCount(); ++ch) {

	    RingBuffer<float> *rb = getReadRingBuffer(ch);

	    if (rb) {

		// this is marginally more likely to leave our channels in
		// sync after a processing failure than just passing "count":
		sv_frame_t request = count;
		if (ch > 0) request = got;

		got = rb->read(buffer[ch], int(request));
	    
#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
		cout << "AudioCallbackPlaySource::getSamples: got " << got << " (of " << count << ") samples on channel " << ch << ", signalling for more (possibly)" << endl;
#endif
	    }

	    for (int ch = 0; ch < getTargetChannelCount(); ++ch) {
		for (int i = got; i < count; ++i) {
		    buffer[ch][i] = 0.0;
		}
	    }
	}

        applyAuditioningEffect(count, buffer);

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySource::getSamples: awakening thread" << endl;
#endif

        m_condition.wakeAll();

	return got;
    }

    int channels = getTargetChannelCount();
    sv_frame_t available;
    sv_frame_t fedToStretcher = 0;
    int warned = 0;

    // The input block for a given output is approx output / ratio,
    // but we can't predict it exactly, for an adaptive timestretcher.

    while ((available = ts->available()) < count) {

        sv_frame_t reqd = lrint(double(count - available) / ratio);
        reqd = std::max(reqd, sv_frame_t(ts->getSamplesRequired()));
        if (reqd == 0) reqd = 1;
                
        sv_frame_t got = reqd;

#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
        cerr << "reqd = " <<reqd << ", channels = " << channels << ", ic = " << m_stretcherInputCount << endl;
#endif

        for (int c = 0; c < channels; ++c) {
            if (c >= m_stretcherInputCount) continue;
            if (reqd > m_stretcherInputSizes[c]) {
                if (c == 0) {
                    cerr << "WARNING: resizing stretcher input buffer from " << m_stretcherInputSizes[c] << " to " << (reqd * 2) << endl;
                }
                delete[] m_stretcherInputs[c];
                m_stretcherInputSizes[c] = reqd * 2;
                m_stretcherInputs[c] = new float[m_stretcherInputSizes[c]];
            }
        }

        for (int c = 0; c < channels; ++c) {
            if (c >= m_stretcherInputCount) continue;
            RingBuffer<float> *rb = getReadRingBuffer(c);
            if (rb) {
                sv_frame_t gotHere;
                if (stretchChannels == 1 && c > 0) {
                    gotHere = rb->readAdding(m_stretcherInputs[0], int(got));
                } else {
                    gotHere = rb->read(m_stretcherInputs[c], int(got));
                }
                if (gotHere < got) got = gotHere;
                
#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
                if (c == 0) {
                    cerr << "feeding stretcher: got " << gotHere
                              << ", " << rb->getReadSpace() << " remain" << endl;
                }
#endif
                
            } else {
                cerr << "WARNING: No ring buffer available for channel " << c << " in stretcher input block" << endl;
            }
        }

        if (got < reqd) {
            cerr << "WARNING: Read underrun in playback ("
                      << got << " < " << reqd << ")" << endl;
        }

        ts->process(m_stretcherInputs, got, false);

        fedToStretcher += got;

        if (got == 0) break;

        if (ts->available() == available) {
            cerr << "WARNING: AudioCallbackPlaySource::getSamples: Added " << got << " samples to time stretcher, created no new available output samples (warned = " << warned << ")" << endl;
            if (++warned == 5) break;
        }
    }

    ts->retrieve(buffer, count);

    for (int c = stretchChannels; c < getTargetChannelCount(); ++c) {
        for (int i = 0; i < count; ++i) {
            buffer[c][i] = buffer[0][i];
        }
    }

    applyAuditioningEffect(count, buffer);

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySource::getSamples [stretched]: awakening thread" << endl;
#endif

    m_condition.wakeAll();

    return count;
}

void
AudioCallbackPlaySource::applyAuditioningEffect(sv_frame_t count, float **buffers)
{
    if (m_auditioningPluginBypassed) return;
    RealTimePluginInstance *plugin = m_auditioningPlugin;
    if (!plugin) return;
    
    if ((int)plugin->getAudioInputCount() != getTargetChannelCount()) {
//        cerr << "plugin input count " << plugin->getAudioInputCount() 
//                  << " != our channel count " << getTargetChannelCount()
//                  << endl;
        return;
    }
    if ((int)plugin->getAudioOutputCount() != getTargetChannelCount()) {
//        cerr << "plugin output count " << plugin->getAudioOutputCount() 
//                  << " != our channel count " << getTargetChannelCount()
//                  << endl;
        return;
    }
    if ((int)plugin->getBufferSize() < count) {
//        cerr << "plugin buffer size " << plugin->getBufferSize() 
//                  << " < our block size " << count
//                  << endl;
        return;
    }

    float **ib = plugin->getAudioInputBuffers();
    float **ob = plugin->getAudioOutputBuffers();

    for (int c = 0; c < getTargetChannelCount(); ++c) {
        for (int i = 0; i < count; ++i) {
            ib[c][i] = buffers[c][i];
        }
    }

    // maybe Vamp:: ?
    plugin->run(RealTime::zeroTime, int(count));
    
    for (int c = 0; c < getTargetChannelCount(); ++c) {
        for (int i = 0; i < count; ++i) {
            buffers[c][i] = ob[c][i];
        }
    }
}    

// Called from fill thread, m_playing true, mutex held
bool
AudioCallbackPlaySource::fillBuffers()
{
    static float *tmp = 0;
    static sv_frame_t tmpSize = 0;

    sv_frame_t space = 0;
    for (int c = 0; c < getTargetChannelCount(); ++c) {
	RingBuffer<float> *wb = getWriteRingBuffer(c);
	if (wb) {
	    sv_frame_t spaceHere = wb->getWriteSpace();
	    if (c == 0 || spaceHere < space) space = spaceHere;
	}
    }
    
    if (space == 0) {
#ifdef DEBUG_AUDIO_PLAY_SOURCE
        cout << "AudioCallbackPlaySourceFillThread: no space to fill" << endl;
#endif
        return false;
    }

    sv_frame_t f = m_writeBufferFill;
	
    bool readWriteEqual = (m_readBuffers == m_writeBuffers);

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    if (!readWriteEqual) {
        cout << "AudioCallbackPlaySourceFillThread: note read buffers != write buffers" << endl;
    }
    cout << "AudioCallbackPlaySourceFillThread: filling " << space << " frames" << endl;
#endif

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "buffered to " << f << " already" << endl;
#endif

    bool resample = (getSourceSampleRate() != getTargetSampleRate());

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << (resample ? "" : "not ") << "resampling (source " << getSourceSampleRate() << ", target " << getTargetSampleRate() << ")" << endl;
#endif

    int channels = getTargetChannelCount();

    sv_frame_t orig = space;
    sv_frame_t got = 0;

    static float **bufferPtrs = 0;
    static int bufferPtrCount = 0;

    if (bufferPtrCount < channels) {
	if (bufferPtrs) delete[] bufferPtrs;
	bufferPtrs = new float *[channels];
	bufferPtrCount = channels;
    }

    sv_frame_t generatorBlockSize = m_audioGenerator->getBlockSize();

    if (resample && !m_converter) {
	static bool warned = false;
	if (!warned) {
	    cerr << "WARNING: sample rates differ, but no converter available!" << endl;
	    warned = true;
	}
    }

    if (resample && m_converter) {

	double ratio =
	    double(getTargetSampleRate()) / double(getSourceSampleRate());
	orig = sv_frame_t(double(orig) / ratio + 0.1);

	// orig must be a multiple of generatorBlockSize
	orig = (orig / generatorBlockSize) * generatorBlockSize;
	if (orig == 0) return false;

	sv_frame_t work = std::max(orig, space);

	// We only allocate one buffer, but we use it in two halves.
	// We place the non-interleaved values in the second half of
	// the buffer (orig samples for channel 0, orig samples for
	// channel 1 etc), and then interleave them into the first
	// half of the buffer.  Then we resample back into the second
	// half (interleaved) and de-interleave the results back to
	// the start of the buffer for insertion into the ringbuffers.
	// What a faff -- especially as we've already de-interleaved
	// the audio data from the source file elsewhere before we
	// even reach this point.
	
	if (tmpSize < channels * work * 2) {
	    delete[] tmp;
	    tmp = new float[channels * work * 2];
	    tmpSize = channels * work * 2;
	}

	float *nonintlv = tmp + channels * work;
	float *intlv = tmp;
	float *srcout = tmp + channels * work;
	
	for (int c = 0; c < channels; ++c) {
	    for (int i = 0; i < orig; ++i) {
		nonintlv[channels * i + c] = 0.0f;
	    }
	}

	for (int c = 0; c < channels; ++c) {
	    bufferPtrs[c] = nonintlv + c * orig;
	}

	got = mixModels(f, orig, bufferPtrs); // also modifies f

	// and interleave into first half
	for (int c = 0; c < channels; ++c) {
	    for (int i = 0; i < got; ++i) {
		float sample = nonintlv[c * got + i];
		intlv[channels * i + c] = sample;
	    }
	}
		
	SRC_DATA data;
	data.data_in = intlv;
	data.data_out = srcout;
	data.input_frames = got;
	data.output_frames = work;
	data.src_ratio = ratio;
	data.end_of_input = 0;
	
	int err = 0;

        if (m_timeStretcher && m_timeStretcher->getTimeRatio() < 0.4) {
#ifdef DEBUG_AUDIO_PLAY_SOURCE
            cout << "Using crappy converter" << endl;
#endif
            err = src_process(m_crapConverter, &data);
        } else {
            err = src_process(m_converter, &data);
        }

	sv_frame_t toCopy = sv_frame_t(double(got) * ratio + 0.1);

	if (err) {
	    cerr
		<< "AudioCallbackPlaySourceFillThread: ERROR in samplerate conversion: "
		<< src_strerror(err) << endl;
	    //!!! Then what?
	} else {
	    got = data.input_frames_used;
	    toCopy = data.output_frames_gen;
#ifdef DEBUG_AUDIO_PLAY_SOURCE
	    cout << "Resampled " << got << " frames to " << toCopy << " frames" << endl;
#endif
	}
	
	for (int c = 0; c < channels; ++c) {
	    for (int i = 0; i < toCopy; ++i) {
		tmp[i] = srcout[channels * i + c];
	    }
	    RingBuffer<float> *wb = getWriteRingBuffer(c);
	    if (wb) wb->write(tmp, int(toCopy));
	}

	m_writeBufferFill = f;
	if (readWriteEqual) m_readBufferFill = f;

    } else {

	// space must be a multiple of generatorBlockSize
        sv_frame_t reqSpace = space;
	space = (reqSpace / generatorBlockSize) * generatorBlockSize;
	if (space == 0) {
#ifdef DEBUG_AUDIO_PLAY_SOURCE
            cout << "requested fill of " << reqSpace
                      << " is less than generator block size of "
                      << generatorBlockSize << ", leaving it" << endl;
#endif
            return false;
        }

	if (tmpSize < channels * space) {
	    delete[] tmp;
	    tmp = new float[channels * space];
	    tmpSize = channels * space;
	}

	for (int c = 0; c < channels; ++c) {

	    bufferPtrs[c] = tmp + c * space;
	    
	    for (int i = 0; i < space; ++i) {
		tmp[c * space + i] = 0.0f;
	    }
	}

	sv_frame_t got = mixModels(f, space, bufferPtrs); // also modifies f

	for (int c = 0; c < channels; ++c) {

	    RingBuffer<float> *wb = getWriteRingBuffer(c);
	    if (wb) {
                int actual = wb->write(bufferPtrs[c], int(got));
#ifdef DEBUG_AUDIO_PLAY_SOURCE
		cout << "Wrote " << actual << " samples for ch " << c << ", now "
			  << wb->getReadSpace() << " to read" 
			  << endl;
#endif
                if (actual < got) {
                    cerr << "WARNING: Buffer overrun in channel " << c
                              << ": wrote " << actual << " of " << got
                              << " samples" << endl;
                }
            }
	}

	m_writeBufferFill = f;
	if (readWriteEqual) m_readBufferFill = f;

#ifdef DEBUG_AUDIO_PLAY_SOURCE
        cout << "Read buffer fill is now " << m_readBufferFill << endl;
#endif

	//!!! how do we know when ended? need to mark up a fully-buffered flag and check this if we find the buffers empty in getSourceSamples
    }

    return true;
}    

sv_frame_t
AudioCallbackPlaySource::mixModels(sv_frame_t &frame, sv_frame_t count, float **buffers)
{
    sv_frame_t processed = 0;
    sv_frame_t chunkStart = frame;
    sv_frame_t chunkSize = count;
    sv_frame_t selectionSize = 0;
    sv_frame_t nextChunkStart = chunkStart + chunkSize;
    
    bool looping = m_viewManager->getPlayLoopMode();
    bool constrained = (m_viewManager->getPlaySelectionMode() &&
			!m_viewManager->getSelections().empty());

    static float **chunkBufferPtrs = 0;
    static int chunkBufferPtrCount = 0;
    int channels = getTargetChannelCount();

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "Selection playback: start " << frame << ", size " << count <<", channels " << channels << endl;
#endif

    if (chunkBufferPtrCount < channels) {
	if (chunkBufferPtrs) delete[] chunkBufferPtrs;
	chunkBufferPtrs = new float *[channels];
	chunkBufferPtrCount = channels;
    }

    for (int c = 0; c < channels; ++c) {
	chunkBufferPtrs[c] = buffers[c];
    }

    while (processed < count) {
	
	chunkSize = count - processed;
	nextChunkStart = chunkStart + chunkSize;
	selectionSize = 0;

	sv_frame_t fadeIn = 0, fadeOut = 0;

	if (constrained) {

            sv_frame_t rChunkStart =
                m_viewManager->alignPlaybackFrameToReference(chunkStart);
	    
	    Selection selection =
		m_viewManager->getContainingSelection(rChunkStart, true);
	    
	    if (selection.isEmpty()) {
		if (looping) {
		    selection = *m_viewManager->getSelections().begin();
		    chunkStart = m_viewManager->alignReferenceToPlaybackFrame
                        (selection.getStartFrame());
		    fadeIn = 50;
		}
	    }

	    if (selection.isEmpty()) {

		chunkSize = 0;
		nextChunkStart = chunkStart;

	    } else {

                sv_frame_t sf = m_viewManager->alignReferenceToPlaybackFrame
                    (selection.getStartFrame());
                sv_frame_t ef = m_viewManager->alignReferenceToPlaybackFrame
                    (selection.getEndFrame());

		selectionSize = ef - sf;

		if (chunkStart < sf) {
		    chunkStart = sf;
		    fadeIn = 50;
		}

		nextChunkStart = chunkStart + chunkSize;

		if (nextChunkStart >= ef) {
		    nextChunkStart = ef;
		    fadeOut = 50;
		}

		chunkSize = nextChunkStart - chunkStart;
	    }
	
	} else if (looping && m_lastModelEndFrame > 0) {

	    if (chunkStart >= m_lastModelEndFrame) {
		chunkStart = 0;
	    }
	    if (chunkSize > m_lastModelEndFrame - chunkStart) {
		chunkSize = m_lastModelEndFrame - chunkStart;
	    }
	    nextChunkStart = chunkStart + chunkSize;
	}
	
//	cout << "chunkStart " << chunkStart << ", chunkSize " << chunkSize << ", nextChunkStart " << nextChunkStart << ", frame " << frame << ", count " << count << ", processed " << processed << endl;

	if (!chunkSize) {
#ifdef DEBUG_AUDIO_PLAY_SOURCE
	    cout << "Ending selection playback at " << nextChunkStart << endl;
#endif
	    // We need to maintain full buffers so that the other
	    // thread can tell where it's got to in the playback -- so
	    // return the full amount here
	    frame = frame + count;
	    return count;
	}

#ifdef DEBUG_AUDIO_PLAY_SOURCE
	cout << "Selection playback: chunk at " << chunkStart << " -> " << nextChunkStart << " (size " << chunkSize << ")" << endl;
#endif

	if (selectionSize < 100) {
	    fadeIn = 0;
	    fadeOut = 0;
	} else if (selectionSize < 300) {
	    if (fadeIn > 0) fadeIn = 10;
	    if (fadeOut > 0) fadeOut = 10;
	}

	if (fadeIn > 0) {
	    if (processed * 2 < fadeIn) {
		fadeIn = processed * 2;
	    }
	}

	if (fadeOut > 0) {
	    if ((count - processed - chunkSize) * 2 < fadeOut) {
		fadeOut = (count - processed - chunkSize) * 2;
	    }
	}

	for (std::set<Model *>::iterator mi = m_models.begin();
	     mi != m_models.end(); ++mi) {
	    
	    (void) m_audioGenerator->mixModel(*mi, chunkStart, 
                                              chunkSize, chunkBufferPtrs,
                                              fadeIn, fadeOut);
	}

	for (int c = 0; c < channels; ++c) {
	    chunkBufferPtrs[c] += chunkSize;
	}

	processed += chunkSize;
	chunkStart = nextChunkStart;
    }

#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "Returning selection playback " << processed << " frames to " << nextChunkStart << endl;
#endif

    frame = nextChunkStart;
    return processed;
}

void
AudioCallbackPlaySource::unifyRingBuffers()
{
    if (m_readBuffers == m_writeBuffers) return;

    // only unify if there will be something to read
    for (int c = 0; c < getTargetChannelCount(); ++c) {
	RingBuffer<float> *wb = getWriteRingBuffer(c);
	if (wb) {
	    if (wb->getReadSpace() < m_blockSize * 2) {
		if ((m_writeBufferFill + m_blockSize * 2) < 
		    m_lastModelEndFrame) {
		    // OK, we don't have enough and there's more to
		    // read -- don't unify until we can do better
#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
                    cerr << "AudioCallbackPlaySource::unifyRingBuffers: Not unifying: write buffer has less (" << wb->getReadSpace() << ") than " << m_blockSize*2 << " to read and write buffer fill (" << m_writeBufferFill << ") is not close to end frame (" << m_lastModelEndFrame << ")" << endl;
#endif
		    return;
		}
	    }
	    break;
	}
    }

    sv_frame_t rf = m_readBufferFill;
    RingBuffer<float> *rb = getReadRingBuffer(0);
    if (rb) {
	int rs = rb->getReadSpace();
	//!!! incorrect when in non-contiguous selection, see comments elsewhere
//	cout << "rs = " << rs << endl;
	if (rs < rf) rf -= rs;
	else rf = 0;
    }
    
#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
    cerr << "AudioCallbackPlaySource::unifyRingBuffers: m_readBufferFill = " << m_readBufferFill << ", rf = " << rf << ", m_writeBufferFill = " << m_writeBufferFill << endl;
#endif

    sv_frame_t wf = m_writeBufferFill;
    sv_frame_t skip = 0;
    for (int c = 0; c < getTargetChannelCount(); ++c) {
	RingBuffer<float> *wb = getWriteRingBuffer(c);
	if (wb) {
	    if (c == 0) {
		
		int wrs = wb->getReadSpace();
//		cout << "wrs = " << wrs << endl;

		if (wrs < wf) wf -= wrs;
		else wf = 0;
//		cout << "wf = " << wf << endl;
		
		if (wf < rf) skip = rf - wf;
		if (skip == 0) break;
	    }

//	    cout << "skipping " << skip << endl;
	    wb->skip(int(skip));
	}
    }
		    
    m_bufferScavenger.claim(m_readBuffers);
    m_readBuffers = m_writeBuffers;
    m_readBufferFill = m_writeBufferFill;
#ifdef DEBUG_AUDIO_PLAY_SOURCE_PLAYING
    cerr << "unified" << endl;
#endif
}

void
AudioCallbackPlaySource::FillThread::run()
{
    AudioCallbackPlaySource &s(m_source);
    
#ifdef DEBUG_AUDIO_PLAY_SOURCE
    cout << "AudioCallbackPlaySourceFillThread starting" << endl;
#endif

    s.m_mutex.lock();

    bool previouslyPlaying = s.m_playing;
    bool work = false;

    while (!s.m_exiting) {

	s.unifyRingBuffers();
	s.m_bufferScavenger.scavenge();
        s.m_pluginScavenger.scavenge();

	if (work && s.m_playing && s.getSourceSampleRate()) {
	    
#ifdef DEBUG_AUDIO_PLAY_SOURCE
	    cout << "AudioCallbackPlaySourceFillThread: not waiting" << endl;
#endif

	    s.m_mutex.unlock();
	    s.m_mutex.lock();

	} else {
	    
	    double ms = 100;
	    if (s.getSourceSampleRate() > 0) {
		ms = double(s.m_ringBufferSize) / s.getSourceSampleRate() * 1000.0;
	    }
	    
	    if (s.m_playing) ms /= 10;

#ifdef DEBUG_AUDIO_PLAY_SOURCE
            if (!s.m_playing) cout << endl;
	    cout << "AudioCallbackPlaySourceFillThread: waiting for " << ms << "ms..." << endl;
#endif
	    
	    s.m_condition.wait(&s.m_mutex, int(ms));
	}

#ifdef DEBUG_AUDIO_PLAY_SOURCE
	cout << "AudioCallbackPlaySourceFillThread: awoken" << endl;
#endif

	work = false;

	if (!s.getSourceSampleRate()) {
#ifdef DEBUG_AUDIO_PLAY_SOURCE
            cout << "AudioCallbackPlaySourceFillThread: source sample rate is zero" << endl;
#endif
            continue;
        }

	bool playing = s.m_playing;

	if (playing && !previouslyPlaying) {
#ifdef DEBUG_AUDIO_PLAY_SOURCE
	    cout << "AudioCallbackPlaySourceFillThread: playback state changed, resetting" << endl;
#endif
	    for (int c = 0; c < s.getTargetChannelCount(); ++c) {
		RingBuffer<float> *rb = s.getReadRingBuffer(c);
		if (rb) rb->reset();
	    }
	}
	previouslyPlaying = playing;

	work = s.fillBuffers();
    }

    s.m_mutex.unlock();
}

