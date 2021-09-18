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

#include "FeatureExtractionModelTransformer.h"

#include "plugin/FeatureExtractionPluginFactory.h"
#include "plugin/PluginXml.h"
#include <vamp-hostsdk/Plugin.h>

#include "data/model/Model.h"
#include "base/Window.h"
#include "base/Exceptions.h"
#include "data/model/SparseOneDimensionalModel.h"
#include "data/model/SparseTimeValueModel.h"
#include "data/model/EditableDenseThreeDimensionalModel.h"
#include "data/model/DenseTimeValueModel.h"
#include "data/model/NoteModel.h"
#include "data/model/FlexiNoteModel.h"
#include "data/model/RegionModel.h"
#include "data/model/FFTModel.h"
#include "data/model/WaveFileModel.h"
#include "rdf/PluginRDFDescription.h"

#include "TransformFactory.h"

#include <iostream>

#include <QSettings>

FeatureExtractionModelTransformer::FeatureExtractionModelTransformer(Input in,
                                                                     const Transform &transform) :
    ModelTransformer(in, transform),
    m_plugin(0)
{
//    cerr << "FeatureExtractionModelTransformer::FeatureExtractionModelTransformer: plugin " << pluginId << ", outputName " << m_transform.getOutput() << endl;

    initialise();
}

FeatureExtractionModelTransformer::FeatureExtractionModelTransformer(Input in,
                                                                     const Transforms &transforms) :
    ModelTransformer(in, transforms),
    m_plugin(0)
{
//    cerr << "FeatureExtractionModelTransformer::FeatureExtractionModelTransformer: plugin " << pluginId << ", outputName " << m_transform.getOutput() << endl;

    initialise();
}

static bool
areTransformsSimilar(const Transform &t1, const Transform &t2)
{
    Transform t2o(t2);
    t2o.setOutput(t1.getOutput());
    return t1 == t2o;
}

