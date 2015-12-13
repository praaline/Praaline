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

#include "FFTModel.h"
#include "DenseTimeValueModel.h"
#include "AggregateWaveModel.h"

#include "base/Profiler.h"
#include "base/Pitch.h"

#include <algorithm>

#include <cassert>

#ifndef __GNUC__
#include <alloca.h>
#endif

FFTModel::FFTModel(const DenseTimeValueModel *model,
                   int channel,
                   WindowType windowType,
                   int windowSize,
                   int windowIncrement,
                   int fftSize,
                   bool polar,
                   StorageAdviser::Criteria criteria,
                   sv_frame_t fillFromFrame) :
    //!!! ZoomConstraint!
    m_server(0),
    m_xshift(0),
    m_yshift(0)
{
    setSourceModel(const_cast<DenseTimeValueModel *>(model)); //!!! hmm.

    m_server = getServer(model,
                         channel,
                         windowType,
                         windowSize,
                         windowIncrement,
                         fftSize,
                         polar,
                         criteria,
                         fillFromFrame);

    if (!m_server) return; // caller should check isOK()

    int xratio = windowIncrement / m_server->getWindowIncrement();
    int yratio = m_server->getFFTSize() / fftSize;

    while (xratio > 1) {
        if (xratio & 0x1) {
            cerr << "ERROR: FFTModel: Window increment ratio "
                      << windowIncrement << " / "
                      << m_server->getWindowIncrement()
                      << " must be a power of two" << endl;
            assert(!(xratio & 0x1));
        }
        ++m_xshift;
        xratio >>= 1;
    }

    while (yratio > 1) {
        if (yratio & 0x1) {
            cerr << "ERROR: FFTModel: FFT size ratio "
                      << m_server->getFFTSize() << " / " << fftSize
                      << " must be a power of two" << endl;
            assert(!(yratio & 0x1));
        }
        ++m_yshift;
        yratio >>= 1;
    }
}

FFTModel::~FFTModel()
{
    if (m_server) FFTDataServer::releaseInstance(m_server);
}

void
FFTModel::sourceModelAboutToBeDeleted()
{
    if (m_sourceModel) {
        cerr << "FFTModel[" << this << "]::sourceModelAboutToBeDeleted(" << m_sourceModel << ")" << endl;
        if (m_server) {
            FFTDataServer::releaseInstance(m_server);
            m_server = 0;
        }
        FFTDataServer::modelAboutToBeDeleted(m_sourceModel);
    }
}

FFTDataServer *
FFTModel::getServer(const DenseTimeValueModel *model,
                    int channel,
                    WindowType windowType,
                    int windowSize,
                    int windowIncrement,
                    int fftSize,
                    bool polar,
                    StorageAdviser::Criteria criteria,
                    sv_frame_t fillFromFrame)
{
    // Obviously, an FFT model of channel C (where C != -1) of an
    // aggregate model is the same as the FFT model of the appropriate
    // channel of whichever model that aggregate channel is drawn
    // from.  We should use that model here, in case we already have
    // the data for it or will be wanting the same data again later.

    // If the channel is -1 (i.e. mixture of all channels), then we
    // can't do this shortcut unless the aggregate model only has one
    // channel or contains exactly all of the channels of a single
    // other model.  That isn't very likely -- if it were the case,
    // why would we be using an aggregate model?

    if (channel >= 0) {

        const AggregateWaveModel *aggregate =
            dynamic_cast<const AggregateWaveModel *>(model);

        if (aggregate && channel < aggregate->getComponentCount()) {

            AggregateWaveModel::ModelChannelSpec spec =
                aggregate->getComponent(channel);

            return getServer(spec.model,
                             spec.channel,
                             windowType,
                             windowSize,
                             windowIncrement,
                             fftSize,
                             polar,
                             criteria,
                             fillFromFrame);
        }
    }

    // The normal case

    return FFTDataServer::getFuzzyInstance(model,
                                           channel,
                                           windowType,
                                           windowSize,
                                           windowIncrement,
                                           fftSize,
                                           polar,
                                           criteria,
                                           fillFromFrame);
}

sv_samplerate_t
FFTModel::getSampleRate() const
{
    return isOK() ? m_server->getModel()->getSampleRate() : 0;
}

