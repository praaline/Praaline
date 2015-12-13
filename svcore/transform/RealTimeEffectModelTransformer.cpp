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

#include "RealTimeEffectModelTransformer.h"

#include "plugin/RealTimePluginFactory.h"
#include "plugin/RealTimePluginInstance.h"
#include "plugin/PluginXml.h"

#include "data/model/Model.h"
#include "data/model/SparseTimeValueModel.h"
#include "data/model/DenseTimeValueModel.h"
#include "data/model/WritableWaveFileModel.h"
#include "data/model/WaveFileModel.h"

#include "TransformFactory.h"

#include <iostream>

RealTimeEffectModelTransformer::RealTimeEffectModelTransformer(Input in,
                                                               const Transform &t) :
    ModelTransformer(in, t),
    m_plugin(0)
{
    Transform transform(t);
    if (!transform.getBlockSize()) {
        transform.setBlockSize(1024);
        m_transforms[0] = transform;
    }

    m_units = TransformFactory::getInstance()->getTransformUnits
        (transform.getIdentifier());
    m_outputNo =
        (transform.getOutput() == "A") ? -1 : transform.getOutput().toInt();

    QString pluginId = transform.getPluginIdentifier();

//    cerr << "RealTimeEffectModelTransformer::RealTimeEffectModelTransformer: plugin " << pluginId << ", output " << output << endl;

    RealTimePluginFactory *factory =
	RealTimePluginFactory::instanceFor(pluginId);

    if (!factory) {
	cerr << "RealTimeEffectModelTransformer: No factory available for plugin id \""
		  << pluginId << "\"" << endl;
	return;
    }

    DenseTimeValueModel *input = getConformingInput();
    if (!input) return;

    m_plugin = factory->instantiatePlugin(pluginId, 0, 0,
                                          input->getSampleRate(),
                                          transform.getBlockSize(),
                                          input->getChannelCount());

    if (!m_plugin) {
	cerr << "RealTimeEffectModelTransformer: Failed to instantiate plugin \""
             << pluginId << "\"" << endl;
	return;
    }

    TransformFactory::getInstance()->setPluginParameters(transform, m_plugin);

    if (m_outputNo >= 0 &&
        m_outputNo >= int(m_plugin->getControlOutputCount())) {
        cerr << "RealTimeEffectModelTransformer: Plugin has fewer than desired " << m_outputNo << " control outputs" << endl;
        return;
    }

    if (m_outputNo == -1) {

        int outputChannels = (int)m_plugin->getAudioOutputCount();
        if (outputChannels > input->getChannelCount()) {
            outputChannels = input->getChannelCount();
        }

        WritableWaveFileModel *model = new WritableWaveFileModel
            (input->getSampleRate(), outputChannels);

        m_outputs.push_back(model);

    } else {
	
        SparseTimeValueModel *model = new SparseTimeValueModel
            (input->getSampleRate(), transform.getBlockSize(), 0.0, 0.0, false);

        if (m_units != "") model->setScaleUnits(m_units);

        m_outputs.push_back(model);
    }
}

RealTimeEffectModelTransformer::~RealTimeEffectModelTransformer()
{
    delete m_plugin;
}

DenseTimeValueModel *
RealTimeEffectModelTransformer::getConformingInput()
{
    DenseTimeValueModel *dtvm =
	dynamic_cast<DenseTimeValueModel *>(getInputModel());
    if (!dtvm) {
	cerr << "RealTimeEffectModelTransformer::getConformingInput: WARNING: Input model is not conformable to DenseTimeValueModel" << endl;
    }
    return dtvm;
}

