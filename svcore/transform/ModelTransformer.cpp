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

#include "ModelTransformer.h"

ModelTransformer::ModelTransformer(Input input, const Transform &transform) :
    m_input(input),
    m_detached(false),
    m_detachedAdd(false),
    m_abandoned(false)
{
    m_transforms.push_back(transform);
}

ModelTransformer::ModelTransformer(Input input, const Transforms &transforms) :
    m_transforms(transforms),
    m_input(input),
    m_detached(false),
    m_detachedAdd(false),
    m_abandoned(false)
{
}

ModelTransformer::~ModelTransformer()
{
    m_abandoned = true;
    wait();
    if (!m_detached) {
        Models mine = getOutputModels();
        foreach (Model *m, mine) delete m;
    }
    if (!m_detachedAdd) {
        Models mine = getAdditionalOutputModels();
        foreach (Model *m, mine) delete m;
    }
}

