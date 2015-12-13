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

#ifndef _TRANSFORMER_H_
#define _TRANSFORMER_H_

#include "base/Thread.h"

#include "data/model/Model.h"

#include "Transform.h"

/**
 * A ModelTransformer turns one data model into another.
 *
 * Typically in this application, a ModelTransformer might have a
 * DenseTimeValueModel as its input (e.g. an audio waveform) and a
 * SparseOneDimensionalModel (e.g. detected beats) as its output.
 *
 * The ModelTransformer typically runs in the background, as a
 * separate thread populating the output model.  The model is
 * available to the user of the ModelTransformer immediately, but may
 * be initially empty until the background thread has populated it.
 */

class ModelTransformer : public Thread
{
public:
    virtual ~ModelTransformer();

    typedef std::vector<Model *> Models;

    class Input {
    public:
        Input(Model *m) : m_model(m), m_channel(-1) { }
        Input(Model *m, int c) : m_model(m), m_channel(c) { }

        Model *getModel() const { return m_model; }
        void setModel(Model *m) { m_model = m; }

        int getChannel() const { return m_channel; }
        void setChannel(int c) { m_channel = c; }

    protected:
        Model *m_model;
        int m_channel;
    };

    /**
     * Hint to the processing thread that it should give up, for
     * example because the process is going to exit or we want to get
     * rid of the input model.  Caller should still wait() and/or
     * delete the transform before assuming its input and output
     * models are no longer required.
     */
    void abandon() { m_abandoned = true; }

    /**
     * Return the input model for the transform.
     */
    Model *getInputModel()  { return m_input.getModel(); }

    /**
     * Return the input channel spec for the transform.
     */
    int getInputChannel() { return m_input.getChannel(); }

    /**
     * Return the set of output models created by the transform or
     * transforms.  Returns an empty list if any transform could not
     * be initialised; an error message may be available via
     * getMessage() in this situation.
     */
    Models getOutputModels() { return m_outputs; }

    /**
     * Return the set of output models, also detaching them from the
     * transformer so that they will not be deleted when the
     * transformer is.  The caller takes ownership of the models.
     */
    Models detachOutputModels() { 
        m_detached = true; 
        return getOutputModels(); 
    }

    /**
     * Return any additional models that were created during
     * processing. This might happen if, for example, a transform was
     * configured to split a multi-bin output into separate single-bin
     * models as it processed. These should not be queried until after
     * the transform has completed.
     */
    virtual Models getAdditionalOutputModels() { return Models(); }

    /**
     * Return true if the current transform is one that may produce
     * additional models (to be retrieved through
     * getAdditionalOutputModels above).
     */
    virtual bool willHaveAdditionalOutputModels() { return false; }

    /**
     * Return the set of additional models, also detaching them from
     * the transformer.  The caller takes ownership of the models.
     */
    virtual Models detachAdditionalOutputModels() { 
        m_detachedAdd = true;
        return getAdditionalOutputModels();
    }

    /**
     * Return a warning or error message.  If getOutputModel returned
     * a null pointer, this should contain a fatal error message for
     * the transformer; otherwise it may contain a warning to show to
     * the user about e.g. suboptimal block size or whatever.  
     */
    QString getMessage() const { return m_message; }

protected:
    ModelTransformer(Input input, const Transform &transform);
    ModelTransformer(Input input, const Transforms &transforms);

    Transforms m_transforms;
    Input m_input; // I don't own the model in this
    Models m_outputs; // I own this, unless...
    bool m_detached; // ... this is true.
    bool m_detachedAdd;
    bool m_abandoned;
    QString m_message;
};

#endif
