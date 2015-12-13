/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "AudioGenerator.h"

#include "base/TempDirectory.h"
#include "base/PlayParameters.h"
#include "base/PlayParameterRepository.h"
#include "base/Pitch.h"
#include "base/Exceptions.h"

#include "data/model/NoteModel.h"
#include "data/model/FlexiNoteModel.h"
#include "data/model/DenseTimeValueModel.h"
#include "data/model/SparseTimeValueModel.h"
#include "data/model/SparseOneDimensionalModel.h"
#include "data/model/NoteData.h"

#include "ClipMixer.h"
#include "ContinuousSynth.h"

#include <iostream>
#include <cmath>

#include <QDir>
#include <QFile>

const sv_frame_t
AudioGenerator::m_processingBlockSize = 1024;

QString
AudioGenerator::m_sampleDir = "";

//#define DEBUG_AUDIO_GENERATOR 1

AudioGenerator::AudioGenerator() :
    m_sourceSampleRate(0),
    m_targetChannelCount(1),
    m_waveType(0),
    m_soloing(false),
    m_channelBuffer(0),
    m_channelBufSiz(0),
    m_channelBufCount(0)
{
    initialiseSampleDir();

    connect(PlayParameterRepository::getInstance(),
            SIGNAL(playClipIdChanged(const Playable *, QString)),
            this,
            SLOT(playClipIdChanged(const Playable *, QString)));
}

AudioGenerator::~AudioGenerator()
{
#ifdef DEBUG_AUDIO_GENERATOR
    cerr << "AudioGenerator::~AudioGenerator" << endl;
#endif
}

void
AudioGenerator::initialiseSampleDir()
{
    if (m_sampleDir != "") return;

    try {
        m_sampleDir = TempDirectory::getInstance()->getSubDirectoryPath("samples");
    } catch (DirectoryCreationFailed f) {
        cerr << "WARNING: AudioGenerator::initialiseSampleDir:"
                  << " Failed to create temporary sample directory"
                  << endl;
        m_sampleDir = "";
        return;
    }

    QDir sampleResourceDir(":/samples", "*.wav");

    for (unsigned int i = 0; i < sampleResourceDir.count(); ++i) {

        QString fileName(sampleResourceDir[i]);
        QFile file(sampleResourceDir.filePath(fileName));
        QString target = QDir(m_sampleDir).filePath(fileName);

        if (!file.copy(target)) {
            cerr << "WARNING: AudioGenerator::getSampleDir: "
                      << "Unable to copy " << fileName
                      << " into temporary directory \""
                      << m_sampleDir << "\"" << endl;
        } else {
            QFile tf(target);
            tf.setPermissions(tf.permissions() |
                              QFile::WriteOwner |
                              QFile::WriteUser);
        }
    }
}

bool
AudioGenerator::addModel(Model *model)
{
    if (m_sourceSampleRate == 0) {

	m_sourceSampleRate = model->getSampleRate();

    } else {

	DenseTimeValueModel *dtvm =
	    dynamic_cast<DenseTimeValueModel *>(model);

	if (dtvm) {
	    m_sourceSampleRate = model->getSampleRate();
	    return true;
	}
    }

    const Playable *playable = model;
    if (!playable || !playable->canPlay()) return 0;

    PlayParameters *parameters =
	PlayParameterRepository::getInstance()->getPlayParameters(playable);

    bool willPlay = !parameters->isPlayMuted();
    
    if (usesClipMixer(model)) {
        ClipMixer *mixer = makeClipMixerFor(model);
        if (mixer) {
            QMutexLocker locker(&m_mutex);
            m_clipMixerMap[model] = mixer;
            return willPlay;
        }
    }

    if (usesContinuousSynth(model)) {
        ContinuousSynth *synth = makeSynthFor(model);
        if (synth) {
            QMutexLocker locker(&m_mutex);
            m_continuousSynthMap[model] = synth;
            return willPlay;
        }
    }

    return false;
}

void
AudioGenerator::playClipIdChanged(const Playable *playable, QString)
{
    const Model *model = dynamic_cast<const Model *>(playable);
    if (!model) {
        cerr << "WARNING: AudioGenerator::playClipIdChanged: playable "
                  << playable << " is not a supported model type"
                  << endl;
        return;
    }

    if (m_clipMixerMap.find(model) == m_clipMixerMap.end()) return;

    ClipMixer *mixer = makeClipMixerFor(model);
    if (mixer) {
        QMutexLocker locker(&m_mutex);
        m_clipMixerMap[model] = mixer;
    }
}

