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

#ifndef _WAVE_FILE_MODEL_H_
#define _WAVE_FILE_MODEL_H_

#include "base/Thread.h"
#include <QMutex>
#include <QTimer>

#include "data/fileio/FileSource.h"

#include "RangeSummarisableTimeValueModel.h"
#include "PowerOfSqrtTwoZoomConstraint.h"

#include <stdlib.h>

class AudioFileReader;

class WaveFileModel : public RangeSummarisableTimeValueModel
{
    Q_OBJECT

public:
    WaveFileModel(FileSource source, sv_samplerate_t targetRate = 0);
    WaveFileModel(FileSource source, AudioFileReader *reader);
    ~WaveFileModel();

    bool isOK() const;
    bool isReady(int *) const;

    const ZoomConstraint *getZoomConstraint() const { return &m_zoomConstraint; }

    sv_frame_t getFrameCount() const;
    int getChannelCount() const;
    sv_samplerate_t getSampleRate() const;
    sv_samplerate_t getNativeRate() const;

    QString getTitle() const;
    QString getMaker() const;
    QString getLocation() const;

    QString getLocalFilename() const;

    float getValueMinimum() const { return -1.0f; }
    float getValueMaximum() const { return  1.0f; }

    virtual sv_frame_t getStartFrame() const { return m_startFrame; }
    virtual sv_frame_t getEndFrame() const { return m_startFrame + getFrameCount(); }

    void setStartFrame(sv_frame_t startFrame) { m_startFrame = startFrame; }

    virtual sv_frame_t getData(int channel, sv_frame_t start, sv_frame_t count,
                        float *buffer) const;

    virtual sv_frame_t getData(int channel, sv_frame_t start, sv_frame_t count,
                        double *buffer) const;

    virtual sv_frame_t getData(int fromchannel, int tochannel,
                        sv_frame_t start, sv_frame_t count,
                        float **buffers) const;

    virtual int getSummaryBlockSize(int desired) const;

    virtual void getSummaries(int channel, sv_frame_t start, sv_frame_t count,
                              RangeBlock &ranges,
                              int &blockSize) const;

    virtual Range getSummary(int channel, sv_frame_t start, sv_frame_t count) const;

    QString getTypeName() const { return tr("Wave File"); }
    std::string getType() const { return "WaveFileModel"; }

    virtual void toXml(QTextStream &out,
                       QString indent = "",
                       QString extraAttributes = "") const;

protected slots:
    void fillTimerTimedOut();
    void cacheFilled();
    
protected:
    void initialize();

    class RangeCacheFillThread : public Thread
    {
    public:
        RangeCacheFillThread(WaveFileModel &model) :
	    m_model(model), m_fillExtent(0),
            m_frameCount(model.getFrameCount()) { }
    
	sv_frame_t getFillExtent() const { return m_fillExtent; }
        virtual void run();

    protected:
        WaveFileModel &m_model;
	sv_frame_t m_fillExtent;
        sv_frame_t m_frameCount;
    };
         
    void fillCache();

    FileSource m_source;
    QString m_path;
    AudioFileReader *m_reader;
    bool m_myReader;

    sv_frame_t m_startFrame;

    RangeBlock m_cache[2]; // interleaved at two base resolutions
    mutable QMutex m_mutex;
    RangeCacheFillThread *m_fillThread;
    QTimer *m_updateTimer;
    sv_frame_t m_lastFillExtent;
    bool m_exiting;
    static PowerOfSqrtTwoZoomConstraint m_zoomConstraint;

    mutable SampleBlock m_directRead;
    mutable sv_frame_t m_lastDirectReadStart;
    mutable sv_frame_t m_lastDirectReadCount;
    mutable QMutex m_directReadMutex;
};    

#endif