FFTModel::Column
FFTModel::getColumn(int x) const
{
    Profiler profiler("FFTModel::getColumn", false);

    Column result;

    result.clear();
    int h = getHeight();
    result.reserve(h);

#ifdef __GNUC__
    float magnitudes[h];
#else
    float *magnitudes = (float *)alloca(h * sizeof(float));
#endif

    if (m_server->getMagnitudesAt(x << m_xshift, magnitudes)) {

        for (int y = 0; y < h; ++y) {
            result.push_back(magnitudes[y]);
        }

    } else {
        for (int i = 0; i < h; ++i) result.push_back(0.f);
    }

    return result;
}

QString
FFTModel::getBinName(int n) const
{
    sv_samplerate_t sr = getSampleRate();
    if (!sr) return "";
    QString name = tr("%1 Hz").arg((n * sr) / ((getHeight()-1) * 2));
    return name;
}

bool
FFTModel::estimateStableFrequency(int x, int y, double &frequency)
{
    if (!isOK()) return false;

    sv_samplerate_t sampleRate = m_server->getModel()->getSampleRate();

    int fftSize = m_server->getFFTSize() >> m_yshift;
    frequency = double(y * sampleRate) / fftSize;

    if (x+1 >= getWidth()) return false;

    // At frequency f, a phase shift of 2pi (one cycle) happens in 1/f sec.
    // At hopsize h and sample rate sr, one hop happens in h/sr sec.
    // At window size w, for bin b, f is b*sr/w.
    // thus 2pi phase shift happens in w/(b*sr) sec.
    // We need to know what phase shift we expect from h/sr sec.
    // -> 2pi * ((h/sr) / (w/(b*sr)))
    //  = 2pi * ((h * b * sr) / (w * sr))
    //  = 2pi * (h * b) / w.

    double oldPhase = getPhaseAt(x, y);
    double newPhase = getPhaseAt(x+1, y);

    int incr = getResolution();

    double expectedPhase = oldPhase + (2.0 * M_PI * y * incr) / fftSize;

    double phaseError = princarg(newPhase - expectedPhase);

//    bool stable = (fabsf(phaseError) < (1.1f * (m_windowIncrement * M_PI) / m_fftSize));

    // The new frequency estimate based on the phase error resulting
    // from assuming the "native" frequency of this bin

    frequency =
        (sampleRate * (expectedPhase + phaseError - oldPhase)) /
        (2.0 * M_PI * incr);

    return true;
}

FFTModel::PeakLocationSet
FFTModel::getPeaks(PeakPickType type, int x, int ymin, int ymax)
{
    Profiler profiler("FFTModel::getPeaks");

    FFTModel::PeakLocationSet peaks;
    if (!isOK()) return peaks;

    if (ymax == 0 || ymax > getHeight() - 1) {
        ymax = getHeight() - 1;
    }

    if (type == AllPeaks) {
        int minbin = ymin;
        if (minbin > 0) minbin = minbin - 1;
        int maxbin = ymax;
        if (maxbin < getHeight() - 1) maxbin = maxbin + 1;
        const int n = maxbin - minbin + 1;
#ifdef __GNUC__
        float values[n];
#else
        float *values = (float *)alloca(n * sizeof(float));
#endif
        getMagnitudesAt(x, values, minbin, maxbin - minbin + 1);
        for (int bin = ymin; bin <= ymax; ++bin) {
            if (bin == minbin || bin == maxbin) continue;
            if (values[bin - minbin] > values[bin - minbin - 1] &&
                values[bin - minbin] > values[bin - minbin + 1]) {
                peaks.insert(bin);
            }
        }
        return peaks;
    }

    Column values = getColumn(x);

    float mean = 0.f;
    for (int i = 0; i < values.size(); ++i) mean += values[i];
    if (values.size() > 0) mean = mean / float(values.size());
    
    // For peak picking we use a moving median window, picking the
    // highest value within each continuous region of values that
    // exceed the median.  For pitch adaptivity, we adjust the window
    // size to a roughly constant pitch range (about four tones).

    sv_samplerate_t sampleRate = getSampleRate();

    std::deque<float> window;
    std::vector<int> inrange;
    float dist = 0.5;

    int medianWinSize = getPeakPickWindowSize(type, sampleRate, ymin, dist);
    int halfWin = medianWinSize/2;

    int binmin;
    if (ymin > halfWin) binmin = ymin - halfWin;
    else binmin = 0;

    int binmax;
    if (ymax + halfWin < values.size()) binmax = ymax + halfWin;
    else binmax = values.size()-1;

    int prevcentre = 0;

    for (int bin = binmin; bin <= binmax; ++bin) {

        float value = values[bin];

        window.push_back(value);

        // so-called median will actually be the dist*100'th percentile
        medianWinSize = getPeakPickWindowSize(type, sampleRate, bin, dist);
        halfWin = medianWinSize/2;

        while ((int)window.size() > medianWinSize) {
            window.pop_front();
        }

        int actualSize = int(window.size());

        if (type == MajorPitchAdaptivePeaks) {
            if (ymax + halfWin < values.size()) binmax = ymax + halfWin;
            else binmax = values.size()-1;
        }

        std::deque<float> sorted(window);
        std::sort(sorted.begin(), sorted.end());
        float median = sorted[int(float(sorted.size()) * dist)];

        int centrebin = 0;
        if (bin > actualSize/2) centrebin = bin - actualSize/2;
        
        while (centrebin > prevcentre || bin == binmin) {

            if (centrebin > prevcentre) ++prevcentre;

            float centre = values[prevcentre];

            if (centre > median) {
                inrange.push_back(centrebin);
            }

            if (centre <= median || centrebin+1 == values.size()) {
                if (!inrange.empty()) {
                    int peakbin = 0;
                    float peakval = 0.f;
                    for (int i = 0; i < (int)inrange.size(); ++i) {
                        if (i == 0 || values[inrange[i]] > peakval) {
                            peakval = values[inrange[i]];
                            peakbin = inrange[i];
                        }
                    }
                    inrange.clear();
                    if (peakbin >= ymin && peakbin <= ymax) {
                        peaks.insert(peakbin);
                    }
                }
            }

            if (bin == binmin) break;
        }
    }

    return peaks;
}