bool
FeatureExtractionModelTransformer::initialise()
{
    // All transforms must use the same plugin, parameters, and
    // inputs: they can differ only in choice of plugin output. So we
    // initialise based purely on the first transform in the list (but
    // first check that they are actually similar as promised)

    for (int j = 1; j < (int)m_transforms.size(); ++j) {
        if (!areTransformsSimilar(m_transforms[0], m_transforms[j])) {
            m_message = tr("Transforms supplied to a single FeatureExtractionModelTransformer instance must be similar in every respect except plugin output");
            return false;
        }
    }

    Transform primaryTransform = m_transforms[0];

    QString pluginId = primaryTransform.getPluginIdentifier();

    FeatureExtractionPluginFactory *factory =
	FeatureExtractionPluginFactory::instanceFor(pluginId);

    if (!factory) {
        m_message = tr("No factory available for feature extraction plugin id \"%1\" (unknown plugin type, or internal error?)").arg(pluginId);
	return false;
    }

    DenseTimeValueModel *input = getConformingInput();
    if (!input) {
        m_message = tr("Input model for feature extraction plugin \"%1\" is of wrong type (internal error?)").arg(pluginId);
        return false;
    }

    m_plugin = factory->instantiatePlugin(pluginId, input->getSampleRate());
    if (!m_plugin) {
        m_message = tr("Failed to instantiate plugin \"%1\"").arg(pluginId);
	return false;
    }

    TransformFactory::getInstance()->makeContextConsistentWithPlugin
        (primaryTransform, m_plugin);

    TransformFactory::getInstance()->setPluginParameters
        (primaryTransform, m_plugin);

    int channelCount = input->getChannelCount();
    if ((int)m_plugin->getMaxChannelCount() < channelCount) {
	channelCount = 1;
    }
    if ((int)m_plugin->getMinChannelCount() > channelCount) {
        m_message = tr("Cannot provide enough channels to feature extraction plugin \"%1\" (plugin min is %2, max %3; input model has %4)")
            .arg(pluginId)
            .arg(m_plugin->getMinChannelCount())
            .arg(m_plugin->getMaxChannelCount())
            .arg(input->getChannelCount());
	return false;
    }

    cerr << "Initialising feature extraction plugin with channels = "
              << channelCount << ", step = " << primaryTransform.getStepSize()
              << ", block = " << primaryTransform.getBlockSize() << endl;

    if (!m_plugin->initialise(channelCount,
                              primaryTransform.getStepSize(),
                              primaryTransform.getBlockSize())) {

        int pstep = primaryTransform.getStepSize();
        int pblock = primaryTransform.getBlockSize();

///!!! hang on, this isn't right -- we're modifying a copy
        primaryTransform.setStepSize(0);
        primaryTransform.setBlockSize(0);
        TransformFactory::getInstance()->makeContextConsistentWithPlugin
            (primaryTransform, m_plugin);

        if (primaryTransform.getStepSize() != pstep ||
            primaryTransform.getBlockSize() != pblock) {
            
            if (!m_plugin->initialise(channelCount,
                                      primaryTransform.getStepSize(),
                                      primaryTransform.getBlockSize())) {

                m_message = tr("Failed to initialise feature extraction plugin \"%1\"").arg(pluginId);
                return false;

            } else {

                m_message = tr("Feature extraction plugin \"%1\" rejected the given step and block sizes (%2 and %3); using plugin defaults (%4 and %5) instead")
                    .arg(pluginId)
                    .arg(pstep)
                    .arg(pblock)
                    .arg(primaryTransform.getStepSize())
                    .arg(primaryTransform.getBlockSize());
            }

        } else {

            m_message = tr("Failed to initialise feature extraction plugin \"%1\"").arg(pluginId);
            return false;
        }
    }

    if (primaryTransform.getPluginVersion() != "") {
        QString pv = QString("%1").arg(m_plugin->getPluginVersion());
        if (pv != primaryTransform.getPluginVersion()) {
            QString vm = tr("Transform was configured for version %1 of plugin \"%2\", but the plugin being used is version %3")
                .arg(primaryTransform.getPluginVersion())
                .arg(pluginId)
                .arg(pv);
            if (m_message != "") {
                m_message = QString("%1; %2").arg(vm).arg(m_message);
            } else {
                m_message = vm;
            }
        }
    }

    Vamp::Plugin::OutputList outputs = m_plugin->getOutputDescriptors();

    if (outputs.empty()) {
        m_message = tr("Plugin \"%1\" has no outputs").arg(pluginId);
	return false;
    }

    for (int j = 0; j < (int)m_transforms.size(); ++j) {

        for (int i = 0; i < (int)outputs.size(); ++i) {
//        cerr << "comparing output " << i << " name \"" << outputs[i].identifier << "\" with expected \"" << m_transform.getOutput() << "\"" << endl;
            if (m_transforms[j].getOutput() == "" ||
                outputs[i].identifier == m_transforms[j].getOutput().toStdString()) {
                m_outputNos.push_back(i);
                m_descriptors.push_back(new Vamp::Plugin::OutputDescriptor(outputs[i]));
                m_fixedRateFeatureNos.push_back(-1); // we increment before use
                break;
            }
        }

        if ((int)m_descriptors.size() <= j) {
            m_message = tr("Plugin \"%1\" has no output named \"%2\"")
                .arg(pluginId)
                .arg(m_transforms[j].getOutput());
            return false;
        }
    }

    for (int j = 0; j < (int)m_transforms.size(); ++j) {
        createOutputModels(j);
    }

    return true;
}

