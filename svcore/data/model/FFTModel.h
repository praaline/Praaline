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

#ifndef _FFT_MODEL_H_
#define _FFT_MODEL_H_

#include "data/fft/FFTDataServer.h"
#include "DenseThreeDimensionalModel.h"

#include <set>
#include <map>

/**
 * An implementation of DenseThreeDimensionalModel that makes FFT data
 * derived from a DenseTimeValueModel available as a generic data
 * grid.  The FFT data is acquired using FFTDataServer.  Note that any
 * of the accessor functions may throw AllocationFailed if a cache
 * resize fails.
 */

class FFTModel : public DenseThreeDimensionalModel
{
    Q_OBJECT

public:
    /**
     * Construct an FFT model derived from the given
     * DenseTimeValueModel, with the given window parameters and FFT
     * size (which may exceed the window size, for zero-padded FFTs).
     * 
     * If the model has multiple channels use only the given channel,
     * unless the channel is -1 in which case merge all available
     * channels.
     * 
     * If polar is true, the data will normally be retrieved from the
     * FFT model in magnitude/phase form; otherwise it will normally
     * be retrieved in "cartesian" real/imaginary form.  The results
     * should be the same either way, but a "polar" model addressed in
     * "cartesian" form or vice versa may suffer a performance
     * penalty.
     *
     * The fillFromColumn argument gives a hint that the FFT data
     * server should aim to start calculating FFT data at that column
     * number if possible, as that is likely to be requested first.
     */
    FFTModel(const DenseTimeValueModel *model,
             int channel,
             WindowType windowType,
             int windowSize,
             int windowIncrement,
             int fftSize,
             bool polar,
             StorageAdviser::Criteria criteria = StorageAdviser::NoCriteria,
             sv_frame_t fillFromFrame = 0);
    ~FFTModel();

    inline float getMagnitudeAt(int x, int y) {
        return m_server->getMagnitudeAt(x << m_xshift, y << m_yshift);
    }
    inline float getNormalizedMagnitudeAt(int x, int y) {
        return m_server->getNormalizedMagnitudeAt(x << m_xshift, y << m_yshift);
    }
    inline float getMaximumMagnitudeAt(int x) {
        return m_server->getMaximumMagnitudeAt(x << m_xshift);
    }
    inline float getPhaseAt(int x, int y) {
        return m_server->getPhaseAt(x << m_xshift, y << m_yshift);
    }
    inline void getValuesAt(int x, int y, float &real, float &imaginary) {
        m_server->getValuesAt(x << m_xshift, y << m_yshift, real, imaginary);
    }
    inline bool isColumnAvailable(int x) const {
        return m_server->isColumnReady(x << m_xshift);
    }

    inline bool getMagnitudesAt(int x, float *values, int minbin = 0, int count = 0) {
        return m_server->getMagnitudesAt(x << m_xshift, values, minbin << m_yshift, count, getYRatio());
    }
    inline bool getNormalizedMagnitudesAt(int x, float *values, int minbin = 0, int count = 0) {
        return m_server->getNormalizedMagnitudesAt(x << m_xshift, values, minbin << m_yshift, count, getYRatio());
    }
    inline bool getPhasesAt(int x, float *values, int minbin = 0, int count = 0) {
        return m_server->getPhasesAt(x << m_xshift, values, minbin << m_yshift, count, getYRatio());
    }
    inline bool getValuesAt(int x, float *reals, float *imaginaries, int minbin = 0, int count = 0) {
        return m_server->getValuesAt(x << m_xshift, reals, imaginaries, minbin << m_yshift, count, getYRatio());
    }

    inline sv_frame_t getFillExtent() const { return m_server->getFillExtent(); }

    // DenseThreeDimensionalModel and Model methods:
    //
    inline virtual int getWidth() const {
        return m_server->getWidth() >> m_xshift;
    }
    inline virtual int getHeight() const {
        // If there is no y-shift, the server's height (based on its
        // fftsize/2 + 1) is correct.  If there is a shift, then the
        // server is using a larger fft size than we want, so we shift
        // it right as many times as necessary, but then we need to
        // re-add the "+1" part (because ((fftsize*2)/2 + 1) / 2 !=
        // fftsize/2 + 1).
        return (m_server->getHeight() >> m_yshift) + (m_yshift > 0 ? 1 : 0);
    }
    virtual float getValueAt(int x, int y) const {
        return const_cast<FFTModel *>(this)->getMagnitudeAt(x, y);
    }
    virtual bool isOK() const {
        return m_server && m_server->getModel();
    }
    virtual sv_frame_t getStartFrame() const {
        return 0;
    }
    virtual sv_frame_t getEndFrame() const {
        return sv_frame_t(getWidth()) * getResolution() + getResolution();
    }
    virtual sv_samplerate_t getSampleRate() const;
    virtual int getResolution() const {
        return m_server->getWindowIncrement() << m_xshift;
    }
    virtual int getYBinCount() const {
        return getHeight();
    }
    virtual float getMinimumLevel() const {
        return 0.f; // Can't provide
    }
    virtual float getMaximumLevel() const {
        return 1.f; // Can't provide
    }
    virtual Column getColumn(int x) const;
    virtual QString getBinName(int n) const;

    virtual bool shouldUseLogValueScale() const {
        return true; // Although obviously it's up to the user...
    }

    /**
     * Calculate an estimated frequency for a stable signal in this
     * bin, using phase unwrapping.  This will be completely wrong if
     * the signal is not stable here.
     */
    virtual bool estimateStableFrequency(int x, int y, double &frequency);

    enum PeakPickType
    {
        AllPeaks,                /// Any bin exceeding its immediate neighbours
        MajorPeaks,              /// Peaks picked using sliding median window
        MajorPitchAdaptivePeaks  /// Bigger window for higher frequencies
    };

    typedef std::set<int> PeakLocationSet; // bin
    typedef std::map<int, double> PeakSet; // bin -> freq

    /**
     * Return locations of peak bins in the range [ymin,ymax].  If
     * ymax is zero, getHeight()-1 will be used.
     */
    virtual PeakLocationSet getPeaks(PeakPickType type, int x,
                                     int ymin = 0, int ymax = 0);

    /**
     * Return locations and estimated stable frequencies of peak bins.
     */
    virtual PeakSet getPeakFrequencies(PeakPickType type, int x,
                                       int ymin = 0, int ymax = 0);

    virtual int getCompletion() const { return m_server->getFillCompletion(); }
    virtual QString getError() const { return m_server->getError(); }

    virtual void suspend() { m_server->suspend(); }
    virtual void suspendWrites() { m_server->suspendWrites(); }
    virtual void resume() { m_server->resume(); }

    QString getTypeName() const { return tr("FFT"); }
    std::string getType() const { return "FFTModel"; }

public slots:
    void sourceModelAboutToBeDeleted();

private:
    FFTModel(const FFTModel &); // not implemented
    FFTModel &operator=(const FFTModel &); // not implemented

    FFTDataServer *m_server;
    int m_xshift;
    int m_yshift;

    FFTDataServer *getServer(const DenseTimeValueModel *,
                             int, WindowType, int, int, int,
                             bool, StorageAdviser::Criteria, sv_frame_t);

    int getPeakPickWindowSize(PeakPickType type, sv_samplerate_t sampleRate,
                              int bin, float &percentile) const;

    int getYRatio() {
        int ys = m_yshift;
        int r = 1;
        while (ys) { --ys; r <<= 1; }
        return r;
    }
};

#endif