int
FFTModel::getPeakPickWindowSize(PeakPickType type, sv_samplerate_t sampleRate,
                                int bin, float &percentile) const
{
    percentile = 0.5;
    if (type == MajorPeaks) return 10;
    if (bin == 0) return 3;

    int fftSize = m_server->getFFTSize() >> m_yshift;
    double binfreq = (sampleRate * bin) / fftSize;
    double hifreq = Pitch::getFrequencyForPitch(73, 0, binfreq);

    int hibin = int(lrint((hifreq * fftSize) / sampleRate));
    int medianWinSize = hibin - bin;
    if (medianWinSize < 3) medianWinSize = 3;

    percentile = 0.5f + float(binfreq / sampleRate);

    return medianWinSize;
}

FFTModel::PeakSet
FFTModel::getPeakFrequencies(PeakPickType type, int x,
                             int ymin, int ymax)
{
    Profiler profiler("FFTModel::getPeakFrequencies");

    PeakSet peaks;
    if (!isOK()) return peaks;
    PeakLocationSet locations = getPeaks(type, x, ymin, ymax);

    sv_samplerate_t sampleRate = getSampleRate();
    int fftSize = m_server->getFFTSize() >> m_yshift;
    int incr = getResolution();

    // This duplicates some of the work of estimateStableFrequency to
    // allow us to retrieve the phases in two separate vertical
    // columns, instead of jumping back and forth between columns x and
    // x+1, which may be significantly slower if re-seeking is needed

    std::vector<float> phases;
    for (PeakLocationSet::iterator i = locations.begin();
         i != locations.end(); ++i) {
        phases.push_back(getPhaseAt(x, *i));
    }

    int phaseIndex = 0;
    for (PeakLocationSet::iterator i = locations.begin();
         i != locations.end(); ++i) {
        double oldPhase = phases[phaseIndex];
        double newPhase = getPhaseAt(x+1, *i);
        double expectedPhase = oldPhase + (2.0 * M_PI * *i * incr) / fftSize;
        double phaseError = princarg(newPhase - expectedPhase);
        double frequency =
            (sampleRate * (expectedPhase + phaseError - oldPhase))
            / (2 * M_PI * incr);
//        bool stable = (fabsf(phaseError) < (1.1f * (incr * M_PI) / fftSize));
//        if (stable)
        peaks[*i] = frequency;
        ++phaseIndex;
    }

    return peaks;
}

FFTModel::FFTModel(const FFTModel &model) :
    DenseThreeDimensionalModel(),
    m_server(model.m_server),
    m_xshift(model.m_xshift),
    m_yshift(model.m_yshift)
{
    FFTDataServer::claimInstance(m_server);
}