void
FeatureExtractionModelTransformer::createOutputModels(int n)
{
    DenseTimeValueModel *input = getConformingInput();

//    cerr << "FeatureExtractionModelTransformer::createOutputModel: sample type " << m_descriptor->sampleType << ", rate " << m_descriptor->sampleRate << endl;
    
    PluginRDFDescription description(m_transforms[n].getPluginIdentifier());
    QString outputId = m_transforms[n].getOutput();

    int binCount = 1;
    float minValue = 0.0, maxValue = 0.0;
    bool haveExtents = false;
    bool haveBinCount = m_descriptors[n]->hasFixedBinCount;

    if (haveBinCount) {
	binCount = (int)m_descriptors[n]->binCount;
    }

    m_needAdditionalModels[n] = false;

//    cerr << "FeatureExtractionModelTransformer: output bin count "
//	      << binCount << endl;

    if (binCount > 0 && m_descriptors[n]->hasKnownExtents) {
	minValue = m_descriptors[n]->minValue;
	maxValue = m_descriptors[n]->maxValue;
        haveExtents = true;
    }

    sv_samplerate_t modelRate = input->getSampleRate();
    int modelResolution = 1;

    if (m_descriptors[n]->sampleType != 
        Vamp::Plugin::OutputDescriptor::OneSamplePerStep) {
        if (m_descriptors[n]->sampleRate > input->getSampleRate()) {
            cerr << "WARNING: plugin reports output sample rate as "
                      << m_descriptors[n]->sampleRate << " (can't display features with finer resolution than the input rate of " << input->getSampleRate() << ")" << endl;
        }
    }

    switch (m_descriptors[n]->sampleType) {

    case Vamp::Plugin::OutputDescriptor::VariableSampleRate:
	if (m_descriptors[n]->sampleRate != 0.0) {
	    modelResolution = int(round(modelRate / m_descriptors[n]->sampleRate));
	}
	break;

    case Vamp::Plugin::OutputDescriptor::OneSamplePerStep:
	modelResolution = m_transforms[n].getStepSize();
	break;

    case Vamp::Plugin::OutputDescriptor::FixedSampleRate:
        //!!! SV doesn't actually support display of models that have
        //!!! different underlying rates together -- so we always set
        //!!! the model rate to be the input model's rate, and adjust
        //!!! the resolution appropriately.  We can't properly display
        //!!! data with a higher resolution than the base model at all
        if (m_descriptors[n]->sampleRate > input->getSampleRate()) {
            modelResolution = 1;
        } else {
            modelResolution = int(round(modelRate / m_descriptors[n]->sampleRate));
        }
	break;
    }

    bool preDurationPlugin = (m_plugin->getVampApiVersion() < 2);

    Model *out = 0;

    if (binCount == 0 &&
        (preDurationPlugin || !m_descriptors[n]->hasDuration)) {

        // Anything with no value and no duration is an instant

        out = new SparseOneDimensionalModel(modelRate, modelResolution, false);
        QString outputEventTypeURI = description.getOutputEventTypeURI(outputId);
        out->setRDFTypeURI(outputEventTypeURI);

    } else if ((preDurationPlugin && binCount > 1 &&
                (m_descriptors[n]->sampleType ==
                 Vamp::Plugin::OutputDescriptor::VariableSampleRate)) ||
               (!preDurationPlugin && m_descriptors[n]->hasDuration)) {

        // For plugins using the old v1 API without explicit duration,
        // we treat anything that has multiple bins (i.e. that has the
        // potential to have value and duration) and a variable sample
        // rate as a note model, taking its values as pitch, duration
        // and velocity (if present) respectively.  This is the same
        // behaviour as always applied by SV to these plugins in the
        // past.

        // For plugins with the newer API, we treat anything with
        // duration as either a note model with pitch and velocity, or
        // a region model.

        // How do we know whether it's an interval or note model?
        // What's the essential difference?  Is a note model any
        // interval model using a Hz or "MIDI pitch" scale?  There
        // isn't really a reliable test for "MIDI pitch"...  Does a
        // note model always have velocity?  This is a good question
        // to be addressed by accompanying RDF, but for the moment we
        // will do the following...

        bool isNoteModel = false;
        
        // Regions have only value (and duration -- we can't extract a
        // region model from an old-style plugin that doesn't support
        // duration)
        if (binCount > 1) isNoteModel = true;

        // Regions do not have units of Hz or MIDI things (a sweeping
        // assumption!)
        if (m_descriptors[n]->unit == "Hz" ||
            m_descriptors[n]->unit.find("MIDI") != std::string::npos ||
            m_descriptors[n]->unit.find("midi") != std::string::npos) {
            isNoteModel = true;
        }

        // If we had a "sparse 3D model", we would have the additional
        // problem of determining whether to use that here (if bin
        // count > 1).  But we don't.

        QSettings settings;
        settings.beginGroup("Transformer");
        bool flexi = settings.value("use-flexi-note-model", false).toBool();
        settings.endGroup();

        cerr << "flexi = " << flexi << endl;

        if (isNoteModel && !flexi) {

            NoteModel *model;
            if (haveExtents) {
                model = new NoteModel
                    (modelRate, modelResolution, minValue, maxValue, false);
            } else {
                model = new NoteModel
                    (modelRate, modelResolution, false);
            }
            model->setScaleUnits(m_descriptors[n]->unit.c_str());
            out = model;

        } else if (isNoteModel && flexi) {

            FlexiNoteModel *model;
            if (haveExtents) {
                model = new FlexiNoteModel
                    (modelRate, modelResolution, minValue, maxValue, false);
            } else {
                model = new FlexiNoteModel
                    (modelRate, modelResolution, false);
            }
            model->setScaleUnits(m_descriptors[n]->unit.c_str());
            out = model;

        } else {

            RegionModel *model;
            if (haveExtents) {
                model = new RegionModel
                    (modelRate, modelResolution, minValue, maxValue, false);
            } else {
                model = new RegionModel
                    (modelRate, modelResolution, false);
            }
            model->setScaleUnits(m_descriptors[n]->unit.c_str());
            out = model;
        }

        QString outputEventTypeURI = description.getOutputEventTypeURI(outputId);
        out->setRDFTypeURI(outputEventTypeURI);

    } else if (binCount == 1 ||
               (m_descriptors[n]->sampleType == 
                Vamp::Plugin::OutputDescriptor::VariableSampleRate)) {

        // Anything that is not a 1D, note, or interval model and that
        // has only one value per result must be a sparse time value
        // model.

        // Anything that is not a 1D, note, or interval model and that
        // has a variable sample rate is treated as a set of sparse
        // time value models, one per output bin, because we lack a
        // sparse 3D model.

        // Anything that is not a 1D, note, or interval model and that
        // has a fixed sample rate but an unknown number of values per
        // result is also treated as a set of sparse time value models.

        // For sets of sparse time value models, we create a single
        // model first as the "standard" output and then create models
        // for bins 1+ in the additional model map (mapping the output
        // descriptor to a list of models indexed by bin-1). But we
        // don't create the additional models yet, as this case has to
        // work even if the number of bins is unknown at this point --
        // we create an additional model (copying its parameters from
        // the default one) each time a new bin is encountered.

        if (!haveBinCount || binCount > 1) {
            m_needAdditionalModels[n] = true;
        }

        SparseTimeValueModel *model;
        if (haveExtents) {
            model = new SparseTimeValueModel
                (modelRate, modelResolution, minValue, maxValue, false);
        } else {
            model = new SparseTimeValueModel
                (modelRate, modelResolution, false);
        }

        Vamp::Plugin::OutputList outputs = m_plugin->getOutputDescriptors();
        model->setScaleUnits(outputs[m_outputNos[n]].unit.c_str());

        out = model;

        QString outputEventTypeURI = description.getOutputEventTypeURI(outputId);
        out->setRDFTypeURI(outputEventTypeURI);

    } else {

        // Anything that is not a 1D, note, or interval model and that
        // has a fixed sample rate and more than one value per result
        // must be a dense 3D model.

        EditableDenseThreeDimensionalModel *model =
            new EditableDenseThreeDimensionalModel
            (modelRate, modelResolution, binCount,
             EditableDenseThreeDimensionalModel::BasicMultirateCompression,
             false);

	if (!m_descriptors[n]->binNames.empty()) {
	    std::vector<QString> names;
	    for (int i = 0; i < (int)m_descriptors[n]->binNames.size(); ++i) {
		names.push_back(m_descriptors[n]->binNames[i].c_str());
	    }
	    model->setBinNames(names);
	}
        
        out = model;

        QString outputSignalTypeURI = description.getOutputSignalTypeURI(outputId);
        out->setRDFTypeURI(outputSignalTypeURI);
    }

    if (out) {
        out->setSourceModel(input);
        m_outputs.push_back(out);
    }
}

