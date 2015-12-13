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

#include "AggregateWaveModel.h"

#include <iostream>

#include <QTextStream>

PowerOfSqrtTwoZoomConstraint
AggregateWaveModel::m_zoomConstraint;

AggregateWaveModel::AggregateWaveModel(ChannelSpecList channelSpecs) :
    m_components(channelSpecs)
{
    for (ChannelSpecList::const_iterator i = channelSpecs.begin();
         i != channelSpecs.end(); ++i) {
        if (i->model->getSampleRate() !=
            channelSpecs.begin()->model->getSampleRate()) {
            cerr << "AggregateWaveModel::AggregateWaveModel: WARNING: Component models do not all have the same sample rate" << endl;
            break;
        }
    }
}

AggregateWaveModel::~AggregateWaveModel()
{
}

bool
AggregateWaveModel::isOK() const
{
    for (ChannelSpecList::const_iterator i = m_components.begin();
         i != m_components.end(); ++i) {
        if (!i->model->isOK()) return false;
    }
    return true;
}

bool
AggregateWaveModel::isReady(int *completion) const
{
    if (completion) *completion = 100;
    bool ready = true;
    for (ChannelSpecList::const_iterator i = m_components.begin();
         i != m_components.end(); ++i) {
        int completionHere = 100;
        if (!i->model->isReady(&completionHere)) ready = false;
        if (completion && completionHere < *completion) {
            *completion = completionHere;
        }
    }
    return ready;
}

sv_frame_t
AggregateWaveModel::getFrameCount() const
{
    sv_frame_t count = 0;

    for (ChannelSpecList::const_iterator i = m_components.begin();
         i != m_components.end(); ++i) {
        sv_frame_t thisCount = i->model->getEndFrame() - i->model->getStartFrame();
        if (thisCount > count) count = thisCount;
    }

    return count;
}

int
AggregateWaveModel::getChannelCount() const
{
    return int(m_components.size());
}

sv_samplerate_t
AggregateWaveModel::getSampleRate() const
{
    if (m_components.empty()) return 0;
    return m_components.begin()->model->getSampleRate();
}

sv_frame_t
AggregateWaveModel::getData(int channel, sv_frame_t start, sv_frame_t count,
                            float *buffer) const
{
    int ch0 = channel, ch1 = channel;
    bool mixing = false;
    if (channel == -1) {
        ch0 = 0;
        ch1 = getChannelCount()-1;
        mixing = true;
    }

    float *readbuf = buffer;
    if (mixing) {
        readbuf = new float[count];
        for (sv_frame_t i = 0; i < count; ++i) {
            buffer[i] = 0.f;
        }
    }

    sv_frame_t longest = 0;
    
    for (int c = ch0; c <= ch1; ++c) {
        sv_frame_t here = 
            m_components[c].model->getData(m_components[c].channel,
                                           start, count,
                                           readbuf);
        if (here > longest) {
            longest = here;
        }
        if (here < count) {
            for (sv_frame_t i = here; i < count; ++i) {
                readbuf[i] = 0.f;
            }
        }
        if (mixing) {
            for (sv_frame_t i = 0; i < count; ++i) {
                buffer[i] += readbuf[i];
            }
        }
    }

    if (mixing) delete[] readbuf;
    return longest;
}
         
sv_frame_t
AggregateWaveModel::getData(int channel, sv_frame_t start, sv_frame_t count,
                            double *buffer) const
{
    int ch0 = channel, ch1 = channel;
    bool mixing = false;
    if (channel == -1) {
        ch0 = 0;
        ch1 = getChannelCount()-1;
        mixing = true;
    }

    double *readbuf = buffer;
    if (mixing) {
        readbuf = new double[count];
        for (sv_frame_t i = 0; i < count; ++i) {
            buffer[i] = 0.0;
        }
    }

    sv_frame_t longest = 0;
    
    for (int c = ch0; c <= ch1; ++c) {
        sv_frame_t here = 
            m_components[c].model->getData(m_components[c].channel,
                                           start, count,
                                           readbuf);
        if (here > longest) {
            longest = here;
        }
        if (here < count) {
            for (sv_frame_t i = here; i < count; ++i) {
                readbuf[i] = 0.;
            }
        }
        if (mixing) {
            for (sv_frame_t i = 0; i < count; ++i) {
                buffer[i] += readbuf[i];
            }
        }
    }
    
    if (mixing) delete[] readbuf;
    return longest;
}

sv_frame_t
AggregateWaveModel::getData(int fromchannel, int tochannel,
                            sv_frame_t start, sv_frame_t count,
                            float **buffer) const
{
    sv_frame_t min = count;

    for (int c = fromchannel; c <= tochannel; ++c) {
        sv_frame_t here = getData(c, start, count, buffer[c - fromchannel]);
        if (here < min) min = here;
    }
    
    return min;
}

int
AggregateWaveModel::getSummaryBlockSize(int desired) const
{
    //!!! complete
    return desired;
}
        
void
AggregateWaveModel::getSummaries(int, sv_frame_t, sv_frame_t,
                                 RangeBlock &, int &) const
{
    //!!! complete
}

AggregateWaveModel::Range
AggregateWaveModel::getSummary(int, sv_frame_t, sv_frame_t) const
{
    //!!! complete
    return Range();
}
        
int
AggregateWaveModel::getComponentCount() const
{
    return int(m_components.size());
}

AggregateWaveModel::ModelChannelSpec
AggregateWaveModel::getComponent(int c) const
{
    return m_components[c];
}

void
AggregateWaveModel::componentModelChanged()
{
    emit modelChanged();
}

void
AggregateWaveModel::componentModelChangedWithin(sv_frame_t start, sv_frame_t end)
{
    emit modelChangedWithin(start, end);
}

void
AggregateWaveModel::componentModelCompletionChanged()
{
    emit completionChanged();
}

void
AggregateWaveModel::toXml(QTextStream &,
                          QString ,
                          QString ) const
{
    //!!! complete
}

