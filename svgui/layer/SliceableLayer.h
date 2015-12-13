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

#ifndef _SLICEABLE_LAYER_H_
#define _SLICEABLE_LAYER_H_

#include "Layer.h"

/**
 * Base class for layers that can be sliced, that is, that contain
 * models appropriate for use in a SliceLayer.
 */

class SliceableLayer : public Layer
{
    Q_OBJECT

public:
    // Get a model that can be sliced, i.e. a
    // DenseThreeDimensionalModel.  This may be the layer's usual
    // model, or it may be a model derived from it (e.g. FFTModel in a
    // spectrogram that was constructed from a DenseTimeValueModel).
    // The SliceableLayer retains ownership of the model, and will
    // emit sliceableModelReplaced if it is about to become invalid.
    virtual const Model *getSliceableModel() const = 0;

signals:
    // Emitted when a model that was obtained through
    // getSliceableModel is about to be deleted.  If replacement is
    // non-NULL, it may be used instead.
    void sliceableModelReplaced(const Model *modelToBeReplaced,
                                const Model *replacement);
};

#endif



