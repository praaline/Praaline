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

#ifndef _AGGREGATE_WAVE_MODEL_H_
#define _AGGREGATE_WAVE_MODEL_H_

#include "RangeSummarisableTimeValueModel.h"
#include "PowerOfSqrtTwoZoomConstraint.h"

#include <vector>

class AggregateWaveModel : public RangeSummarisableTimeValueModel
{
    Q_OBJECT

public:
    struct ModelChannelSpec
    {
        ModelChannelSpec(RangeSummarisableTimeValueModel *m, int c) :
            model(m), channel(c) { }
        RangeSummarisableTimeValueModel *model;
        int channel;
    };

    typedef std::vector<ModelChannelSpec> ChannelSpecList;

    AggregateWaveModel(ChannelSpecList channelSpecs);
    ~AggregateWaveModel();

    bool isOK() const;
    bool isReady(int *) const;

    QString getTypeName() const { return tr("Aggregate Wave"); }
    std::string getType() const { return "AggregateWaveModel"; }

    int getComponentCount() const;
    ModelChannelSpec getComponent(int c) const;

    const ZoomConstraint *getZoomConstraint() const { return &m_zoomConstraint; }

    sv_frame_t getFrameCount() const;
    int getChannelCount() const;
    sv_samplerate_t getSampleRate() const;

    float getValueMinimum() const { return -1.0f; }
    float getValueMaximum() const { return  1.0f; }

    virtual sv_frame_t getStartFrame() const { return 0; }
    virtual sv_frame_t getEndFrame() const { return getFrameCount(); }

    virtual sv_frame_t getData(int channel, sv_frame_t start, sv_frame_t count,
                               float *buffer) const;

    virtual sv_frame_t getData(int channel, sv_frame_t start, sv_frame_t count,
                               double *buffer) const;

    virtual sv_frame_t getData(int fromchannel, int tochannel,
                               sv_frame_t start, sv_frame_t count,
                               float **buffer) const;

    virtual int getSummaryBlockSize(int desired) const;

    virtual void getSummaries(int channel, sv_frame_t start, sv_frame_t count,
                              RangeBlock &ranges,
                              int &blockSize) const;

    virtual Range getSummary(int channel, sv_frame_t start, sv_frame_t count) const;

    virtual void toXml(QTextStream &out,
                       QString indent = "",
                       QString extraAttributes = "") const;

signals:
    void modelChanged();
    void modelChangedWithin(sv_frame_t, sv_frame_t);
    void completionChanged();

protected slots:
    void componentModelChanged();
    void componentModelChangedWithin(sv_frame_t, sv_frame_t);
    void componentModelCompletionChanged();

protected:
    ChannelSpecList m_components;
    static PowerOfSqrtTwoZoomConstraint m_zoomConstraint;
};

#endif