bool
AudioGenerator::usesClipMixer(const Model *model)
{
    bool clip = 
        (qobject_cast<const SparseOneDimensionalModel *>(model) ||
         qobject_cast<const NoteModel *>(model) ||
         qobject_cast<const FlexiNoteModel *>(model));
    return clip;
}

bool
AudioGenerator::wantsQuieterClips(const Model *model)
{
    // basically, anything that usually has sustain (like notes) or
    // often has multiple sounds at once (like notes) wants to use a
    // quieter level than simple click tracks
    bool does = 
        (qobject_cast<const NoteModel *>(model) ||
         qobject_cast<const FlexiNoteModel *>(model));
    return does;
}

bool
AudioGenerator::usesContinuousSynth(const Model *model)
{
    bool cont = 
        (qobject_cast<const SparseTimeValueModel *>(model));
    return cont;
}

ClipMixer *
AudioGenerator::makeClipMixerFor(const Model *model)
{
    QString clipId;

    const Playable *playable = model;
    if (!playable || !playable->canPlay()) return 0;

    PlayParameters *parameters =
	PlayParameterRepository::getInstance()->getPlayParameters(playable);
    if (parameters) {
        clipId = parameters->getPlayClipId();
    }

#ifdef DEBUG_AUDIO_GENERATOR
    std::cerr << "AudioGenerator::makeClipMixerFor(" << model << "): sample id = " << clipId << std::endl;
#endif

    if (clipId == "") {
        cerr << "AudioGenerator::makeClipMixerFor(" << model << "): no sample, skipping" << endl;
        return 0;
    }

    ClipMixer *mixer = new ClipMixer(m_targetChannelCount,
                                     m_sourceSampleRate,
                                     m_processingBlockSize);

    double clipF0 = Pitch::getFrequencyForPitch(60, 0, 440.0); // required

    QString clipPath = QString("%1/%2.wav").arg(m_sampleDir).arg(clipId);

    double level = wantsQuieterClips(model) ? 0.5 : 1.0;
    if (!mixer->loadClipData(clipPath, clipF0, level)) {
        delete mixer;
        return 0;
    }

#ifdef DEBUG_AUDIO_GENERATOR
    std::cerr << "AudioGenerator::makeClipMixerFor(" << model << "): loaded clip " << clipId << std::endl;
#endif

    return mixer;
}

ContinuousSynth *
AudioGenerator::makeSynthFor(const Model *model)
{
    const Playable *playable = model;
    if (!playable || !playable->canPlay()) return 0;

    ContinuousSynth *synth = new ContinuousSynth(m_targetChannelCount,
                                                 m_sourceSampleRate,
                                                 m_processingBlockSize,
                                                 m_waveType);

#ifdef DEBUG_AUDIO_GENERATOR
    std::cerr << "AudioGenerator::makeSynthFor(" << model << "): created synth" << std::endl;
#endif

    return synth;
}

void
AudioGenerator::removeModel(Model *model)
{
    SparseOneDimensionalModel *sodm =
	dynamic_cast<SparseOneDimensionalModel *>(model);
    if (!sodm) return; // nothing to do

    QMutexLocker locker(&m_mutex);

    if (m_clipMixerMap.find(sodm) == m_clipMixerMap.end()) return;

    ClipMixer *mixer = m_clipMixerMap[sodm];
    m_clipMixerMap.erase(sodm);
    delete mixer;
}

void
AudioGenerator::clearModels()
{
    QMutexLocker locker(&m_mutex);

    while (!m_clipMixerMap.empty()) {
        ClipMixer *mixer = m_clipMixerMap.begin()->second;
	m_clipMixerMap.erase(m_clipMixerMap.begin());
	delete mixer;
    }
}    

void
AudioGenerator::reset()
{
    QMutexLocker locker(&m_mutex);

#ifdef DEBUG_AUDIO_GENERATOR
    cerr << "AudioGenerator::reset()" << endl;
#endif

    for (ClipMixerMap::iterator i = m_clipMixerMap.begin(); i != m_clipMixerMap.end(); ++i) {
	if (i->second) {
	    i->second->reset();
	}
    }

    m_noteOffs.clear();
}