void
RealTimeEffectModelTransformer::run()
{
    DenseTimeValueModel *input = getConformingInput();
    if (!input) return;

    while (!input->isReady() && !m_abandoned) {
        cerr << "RealTimeEffectModelTransformer::run: Waiting for input model to be ready..." << endl;
        usleep(500000);
    }
    if (m_abandoned) return;

    SparseTimeValueModel *stvm = dynamic_cast<SparseTimeValueModel *>(m_outputs[0]);
    WritableWaveFileModel *wwfm = dynamic_cast<WritableWaveFileModel *>(m_outputs[0]);
    if (!stvm && !wwfm) return;

    if (stvm && (m_outputNo >= int(m_plugin->getControlOutputCount()))) return;

    sv_samplerate_t sampleRate = input->getSampleRate();
    int channelCount = input->getChannelCount();
    if (!wwfm && m_input.getChannel() != -1) channelCount = 1;

    sv_frame_t blockSize = m_plugin->getBufferSize();

    float **inbufs = m_plugin->getAudioInputBuffers();

    sv_frame_t startFrame = m_input.getModel()->getStartFrame();
    sv_frame_t endFrame = m_input.getModel()->getEndFrame();

    Transform transform = m_transforms[0];
    
    RealTime contextStartRT = transform.getStartTime();
    RealTime contextDurationRT = transform.getDuration();

    sv_frame_t contextStart =
        RealTime::realTime2Frame(contextStartRT, sampleRate);

    sv_frame_t contextDuration =
        RealTime::realTime2Frame(contextDurationRT, sampleRate);

    if (contextStart == 0 || contextStart < startFrame) {
        contextStart = startFrame;
    }

    if (contextDuration == 0) {
        contextDuration = endFrame - contextStart;
    }
    if (contextStart + contextDuration > endFrame) {
        contextDuration = endFrame - contextStart;
    }

    if (wwfm) {
        wwfm->setStartFrame(contextStart);
    }

    sv_frame_t blockFrame = contextStart;

    int prevCompletion = 0;

    sv_frame_t latency = m_plugin->getLatency();

    while (blockFrame < contextStart + contextDuration + latency &&
           !m_abandoned) {

	int completion = int
	    ((((blockFrame - contextStart) / blockSize) * 99) /
             (1 + ((contextDuration) / blockSize)));

	sv_frame_t got = 0;

	if (channelCount == 1) {
            if (inbufs && inbufs[0]) {
                got = input->getData
                    (m_input.getChannel(), blockFrame, blockSize, inbufs[0]);
                while (got < blockSize) {
                    inbufs[0][got++] = 0.0;
                }          
                for (int ch = 1; ch < (int)m_plugin->getAudioInputCount(); ++ch) {
                    for (sv_frame_t i = 0; i < blockSize; ++i) {
                        inbufs[ch][i] = inbufs[0][i];
                    }
                }
            }
	} else {
            if (inbufs && inbufs[0]) {
                got = input->getData(0, channelCount - 1,
                                     blockFrame, blockSize,
                                     inbufs);
                while (got < blockSize) {
                    for (int ch = 0; ch < channelCount; ++ch) {
                        inbufs[ch][got] = 0.0;
                    }
                    ++got;
                }
                for (int ch = channelCount; ch < (int)m_plugin->getAudioInputCount(); ++ch) {
                    for (sv_frame_t i = 0; i < blockSize; ++i) {
                        inbufs[ch][i] = inbufs[ch % channelCount][i];
                    }
                }
            }
	}

/*
        cerr << "Input for plugin: " << m_plugin->getAudioInputCount() << " channels "<< endl;

        for (int ch = 0; ch < m_plugin->getAudioInputCount(); ++ch) {
            cerr << "Input channel " << ch << endl;
            for (int i = 0; i < 100; ++i) {
                cerr << inbufs[ch][i] << " ";
                if (isnan(inbufs[ch][i])) {
                    cerr << "\n\nWARNING: NaN in audio input" << endl;
                }
            }
        }
*/

        m_plugin->run(RealTime::frame2RealTime(blockFrame, sampleRate));

        if (stvm) {

            float value = m_plugin->getControlOutputValue(m_outputNo);

            sv_frame_t pointFrame = blockFrame;
            if (pointFrame > latency) pointFrame -= latency;
            else pointFrame = 0;

            stvm->addPoint(SparseTimeValueModel::Point
                           (pointFrame, value, ""));

        } else if (wwfm) {

            float **outbufs = m_plugin->getAudioOutputBuffers();

            if (outbufs) {

                if (blockFrame >= latency) {
                    sv_frame_t writeSize = std::min
                        (blockSize,
                         contextStart + contextDuration + latency - blockFrame);
                    wwfm->addSamples(outbufs, writeSize);
                } else if (blockFrame + blockSize >= latency) {
                    sv_frame_t offset = latency - blockFrame;
                    sv_frame_t count = blockSize - offset;
                    float **tmp = new float *[channelCount];
                    for (int c = 0; c < channelCount; ++c) {
                        tmp[c] = outbufs[c] + offset;
                    }
                    wwfm->addSamples(tmp, count);
                    delete[] tmp;
                }
            }
        }

	if (blockFrame == contextStart || completion > prevCompletion) {
	    if (stvm) stvm->setCompletion(completion);
	    if (wwfm) wwfm->setCompletion(completion);
	    prevCompletion = completion;
	}
        
	blockFrame += blockSize;
    }

    if (m_abandoned) return;
    
    if (stvm) stvm->setCompletion(100);
    if (wwfm) wwfm->setCompletion(100);
}