FeatureExtractionModelTransformer::~FeatureExtractionModelTransformer()
{
//    cerr << "FeatureExtractionModelTransformer::~FeatureExtractionModelTransformer()" << endl;
    delete m_plugin;
    for (int j = 0; j < (int)m_descriptors.size(); ++j) {
        delete m_descriptors[j];
    }
}

FeatureExtractionModelTransformer::Models
FeatureExtractionModelTransformer::getAdditionalOutputModels()
{
    Models mm;
    for (AdditionalModelMap::iterator i = m_additionalModels.begin();
         i != m_additionalModels.end(); ++i) {
        for (std::map<int, SparseTimeValueModel *>::iterator j =
                 i->second.begin();
             j != i->second.end(); ++j) {
            SparseTimeValueModel *m = j->second;
            if (m) mm.push_back(m);
        }
    }
    return mm;
}

bool
FeatureExtractionModelTransformer::willHaveAdditionalOutputModels()
{
    for (std::map<int, bool>::const_iterator i =
             m_needAdditionalModels.begin(); 
         i != m_needAdditionalModels.end(); ++i) {
        if (i->second) return true;
    }
    return false;
}

SparseTimeValueModel *
FeatureExtractionModelTransformer::getAdditionalModel(int n, int binNo)
{
//    std::cerr << "getAdditionalModel(" << n << ", " << binNo << ")" << std::endl;

    if (binNo == 0) {
        std::cerr << "Internal error: binNo == 0 in getAdditionalModel (should be using primary model)" << std::endl;
        return 0;
    }

    if (!m_needAdditionalModels[n]) return 0;
    if (!isOutput<SparseTimeValueModel>(n)) return 0;
    if (m_additionalModels[n][binNo]) return m_additionalModels[n][binNo];

    std::cerr << "getAdditionalModel(" << n << ", " << binNo << "): creating" << std::endl;

    SparseTimeValueModel *baseModel = getConformingOutput<SparseTimeValueModel>(n);
    if (!baseModel) return 0;

    std::cerr << "getAdditionalModel(" << n << ", " << binNo << "): (from " << baseModel << ")" << std::endl;

    SparseTimeValueModel *additional =
        new SparseTimeValueModel(baseModel->getSampleRate(),
                                 baseModel->getResolution(),
                                 baseModel->getValueMinimum(),
                                 baseModel->getValueMaximum(),
                                 false);

    additional->setScaleUnits(baseModel->getScaleUnits());
    additional->setRDFTypeURI(baseModel->getRDFTypeURI());

    m_additionalModels[n][binNo] = additional;
    return additional;
}