void
AudioGenerator::setTargetChannelCount(int targetChannelCount)
{
    if (m_targetChannelCount == targetChannelCount) return;

//    cerr << "AudioGenerator::setTargetChannelCount(" << targetChannelCount << ")" << endl;

    QMutexLocker locker(&m_mutex);
    m_targetChannelCount = targetChannelCount;

    for (ClipMixerMap::iterator i = m_clipMixerMap.begin(); i != m_clipMixerMap.end(); ++i) {
	if (i->second) i->second->setChannelCount(targetChannelCount);
    }
}

sv_frame_t
AudioGenerator::getBlockSize() const
{
    return m_processingBlockSize;
}

void
AudioGenerator::setSoloModelSet(std::set<Model *> s)
{
    QMutexLocker locker(&m_mutex);

    m_soloModelSet = s;
    m_soloing = true;
}

void
AudioGenerator::clearSoloModelSet()
{
    QMutexLocker locker(&m_mutex);

    m_soloModelSet.clear();
    m_soloing = false;
}

sv_frame_t
AudioGenerator::mixModel(Model *model, sv_frame_t startFrame, sv_frame_t frameCount,
			 float **buffer, sv_frame_t fadeIn, sv_frame_t fadeOut)
{
    if (m_sourceSampleRate == 0) {
	cerr << "WARNING: AudioGenerator::mixModel: No base source sample rate available" << endl;
	return frameCount;
    }

    QMutexLocker locker(&m_mutex);

    Playable *playable = model;
    if (!playable || !playable->canPlay()) return frameCount;

    PlayParameters *parameters =
	PlayParameterRepository::getInstance()->getPlayParameters(playable);
    if (!parameters) return frameCount;

    bool playing = !parameters->isPlayMuted();
    if (!playing) {
#ifdef DEBUG_AUDIO_GENERATOR
        cout << "AudioGenerator::mixModel(" << model << "): muted" << endl;
#endif
        return frameCount;
    }

    if (m_soloing) {
        if (m_soloModelSet.find(model) == m_soloModelSet.end()) {
#ifdef DEBUG_AUDIO_GENERATOR
            cout << "AudioGenerator::mixModel(" << model << "): not one of the solo'd models" << endl;
#endif
            return frameCount;
        }
    }

    float gain = parameters->getPlayGain();
    float pan = parameters->getPlayPan();

    DenseTimeValueModel *dtvm = dynamic_cast<DenseTimeValueModel *>(model);
    if (dtvm) {
	return mixDenseTimeValueModel(dtvm, startFrame, frameCount,
				      buffer, gain, pan, fadeIn, fadeOut);
    }

    if (usesClipMixer(model)) {
        return mixClipModel(model, startFrame, frameCount,
                            buffer, gain, pan);
    }

    if (usesContinuousSynth(model)) {
        return mixContinuousSynthModel(model, startFrame, frameCount,
                                       buffer, gain, pan);
    }

    std::cerr << "AudioGenerator::mixModel: WARNING: Model " << model << " of type " << model->getTypeName() << " is marked as playable, but I have no mechanism to play it" << std::endl;

    return frameCount;
}

