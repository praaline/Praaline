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

#ifndef _FEATURE_EXTRACTION_MODEL_TRANSFORMER_H_
#define _FEATURE_EXTRACTION_MODEL_TRANSFORMER_H_

#include "ModelTransformer.h"

#include <QString>

#include <vamp-hostsdk/Plugin.h>

#include <iostream>
#include <map>

class DenseTimeValueModel;
class SparseTimeValueModel;

class FeatureExtractionModelTransformer : public ModelTransformer
{
    Q_OBJECT

public:
    FeatureExtractionModelTransformer(Input input,
                                      const Transform &transform);

    // Obtain outputs for a set of transforms that all use the same
    // plugin and input (but with different outputs). i.e. run the
    // plugin once only and collect more than one output from it.
    FeatureExtractionModelTransformer(Input input,
                                      const Transforms &relatedTransforms);

    virtual ~FeatureExtractionModelTransformer();

    // ModelTransformer method, retrieve the additional models
    Models getAdditionalOutputModels();
    bool willHaveAdditionalOutputModels();

protected:
    bool initialise();

    virtual void run();

    Vamp::Plugin *m_plugin;
    std::vector<Vamp::Plugin::OutputDescriptor *> m_descriptors; // per transform
    std::vector<int> m_fixedRateFeatureNos; // to assign times to FixedSampleRate features
    std::vector<int> m_outputNos; // list of plugin output indexes required for this group of transforms

    void createOutputModels(int n);

    std::map<int, bool> m_needAdditionalModels; // transformNo -> necessity
    typedef std::map<int, std::map<int, SparseTimeValueModel *> > AdditionalModelMap;
    AdditionalModelMap m_additionalModels;
    SparseTimeValueModel *getAdditionalModel(int transformNo, int binNo);

    void addFeature(int n,
                    sv_frame_t blockFrame,
		    const Vamp::Plugin::Feature &feature);

    void setCompletion(int, int);

    void getFrames(int channelCount, sv_frame_t startFrame, sv_frame_t size,
                   float **buffer);

    // just casts

    DenseTimeValueModel *getConformingInput();

    template <typename ModelClass> bool isOutput(int n) {
        return dynamic_cast<ModelClass *>(m_outputs[n]) != 0;
    }

    template <typename ModelClass> ModelClass *getConformingOutput(int n) {
        if ((int)m_outputs.size() > n) {
            ModelClass *mc = dynamic_cast<ModelClass *>(m_outputs[n]);
            if (!mc) {
                std::cerr << "FeatureExtractionModelTransformer::getOutput: Output model not conformable" << std::endl;
            }
            return mc;
        } else {
            std::cerr << "FeatureExtractionModelTransformer::getOutput: No such output number " << n << std::endl;
            return 0;
        }
    }
};

#endif