DenseTimeValueModel *
FeatureExtractionModelTransformer::getConformingInput()
{
//    cerr << "FeatureExtractionModelTransformer::getConformingInput: input model is " << getInputModel() << endl;

    DenseTimeValueModel *dtvm =
	dynamic_cast<DenseTimeValueModel *>(getInputModel());
    if (!dtvm) {
    cerr << "FeatureExtractionModelTransformer::getConformingInput: WARNING: Input model is not conformable to DenseTimeValueModel" << endl;
    }
    return dtvm;
}

void
FeatureExtractionModelTransformer::run()
{
    DenseTimeValueModel *input = getConformingInput();
    if (!input) return;

    if (m_outputs.empty()) return;

    Transform primaryTransform = m_transforms[0];

    while (!input->isReady() && !m_abandoned) {
        cerr << "FeatureExtractionModelTransformer::run: Waiting for input model to be ready..." << endl;
        usleep(500000);
    }
    if (m_abandoned) return;

    sv_samplerate_t sampleRate = input->getSampleRate();

    int channelCount = input->getChannelCount();
    if ((int)m_plugin->getMaxChannelCount() < channelCount) {
	channelCount = 1;
    }

    float **buffers = new float*[channelCount];
    for (int ch = 0; ch < channelCount; ++ch) {
	buffers[ch] = new float[primaryTransform.getBlockSize() + 2];
    }

    int stepSize = primaryTransform.getStepSize();
    int blockSize = primaryTransform.getBlockSize();

    bool frequencyDomain = (m_plugin->getInputDomain() ==
                            Vamp::Plugin::FrequencyDomain);
    std::vector<FFTModel *> fftModels;

    if (frequencyDomain) {
        for (int ch = 0; ch < channelCount; ++ch) {
            FFTModel *model = new FFTModel
                                  (getConformingInput(),
                                   channelCount == 1 ? m_input.getChannel() : ch,
                                   primaryTransform.getWindowType(),
                                   blockSize,
                                   stepSize,
                                   blockSize,
                                   false,
                                   StorageAdviser::PrecisionCritical);
            if (!model->isOK()) {
                delete model;
                for (int j = 0; j < (int)m_outputNos.size(); ++j) {
                    setCompletion(j, 100);
                }
                //!!! need a better way to handle this -- previously we were using a QMessageBox but that isn't an appropriate thing to do here either
                throw AllocationFailed("Failed to create the FFT model for this feature extraction model transformer");
            }
            model->resume();
            fftModels.push_back(model);
        }
    }

    sv_frame_t startFrame = m_input.getModel()->getStartFrame();
    sv_frame_t endFrame = m_input.getModel()->getEndFrame();

    RealTime contextStartRT = primaryTransform.getStartTime();
    RealTime contextDurationRT = primaryTransform.getDuration();

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

    sv_frame_t blockFrame = contextStart;

    long prevCompletion = 0;

    for (int j = 0; j < (int)m_outputNos.size(); ++j) {
        setCompletion(j, 0);
    }

    float *reals = 0;
    float *imaginaries = 0;
    if (frequencyDomain) {
        reals = new float[blockSize/2 + 1];
        imaginaries = new float[blockSize/2 + 1];
    }

    QString error = "";

    while (!m_abandoned) {

        if (frequencyDomain) {
            if (blockFrame - int(blockSize)/2 >
                contextStart + contextDuration) break;
        } else {
            if (blockFrame >= 
                contextStart + contextDuration) break;
        }

//	cerr << "FeatureExtractionModelTransformer::run: blockFrame "
//		  << blockFrame << ", endFrame " << endFrame << ", blockSize "
//                  << blockSize << endl;

	int completion = int
	    ((((blockFrame - contextStart) / stepSize) * 99) /
             (contextDuration / stepSize + 1));

	// channelCount is either m_input.getModel()->channelCount or 1

        if (frequencyDomain) {
            for (int ch = 0; ch < channelCount; ++ch) {
                int column = int((blockFrame - startFrame) / stepSize);
                if (fftModels[ch]->getValuesAt(column, reals, imaginaries)) {
                    for (int i = 0; i <= blockSize/2; ++i) {
                        buffers[ch][i*2] = reals[i];
                        buffers[ch][i*2+1] = imaginaries[i];
                    }
                } else {
                    for (int i = 0; i <= blockSize/2; ++i) {
                        buffers[ch][i*2] = 0.f;
                        buffers[ch][i*2+1] = 0.f;
                    }
                }                    
                error = fftModels[ch]->getError();
                if (error != "") {
                    cerr << "FeatureExtractionModelTransformer::run: Abandoning, error is " << error << endl;
                    m_abandoned = true;
                    m_message = error;
                }
            }
        } else {
            getFrames(channelCount, blockFrame, blockSize, buffers);
        }

        if (m_abandoned) break;

    RealTime r = RealTime::frame2RealTime(blockFrame, sampleRate);
	Vamp::Plugin::FeatureSet features = m_plugin->process
        (buffers, Vamp::RealTime(r.sec, r.nsec));

        if (m_abandoned) break;

        for (int j = 0; j < (int)m_outputNos.size(); ++j) {
            for (int fi = 0; fi < (int)features[m_outputNos[j]].size(); ++fi) {
                Vamp::Plugin::Feature feature = features[m_outputNos[j]][fi];
                addFeature(j, blockFrame, feature);
            }
        }

	if (blockFrame == contextStart || completion > prevCompletion) {
            for (int j = 0; j < (int)m_outputNos.size(); ++j) {
                setCompletion(j, completion);
            }
	    prevCompletion = completion;
	}

	blockFrame += stepSize;
    }

    if (!m_abandoned) {
        Vamp::Plugin::FeatureSet features = m_plugin->getRemainingFeatures();

        for (int j = 0; j < (int)m_outputNos.size(); ++j) {
            for (int fi = 0; fi < (int)features[m_outputNos[j]].size(); ++fi) {
                Vamp::Plugin::Feature feature = features[m_outputNos[j]][fi];
                addFeature(j, blockFrame, feature);
            }
        }
    }

    for (int j = 0; j < (int)m_outputNos.size(); ++j) {
        setCompletion(j, 100);
    }

    if (frequencyDomain) {
        for (int ch = 0; ch < channelCount; ++ch) {
            delete fftModels[ch];
        }
        delete[] reals;
        delete[] imaginaries;
    }

    for (int ch = 0; ch < channelCount; ++ch) {
        delete[] buffers[ch];
    }
    delete[] buffers;
}