sv_frame_t
AudioGenerator::mixDenseTimeValueModel(DenseTimeValueModel *dtvm,
				       sv_frame_t startFrame, sv_frame_t frames,
				       float **buffer, float gain, float pan,
				       sv_frame_t fadeIn, sv_frame_t fadeOut)
{
    sv_frame_t maxFrames = frames + std::max(fadeIn, fadeOut);

    int modelChannels = dtvm->getChannelCount();

    if (m_channelBufSiz < maxFrames || m_channelBufCount < modelChannels) {

        for (int c = 0; c < m_channelBufCount; ++c) {
            delete[] m_channelBuffer[c];
        }

	delete[] m_channelBuffer;
        m_channelBuffer = new float *[modelChannels];

        for (int c = 0; c < modelChannels; ++c) {
            m_channelBuffer[c] = new float[maxFrames];
        }

        m_channelBufCount = modelChannels;
	m_channelBufSiz = maxFrames;
    }

    sv_frame_t got = 0;

    if (startFrame >= fadeIn/2) {
        got = dtvm->getData(0, modelChannels - 1,
                            startFrame - fadeIn/2,
                            frames + fadeOut/2 + fadeIn/2,
                            m_channelBuffer);
    } else {
        sv_frame_t missing = fadeIn/2 - startFrame;

        for (int c = 0; c < modelChannels; ++c) {
            m_channelBuffer[c] += missing;
        }

        if (missing > 0) {
            cerr << "note: channelBufSiz = " << m_channelBufSiz
                 << ", frames + fadeOut/2 = " << frames + fadeOut/2 
                 << ", startFrame = " << startFrame 
                 << ", missing = " << missing << endl;
        }

        got = dtvm->getData(0, modelChannels - 1,
                            startFrame,
                            frames + fadeOut/2,
                            m_channelBuffer);

        for (int c = 0; c < modelChannels; ++c) {
            m_channelBuffer[c] -= missing;
        }

        got += missing;
    }	    

    for (int c = 0; c < m_targetChannelCount; ++c) {

	int sourceChannel = (c % modelChannels);

//	cerr << "mixing channel " << c << " from source channel " << sourceChannel << endl;

	float channelGain = gain;
	if (pan != 0.0) {
	    if (c == 0) {
		if (pan > 0.0) channelGain *= 1.0f - pan;
	    } else {
		if (pan < 0.0) channelGain *= pan + 1.0f;
	    }
	}

	for (sv_frame_t i = 0; i < fadeIn/2; ++i) {
	    float *back = buffer[c];
	    back -= fadeIn/2;
	    back[i] +=
                (channelGain * m_channelBuffer[sourceChannel][i] * float(i))
                / float(fadeIn);
	}

	for (sv_frame_t i = 0; i < frames + fadeOut/2; ++i) {
	    float mult = channelGain;
	    if (i < fadeIn/2) {
		mult = (mult * float(i)) / float(fadeIn);
	    }
	    if (i > frames - fadeOut/2) {
		mult = (mult * float((frames + fadeOut/2) - i)) / float(fadeOut);
	    }
            float val = m_channelBuffer[sourceChannel][i];
            if (i >= got) val = 0.f;
	    buffer[c][i] += mult * val;
	}
    }

    return got;
}
  
sv_frame_t
AudioGenerator::mixClipModel(Model *model,
                             sv_frame_t startFrame, sv_frame_t frames,
                             float **buffer, float gain, float pan)
{
    ClipMixer *clipMixer = m_clipMixerMap[model];
    if (!clipMixer) return 0;

    int blocks = int(frames / m_processingBlockSize);
    
    //!!! todo: the below -- it matters

    //!!! hang on -- the fact that the audio callback play source's
    //buffer is a multiple of the plugin's buffer size doesn't mean
    //that we always get called for a multiple of it here (because it
    //also depends on the JACK block size).  how should we ensure that
    //all models write the same amount in to the mix, and that we
    //always have a multiple of the plugin buffer size?  I guess this
    //class has to be queryable for the plugin buffer size & the
    //callback play source has to use that as a multiple for all the
    //calls to mixModel

    sv_frame_t got = blocks * m_processingBlockSize;

#ifdef DEBUG_AUDIO_GENERATOR
    cout << "mixModel [clip]: start " << startFrame << ", frames " << frames
         << ", blocks " << blocks << ", have " << m_noteOffs.size()
         << " note-offs" << endl;
#endif

    ClipMixer::NoteStart on;
    ClipMixer::NoteEnd off;

    NoteOffSet &noteOffs = m_noteOffs[model];

    float **bufferIndexes = new float *[m_targetChannelCount];

    for (int i = 0; i < blocks; ++i) {

	sv_frame_t reqStart = startFrame + i * m_processingBlockSize;

        NoteList notes;
        NoteExportable *exportable = dynamic_cast<NoteExportable *>(model);
        if (exportable) {
            notes = exportable->getNotesWithin(reqStart,
                                               reqStart + m_processingBlockSize);
        }

        std::vector<ClipMixer::NoteStart> starts;
        std::vector<ClipMixer::NoteEnd> ends;

	for (NoteList::const_iterator ni = notes.begin();
             ni != notes.end(); ++ni) {

	    sv_frame_t noteFrame = ni->start;

	    if (noteFrame < reqStart ||
		noteFrame >= reqStart + m_processingBlockSize) continue;

	    while (noteOffs.begin() != noteOffs.end() &&
		   noteOffs.begin()->frame <= noteFrame) {

                sv_frame_t eventFrame = noteOffs.begin()->frame;
                if (eventFrame < reqStart) eventFrame = reqStart;

                off.frameOffset = eventFrame - reqStart;
                off.frequency = noteOffs.begin()->frequency;

#ifdef DEBUG_AUDIO_GENERATOR
		cerr << "mixModel [clip]: adding note-off at frame " << eventFrame << " frame offset " << off.frameOffset << " frequency " << off.frequency << endl;
#endif

                ends.push_back(off);
		noteOffs.erase(noteOffs.begin());
	    }

            on.frameOffset = noteFrame - reqStart;
            on.frequency = ni->getFrequency();
            on.level = float(ni->velocity) / 127.0f;
            on.pan = pan;

#ifdef DEBUG_AUDIO_GENERATOR
	    cout << "mixModel [clip]: adding note at frame " << noteFrame << ", frame offset " << on.frameOffset << " frequency " << on.frequency << ", level " << on.level << endl;
#endif
	    
            starts.push_back(on);
	    noteOffs.insert
                (NoteOff(on.frequency, noteFrame + ni->duration));
	}

	while (noteOffs.begin() != noteOffs.end() &&
	       noteOffs.begin()->frame <= reqStart + m_processingBlockSize) {

            sv_frame_t eventFrame = noteOffs.begin()->frame;
            if (eventFrame < reqStart) eventFrame = reqStart;

            off.frameOffset = eventFrame - reqStart;
            off.frequency = noteOffs.begin()->frequency;

#ifdef DEBUG_AUDIO_GENERATOR
            cerr << "mixModel [clip]: adding leftover note-off at frame " << eventFrame << " frame offset " << off.frameOffset << " frequency " << off.frequency << endl;
#endif

            ends.push_back(off);
            noteOffs.erase(noteOffs.begin());
	}

	for (int c = 0; c < m_targetChannelCount; ++c) {
            bufferIndexes[c] = buffer[c] + i * m_processingBlockSize;
        }

        clipMixer->mix(bufferIndexes, gain, starts, ends);
    }

    delete[] bufferIndexes;

    return got;
}