void
FeatureExtractionModelTransformer::getFrames(int channelCount,
                                             sv_frame_t startFrame,
                                             sv_frame_t size,
                                             float **buffers)
{
    sv_frame_t offset = 0;

    if (startFrame < 0) {
        for (int c = 0; c < channelCount; ++c) {
            for (sv_frame_t i = 0; i < size && startFrame + i < 0; ++i) {
                buffers[c][i] = 0.0f;
            }
        }
        offset = -startFrame;
        size -= offset;
        if (size <= 0) return;
        startFrame = 0;
    }

    DenseTimeValueModel *input = getConformingInput();
    if (!input) return;
    
    sv_frame_t got = 0;

    if (channelCount == 1) {

        got = input->getData(m_input.getChannel(), startFrame, size,
                             buffers[0] + offset);

        if (m_input.getChannel() == -1 && input->getChannelCount() > 1) {
            // use mean instead of sum, as plugin input
            float cc = float(input->getChannelCount());
            for (sv_frame_t i = 0; i < size; ++i) {
                buffers[0][i + offset] /= cc;
            }
        }

    } else {

        float **writebuf = buffers;
        if (offset > 0) {
            writebuf = new float *[channelCount];
            for (int i = 0; i < channelCount; ++i) {
                writebuf[i] = buffers[i] + offset;
            }
        }

        got = input->getData(0, channelCount-1, startFrame, size, writebuf);

        if (writebuf != buffers) delete[] writebuf;
    }

    while (got < size) {
        for (int c = 0; c < channelCount; ++c) {
            buffers[c][got + offset] = 0.0;
        }
        ++got;
    }
}

void
FeatureExtractionModelTransformer::addFeature(int n,
                                              sv_frame_t blockFrame,
                                              const Vamp::Plugin::Feature &feature)
{
    sv_samplerate_t inputRate = m_input.getModel()->getSampleRate();

//    cerr << "FeatureExtractionModelTransformer::addFeature: blockFrame = "
//              << blockFrame << ", hasTimestamp = " << feature.hasTimestamp
//              << ", timestamp = " << feature.timestamp << ", hasDuration = "
//              << feature.hasDuration << ", duration = " << feature.duration
//              << endl;

    sv_frame_t frame = blockFrame;

    if (m_descriptors[n]->sampleType ==
	Vamp::Plugin::OutputDescriptor::VariableSampleRate) {

	if (!feature.hasTimestamp) {
	    cerr
		<< "WARNING: FeatureExtractionModelTransformer::addFeature: "
		<< "Feature has variable sample rate but no timestamp!"
		<< endl;
	    return;
	} else {
        frame = RealTime::realTime2Frame(RealTime(feature.timestamp.sec, feature.timestamp.nsec), inputRate);
	}

    } else if (m_descriptors[n]->sampleType ==
	       Vamp::Plugin::OutputDescriptor::FixedSampleRate) {

        if (!feature.hasTimestamp) {
            ++m_fixedRateFeatureNos[n];
        } else {
            RealTime ts(feature.timestamp.sec, feature.timestamp.nsec);
            m_fixedRateFeatureNos[n] = (int)
                lrint(ts.toDouble() * m_descriptors[n]->sampleRate);
        }

//        cerr << "m_fixedRateFeatureNo = " << m_fixedRateFeatureNo 
//             << ", m_descriptor->sampleRate = " << m_descriptor->sampleRate
//             << ", inputRate = " << inputRate
//             << " giving frame = ";
        frame = lrint((double(m_fixedRateFeatureNos[n])
                       / m_descriptors[n]->sampleRate)
                      * inputRate);
    }

    if (frame < 0) {
        cerr
            << "WARNING: FeatureExtractionModelTransformer::addFeature: "
            << "Negative frame counts are not supported (frame = " << frame
            << " from timestamp " << feature.timestamp
            << "), dropping feature" 
            << endl;
        return;
    }

    // Rather than repeat the complicated tests from the constructor
    // to determine what sort of model we must be adding the features
    // to, we instead test what sort of model the constructor decided
    // to create.

    if (isOutput<SparseOneDimensionalModel>(n)) {

        SparseOneDimensionalModel *model =
            getConformingOutput<SparseOneDimensionalModel>(n);
	if (!model) return;

        model->addPoint(SparseOneDimensionalModel::Point
                       (frame, feature.label.c_str()));
	
    } else if (isOutput<SparseTimeValueModel>(n)) {

	SparseTimeValueModel *model =
            getConformingOutput<SparseTimeValueModel>(n);
	if (!model) return;

        for (int i = 0; i < (int)feature.values.size(); ++i) {

            float value = feature.values[i];

            QString label = feature.label.c_str();
            if (feature.values.size() > 1) {
                label = QString("[%1] %2").arg(i+1).arg(label);
            }

            SparseTimeValueModel *targetModel = model;

            if (m_needAdditionalModels[n] && i > 0) {
                targetModel = getAdditionalModel(n, i);
                if (!targetModel) targetModel = model;
//                std::cerr << "adding point to model " << targetModel
//                          << " for output " << n << " bin " << i << std::endl;
            }

            targetModel->addPoint
                (SparseTimeValueModel::Point(frame, value, label));
        }

    } else if (isOutput<FlexiNoteModel>(n) || isOutput<NoteModel>(n) || isOutput<RegionModel>(n)) { //GF: Added Note Model

        int index = 0;

        float value = 0.0;
        if ((int)feature.values.size() > index) {
            value = feature.values[index++];
        }

        sv_frame_t duration = 1;
        if (feature.hasDuration) {
            duration = RealTime::realTime2Frame(RealTime(feature.duration.sec, feature.duration.nsec), inputRate);
        } else {
            if (in_range_for(feature.values, index)) {
                duration = lrintf(feature.values[index++]);
            }
        }

        if (isOutput<FlexiNoteModel>(n)) { // GF: added for flexi note model

            float velocity = 100;
            if ((int)feature.values.size() > index) {
                velocity = feature.values[index++];
            }
            if (velocity < 0) velocity = 127;
            if (velocity > 127) velocity = 127;

            FlexiNoteModel *model = getConformingOutput<FlexiNoteModel>(n);
            if (!model) return;
            model->addPoint(FlexiNoteModel::Point(frame,
                                                  value, // value is pitch
                                                  duration,
                                                  velocity / 127.f,
                                                  feature.label.c_str()));
			// GF: end -- added for flexi note model
        } else  if (isOutput<NoteModel>(n)) {

            float velocity = 100;
            if ((int)feature.values.size() > index) {
                velocity = feature.values[index++];
            }
            if (velocity < 0) velocity = 127;
            if (velocity > 127) velocity = 127;

            NoteModel *model = getConformingOutput<NoteModel>(n);
            if (!model) return;
            model->addPoint(NoteModel::Point(frame, value, // value is pitch
                                             duration,
                                             velocity / 127.f,
                                             feature.label.c_str()));
        } else {

            RegionModel *model = getConformingOutput<RegionModel>(n);
            if (!model) return;

            if (feature.hasDuration && !feature.values.empty()) {

                for (int i = 0; i < (int)feature.values.size(); ++i) {

                    float value = feature.values[i];

                    QString label = feature.label.c_str();
                    if (feature.values.size() > 1) {
                        label = QString("[%1] %2").arg(i+1).arg(label);
                    }

                    model->addPoint(RegionModel::Point(frame,
                                                       value,
                                                       duration,
                                                       label));
                }
            } else {
            
                model->addPoint(RegionModel::Point(frame,
                                                   value,
                                                   duration,
                                                   feature.label.c_str()));
            }
        }
	
    } else if (isOutput<EditableDenseThreeDimensionalModel>(n)) {
	
	DenseThreeDimensionalModel::Column values =
            DenseThreeDimensionalModel::Column(feature.values.begin(), feature.values.end());
	
	EditableDenseThreeDimensionalModel *model =
            getConformingOutput<EditableDenseThreeDimensionalModel>(n);
	if (!model) return;

//        cerr << "(note: model resolution = " << model->getResolution() << ")"
//             << endl;

        if (!feature.hasTimestamp && m_fixedRateFeatureNos[n] >= 0) {
            model->setColumn(m_fixedRateFeatureNos[n], values);
        } else {
            model->setColumn(int(frame / model->getResolution()), values);
        }

    } else {
        cerr << "FeatureExtractionModelTransformer::addFeature: Unknown output model type!" << endl;
    }
}