sv_frame_t
AudioGenerator::mixContinuousSynthModel(Model *model,
                                        sv_frame_t startFrame,
                                        sv_frame_t frames,
                                        float **buffer,
                                        float gain, 
                                        float pan)
{
    ContinuousSynth *synth = m_continuousSynthMap[model];
    if (!synth) return 0;

    // only type we support here at the moment
    SparseTimeValueModel *stvm = qobject_cast<SparseTimeValueModel *>(model);
    if (stvm->getScaleUnits() != "Hz") return 0;

    int blocks = int(frames / m_processingBlockSize);

    //!!! todo: see comment in mixClipModel

    sv_frame_t got = blocks * m_processingBlockSize;

#ifdef DEBUG_AUDIO_GENERATOR
    cout << "mixModel [synth]: frames " << frames
	      << ", blocks " << blocks << endl;
#endif
    
    float **bufferIndexes = new float *[m_targetChannelCount];

    for (int i = 0; i < blocks; ++i) {

	sv_frame_t reqStart = startFrame + i * m_processingBlockSize;

	for (int c = 0; c < m_targetChannelCount; ++c) {
            bufferIndexes[c] = buffer[c] + i * m_processingBlockSize;
        }

        SparseTimeValueModel::PointList points = 
            stvm->getPoints(reqStart, reqStart + m_processingBlockSize);

        // by default, repeat last frequency
        float f0 = 0.f;

        // go straight to the last freq that is genuinely in this range
        for (SparseTimeValueModel::PointList::const_iterator itr = points.end();
             itr != points.begin(); ) {
            --itr;
            if (itr->frame >= reqStart &&
                itr->frame < reqStart + m_processingBlockSize) {
                f0 = itr->value;
                break;
            }
        }

        // if we found no such frequency and the next point is further
        // away than twice the model resolution, go silent (same
        // criterion TimeValueLayer uses for ending a discrete curve
        // segment)
        if (f0 == 0.f) {
            SparseTimeValueModel::PointList nextPoints = 
                stvm->getNextPoints(reqStart + m_processingBlockSize);
            if (nextPoints.empty() ||
                nextPoints.begin()->frame > reqStart + 2 * stvm->getResolution()) {
                f0 = -1.f;
            }
        }

//        cerr << "f0 = " << f0 << endl;

        synth->mix(bufferIndexes,
                   gain,
                   pan,
                   f0);
    }

    delete[] bufferIndexes;

    return got;
}