void
FeatureExtractionModelTransformer::setCompletion(int n, int completion)
{
//    cerr << "FeatureExtractionModelTransformer::setCompletion("
//              << completion << ")" << endl;

    if (isOutput<SparseOneDimensionalModel>(n)) {

	SparseOneDimensionalModel *model =
            getConformingOutput<SparseOneDimensionalModel>(n);
	if (!model) return;
        if (model->isAbandoning()) abandon();
	model->setCompletion(completion, true);

    } else if (isOutput<SparseTimeValueModel>(n)) {

	SparseTimeValueModel *model =
            getConformingOutput<SparseTimeValueModel>(n);
	if (!model) return;
        if (model->isAbandoning()) abandon();
	model->setCompletion(completion, true);

    } else if (isOutput<NoteModel>(n)) {

	NoteModel *model = getConformingOutput<NoteModel>(n);
	if (!model) return;
        if (model->isAbandoning()) abandon();
	model->setCompletion(completion, true);
	
    } else if (isOutput<FlexiNoteModel>(n)) {

	FlexiNoteModel *model = getConformingOutput<FlexiNoteModel>(n);
	if (!model) return;
        if (model->isAbandoning()) abandon();
	model->setCompletion(completion, true);

    } else if (isOutput<RegionModel>(n)) {

	RegionModel *model = getConformingOutput<RegionModel>(n);
	if (!model) return;
        if (model->isAbandoning()) abandon();
	model->setCompletion(completion, true);

    } else if (isOutput<EditableDenseThreeDimensionalModel>(n)) {

	EditableDenseThreeDimensionalModel *model =
            getConformingOutput<EditableDenseThreeDimensionalModel>(n);
	if (!model) return;
        if (model->isAbandoning()) abandon();
	model->setCompletion(completion, true); //!!!m_context.updates);
    }
}

